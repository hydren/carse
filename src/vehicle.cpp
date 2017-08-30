/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "futil/string_actions.hpp"

#include <cstdlib>
#include <cmath>

#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

using futil::Properties;
using futil::to_lower;
using std::string;

// default float constants
static const float
	DEFAULT_VEHICLE_MASS = 1250,  // kg
	DEFAULT_TIRE_DIAMETER = 678;  // mm

Vehicle::Vehicle()
: type(TYPE_CAR), name(), authors(), credits(), comments(),
  mass(0), tireRadius(0), engine(), speed(0), brakePedalPosition(0),
  approximatedCenterOfGravityHeight(0), approximatedWheelbase(0), acceleration(0),
  engineLocation(ENGINE_LOCATION_ON_FRONT), drivenWheels(DRIVEN_WHEELS_ON_REAR),
  engineSoundProfile(), sprite()
{}

Vehicle::Vehicle(const Properties& prop, Pseudo3DCarseGame& game)
{
	// aux. var
	string key;

	// logic data

	key = "vehicle_type";
	if(prop.containsKey(key))
	{
		string t = to_lower(prop.get(key));
		if(t == "car" or t == "default") type = TYPE_CAR;
		else if(t == "bike") type = TYPE_BIKE;
		else type = TYPE_OTHER;
	}
	else type = TYPE_CAR;

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

	key = "vehicle_mass";
	mass = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_VEHICLE_MASS;

	key = "tire_diameter";
	tireRadius = (isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_TIRE_DIAMETER) * 0.0005;

	key = "engine_location";
	if(isValueSpecified(prop, key))
	{
		const string value = prop.get(key);
		if(value == "mid" or value == "middle") engineLocation = ENGINE_LOCATION_ON_MIDDLE;
		else if(value == "rear") engineLocation = ENGINE_LOCATION_ON_REAR;
		else engineLocation = ENGINE_LOCATION_ON_FRONT;
	}
	else
		engineLocation = ENGINE_LOCATION_ON_FRONT;

	key = "driven_wheels";
	if(isValueSpecified(prop, key))
	{
		const string value = prop.get(key);
		if(value == "all") drivenWheels = DRIVEN_WHEELS_ALL;
		else if(value == "front") drivenWheels = DRIVEN_WHEELS_ON_FRONT;
		else drivenWheels = DRIVEN_WHEELS_ON_REAR;
	}
	else
		drivenWheels = DRIVEN_WHEELS_ON_REAR;

	engine = Engine(prop);

	speed = 0;
	brakePedalPosition = 0;
	acceleration = 0;

	// sound data

	if(EngineSoundProfile::requestsPresetProfile(prop))
		engineSoundProfile = game.getPresetEngineSoundProfile(EngineSoundProfile::getSoundDefinitionFromProperties(prop));
	else
		engineSoundProfile = EngineSoundProfile::loadFromProperties(prop);

	// sprite data

	sprite = Pseudo3DVehicleAnimationProfile(prop);

	// ########################################################################################################################################################
	// These properties need to be loaded after sprite data to make sure that some fields are ready ('depictedVehicleWidth', 'sprite_sheet_file', etc)

	if(sprite.sheetFilename == "DEFAULT")
	{
		// uncomment when there is a default sprite for bikes
//		switch(type)
//		{
//			case TYPE_BIKE:  sprite.sheetFilename = "assets/bike-sheet-default.png"; break;
//			default:
//			case TYPE_OTHER:
//			case TYPE_CAR:   sprite.sheetFilename = "assets/car-sheet-default.png"; break;
//		}

		sprite.sheetFilename = "assets/car-sheet-default.png";
	}

	// attempt to estimate center's of gravity height
	key = "vehicle_height";
	if(isValueSpecified(prop, key))
		approximatedCenterOfGravityHeight = 0.5f*atof(prop.get(key).c_str());  // aprox. half the height
	else
		approximatedCenterOfGravityHeight = 0.3506f * sprite.depictedVehicleWidth * sprite.scale.x * 895.0/24.0;  // proportion aprox. of a fairlady z32


	// attempt to estimate wheelbase
	{
		key = "vehicle_wheelbase";
		if(isValueSpecified(prop, key))
			approximatedWheelbase = atof(prop.get(key).c_str());
		else
			approximatedWheelbase = -1;

		key = "vehicle_length";
		if(approximatedWheelbase == -1 and isValueSpecified(prop, key))
			approximatedWheelbase = atof(prop.get(key).c_str());

		key = "vehicle_width";
		if(approximatedWheelbase == -1 and isValueSpecified(prop, key))
			approximatedWheelbase = 2.5251f * atof(prop.get(key).c_str());  // proportion aprox. of a fairlady z32

		key = "vehicle_height";
		if(approximatedWheelbase == -1 and isValueSpecified(prop, key))
			approximatedWheelbase = 3.6016f * atof(prop.get(key).c_str());  // proportion aprox. of a fairlady z32

		if(approximatedWheelbase == -1)
		{
			approximatedWheelbase = 2.5251f * sprite.depictedVehicleWidth * sprite.scale.x * 895.0/24.0;  // proportion aprox. of a fairlady z32
		}
	}
}
