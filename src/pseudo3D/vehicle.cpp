/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "futil/string/more_operators.hpp"

#include <cstdlib>

using util::Properties;
using std::map;
using std::string;

static const unsigned DEFAULT_SPRITE_WIDTH = 56;
static const unsigned DEFAULT_SPRITE_HEIGHT = 36;

// fixme this factor still doesn't produce satisfactory results
static const float POWER_TORQUE_FACTOR = 5.0/3.0;

Vehicle::Vehicle()
: spriteStateCount(), spriteWidth(), spriteHeight(), spriteFrameDuration(-1), spriteScale(-1),
  mass(1250)
{}

Vehicle::Vehicle(const Properties& prop, Pseudo3DCarseGame& game)
{
	string key;

	key = "vehicle_name";
	name = prop.containsKey(key)? prop.get(key) : "unnamed";

	key = "sprite_sheet_file";
	sheetFilename = prop.containsKey(key)? prop.get(key) : "assets/car.png";

	key = "sprite_state_count";
	spriteStateCount = prop.containsKey(key) and prop.get(key) != "default"? atoi(prop.get(key).c_str()) : 1;

	key = "sprite_frame_width";
	spriteWidth = prop.containsKey(key) and prop.get(key) != "default"? atoi(prop.get(key).c_str()) : DEFAULT_SPRITE_WIDTH;

	key = "sprite_frame_height";
	spriteHeight = prop.containsKey(key) and prop.get(key) != "default"? atoi(prop.get(key).c_str()) : DEFAULT_SPRITE_HEIGHT;

	key = "sprite_frame_duration";
	spriteFrameDuration = prop.containsKey(key) and prop.get(key) != "default"? atof(prop.get(key).c_str()) : -1;

	key = "sprite_scale";
	spriteScale = prop.containsKey(key) and prop.get(key) != "default"? atof(prop.get(key).c_str()) : DEFAULT_SPRITE_HEIGHT / static_cast<float>(spriteHeight);

	for(unsigned stateNumber = 0; stateNumber < spriteStateCount; stateNumber++)
	{
		key = string("sprite_state")+stateNumber+"_frame_count";
		spriteStateFrameCount.push_back(prop.containsKey(key) and prop.get(key) != "default"? atoi(prop.get(key).c_str()) : 1);
	}

	key = "vehicle_mass";
	if(prop.containsKey(key))
		mass = atof(prop.get(key).c_str());

	mass = prop.containsKey(key) and prop.get(key) != "default"? atof(prop.get(key).c_str()) : 1250;

	key = "engine_maximum_rpm";
	engine.maxRpm = prop.containsKey(key) and prop.get(key) != "default"? atoi(prop.get(key).c_str()) : 7000;

	key = "engine_maximum_power";
	float power = prop.containsKey(key) and prop.get(key) != "default"? atof(prop.get(key).c_str()) : 300;
	engine.torque = power*POWER_TORQUE_FACTOR;

	key = "tire_diameter";
	engine.tireRadius = prop.containsKey(key) and prop.get(key) != "default"? 0.0005*atof(prop.get(key).c_str()) : 0.339;

	key = "tyre_diameter";
	engine.tireRadius = prop.containsKey(key) and prop.get(key) != "default"? 0.0005*atof(prop.get(key).c_str()) : 0.339;

	// todo read more data from properties

	key = "gear_count";
	engine.gearCount = prop.containsKey(key) and prop.get(key) != "default"? atoi(prop.get(key).c_str()) : 6;

	engine.gearRatio = new float[engine.gearCount+1];

	// first, set default ratios, then override
	engine.reverseGearRatio = 3.25;
	engine.gearRatio[0] = 3.0;
	for(int g = 1; g <= engine.gearCount; g++)
		engine.gearRatio[g] = 3.0 + 2.0*((g - 1.0)/(1.0 - engine.gearCount)); // generic gear ratio

	key = "gear_ratios";
	if(prop.containsKey(key))
	{
		string ratiosTxt = prop.get(key);
		if(ratiosTxt == "custom")
		{
			key = "gear_differential_ratio";
			if(prop.containsKey(key)) engine.gearRatio[0] = atof(prop.get(key).c_str());

			key = "gear_reverse_ratio";
			if(prop.containsKey(key)) engine.reverseGearRatio = atof(prop.get(key).c_str());

			for(int g = 1; g <= engine.gearCount; g++)
			{
				key = string("gear_") + g + "_ratio";
				if(prop.containsKey(key))
					engine.gearRatio[g] = atof(prop.get(key).c_str());
			}
		}
	}

	if(EngineSoundProfile::requestsPresetProfile(prop))
		engineSoundProfile = game.getPresetEngineSoundProfile(EngineSoundProfile::getSoundDefinitionFromProperties(prop));
	else
		engineSoundProfile = EngineSoundProfile::loadFromProperties(prop);
}
