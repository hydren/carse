/*
 * carse_game_logic.cpp
 *
 *  Created on: 1 de dez de 2017
 *      Author: carlosfaruolo
 */

#include "carse_game_logic.hpp"

#include "carse_game.hpp"

#include "futil/string_extra_operators.hpp"
#include "futil/string_actions.hpp"
#include "futil/string_split.hpp"
#include "futil/round.h"

#include <vector>

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cmath>

using futil::Properties;
using futil::ends_with;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::map;

// to reduce typing is good
#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

// returns true if the given properties requests a preset sound profile instead of specifying a custom one.
static bool isEngineSoundProfileRequestingPreset(const Properties& prop) { return prop.containsKey("sound") and prop.get("sound") != "custom" and prop.get("sound") != "no"; }
static void loadEngineSoundSpec(EngineSoundProfile&, const Properties&);

// logic constructor, booooooring!
CarseGameLogic::CarseGameLogic(Pseudo3DCarseGame& game) : game(game) {}

void CarseGameLogic::initialize()
{
	this->loadPresetEngineSoundProfiles();
	this->loadCourses();
	this->loadVehicles();
}

void CarseGameLogic::onStatesListInitFinished()
{
	this->setNextCourseRandom();  // set default course
	this->setPickedVehicle(vehicles[0]);  // set default vehicle
	this->setImperialUnitEnabled(false);
	this->setSimulationType(Mechanics::SIMULATION_TYPE_SLIPLESS);
}

Pseudo3DRaceState& CarseGameLogic::getRaceState()
{
	return *static_cast<Pseudo3DRaceState*>(game.getState(Pseudo3DCarseGame::RACE_STATE_ID));  //todo create an reference or pointer on logic class to avoid this deference
}

void CarseGameLogic::loadPresetEngineSoundProfiles()
{
	cout << "reading preset engine sound profiles..." << endl;
	vector<string> pendingPresetFiles, presetFiles = fgeal::filesystem::getFilenamesWithinDirectory("assets/sound/engine");
	for(unsigned i = 0; i < presetFiles.size(); i++)
	{
		string& filename = presetFiles[i];
		if(ends_with(filename, ".properties"))
		{
			Properties prop;
			prop.load(filename);

			const string
				filenameWithoutPath = filename.substr(filename.find_last_of("/\\")+1),
				filenameWithoutExtension = filenameWithoutPath.substr(0, filenameWithoutPath.find_last_of(".")),
				presetName = filenameWithoutExtension;

			if(isEngineSoundProfileRequestingPreset(prop))
			{
				pendingPresetFiles.push_back(filename);
				cout << "read engine sound profile: " << presetName << " (alias)" << endl;
			}

			else
			{
				loadEngineSoundSpec(presetEngineSoundProfiles[presetName], prop);
				cout << "read engine sound profile: " << presetName << endl;
			}
		}
	}

	unsigned previousCount = pendingPresetFiles.size();
	while(not pendingPresetFiles.empty())
	{
		for(unsigned i = 0; i < pendingPresetFiles.size(); i++)
		{
			string filename = pendingPresetFiles[i];
			Properties prop;
			prop.load(filename);

			const string
				basePresetName = prop.get("sound"),
				filenameWithoutPath = filename.substr(filename.find_last_of("/\\")+1),
				filenameWithoutExtension = filenameWithoutPath.substr(0, filenameWithoutPath.find_last_of(".")),
				presetName = filenameWithoutExtension;

			if(presetEngineSoundProfiles.find(basePresetName) != presetEngineSoundProfiles.end())
			{
				presetEngineSoundProfiles[presetName] = presetEngineSoundProfiles[basePresetName];
				cout << "copied profile \"" << presetName << "\" from \"" << basePresetName << "\"" << endl;
				pendingPresetFiles.erase(pendingPresetFiles.begin() + i);
				i--;
			}
		}
		if(pendingPresetFiles.size() == previousCount)
		{
			cout << "circular dependency or unresolved reference detected when loading preset engine sound profiles. skipping resolution." << endl;
			cout << "the following preset engine sound profiles could not be loaded: " << endl;
			for(unsigned i = 0; i < pendingPresetFiles.size(); i++)
				cout << pendingPresetFiles[i] << endl;
			break;
		}
		else previousCount = pendingPresetFiles.size();
	}
}

