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
using std::vector;

#define PARAM_RPM 0
#define PARAM_SLOPE 1
#define PARAM_INTERCEPT 2

#ifndef M_PI
	# define M_PI		3.14159265358979323846	/* pi */
#endif

const float Engine::TorqueCurveProfile::TORQUE_CURVE_INITIAL_VALUE = 0.55f,
			Engine::TorqueCurveProfile::TORQUE_CURVE_FINAL_VALUE =   1.f/3.f;  // 0.33333... (one third)

static const float ENGINE_FRICTION_COEFFICIENT = 0.2 * 30.0,
					RAD_TO_RPM = (30.0/M_PI);  // 60/2pi conversion to RPM

// default float constants
static const float
	DEFAULT_MAXIMUM_RPM = 7000,
	DEFAULT_MAXIMUM_POWER = 320,  // bhp
	DEFAULT_GEAR_COUNT = 5,
	DEFAULT_MAX_TORQUE_RPM_POSITION = 2.f/3.f,  // 0.66666... (two thirds)

	// for the time being, assume 70% efficiency
	DEFAULT_TRANSMISSION_EFFICIENCY = 0.7;

#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

/**
 *  Create a linear "curve".
 * */
static vector<float> createLinearCurve(float rpm_i, float torque_i, float rpm_f, float torque_f)
{
	vector<float> param(3);
	param[0] = rpm_f;  // RPM range threshold or right bound.
	param[1] = (torque_f - torque_i)/(rpm_f - rpm_i);  // line slope
	param[2] = (rpm_f * torque_i - rpm_i * torque_f)/(rpm_f - rpm_i);  // line intercept
	return param;
}

Engine::TorqueCurveProfile Engine::TorqueCurveProfile::create(float maxRpm, float rpmMaxTorque)
{
	const float torque_i = TORQUE_CURVE_INITIAL_VALUE,
				torque_f = TORQUE_CURVE_FINAL_VALUE;

	TorqueCurveProfile profile;
	profile.parameters.push_back(createLinearCurve(           1,        0,         1000, torque_i));  // hardcoded linear warmup torque curve in 1-1000rpm range
	profile.parameters.push_back(createLinearCurve(        1000, torque_i, rpmMaxTorque,      1.0));  // curve that leads to the highest torque point (from 1001rpm to rpmMaxTorque)
	profile.parameters.push_back(createLinearCurve(rpmMaxTorque,      1.0,       maxRpm, torque_f));  // curve with decrease in torque as it approuches redline (from rpmMaxTorque to maxRpm)

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
	gearRatio.resize(gearCount);

	// first, set default ratios, then override
	reverseGearRatio = 3.25;
	differentialRatio = 4.0;
	for(int g = 0; g < gearCount; g++)
		gearRatio[g] = 3.0 + g*2.0/(1.0 - gearCount);  // generic gear ratio

	key = "gear_ratios";
	if(prop.containsKey(key))
	{
		string ratiosTxt = prop.get(key);
		if(ratiosTxt == "custom")
		{
			key = "gear_differential_ratio";
			if(prop.containsKey(key))
				differentialRatio = atof(prop.get(key).c_str());

			key = "gear_reverse_ratio";
			if(prop.containsKey(key))
				reverseGearRatio = atof(prop.get(key).c_str());

			for(int g = 0; g < gearCount; g++)
			{
				key = "gear_" + futil::to_string(g+1) + "_ratio";
				if(prop.containsKey(key))
					gearRatio[g] = atof(prop.get(key).c_str());
			}
		}
	}

	throttlePosition = 0;
	rpm = 0;
	minRpm = 1000;
	gear = 1;
}

float Engine::getCurrentTorque()
{
	const vector< vector<float> >& torqueCurve = torqueCurveProfile.parameters;
	return throttlePosition * maximumTorque * ( rpm > maxRpm ? -rpm/maxRpm :
					  rpm < torqueCurve[0][PARAM_RPM] ? torqueCurve[0][PARAM_SLOPE]*rpm + torqueCurve[0][PARAM_INTERCEPT] :
					  rpm < torqueCurve[1][PARAM_RPM] ? torqueCurve[1][PARAM_SLOPE]*rpm + torqueCurve[1][PARAM_INTERCEPT] :
							  	  	  	  	  	  	    torqueCurve[2][PARAM_SLOPE]*rpm + torqueCurve[2][PARAM_INTERCEPT] );
}

float Engine::getDriveTorque()
{
	return gear==0? 0 : this->getCurrentTorque() * gearRatio[gear-1] * differentialRatio * transmissionEfficiency;
}

float Engine::getAngularSpeed()
{
	return rpm / ((gear==0? 1.0 : gearRatio[gear-1]) * differentialRatio * RAD_TO_RPM);
}

void Engine::update(float delta, float wheelAngularSpeed)
{
	if(gear == 0)
	{
		const float rpmRatio = rpm/maxRpm;
		rpm += (getCurrentTorque()*RAD_TO_RPM - (1-throttlePosition)*rpmRatio*rpmRatio*maximumTorque*ENGINE_FRICTION_COEFFICIENT)*delta;
	}
	else
	{
		//synchronize both rotations  (acts like a synchromesh)
		const float rpmDiff = (wheelAngularSpeed * gearRatio[gear-1] * differentialRatio * RAD_TO_RPM) - rpm, synchonizationFactor = 50.0;
		rpm += delta * synchonizationFactor * rpmDiff;
		// fixme the wheel angular speed should change as well when syncing, but previous attempts got strange behavior
	}

	if(rpm < minRpm)
		rpm = minRpm;

	if(rpm > maxRpm+100)
		rpm = maxRpm+100;
}
