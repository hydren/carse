/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "futil/string_actions.hpp"

#include <cstdlib>

using futil::Properties;
using futil::to_lower;
using std::string;

// default float constants
static const float
	DEFAULT_VEHICLE_MASS = 1250,  // kg
	DEFAULT_MAXIMUM_RPM = 7000,
	DEFAULT_MAXIMUM_POWER = 320,  // bhp
	DEFAULT_TIRE_DIAMETER = 678,  // mm
	DEFAULT_GEAR_COUNT = 5,
	DEFAULT_MAX_TORQUE_RPM_POSITION = 2.f/3.f,  // 0.66666... (two thirds)

	// for the time being, assume 70% efficiency
	DEFAULT_TRANSMISSION_EFFICIENCY = 0.7;

Vehicle::Vehicle()
: type(TYPE_CAR), name(), authors(), credits(), comments(),
  mass(0), tireRadius(0), engine(), speed(0), brakePedalPosition(0),
  engineSoundProfile(), gfx()
{}

#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

Vehicle::Vehicle(const Properties& prop, Pseudo3DCarseGame& game)
{
	// aux. var
	string key;

	// logic data

	key = "vehicle_type";
	if(prop.containsKey(key))
	{
		string t = prop.get(key);
		if(to_lower(t) == "car") type = TYPE_CAR;
		else if(to_lower(t) == "bike") type = TYPE_BIKE;
		else type = TYPE_OTHER;
	}

	// info data

	key = "vehicle_name";
	name = prop.containsKey(key)? prop.get(key) : "unnamed";

	key = "authors";
	authors = prop.containsKey(key)? prop.get(key) : "unknown";

	key = "credits";
	credits = prop.containsKey(key)? prop.get(key) : "";

	key = "comments";
	comments = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_configuration";
	engine.configuration = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_aspiration";
	engine.aspiration = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_valvetrain";
	engine.valvetrain = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_displacement";
	engine.displacement = prop.containsKey(key)? atoi(prop.get(key).c_str()) : 0;

	key = "engine_valve_count";
	engine.valveCount = prop.containsKey(key)? atoi(prop.get(key).c_str()) : 0;

	// physics data

	key = "vehicle_mass";
	mass = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_VEHICLE_MASS;

	key = "engine_maximum_rpm";
	engine.maxRpm = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_MAXIMUM_RPM;

	key = "engine_maximum_power";
	const float maxPower = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_MAXIMUM_POWER;

	// xxx generic hardcoded torque rpm position
	const float maxTorqueRpm = (1000.0 + engine.maxRpm)*DEFAULT_MAX_TORQUE_RPM_POSITION;

//	key = "engine_maximum_torque_rpm";
//	float maxTorqueRpm = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : -1;

	// attempt to read max power rpm
	key = "engine_maximum_power_rpm";
	float maxPowerRpm = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : -1;

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

//	key = "engine_maximum_power";
//	engine.torque = (isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_MAXIMUM_POWER)*POWER_TORQUE_FACTOR;
	engine.maximumTorque = conversionFactor * maxPower * (engine.maxRpm - maxTorqueRpm) / (maxPowerRpm*((K - 1.0)*maxPowerRpm + engine.maxRpm - maxTorqueRpm*K));

	engine.torqueCurveProfile = Engine::TorqueCurveProfile::create(engine.maxRpm, maxTorqueRpm);

	// informative-only fields
	engine.maximumPower = maxPower;
	engine.maximumPowerRpm = maxPowerRpm;
	engine.maximumTorqueRpm = maxTorqueRpm;

	key = "tire_diameter";
	tireRadius = (isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_TIRE_DIAMETER) * 0.0005;

	// todo read more data from properties

	engine.transmissionEfficiency = DEFAULT_TRANSMISSION_EFFICIENCY;

	key = "gear_count";
	engine.gearCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_GEAR_COUNT;
	engine.gearRatio.resize(engine.gearCount+1);

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
				key = "gear_" + futil::to_string(g) + "_ratio";
				if(prop.containsKey(key))
					engine.gearRatio[g] = atof(prop.get(key).c_str());
			}
		}
	}

	engine.throttlePosition = 0;
	speed = 0;
	brakePedalPosition = 0;

	// sound data

	if(EngineSoundProfile::requestsPresetProfile(prop))
		engineSoundProfile = game.getPresetEngineSoundProfile(EngineSoundProfile::getSoundDefinitionFromProperties(prop));
	else
		engineSoundProfile = EngineSoundProfile::loadFromProperties(prop);

	// sprite data

	gfx = VehicleGraphics(prop);
}

/** Returns the current driving force. */
float Vehicle::getDriveForce()
{
	return engine.getDriveTorque() / tireRadius;
}

/** Updates the simulation state of this vehicle (RPM, gear, etc). */
void Vehicle::update(float delta)
{
	float wheelAngularSpeed = speed/tireRadius;  // fixme implement a better way to compute wheel angular speed as this formula assumes no wheel spin.
	engine.update(wheelAngularSpeed);
}