EngineSoundProfile& CarseGameLogic::getPresetEngineSoundProfile(const std::string presetName)
{
	if(presetEngineSoundProfiles.find(presetName) != presetEngineSoundProfiles.end())
		return presetEngineSoundProfiles[presetName];
	else
		return presetEngineSoundProfiles["default"];
}

void CarseGameLogic::loadCourses()
{
	cout << "reading courses..." << endl;

	vector<string> courseFiles = fgeal::filesystem::getFilenamesWithinDirectory("data/courses");
	for(unsigned i = 0; i < courseFiles.size(); i++)
	{
		if(ends_with(courseFiles[i], ".properties"))
		{
			Properties prop;
			prop.load(courseFiles[i]);
			courses.push_back(Course::createCourseFromFile(prop));
			courses.back().filename = courseFiles[i];
			cout << "read course: " << courseFiles[i] << endl;
		}
	}
}

const vector<Course>& CarseGameLogic::getCourseList()
{
	return courses;
}

void CarseGameLogic::setNextCourse(unsigned courseIndex)
{
	getRaceState().course = courses[courseIndex];
}

void CarseGameLogic::setNextCourse(const Course& c)
{
	getRaceState().course = c;
}

void CarseGameLogic::setNextCourseRandom()
{
	getRaceState().course = Course::createRandomCourse(200, 3000, 6400, 1.5);
}

void CarseGameLogic::setNextCourseDebug()
{
	getRaceState().course = Course::createDebugCourse(200, 3000);
}

void CarseGameLogic::loadVehicles()
{
	cout << "reading vehicles specs..." << endl;
	vector<string> vehicleFiles = fgeal::filesystem::getFilenamesWithinDirectory("data/vehicles");
	for(unsigned i = 0; i < vehicleFiles.size(); i++)
	{
		const string& filename = vehicleFiles[i];
		if(fgeal::filesystem::isFilenameDirectory(filename))
		{
			vector<string> subfolderFiles = fgeal::filesystem::getFilenamesWithinDirectory(filename);
			for(unsigned j = 0; j < subfolderFiles.size(); j++)
			{
				const string& subfolderFile = subfolderFiles[j];
				if(ends_with(subfolderFile, ".properties"))
				{
					Properties prop;
					prop.load(subfolderFile);
					vehicles.push_back(Pseudo3DVehicle::Spec());
					loadVehicleSpec(vehicles.back(), prop);
					cout << "read vehicle spec: " << subfolderFile << endl;
					break;
				}
			}
		}
		else if(ends_with(filename, ".properties"))
		{
			Properties prop;
			prop.load(filename);
			vehicles.push_back(Pseudo3DVehicle::Spec());
			loadVehicleSpec(vehicles.back(), prop);
			cout << "read vehicle spec: " << filename << endl;
		}
	}
}

const vector<Pseudo3DVehicle::Spec>& CarseGameLogic::getVehicleList()
{
	return vehicles;
}

void CarseGameLogic::setPickedVehicle(unsigned vehicleIndex, int skin)
{
	getRaceState().vehicle = Pseudo3DVehicle(vehicles[vehicleIndex], skin);
}

void CarseGameLogic::setPickedVehicle(const Pseudo3DVehicle::Spec& vspec, int skin)
{
	getRaceState().vehicle = Pseudo3DVehicle(vspec, skin);
}

void CarseGameLogic::setPickedVehicle(const Pseudo3DVehicle& v)
{
	getRaceState().vehicle = v;
}

bool CarseGameLogic::isImperialUnitEnabled()
{
	return getRaceState().isImperialUnit;
}

void CarseGameLogic::setImperialUnitEnabled(bool choice)
{
	getRaceState().isImperialUnit = choice;
}

Mechanics::SimulationType CarseGameLogic::getSimulationType()
{
	return getRaceState().simulationType;
}

void CarseGameLogic::setSimulationType(Mechanics::SimulationType type)
{
	getRaceState().simulationType = type;
}

