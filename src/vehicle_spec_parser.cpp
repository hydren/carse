/*
 * vehicle_spec_parser.cpp
 *
 *  Created on: 3 de dez de 2018
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "carse_logic.hpp"

#include "util.hpp"

#include "futil/properties.hpp"
#include "futil/string_actions.hpp"
#include "futil/string_split.hpp"
#include "futil/round.h"

#include <exception>
#include <cstdlib>
#include <cmath>

#include <iostream>
using std::cout;
using std::endl;

using std::vector;
using std::string;
using futil::Properties;

// to reduce typing is good
#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

// fwd declare
static void loadPowertrainSpec(Pseudo3DVehicle::Spec&, const Properties&);
static void loadChassisSpec(Pseudo3DVehicle::Spec&, const Properties&);
static void loadAnimationSpec(Pseudo3DVehicleAnimationSpec&, const Properties&);

// ========================================================================================================================

// vehicle spec default constants
static const float
	DEFAULT_VEHICLE_MASS_CAR = 1250,
	DEFAULT_VEHICLE_MASS_BIKE = 200,  // kg
	DEFAULT_TIRE_DIAMETER = 678,  // mm

	DEFAULT_CD_CAR  = 0.31,  // drag coefficient (Cd) of a 300ZX Z32
	DEFAULT_CD_BIKE = 0.60,  // estimated drag coefficient (Cd) of a common sporty bike
	DEFAULT_CL_CAR  = 0.20,  // estimated lift coefficient (Cl) of a 300ZX Z32
	DEFAULT_CL_BIKE = 0.10,  // estimated lift coefficient (Cl) of a common sporty bike

	DEFAULT_FRONTAL_AREA_CAR  = 1.81,  // frontal area (in square-meters) of a 300ZX Z32
	DEFAULT_FRONTAL_AREA_BIKE = 0.70,  // estimated frontal area (in square-meters) of a common sporty bike

	DEFAULT_FR_WEIGHT_DISTRIBUTION = 0.45,
	DEFAULT_MR_WEIGHT_DISTRIBUTION = 0.55,
	DEFAULT_RR_WEIGHT_DISTRIBUTION = 0.65,
	DEFAULT_FF_WEIGHT_DISTRIBUTION = 0.40,

	DEFAULT_MAXIMUM_RPM_CAR = 7000,
	DEFAULT_MAXIMUM_POWER_CAR = 320,  // bhp
	DEFAULT_MAXIMUM_RPM_BIKE = 10000,
	DEFAULT_MAXIMUM_POWER_BIKE = 100,  // bhp
	DEFAULT_GEAR_COUNT = 5,

	// for the time being, assume 70% efficiency
	DEFAULT_TRANSMISSION_EFFICIENCY = 0.7;

static const float SPRITE_WORLD_SCALE_FACTOR = 24.0/895.0;

void Pseudo3DVehicle::Spec::loadFromFile(const string& filename)
{
	CarseLogic& logic = CarseLogic::getInstance();
	Properties prop;
	prop.load(filename);

	if(not prop.containsKey("definition") or prop.get("definition") != "vehicle")
		throw std::logic_error("Could not load vehicle spec from file due to missing key values");

	// aux. var
	string key;

	// logic data

	key = "vehicle_type";
	if(prop.containsKey(key))
	{
		string t = futil::to_lower(prop.get(key));
		if(t == "car" or t == "default") type = Mechanics::TYPE_CAR;
		else if(t == "bike") type = Mechanics::TYPE_BIKE;
		else type = Mechanics::TYPE_OTHER;
	}
	else type = Mechanics::TYPE_CAR;

	// info data

	key = "vehicle_name";
	name = prop.containsKey(key)? prop.get(key) : "unnamed";

	key = "authors";
	authors = prop.containsKey(key)? prop.get(key) : "unknown";

	key = "credits";
	credits = prop.containsKey(key)? prop.get(key) : "";

	key = "comments";
	comments = prop.containsKey(key)? prop.get(key) : "";

	// todo read more data from properties

	// physics data
	loadPowertrainSpec(*this, prop);
	loadChassisSpec(*this, prop);

	// read sound data if a custom one is specified
	key = "sound";
	if(prop.get(key) == "custom")
		soundProfile = CarseLogic::createEngineSoundProfileFromFile(filename);
	else if(prop.get(key) != "no")
		soundProfile = logic.getPresetEngineSoundProfile(prop.get(key));

	// sprite data
	prop.put("filename", filename);  // done so we can later get properties filename
	prop.put("base_dir", filename.substr(0, filename.find_last_of("/\\")+1));  // done so we can later get properties base dir
	loadAnimationSpec(sprite, prop);

	// attempt to read up to 32 alternative sprites
	for(unsigned i = 0; i < 32; i++)
	{
		key = "alternate_sprite_sheet" + futil::to_string(i) + "_definition_file";
		if(isValueSpecified(prop, key))
		{
			const string alternateSpritePropFile = getContextualizedFilename(prop.get(key), prop.get("base_dir"), CarseLogic::VEHICLES_FOLDER+"/");
			if(alternateSpritePropFile.empty())
			{
				cout << "warning: alternate sprite sheet " << i << " definition file could not be found!"
				<< " (specified by \"" << filename << "\")" << endl;
				continue;
			}

			Properties alternateSpriteProp;
			alternateSpriteProp.load(alternateSpritePropFile);

			// load some additional properties from main properties file
			alternateSpriteProp["vehicle_width"] = prop.get("vehicle_width");
			alternateSpriteProp["vehicle_height"] = prop.get("vehicle_height");
			alternateSpriteProp["vehicle_width_height_ratio"] = prop.get("vehicle_width_height_ratio");
			alternateSpriteProp.put("filename", alternateSpritePropFile);
			alternateSpriteProp.put("base_dir", prop.get("base_dir"));

			alternateSprites.push_back(Pseudo3DVehicleAnimationSpec());
			loadAnimationSpec(alternateSprites.back(), alternateSpriteProp);
		}
		else
		{
			key = "alternate_sprite_sheet" + futil::to_string(i) + "_file";
			if(isValueSpecified(prop, key))
			{
				const string alternateSpriteSheetFile = getContextualizedFilename(prop.get(key), prop.get("base_dir"), CarseLogic::VEHICLES_FOLDER+"/");
				if(alternateSpriteSheetFile.empty())
				{
					cout << "warning: alternate sprite sheet " << i << " file could not be found!"
					<< " (specified by \"" << filename << "\")" << endl;
					continue;
				}

				int inheritedProfileIndex = -1;
				key = "alternate_sprite_sheet" + futil::to_string(i) + "_inherit_from";
				if(isValueSpecified(prop, key))
				{
					const unsigned inheritIndex = prop.getParsedCStrAllowDefault<int, atoi>(key, 0);
					if(inheritIndex < alternateSprites.size())
						inheritedProfileIndex = inheritIndex;
					else
						cout << "warning: " << key << " specifies an out of bounds index (" << inheritIndex << "). inheriting default instead." << endl;
				}

				if(inheritedProfileIndex != -1)
					alternateSprites.push_back(alternateSprites[inheritedProfileIndex]);
				else
					alternateSprites.push_back(sprite);

				alternateSprites.back().sheetFilename = alternateSpriteSheetFile;
			}
		}
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------
	// These properties need to be loaded after sprite data to make sure that some fields are ready ('depictedVehicleWidth', 'sprite_sheet_file', etc)

	// attempt to estimate center's of gravity height
	key = "vehicle_height";
	if(isValueSpecified(prop, key))
		centerOfGravityHeight = 0.5f*atof(prop.get(key).c_str());  // aprox. half the height
	else
		centerOfGravityHeight = 0.3506f * sprite.depictedVehicleWidth * (sprite.scale.x / SPRITE_WORLD_SCALE_FACTOR);  // proportion aprox. of a 300ZX Z32

	key = "center_of_gravity_height";
	if(isValueSpecified(prop, key))
		centerOfGravityHeight = prop.getParsedCStrAllowDefault<double, atof>(key, centerOfGravityHeight);

	// attempt to estimate wheelbase
	{
		key = "wheelbase";
		if(isValueSpecified(prop, key))
			wheelbase = atof(prop.get(key).c_str());
		else
			wheelbase = -1;

		key = "vehicle_length";
		if(wheelbase == -1 and isValueSpecified(prop, key))
			wheelbase = 0.5682f * atof(prop.get(key).c_str());  // proportion aprox. of a 300ZX Z32

		key = "vehicle_width";
		if(wheelbase == -1 and isValueSpecified(prop, key))
			wheelbase = 2.5251f * atof(prop.get(key).c_str());  // proportion aprox. of a 300ZX Z32

		key = "vehicle_height";
		if(wheelbase == -1 and isValueSpecified(prop, key))
			wheelbase = 3.6016f * atof(prop.get(key).c_str());  // proportion aprox. of a 300ZX Z32

		if(wheelbase == -1)
		{
			wheelbase = 2.5251f * sprite.depictedVehicleWidth * (sprite.scale.x / SPRITE_WORLD_SCALE_FACTOR);  // proportion aprox. of a 300ZX Z32
		}
	}
}

// ========================================================================================================================

static void loadPowertrainSpec(Pseudo3DVehicle::Spec& spec, const Properties& prop)
{
	// powertrain data
	string key = "engine_configuration";
	spec.engineConfiguration = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_aspiration";
	spec.engineAspiration = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_valvetrain";
	spec.engineValvetrain = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_displacement";
	spec.engineDisplacement = prop.containsKey(key)? atoi(prop.get(key).c_str()) : 0;

	key = "engine_valve_count";
	spec.engineValveCount = prop.containsKey(key)? atoi(prop.get(key).c_str()) : 0;

	// actually useful data

	key = "engine_maximum_rpm";
	spec.engineMaximumRpm = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : spec.type == Mechanics::TYPE_BIKE? DEFAULT_MAXIMUM_RPM_BIKE : DEFAULT_MAXIMUM_RPM_CAR;
	spec.engineMinimumRpm = 1000;

	key = "engine_maximum_power";
	spec.engineMaximumPower = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : spec.type == Mechanics::TYPE_BIKE? DEFAULT_MAXIMUM_POWER_BIKE : DEFAULT_MAXIMUM_POWER_CAR;

	// todo re-enable these when proper formulas have been determined from parametrization
//	key = "engine_maximum_power_rpm";
//	maximumPowerRpm = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_MAXIMUM_POWER_RPM_RANGE;

//	key = "engine_maximum_torque";
//	maximumTorque = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : -1;

//	key = "engine_maximum_torque_rpm";
//	maximumTorqueRpm = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : -1;

	key = "engine_power_band";
	string powerBandStr = isValueSpecified(prop, key)? prop.get(key) : "typical";
	if(powerBandStr == "peaky")
		spec.enginePowerBand = Engine::TorqueCurveProfile::POWER_BAND_PEAKY;
	else if(powerBandStr == "torquey")
		spec.enginePowerBand = Engine::TorqueCurveProfile::POWER_BAND_TORQUEY;
	else if(powerBandStr == "semitorquey" or powerBandStr == "semi-torquey" or powerBandStr == "semi torquey")
		spec.enginePowerBand = Engine::TorqueCurveProfile::POWER_BAND_SEMI_TORQUEY;
	else if(powerBandStr == "wide")
		spec.enginePowerBand = Engine::TorqueCurveProfile::POWER_BAND_WIDE;
	else
		spec.enginePowerBand = Engine::TorqueCurveProfile::POWER_BAND_TYPICAL;

	// here an engine object is created just to collect some data
	Engine tmpEngine(spec.engineMaximumRpm, spec.engineMaximumPower, spec.enginePowerBand, DEFAULT_GEAR_COUNT);
	spec.engineMaximumPowerRpm = tmpEngine.maximumPowerRpm;
	spec.engineMaximumTorque = tmpEngine.maximumTorque;
	spec.engineMaximumTorqueRpm = tmpEngine.maximumTorqueRpm;

	key = "transmission_efficiency";
	spec.engineTransmissionEfficiency = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_TRANSMISSION_EFFICIENCY;

	key = "gear_count";
	spec.engineGearCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_GEAR_COUNT;
	spec.engineGearRatio.resize(spec.engineGearCount);

	// first, set default ratios, then override
	spec.engineGearRatio[spec.engineGearCount-1] = 0.8;
	if(spec.type == Mechanics::TYPE_BIKE)
	{
		spec.engineDifferentialRatio = 7.5;
		for(int g = spec.engineGearCount-2; g >= 0; g--)
			spec.engineGearRatio[g] = spec.engineGearRatio[g+1] + 0.1*(spec.engineGearCount-g-1);
	}
	else  // car type or other
	{
		spec.engineDifferentialRatio = 5.0;
		for(int g = spec.engineGearCount-2; g >= 0; g--)
			spec.engineGearRatio[g] = spec.engineGearRatio[g+1] + 0.1*(spec.engineGearCount-g);
		spec.engineReverseGearRatio = 3.25;
	}

	key = "gear_ratios";
	if(prop.containsKey(key))
	{
		string ratiosTxt = prop.get(key);
		if(ratiosTxt == "custom")
		{
			key = "gear_differential_ratio";
			if(prop.containsKey(key))
				spec.engineDifferentialRatio = atof(prop.get(key).c_str());

			key = "gear_reverse_ratio";
			if(prop.containsKey(key))
				spec.engineReverseGearRatio = atof(prop.get(key).c_str());

			for(int g = 0; g < spec.engineGearCount; g++)
			{
				key = "gear_" + futil::to_string(g+1) + "_ratio";
				if(prop.containsKey(key))
					spec.engineGearRatio[g] = atof(prop.get(key).c_str());
			}
		}
	}
}

// ========================================================================================================================

static void loadChassisSpec(Pseudo3DVehicle::Spec& spec, const Properties& prop)
{
	string key = "vehicle_mass";
	spec.mass = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : spec.type == Mechanics::TYPE_BIKE? DEFAULT_VEHICLE_MASS_BIKE : DEFAULT_VEHICLE_MASS_CAR;

	key = "tire_diameter";
	spec.tireRadius = (isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_TIRE_DIAMETER) * 0.0005;

	key = "engine_location";
	if(isValueSpecified(prop, key))
	{
		const string value = prop.get(key);
		if(value == "mid" or value == "middle") spec.engineLocation = Mechanics::ENGINE_LOCATION_ON_MIDDLE;
		else if(value == "rear") spec.engineLocation = Mechanics::ENGINE_LOCATION_ON_REAR;
		else spec.engineLocation = Mechanics::ENGINE_LOCATION_ON_FRONT;
	}
	else
		spec.engineLocation = spec.type == Mechanics::TYPE_BIKE? Mechanics::ENGINE_LOCATION_ON_MIDDLE : Mechanics::ENGINE_LOCATION_ON_FRONT;

	key = "driven_wheels";
	if(isValueSpecified(prop, key))
	{
		const string value = prop.get(key);
		if(value == "all") spec.drivenWheelsType = Mechanics::DRIVEN_WHEELS_ALL;
		else if(value == "front") spec.drivenWheelsType = Mechanics::DRIVEN_WHEELS_ON_FRONT;
		else spec.drivenWheelsType = Mechanics::DRIVEN_WHEELS_ON_REAR;
	}
	else
		spec.drivenWheelsType = Mechanics::DRIVEN_WHEELS_ON_REAR;

	key = "vehicle_frontal_area";
	const float referenceArea = prop.getParsedCStrAllowDefault<double, atof>(key, (spec.type == Mechanics::TYPE_BIKE? DEFAULT_FRONTAL_AREA_BIKE : DEFAULT_FRONTAL_AREA_CAR));

	key = "drag_coefficient";
	const float dragCoefficient = prop.getParsedCStrAllowDefault<double, atof>(key, (spec.type == Mechanics::TYPE_BIKE? DEFAULT_CD_BIKE : DEFAULT_CD_CAR));

	key = "drag_area";
	spec.dragArea = referenceArea * dragCoefficient;
	if(isValueSpecified(prop, key))
		spec.dragArea = prop.getParsedCStrAllowDefault<double, atof>(key, spec.dragArea);

	key = "lift_coefficient";
	const float liftCoefficient = prop.getParsedCStrAllowDefault<double, atof>(key, (spec.type == Mechanics::TYPE_BIKE? DEFAULT_CL_BIKE : DEFAULT_CL_CAR));

	key = "lift_area";
	spec.liftArea = referenceArea * liftCoefficient;
	if(isValueSpecified(prop, key))
		spec.liftArea = prop.getParsedCStrAllowDefault<double, atof>(key, spec.liftArea);

	if(spec.engineLocation == Mechanics::ENGINE_LOCATION_ON_FRONT)
	{
		if(spec.drivenWheelsType == Mechanics::DRIVEN_WHEELS_ON_FRONT)
			spec.weightDistribution = DEFAULT_FF_WEIGHT_DISTRIBUTION;

		else /* spec.drivenWheelsType == Mechanics::DRIVEN_WHEELS_ON_REAR or Mechanics::DRIVEN_WHEELS_ALL */
			spec.weightDistribution = DEFAULT_FR_WEIGHT_DISTRIBUTION;
	}
	else if(spec.engineLocation == Mechanics::ENGINE_LOCATION_ON_REAR)
		spec.weightDistribution = DEFAULT_RR_WEIGHT_DISTRIBUTION;

	else /* Mechanics::ENGINE_LOCATION_ON_MIDDLE */
		spec.weightDistribution = DEFAULT_MR_WEIGHT_DISTRIBUTION;

	key = "weight_distribution";
	if(isValueSpecified(prop, key))
		spec.weightDistribution = prop.getParsedCStrAllowDefault<double, atof>(key, spec.weightDistribution);
}

