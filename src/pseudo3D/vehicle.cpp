/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include <cstdlib>

using util::Properties;
using std::map;
using std::string;

Vehicle::Vehicle()
: name(), sheetFilename(),
  soundsFilenames(),
  engine(),
  mass(1250)
{}

Vehicle::Vehicle(const Properties& prop)
{
	string key;

	key = "vehicle_name";
	name = prop.containsKey(key)? prop.get(key) : "unnamed";

	key = "sprite_sheet_file";
	sheetFilename = prop.containsKey(key)? prop.get(key) : "car.png";

	key = "vehicle_mass";
	if(prop.containsKey(key))
		mass = atof(prop.get(key).c_str());

	key = "gear_count";
	engine.gearCount = prop.containsKey(key)? atoi(prop.get(key).c_str()) : 6;

	engine.gearRatio = new float[engine.gearCount];

	key = "gear_ratios";
	if(prop.containsKey(key))
	{
		string ratiosTxt = prop.get(key);
		if(ratiosTxt == "default")
		{
			//todo use default ratios
		}
		else
		{
			//todo split values and set ratios
		}
	}

	key = "gear_differential_ratio";
	engine.gearRatio[0] = prop.containsKey(key)? atof(prop.get(key).c_str()) : 0.4; // fixme choose reasonable default value

	key = "gear_reverse_ratio";
	engine.reverseGearRatio = prop.containsKey(key)? atof(prop.get(key).c_str()) : 0.4; // fixme choose reasonable default value

	key = "engine_maximum_rpm";
	engine.maxRpm = prop.containsKey(key)? atoi(prop.get(key).c_str()) : 7000;

	key = "engine_torque";
	engine.torque = prop.containsKey(key)? atof(prop.get(key).c_str()) : 500;

	key = "wheel_radius";
	engine.wheelRadius = prop.containsKey(key)? atof(prop.get(key).c_str()) : 0.34;

	// todo read more data from properties

	if(prop.containsKey("sound") and prop.get("sound") != "no")
	{
		string soundOption = prop.get("sound");
		if(soundOption == "no")
			soundsFilenames.clear();

		else if(soundOption == "default")
		{
			//todo
		}
		else if(soundOption == "custom")
		{
			//todo
		}
	}
}