// ----------------------------------------------------------------------------------------------------------

// vehicle spec default constants
static const float
	DEFAULT_VEHICLE_MASS = 1250,  // kg
	DEFAULT_TIRE_DIAMETER = 678,  // mm

	DEFAULT_FRONTAL_AREA_CAR  = 1.81,  // frontal area (in square-meters) of a Nissan 300ZX (Z32)
	DEFAULT_FRONTAL_AREA_BIKE = 0.70,  // estimated frontal area (in square-meters) of a common sporty bike

	DEFAULT_CDA_CAR  = 0.31 * DEFAULT_FRONTAL_AREA_CAR,   // drag coefficient (Cd) of a Nissan 300ZX (Z32)
	DEFAULT_CDA_BIKE = 0.60 * DEFAULT_FRONTAL_AREA_BIKE,  // estimated drag coefficient (Cd) of a common sporty bike
	DEFAULT_CLA_CAR  = 0.20 * DEFAULT_FRONTAL_AREA_CAR,    // estimated lift coefficient (Cl) of a Nissan 300ZX (Z32)
	DEFAULT_CLA_BIKE = 0.10 * DEFAULT_FRONTAL_AREA_BIKE,   // estimated lift coefficient (Cl) of a common sporty bike

	DEFAULT_FR_WEIGHT_DISTRIBUITION = 0.45,
	DEFAULT_MR_WEIGHT_DISTRIBUITION = 0.55,
	DEFAULT_RR_WEIGHT_DISTRIBUITION = 0.65,
	DEFAULT_FF_WEIGHT_DISTRIBUITION = 0.40,

	DEFAULT_MAXIMUM_RPM = 7000,
	DEFAULT_MAXIMUM_POWER = 320,  // bhp
	DEFAULT_GEAR_COUNT = 5,

	// for the time being, assume 70% efficiency
	DEFAULT_TRANSMISSION_EFFICIENCY = 0.7;

static void loadPowertrainSpec(Pseudo3DVehicle::Spec&, const Properties&);
static void loadChassisSpec(Pseudo3DVehicle::Spec&, const Properties&);
static void loadEngineSoundSpec(EngineSoundProfile&, const Properties&);
static void loadAnimationSpec(Pseudo3DVehicleAnimationSpec&, const Properties&);

