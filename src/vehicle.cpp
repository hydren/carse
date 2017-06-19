/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "futil/string/more_operators.hpp"

#include <cstdlib>

using futil::Properties;
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

	spriteStateCount = prop.getAsValueOrDefault<int, atoi>("sprite_state_count", 1);
	spriteWidth = prop.getAsValueOrDefault<int, atoi>("sprite_frame_width", DEFAULT_SPRITE_WIDTH);
	spriteHeight = prop.getAsValueOrDefault<int, atoi>("sprite_frame_height", DEFAULT_SPRITE_HEIGHT);
	spriteFrameDuration = prop.getAsValueOrDefault<double, atof>("sprite_frame_duration", -1);
	spriteScale = prop.getAsValueOrDefault<double, atof>("sprite_scale", DEFAULT_SPRITE_HEIGHT / static_cast<float>(spriteHeight));

	for(unsigned stateNumber = 0; stateNumber < spriteStateCount; stateNumber++)
		spriteStateFrameCount.push_back(prop.getAsValueOrDefault<int, atoi>(string("sprite_state")+stateNumber+"_frame_count", 1));

	mass = prop.getAsValueOrDefault<double, atof>("vehicle_mass", 1250);

	engine.maxRpm = prop.getAsValueOrDefault<int, atoi>("engine_maximum_rpm", 7000);
	engine.torque = prop.getAsValueOrDefault<double, atof>("engine_maximum_power", 300) * POWER_TORQUE_FACTOR;

	engine.tireRadius = prop.getAsValueOrDefault<double, atof>("tire_diameter", 678) * 0.0005;

	// todo read more data from properties

	engine.transmissionEfficiency = 0.7;  // for the time being, assume 70% efficiency

	engine.gearCount = prop.getAsValueOrDefault<int, atoi>("gear_count", 6);

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