// ========================================================================================================================

// default sprite uint constants
static const unsigned
	DEFAULT_SPRITE_WIDTH_CAR = 60,
	DEFAULT_SPRITE_HEIGHT_CAR = 35,
	DEFAULT_SPRITE_WIDTH_BIKE = 43,
	DEFAULT_SPRITE_HEIGHT_BIKE = 70,
	DEFAULT_BRAKELIGHTS_SPRITE_WIDTH = 32,
	DEFAULT_BRAKELIGHTS_SPRITE_HEIGHT = 32;

// default sprite float constants
static const float
	PSEUDO_ANGLE_THRESHOLD = 0.1,
	SPRITE_IDEAL_MAX_DEPICTED_TURN_ANGLE = 45, // 45 degrees, pi/4 radians
	DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE = SPRITE_IDEAL_MAX_DEPICTED_TURN_ANGLE,
	DEFAULT_SPRITE_DEPICTED_VEHICLE_WIDTH_PROPORTION = 0.857142857143;  // ~0,857

static const string DEFAULT_CAR_SHEET = "assets/car-sheet-default.png",
					DEFAULT_BIKE_SHEET = "assets/bike-sheet-default.png";

static void loadAnimationSpec(Pseudo3DVehicleAnimationSpec& spec, const Properties& prop)
{
	// aux. vars
	string key, key2, key3;

	key = "vehicle_type";
	const bool isBike = prop.containsKey(key) and futil::to_lower(prop.get(key)) == "bike";

	key = "sprite_sheet_file";
	spec.sheetFilename = isValueSpecified(prop, key)? prop.get(key) : "default";

	if(not spec.sheetFilename.empty() and spec.sheetFilename != "default")
		spec.sheetFilename = getContextualizedFilename(spec.sheetFilename, prop.get("base_dir"), CarseLogic::VEHICLES_FOLDER+"/");

	if(spec.sheetFilename == "default" or spec.sheetFilename.empty())
	{
		if(spec.sheetFilename.empty())
			cout << "warning: sheet file \"" << prop.get(key) << "\" could not be found!"
		<< " (specified by \"" << prop.get("filename") << "\"). using default sheet instead..." << endl;

		spec.sheetFilename = isBike? DEFAULT_BIKE_SHEET : DEFAULT_CAR_SHEET;
	}

	key = "sprite_state_count";
	spec.stateCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 1;

	key = "sprite_frame_width";
	spec.frameWidth = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : isBike? DEFAULT_SPRITE_WIDTH_BIKE : DEFAULT_SPRITE_WIDTH_CAR;

	key = "sprite_frame_height";
	spec.frameHeight = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : isBike? DEFAULT_SPRITE_HEIGHT_BIKE : DEFAULT_SPRITE_HEIGHT_CAR;

	key = "sprite_vehicle_width";
	spec.depictedVehicleWidth = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : futil::round(spec.frameWidth*DEFAULT_SPRITE_DEPICTED_VEHICLE_WIDTH_PROPORTION);

	// default scale
	spec.scale.x = spec.scale.y = 1.0;

	bool keepAspectRatio = false;
	key = "sprite_keep_aspect_ratio";
	if(isValueSpecified(prop, key))
	{
		const string value = futil::to_lower(futil::trim(prop.get(key)));
		if(value == "true" or value == "yes")
			keepAspectRatio = true;
	}

	if(isValueSpecified(prop, "vehicle_width"))  // if vehicle width is available, compute recommended scale factor
	{
		const float vehicleWidth = atof(prop.get("vehicle_width").c_str());  // the real-life vehicle width, in mm

		key = "sprite_vehicle_height"; key2 = "vehicle_height"; key3 = "vehicle_width_height_ratio";
		if(not keepAspectRatio and isValueSpecified(prop, key)  // if vehicle height (both real-life and in sprite) are available, adjust scale factor (if allowed)
		and (isValueSpecified(prop, key2) or isValueSpecified(prop, key3)))  // ratios can be obtained by specifing height or ratio itself
		{
			// adjust scale factor to account for width/height ratio discrepancies
			const float spriteVehicleHeight = atoi(prop.get(key).c_str()),  // the vehicle height on the sprite, in pixels
						spriteWHRatio = spec.depictedVehicleWidth / spriteVehicleHeight,  // sprite width/height (WH) ratio
						vehicleHeight = isValueSpecified(prop, key2)? atof(prop.get(key2).c_str()) : -1;  // the real-life vehicle height (if available), in mm.

			if(vehicleHeight == 0 and not isValueSpecified(prop, key3))
				throw std::invalid_argument("vehicle height must not be specified as zero");

			const float vehicleWHRatio = isValueSpecified(prop, key3)? atof(prop.get(key3).c_str())  // if real-life width/height ratio is available, prefer to use it
																	: (vehicleWidth / vehicleHeight),  // otherwise calculate it from the available real-life width and height
						ratioFixFactor = vehicleWHRatio / spriteWHRatio;  // multiplier that makes the sprite width/height ratio match the real-life width/height ratio

			// recommended scale factors, making sprite width/height ratio match the real-life width/height ratio
			spec.scale.x = (vehicleWidth / spec.depictedVehicleWidth) * SPRITE_WORLD_SCALE_FACTOR;
			spec.scale.y = spec.scale.x / ratioFixFactor;
		}
		else  // no data about vehicle height or width/height ratio given; assume no width/height ratio discrepancies between real-life
		{
			spec.scale.x = spec.scale.y = (vehicleWidth / spec.depictedVehicleWidth) * SPRITE_WORLD_SCALE_FACTOR;  // recommended scale factor assuming no width/height ratio discrepancies
		}
	}
	else if(isValueSpecified(prop, "vehicle_height") and isValueSpecified(prop, "sprite_vehicle_height"))  // otherwise if vehicle height is available, compute recommended scale factor
	{
		const float vehicleHeight = atof(prop.get("vehicle_height").c_str()),  // the real-life vehicle height, in mm
					spriteVehicleHeight = atoi(prop.get("sprite_vehicle_height").c_str());  // the vehicle height on the sprite, in pixels

		if(not keepAspectRatio and isValueSpecified(prop, "vehicle_width_height_ratio"))
		{
			const float vehicleWHRatio = atof(prop.get("vehicle_width_height_ratio").c_str()),  // vehicle width/height (WH) ratio
						spriteWHRatio = spec.depictedVehicleWidth / spriteVehicleHeight,  // sprite width/height (WH) ratio
						ratioFixFactor = vehicleWHRatio / spriteWHRatio;  // multiplier that makes the sprite width/height ratio match the real-life width/height ratio

			// recommended scale factors, making sprite width/height ratio match the real-life width/height ratio
			spec.scale.y = (vehicleHeight / spriteVehicleHeight) * SPRITE_WORLD_SCALE_FACTOR;
			spec.scale.x = spec.scale.y * ratioFixFactor;
		}
		else  // no data about vehicle width/height ratio given; assume no width/height ratio discrepancies between real-life
		{
			spec.scale.x = spec.scale.y = (vehicleHeight / spriteVehicleHeight) * SPRITE_WORLD_SCALE_FACTOR;  // recommended scale factor assuming no width/height ratio discrepancies
		}
	}

	key = "sprite_scale";
	if(isValueSpecified(prop, key))  // if scale factor is available, override previous definitions
	{
		spec.scale.x = spec.scale.y = atof(prop.get(key).c_str());
	}

	key = "sprite_scale_x";
	spec.scale.x = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : spec.scale.x;  // if x-scale factor is available, override previous definition

	key = "sprite_scale_y";
	spec.scale.y = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : spec.scale.y;  // if y-scale factor is available, override previous definition

	key = "sprite_contact_offset";
	spec.contactOffset = 0;
	if(prop.containsKey(key))
	{
		const string value = futil::trim(prop.get(key));
		if(value == "default")
			spec.contactOffset = 2;
		else if(not value.empty())
			spec.contactOffset = atof(prop.get(key).c_str());
	}

	spec.asymmetrical = false;
	key = "sprite_asymmetric";
	if(isValueSpecified(prop, key))
	{
		const string value = futil::to_lower(futil::trim(prop.get(key)));
		if(value == "true" or value == "yes")
			spec.asymmetrical = true;
	}

	// with these fields, values for all states (except state 0) must be specified, or no values should be specified at all.
	for(unsigned stateNumber = 0; stateNumber < spec.stateCount; stateNumber++)
	{
		key = "sprite_state" + futil::to_string(stateNumber) + "_depicted_turn_angle";

		if(isValueSpecified(prop, key))
		{
			if(spec.depictedTurnAngle.empty())
			{
				if(stateNumber == 1)
					spec.depictedTurnAngle.push_back(0);

				else if(stateNumber > 1)
				{
					cout << "warning: missing depicted turn angles for sprite states before state " << stateNumber
						 << "; ignoring all values for safety..." << endl;
					break;
				}
			}

			spec.depictedTurnAngle.push_back(atof(prop.get(key).c_str())/SPRITE_IDEAL_MAX_DEPICTED_TURN_ANGLE);
		}
		else if(not spec.depictedTurnAngle.empty())
		{
			cout << "warning: missing depicted turn angle for sprite state " << stateNumber << "; ignoring all values for safety..." << endl;
			spec.depictedTurnAngle.clear();
			break;
		}
	}

	// attempt to load turn angles from single property if they were not specified individually
	if(spec.depictedTurnAngle.empty())
	{
		key = "sprite_depicted_turn_angles";
		if(isValueSpecified(prop, key))
		{
			vector<string> tokens = futil::split(prop.get(key), ',');
			if(tokens.size() == spec.stateCount)
				for(unsigned i = 0; i < tokens.size(); i++)
					spec.depictedTurnAngle.push_back(atof(tokens[i].c_str())/SPRITE_IDEAL_MAX_DEPICTED_TURN_ANGLE);
			else
				cout << "warning: incorrect number of values specified for turn angles; ignoring values for safety..." << endl;
		}
	}

	// check depicted turn angles for consistency
	if(not spec.depictedTurnAngle.empty())
		for(unsigned i = 0; i < spec.depictedTurnAngle.size(); i++)
			for(unsigned j = i+1; j < spec.depictedTurnAngle.size(); j++)
			{
				if(spec.depictedTurnAngle[j] == spec.depictedTurnAngle[i])
				{
					cout << "warning: depicted turn angle specified for state " << i << " and state " << j
						 << " have the same value (" << spec.depictedTurnAngle[i] << ");"
						 << " ignoring values for safety..." << endl;
					spec.depictedTurnAngle.clear();
					goto depictedTurnAngleCheckEnd;
				}
				else if(spec.depictedTurnAngle[j] < spec.depictedTurnAngle[i])
				{
					cout << "warning: depicted turn angle specified for state " << j << " (" << spec.depictedTurnAngle[j] << ")"
					     << " is smaller than previous state " << i << " (" << spec.depictedTurnAngle[i] << ");"
						 << " ignoring values for safety..." << endl;
					spec.depictedTurnAngle.clear();
					goto depictedTurnAngleCheckEnd;
				}
			}
	depictedTurnAngleCheckEnd:

	// attempt to load max depicted turn angle when no individual turn angles were specified. also, do this ONLY when there is more than one frame.
	if(spec.depictedTurnAngle.empty() and spec.stateCount > 1)
	{
		spec.depictedTurnAngle.resize(spec.stateCount);

		key = "sprite_max_depicted_turn_angle";
		const float maxTurnAngle = (isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE)/SPRITE_IDEAL_MAX_DEPICTED_TURN_ANGLE;

		key = "sprite_depicted_turn_angle_progression";
		if(isValueSpecified(prop, key))
		{
			const string progression = futil::trim(prop.get(key));

			if(progression == "linear-with-threshold" or progression == "linear with threshold")
				goto linearWithThresholdProgression;

			else if(progression == "linear")
				for(unsigned s = 0; s < spec.stateCount; s++)
					spec.depictedTurnAngle[s] = s*(maxTurnAngle/(spec.stateCount-1));  // linear sprite progression

			else if(progression == "exponential")
				for(unsigned s = 0; s < spec.stateCount; s++)
					spec.depictedTurnAngle[s] = log(1+s*(exp(maxTurnAngle)-1)/(spec.stateCount-1));  // exponential sprite progression
			else
				cout << "warning: unknown turn angle progression type; using linear with threshold by default..." << endl;
		}
		else  // case for default progression (linear with threshold)
		{
			// linear sprite progression with 1-index advance at threshold angle
			linearWithThresholdProgression:
			spec.depictedTurnAngle[0] = 0;
			spec.depictedTurnAngle[1] = PSEUDO_ANGLE_THRESHOLD;
			for(unsigned s = 2; s < spec.stateCount; s++)
				spec.depictedTurnAngle[s] = PSEUDO_ANGLE_THRESHOLD + (s-1)*(maxTurnAngle-PSEUDO_ANGLE_THRESHOLD)/(spec.stateCount-2);
		}
	}

	key = "sprite_frame_count";
	const float globalFrameCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 1;

	for(unsigned stateNumber = 0; stateNumber < spec.stateCount; stateNumber++)
	{
		key = "sprite_state" + futil::to_string(stateNumber) + "_frame_count";
		const unsigned frameCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : globalFrameCount;
		spec.stateFrameCount.push_back(frameCount);
	}

	key = "sprite_frame_duration";
	if(prop.get(key) == "dynamic")
	{
		spec.frameDurationProportionalToSpeed = true;
		spec.frameDuration = -1;
	}
	else
	{
		spec.frameDurationProportionalToSpeed = false;
		spec.frameDuration = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : 0.25;
	}

	key = "sprite_animation_speed_factor";
	spec.animationSpeedFactor = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : 1.0;

	key = "brakelights_sprite_filename";
	spec.brakelightsSheetFilename = prop.get(key);

	bool usingDefaultBrakelights = false;
	if(spec.brakelightsSheetFilename == "builtin" or spec.brakelightsSheetFilename == "default")
	{
		usingDefaultBrakelights = true;
		spec.brakelightsSheetFilename = "assets/default-brakelight-effect.png";
		spec.brakelightsSpriteScale.x = spec.brakelightsSpriteScale.y = 1.0f;  // if no specified sheet, assume no scaling
	}
	else if(not spec.brakelightsSheetFilename.empty() and spec.brakelightsSheetFilename != "none")
	{
		spec.brakelightsSheetFilename = getContextualizedFilename(spec.brakelightsSheetFilename, prop.get("base_dir"), CarseLogic::VEHICLES_FOLDER+"/");

		if(spec.brakelightsSheetFilename.empty())
			cout << "warning: brakelight sprite file \"" << prop.get(key) << "\" could not be found!"
			<< " (specified by \"" << prop.get("filename") << "\"). ignoring brakelights definitions..." << endl;
		else
			spec.brakelightsSpriteScale = spec.scale;  // if specified sheet, assume same scale as main sprite
	}
	else if(spec.brakelightsSheetFilename != "none")
		spec.brakelightsSheetFilename.clear();

	// at this point either a valid brakelights sheet filename is set or it is empty (meaning that there won't be any brakelights effect)
	if(not spec.brakelightsSheetFilename.empty())
	{
		key = "brakelights_sprite_scale";
		if(isValueSpecified(prop, key))  // if brakelights scale factor is available, override previous definitions
		{
			spec.brakelightsSpriteScale.x = spec.brakelightsSpriteScale.y = atof(prop.get(key).c_str());
		}

		spec.brakelightsMultipleSprites = false;
		key = "brakelights_multiple_sprites";
		if(isValueSpecified(prop, key))
		{
			const string value = futil::to_lower(futil::trim(prop.get(key)));
			if(value == "true" or value == "yes")
				spec.brakelightsMultipleSprites = true;
		}

		for(unsigned stateNumber = 0; stateNumber < spec.stateCount; stateNumber++)
		{
			fgeal::Point brakelightsPosition;
			bool defaultedX = false, defaultedY = false;
			key = "brakelights_position" + futil::to_string(stateNumber);
			if(isValueSpecified(prop, key))
			{
				vector<string> tokens = futil::split(prop.get(key), ',');
				if(tokens.size() > 1)
				{
					brakelightsPosition.x = atof(tokens[0].c_str());
					brakelightsPosition.y = atof(tokens[1].c_str());
				}
			}
			else defaultedX = defaultedY = true;

			string key2 = key + "_x";
			if(isValueSpecified(prop, key2))
			{
				brakelightsPosition.x = atof(prop.get(key2).c_str());
				defaultedX = false;
			}

			key2 = key + "_y";
			if(isValueSpecified(prop, key2))
			{
				brakelightsPosition.y = atof(prop.get(key2).c_str());
				defaultedY = false;
			}

			if(defaultedX)
			{
				if(spec.brakelightsMultipleSprites)
					brakelightsPosition.x = 0;
				else
				{
					if(isBike)
						brakelightsPosition.x = 0.5*spec.frameWidth;
					else
						brakelightsPosition.x = 3.0*0.5*(spec.frameWidth - spec.depictedVehicleWidth) + stateNumber*0.0357*spec.frameWidth;
				}
			}
			if(defaultedY)
			{
				if(spec.brakelightsMultipleSprites)
					brakelightsPosition.y = 0;
				else
					brakelightsPosition.y = 0.5*spec.frameHeight;
			}

			spec.brakelightsPositions.push_back(brakelightsPosition);
		}

		key = "brakelights_sprite_offset_x";
		if(isValueSpecified(prop, key))
			spec.brakelightsOffset.x = atof(prop.get(key).c_str());
		else if(usingDefaultBrakelights)
			spec.brakelightsOffset.x = -0.5*DEFAULT_BRAKELIGHTS_SPRITE_WIDTH;
		else
			spec.brakelightsOffset.x = 0;

		key = "brakelights_sprite_offset_y";
		if(isValueSpecified(prop, key))
			spec.brakelightsOffset.y = atof(prop.get(key).c_str());
		else if(usingDefaultBrakelights)
			spec.brakelightsOffset.y = -0.5*DEFAULT_BRAKELIGHTS_SPRITE_HEIGHT;
		else
			spec.brakelightsOffset.y = 0;

		spec.brakelightsMirrowed = isBike? false : true;
		key = "brakelights_mirrowed";
		if(isValueSpecified(prop, key))
		{
			const string value = futil::to_lower(futil::trim(prop.get(key)));
			if(value == "false" or value == "no")
				spec.brakelightsMirrowed = false;
		}
	}

	key = "shadow_sprite_filename";
	spec.shadowSheetFilename = isValueSpecified(prop, key)? prop.get(key) : "default";

	if(spec.shadowSheetFilename == "none" or spec.shadowSheetFilename == "default")
		spec.shadowSheetFilename.clear();

	else if(not spec.shadowSheetFilename.empty())
	{
		spec.shadowSheetFilename = getContextualizedFilename(spec.shadowSheetFilename, prop.get("base_dir"), CarseLogic::VEHICLES_FOLDER+"/");

		if(spec.shadowSheetFilename.empty())
			cout << "warning: shadow sprite file \"" << prop.get(key) << "\" could not be found!"
			<< " (specified by \"" << prop.get("filename") << "\"). ignoring shadow definitions..." << endl;
	}

	if(not spec.shadowSheetFilename.empty()) for(unsigned stateNumber = 0; stateNumber < spec.stateCount; stateNumber++)
	{
		fgeal::Point shadowPosition;
		key = "shadow_position" + futil::to_string(stateNumber);
		if(isValueSpecified(prop, key))
		{
			vector<string> tokens = futil::split(prop.get(key), ',');
			if(tokens.size() > 1)
			{
				shadowPosition.x = atof(tokens[0].c_str());
				shadowPosition.y = atof(tokens[1].c_str());
			}
		}
		else shadowPosition.x = shadowPosition.y = 0;

		string key2 = key + "_x";
		shadowPosition.x = isValueSpecified(prop, key2)? atof(prop.get(key2).c_str()) : 0;

		key2 = key + "_y";
		shadowPosition.y = isValueSpecified(prop, key2)? atof(prop.get(key2).c_str()) : 0;

		spec.shadowPositions.push_back(shadowPosition);
	}
}