void CarseGameLogic::loadVehicleSpec(Pseudo3DVehicle::Spec& spec, const futil::Properties& prop)
{
	// aux. var
	string key;

	// logic data

	key = "vehicle_type";
	if(prop.containsKey(key))
	{
		string t = futil::to_lower(prop.get(key));
		if(t == "car" or t == "default") spec.type = Mechanics::TYPE_CAR;
		else if(t == "bike") spec.type = Mechanics::TYPE_BIKE;
		else spec.type = Mechanics::TYPE_OTHER;
	}
	else spec.type = Mechanics::TYPE_CAR;

	// info data

	key = "vehicle_name";
	spec.name = prop.containsKey(key)? prop.get(key) : "unnamed";

	key = "authors";
	spec.authors = prop.containsKey(key)? prop.get(key) : "unknown";

	key = "credits";
	spec.credits = prop.containsKey(key)? prop.get(key) : "";

	key = "comments";
	spec.comments = prop.containsKey(key)? prop.get(key) : "";

	// todo read more data from properties

	loadPowertrainSpec(spec, prop);
	loadChassisSpec(spec, prop);

	// sound data
	if(isEngineSoundProfileRequestingPreset(prop))
		spec.sound = getPresetEngineSoundProfile(prop.get("sound"));
	else
		loadEngineSoundSpec(spec.sound, prop);

	// sprite data
	loadAnimationSpec(spec.sprite, prop);

	// attempt to read up to 32 alternative sprites
	for(unsigned i = 0; i < 32; i++)
	{
		key = "alternate_sprite_sheet" + futil::to_string(i) + "_definition_file";
		if(isValueSpecified(prop, key))
		{
			const string alternateSpritePropFile = prop.get(key);
			Properties alternateSpriteProp;
			alternateSpriteProp.load(alternateSpritePropFile);
			spec.alternateSprites.push_back(Pseudo3DVehicleAnimationSpec());
			loadAnimationSpec(spec.alternateSprites.back(), alternateSpriteProp);
		}
		else
		{
			key = "alternate_sprite_sheet" + futil::to_string(i) + "_file";
			if(isValueSpecified(prop, key))
			{
				const string alternateSpriteSheetFile = prop.get(key);

				int inheritedProfileIndex = -1;
				key = "alternate_sprite_sheet" + futil::to_string(i) + "_inherit_from";
				if(isValueSpecified(prop, key))
				{
					const unsigned inheritIndex = prop.getParsedCStrAllowDefault<int, atoi>(key, 0);
					if(inheritIndex < spec.alternateSprites.size())
						inheritedProfileIndex = inheritIndex;
					else
						cout << "warning: " << key << " specifies an out of bounds index (" << inheritIndex << "). inheriting default instead." << endl;
				}

				if(inheritedProfileIndex != -1)
					spec.alternateSprites.push_back(spec.alternateSprites[inheritedProfileIndex]);
				else
					spec.alternateSprites.push_back(spec.sprite);

				spec.alternateSprites.back().sheetFilename = alternateSpriteSheetFile;
			}
		}
	}

	// ----------------------------------------------------------------------------------------------------------
	// These properties need to be loaded after sprite data to make sure that some fields are ready ('depictedVehicleWidth', 'sprite_sheet_file', etc)

	// attempt to estimate center's of gravity height
	key = "vehicle_height";
	if(isValueSpecified(prop, key))
		spec.centerOfGravityHeight = 0.5f*atof(prop.get(key).c_str());  // aprox. half the height
	else
		spec.centerOfGravityHeight = 0.3506f * spec.sprite.depictedVehicleWidth * spec.sprite.scale.x * 895.0/24.0;  // proportion aprox. of a fairlady z32


	// attempt to estimate wheelbase
	{
		key = "vehicle_wheelbase";
		if(isValueSpecified(prop, key))
			spec.wheelbase = atof(prop.get(key).c_str());
		else
			spec.wheelbase = -1;

		key = "vehicle_length";
		if(spec.wheelbase == -1 and isValueSpecified(prop, key))
			spec.wheelbase = atof(prop.get(key).c_str());

		key = "vehicle_width";
		if(spec.wheelbase == -1 and isValueSpecified(prop, key))
			spec.wheelbase = 2.5251f * atof(prop.get(key).c_str());  // proportion aprox. of a fairlady z32

		key = "vehicle_height";
		if(spec.wheelbase == -1 and isValueSpecified(prop, key))
			spec.wheelbase = 3.6016f * atof(prop.get(key).c_str());  // proportion aprox. of a fairlady z32

		if(spec.wheelbase == -1)
		{
			spec.wheelbase = 2.5251f * spec.sprite.depictedVehicleWidth * spec.sprite.scale.x * 895.0/24.0;  // proportion aprox. of a fairlady z32
		}
	}
}

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
	spec.engineMaximumRpm = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_MAXIMUM_RPM;
	spec.engineMinimumRpm = 1000;

	key = "engine_maximum_power";
	spec.engineMaximumPower = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_MAXIMUM_POWER;

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

	spec.engineTransmissionEfficiency = DEFAULT_TRANSMISSION_EFFICIENCY;

	key = "gear_count";
	spec.engineGearCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_GEAR_COUNT;
	spec.engineGearRatio.resize(spec.engineGearCount);

	// first, set default ratios, then override
	spec.engineReverseGearRatio = 3.25;
	spec.engineDifferentialRatio = 4.0;
	for(int g = 0; g < spec.engineGearCount; g++)
		spec.engineGearRatio[g] = 3.0 + g*2.0/(1.0 - spec.engineGearCount);  // generic gear ratio

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

