/*
 * motor.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "motor.hpp"

#include <cmath>
#include <cstdlib>

#include "futil/string_actions.hpp"

using std::string;

#define differential 0

#define PARAM_RPM 0
#define PARAM_SLOPE 1
#define PARAM_INTERCEPT 2

#ifndef M_PI
	# define M_PI		3.14159265358979323846	/* pi */
#endif

#define LINEAR_SLOPE(x1, y1, x2, y2) ((y2 - y1)/(x2 - x1))
#define LINEAR_INTERCEPT(x1, y1, x2, y2) ((x1 * y2 - x2 * y1)/(x1 - x2))
#define LINEAR_PARAMETERS(x1, y1, x2, y2) { x2, LINEAR_SLOPE(x1, y1, x2, y2), LINEAR_INTERCEPT(x1, y1, x2, y2) }

const float Engine::TorqueCurveProfile::TORQUE_CURVE_INITIAL_VALUE = 0.55f,
			Engine::TorqueCurveProfile::TORQUE_CURVE_FINAL_VALUE =   1.f/3.f;  // 0.33333... (one third)

// default float constants
static const float
	DEFAULT_MAXIMUM_RPM = 7000,
	DEFAULT_MAXIMUM_POWER = 320,  // bhp
	DEFAULT_GEAR_COUNT = 5,
	DEFAULT_MAX_TORQUE_RPM_POSITION = 2.f/3.f,  // 0.66666... (two thirds)

	// for the time being, assume 70% efficiency
	DEFAULT_TRANSMISSION_EFFICIENCY = 0.7;

#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

Engine::TorqueCurveProfile Engine::TorqueCurveProfile::create(float maxRpm, float rpmMaxTorque)
{
	const float torque_i = TORQUE_CURVE_INITIAL_VALUE,
				torque_f = TORQUE_CURVE_FINAL_VALUE;

	TorqueCurveProfile profile = {{
		LINEAR_PARAMETERS(1.0f, 0.0f, 1000.f, torque_i),  // hardcoded linear warmup torque curve in 0-1000rpm range
		LINEAR_PARAMETERS(1000.f, torque_i, rpmMaxTorque, 1.0f),  // curve that leads to the highest torque point
		LINEAR_PARAMETERS(rpmMaxTorque, 1.0f, maxRpm, torque_f)  // curve with decrease in torque as it approuches redline
	}};

	return profile;
}

Engine::Engine() {}   // @suppress("Class members should be properly initialized")

Engine::Engine(const futil::Properties& prop)
{
	// aux. var
	string key;

	// info data

	key = "engine_configuration";
	configuration = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_aspiration";
	aspiration = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_valvetrain";
	valvetrain = prop.containsKey(key)? prop.get(key) : "";

	key = "engine_displacement";
	displacement = prop.containsKey(key)? atoi(prop.get(key).c_str()) : 0;

	key = "engine_valve_count";
	valveCount = prop.containsKey(key)? atoi(prop.get(key).c_str()) : 0;

	// actually useful data

	key = "engine_maximum_rpm";
	maxRpm = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_MAXIMUM_RPM;

	key = "engine_maximum_power";
	const float maxPower = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_MAXIMUM_POWER;

	// xxx generic hardcoded torque rpm position
	const float maxTorqueRpm = (1000.0 + maxRpm)*DEFAULT_MAX_TORQUE_RPM_POSITION;

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
//			maxTorqueRpm = 2.f*maxPowerRpm*(2.f - 2.f/K) + maxRpm/K;
//		}
//		else maxTorqueRpm = (maxRpm + 1000.f)*DEFAULT_MAX_TORQUE_RPM_POSITION;  // generic torque rpm position
//	}

	// if no rpm of max power is specified, estimate one
	if(maxPowerRpm < 0)
		// estimate max power rpm
		maxPowerRpm = (maxTorqueRpm + maxRpm)*0.5;

//	key = "engine_maximum_power";
//	torque = (isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_MAXIMUM_POWER)*POWER_TORQUE_FACTOR;
	maximumTorque = conversionFactor * maxPower * (maxRpm - maxTorqueRpm) / (maxPowerRpm*((K - 1.0)*maxPowerRpm + maxRpm - maxTorqueRpm*K));

	torqueCurveProfile = Engine::TorqueCurveProfile::create(maxRpm, maxTorqueRpm);

	// informative-only fields
	maximumPower = maxPower;
	maximumPowerRpm = maxPowerRpm;
	maximumTorqueRpm = maxTorqueRpm;

	transmissionEfficiency = DEFAULT_TRANSMISSION_EFFICIENCY;

	key = "gear_count";
	gearCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_GEAR_COUNT;
	gearRatio.resize(gearCount+1);

	// first, set default ratios, then override
	reverseGearRatio = 3.25;
	gearRatio[0] = 3.0;
	for(int g = 1; g <= gearCount; g++)
		gearRatio[g] = 3.0 + 2.0*((g - 1.0)/(1.0 - gearCount)); // generic gear ratio

	key = "gear_ratios";
	if(prop.containsKey(key))
	{
		string ratiosTxt = prop.get(key);
		if(ratiosTxt == "custom")
		{
			key = "gear_differential_ratio";
			if(prop.containsKey(key)) gearRatio[0] = atof(prop.get(key).c_str());

			key = "gear_reverse_ratio";
			if(prop.containsKey(key)) reverseGearRatio = atof(prop.get(key).c_str());

			for(int g = 1; g <= gearCount; g++)
			{
				key = "gear_" + futil::to_string(g) + "_ratio";
				if(prop.containsKey(key))
					gearRatio[g] = atof(prop.get(key).c_str());
			}
		}
	}

	throttlePosition = 0;
	rpm = 0;
	minRpm = 1000;
	gear = 1;
	automaticShiftingEnabled = true;
	automaticShiftingLowerThreshold = 0.4 * maxRpm;
	automaticShiftingUpperThreshold = 0.8 * maxRpm;
}

float Engine::getCurrentTorque()
{
	#define torqueCurve torqueCurveProfile.parameters
	return throttlePosition * maximumTorque * ( rpm > maxRpm ? -rpm/maxRpm :
					  rpm < torqueCurve[0][PARAM_RPM] ? torqueCurve[0][PARAM_SLOPE]*rpm + torqueCurve[0][PARAM_INTERCEPT] :
					  rpm < torqueCurve[1][PARAM_RPM] ? torqueCurve[1][PARAM_SLOPE]*rpm + torqueCurve[1][PARAM_INTERCEPT] :
							  	  	  	  	  	  	    torqueCurve[2][PARAM_SLOPE]*rpm + torqueCurve[2][PARAM_INTERCEPT] );
	#undef torqueCurve
}

float Engine::getDriveTorque()
{
	return this->getCurrentTorque() * gearRatio[gear] * gearRatio[differential] * transmissionEfficiency;
}

float Engine::getAngularSpeed()
{
	return rpm / (gearRatio[gear] * gearRatio[differential] * (30.0/M_PI));  // 60/2pi conversion to RPM
}

void Engine::update(float wheelAngularSpeed)
{
	rpm = wheelAngularSpeed * gearRatio[gear] * gearRatio[differential] * (30.0/M_PI);  // 60/2pi conversion to RPM

	if(rpm < minRpm)
		rpm = minRpm;

	if(automaticShiftingEnabled)
	{
		if(gear < gearCount and rpm > automaticShiftingUpperThreshold*maxRpm)
			gear++;

		if(gear > 1 and rpm < automaticShiftingLowerThreshold*maxRpm)
			gear--;
	}
}
