/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "futil/string_extra_operators.hpp"

#include <cstdlib>

using futil::Properties;
using std::map;
using std::string;

// default float constants
static const float
	DEFAULT_VEHICLE_MASS = 1250,  // kg
	DEFAULT_MAXIMUM_RPM = 7000,
	DEFAULT_MAXIMUM_POWER = 320,  // bhp
	DEFAULT_TIRE_DIAMETER = 678,  // mm
	DEFAULT_GEAR_COUNT = 5,
	DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE = 45, // 45 degrees, pi/4 radians
	DEFAULT_MAX_TORQUE_RPM_POSITION = 2.f/3.f,  // 0.66666... (two thirds)

	// for the time being, assume 70% efficiency
	DEFAULT_TRANSMISSION_EFFICIENCY = 0.7;

// default uint constants
static const unsigned
	DEFAULT_SPRITE_WIDTH = 56,
	DEFAULT_SPRITE_HEIGHT = 36;

Vehicle::Vehicle()
: spriteStateCount(), spriteWidth(), spriteHeight(), spriteFrameDuration(-1), spriteScale(-1), spriteMaxDepictedTurnAngle(1),
  mass(1250)
{}

Vehicle::Vehicle(const Properties& prop, Pseudo3DCarseGame& game)
{
	string key;

	key = "vehicle_name";
	name = prop.containsKey(key)? prop.get(key) : "unnamed";

	key = "sprite_sheet_file";
	sheetFilename = prop.containsKey(key)? prop.get(key) : "assets/car.png";

	spriteStateCount = prop.getParsedCStrAllowDefault<int, atoi>("sprite_state_count", 1);
	spriteWidth = prop.getParsedCStrAllowDefault<int, atoi>("sprite_frame_width", DEFAULT_SPRITE_WIDTH);
	spriteHeight = prop.getParsedCStrAllowDefault<int, atoi>("sprite_frame_height", DEFAULT_SPRITE_HEIGHT);
	spriteFrameDuration = prop.getParsedCStrAllowDefault<double, atof>("sprite_frame_duration", -1.0);
	spriteScale = prop.getParsedCStrAllowDefault<double, atof>("sprite_scale", DEFAULT_SPRITE_HEIGHT / static_cast<float>(spriteHeight));
	spriteMaxDepictedTurnAngle = prop.getParsedCStrAllowDefault<double, atof>("sprite_max_depicted_turn_angle", DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE)/DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE;

	for(unsigned stateNumber = 0; stateNumber < spriteStateCount; stateNumber++)
		spriteStateFrameCount.push_back(prop.getParsedCStrAllowDefault<int, atoi>(string("sprite_state")+stateNumber+"_frame_count", 1));

	mass = prop.getParsedCStrAllowDefault<double, atof>("vehicle_mass", DEFAULT_VEHICLE_MASS);

	engine.maxRpm = prop.getParsedCStrAllowDefault<int, atoi>("engine_maximum_rpm", DEFAULT_MAXIMUM_RPM);

	const float maxPower = prop.getParsedCStrAllowDefault<double, atof>("engine_maximum_power", DEFAULT_MAXIMUM_POWER);

	// generic torque rpm position
	const float maxTorqueRpm = (1000.0 + engine.maxRpm)*DEFAULT_MAX_TORQUE_RPM_POSITION;
	//float maxTorqueRpm = prop.getParsedCStrAllowDefault<int, atoi>("engine_maximum_torque_rpm", -1);

	// attempt to read max power rpm
	float maxPowerRpm = prop.getParsedCStrAllowDefault<int, atoi>("engine_maximum_power_rpm", -1);

	const float conversionFactor = 5252.0 * 1.355818, K = Engine::TorqueCurveProfile::TORQUE_CURVE_FINAL_VALUE;

//	if(maxTorqueRpm < 0)  // if no rpm of max torque is specified, estimate one
//	{
//		if(maxPowerRpm > 0)  // if specified max power rpm, use this to estimate max torque
//		{
//			maxTorqueRpm = 2.f*maxPowerRpm*(2.f - 2.f/K) + engine.maxRpm/K;
//		}
//		else maxTorqueRpm = (engine.maxRpm + 1000.f)*DEFAULT_MAX_TORQUE_RPM_POSITION;  // generic torque rpm position
//	}

	// if no rpm of max power is specified, estimate one
	if(maxPowerRpm < 0)
		// estimate max power rpm
		maxPowerRpm = (maxTorqueRpm + engine.maxRpm)*0.5;

	engine.torque = conversionFactor * maxPower * (engine.maxRpm - maxTorqueRpm) / (maxPowerRpm*((K - 1.0)*maxPowerRpm + engine.maxRpm - maxTorqueRpm*K));
//	engine.torque = prop.getParsedCStrAllowDefault<double, atof>("engine_maximum_power", DEFAULT_MAXIMUM_POWER) * POWER_TORQUE_FACTOR;

	engine.torqueCurveProfile = Engine::TorqueCurveProfile::create(engine.maxRpm, maxTorqueRpm);

	engine.tireRadius = prop.getParsedCStrAllowDefault<double, atof>("tire_diameter", DEFAULT_TIRE_DIAMETER) * 0.0005;

	// todo read more data from properties

	engine.transmissionEfficiency = DEFAULT_TRANSMISSION_EFFICIENCY;

	engine.gearCount = prop.getParsedCStrAllowDefault<int, atoi>("gear_count", DEFAULT_GEAR_COUNT);

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