static void loadChassisSpec(Pseudo3DVehicle::Spec& spec, const Properties& prop)
{
	string key = "vehicle_mass";
	spec.mass = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_VEHICLE_MASS;

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
		spec.engineLocation = Mechanics::ENGINE_LOCATION_ON_FRONT;

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

	spec.dragArea = spec.type == Mechanics::TYPE_CAR? DEFAULT_CDA_CAR : spec.type == Mechanics::TYPE_BIKE? DEFAULT_CDA_BIKE : 0.5;
	spec.liftArea = spec.type == Mechanics::TYPE_CAR? DEFAULT_CLA_CAR : spec.type == Mechanics::TYPE_BIKE? DEFAULT_CLA_BIKE : 0.5;

	if(spec.engineLocation == Mechanics::ENGINE_LOCATION_ON_FRONT)
	{
		if(spec.drivenWheelsType == Mechanics::DRIVEN_WHEELS_ON_FRONT)
			spec.weightDistribuition = DEFAULT_FF_WEIGHT_DISTRIBUITION;

		else /* spec.drivenWheelsType == Mechanics::DRIVEN_WHEELS_ON_REAR or Mechanics::DRIVEN_WHEELS_ALL */
			spec.weightDistribuition = DEFAULT_FR_WEIGHT_DISTRIBUITION;
	}
	else if(spec.engineLocation == Mechanics::ENGINE_LOCATION_ON_REAR)
		spec.weightDistribuition = DEFAULT_RR_WEIGHT_DISTRIBUITION;

	else /* Mechanics::ENGINE_LOCATION_ON_MIDDLE */
		spec.weightDistribuition = DEFAULT_MR_WEIGHT_DISTRIBUITION;
}

static bool rangeProfileCompareFunction(const EngineSoundProfile::RangeProfile& p1, const EngineSoundProfile::RangeProfile& p2)
{
	return p1.isRedline? false : p2.isRedline? true : p1.rpm < p2.rpm;
}

// load a custom sound profile from properties. if a non-custom (preset) profile is specified, a std::logic_error is thrown.
static void loadEngineSoundSpec(EngineSoundProfile& profile, const Properties& prop)
{
	short maxRpm = 0;

	maxRpm = prop.getParsedCStrAllowDefault<int, atoi>("engine_maximum_rpm", 7000);

	string baseKey = "sound";
	if(prop.containsKey(baseKey))
	{
		if(prop.get(baseKey) == "none")
		{
			profile.ranges.clear();
		}
		else if(prop.get(baseKey) == "custom")
		{
			int i = 0;
			string key = baseKey + i;
			while(prop.containsKey(key))
			{
				string filename = prop.get(key);

				// now try to read _rpm property
				key += "_rpm";
				short rpm = -1;
				bool isRedline = false;
				if(prop.containsKey(key))
				{
					if(prop.get(key) == "redline")
						isRedline = true;
					else
						rpm = atoi(prop.get(key).c_str());
				}

				// if rpm < 0, either rpm wasn't specified, or was intentionally left -1 (or other negative number)
				if(rpm < 0 and not isRedline)
				{
					if(i == 0) rpm = 0;
					else       rpm = (maxRpm - profile.ranges.rbegin()->rpm)/2;
				}

				// save filename and settings for given rpm
				EngineSoundProfile::RangeProfile range = {rpm, filename, isRedline};
				profile.ranges.push_back(range);
				i += 1;
				key = baseKey + i;
			}

			std::stable_sort(profile.ranges.begin(), profile.ranges.end(), rangeProfileCompareFunction);
		}
		else throw std::logic_error("properties specify a preset profile instead of a custom one");
	}
}

// default sprite uint constants
static const unsigned
	DEFAULT_SPRITE_WIDTH = 56,
	DEFAULT_SPRITE_HEIGHT = 36;

// default sprite float constants
static const float
	DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE = 45, // 45 degrees, pi/4 radians
	DEFAULT_SPRITE_DEPICTED_VEHICLE_WIDTH_PROPORTION = 0.857142857143;  // ~0,857

static void loadAnimationSpec(Pseudo3DVehicleAnimationSpec& spec, const Properties& prop)
{
	// aux. vars
	string key, key2, key3;

	key = "sprite_sheet_file";
	spec.sheetFilename = isValueSpecified(prop, key)? prop.get(key) : "DEFAULT";

	if(spec.sheetFilename == "DEFAULT")
	{
		// uncomment when there is a default sprite for bikes
//		switch(type)
//		{
//			case TYPE_BIKE:  spec.sheetFilename = "assets/bike-sheet-default.png"; break;
//			default:
//			case TYPE_OTHER:
//			case TYPE_CAR:   spec.sheetFilename = "assets/car-sheet-default.png"; break;
//		}

		spec.sheetFilename = "assets/car-sheet-default.png";
	}

	key = "sprite_state_count";
	spec.stateCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 1;

	key = "sprite_frame_width";
	spec.frameWidth = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_SPRITE_WIDTH;

	key = "sprite_frame_height";
	spec.frameHeight = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_SPRITE_HEIGHT;

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

	key = "vehicle_width";
	if(isValueSpecified(prop, key))  // if vehicle width is available, compute recommended scale factor
	{
		const float vehicleWidth = atoi(prop.get(key).c_str());  // the real-life vehicle width, in mm

		key = "sprite_vehicle_height"; key2 = "vehicle_height"; key3 = "vehicle_width_height_ratio";
		if(not keepAspectRatio and isValueSpecified(prop, key)  // if vehicle height (both real-life and in sprite) are available, adjust scale factor (if allowed)
		and (isValueSpecified(prop, key2) or isValueSpecified(prop, key3)))  // ratios can be obtained by specifing height or ratio itself
		{
			// adjust scale factor to account for width/height ratio discrepancies
			const float spriteVehicleHeight = atoi(prop.get(key).c_str()),  // the vehicle width on the sprite, in pixels
						spriteWHRatio = ((float) spec.depictedVehicleWidth) / spriteVehicleHeight;  // sprite width/height (WH) ratio

			// the real-life vehicle height (if available), in mm.
			const float vehicleHeight = isValueSpecified(prop, key2)? atoi(prop.get(key2).c_str()) : -1;

			if(vehicleHeight == 0)
				throw std::invalid_argument("vehicle height is zero!");

			const float vehicleWHRatio = isValueSpecified(prop, key3)? atof(prop.get(key3).c_str())  // if real-life width/height ratio is available, prefer to use it
															   : (vehicleWidth / vehicleHeight);  // otherwise calculate it from the available real-life width and height

			const float ratioFixFactor = vehicleWHRatio / spriteWHRatio,  // multiplier that makes the sprite width/height ratio match the real-life width/height ratio
						fixedDepictedVehicleWidth = spec.depictedVehicleWidth * ratioFixFactor;  // corrected in-sprite width

			// recommended scale factors, making sprite width/height ratio match the real-life width/height ratio
			spec.scale.y = (vehicleWidth / fixedDepictedVehicleWidth) * (24.0/895.0);
			spec.scale.x = spec.scale.y * ratioFixFactor;
		}
		else  // no data about vehicle height or width/height ratio given; assume no no width/height ratio discrepancies between real-life
		{
			spec.scale.x = spec.scale.y = (vehicleWidth /(float) spec.depictedVehicleWidth) * (24.0/895.0);  // recommended scale factor assuming no width/height ratio discrepancies
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
	spec.contactOffset = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 0;

	spec.asymmetrical = false;
	key = "sprite_asymmetric";
	if(isValueSpecified(prop, key))
	{
		const string value = futil::to_lower(futil::trim(prop.get(key)));
		if(value == "true" or value == "yes")
			spec.asymmetrical = true;
	}

	key = "sprite_max_depicted_turn_angle";
	const float absoluteTurnAngle = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE;
	spec.maxDepictedTurnAngle = absoluteTurnAngle/DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE;

	key = "sprite_frame_duration";
	spec.frameDuration = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : -1.0;

	for(unsigned stateNumber = 0; stateNumber < spec.stateCount; stateNumber++)
	{
		key = "sprite_state" + futil::to_string(stateNumber) + "_frame_count";
		const unsigned frameCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 1;
		spec.stateFrameCount.push_back(frameCount);
	}
}

// ########################################################################################################################################################

CarseSharedResources::CarseSharedResources()
: sndCursorMove("assets/sound/cursor_move.ogg"),
  sndCursorIn("assets/sound/cursor_accept.ogg"),
  sndCursorOut("assets/sound/cursor_out.ogg"),
  fontDev("assets/font.ttf", 12)
{}
