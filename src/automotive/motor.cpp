/*
 * motor.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "motor.hpp"

#include <cmath>

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

static const float TORQUE_CURVE_INITIAL_RPM =    1000,
				   TORQUE_CURVE_INITIAL_VALUE = 0.55f,
				   TORQUE_CURVE_FINAL_VALUE =   1.f/3.f,  // 0.33333... (one third)
				   TORQUE_CURVE_DEFAULT_MAX_TORQUE_RPM_POSITION = 2.f/3.f;  // 0.66666... (two thirds)

Engine::TorqueCurveProfile Engine::TorqueCurveProfile::create(float maxRpm, float rpmMaxTorque)
{
	if(rpmMaxTorque < 0)  // if not rpm of max torque is specified, generate one
		rpmMaxTorque = (maxRpm + TORQUE_CURVE_INITIAL_RPM)*TORQUE_CURVE_DEFAULT_MAX_TORQUE_RPM_POSITION;

	const float rpm_i = TORQUE_CURVE_INITIAL_RPM,
				torque_i = TORQUE_CURVE_INITIAL_VALUE,
				torque_f = TORQUE_CURVE_FINAL_VALUE;

	TorqueCurveProfile profile = {{
		LINEAR_PARAMETERS(1.0f, 0.0f, rpm_i, torque_i),  // hardcoded linear warmup torque curve in 0-1000rpm range
		LINEAR_PARAMETERS(rpm_i, torque_i, rpmMaxTorque, 1.0f),  // curve that leads to the highest torque point
		LINEAR_PARAMETERS(rpmMaxTorque, 1.0f, maxRpm, torque_f)  // curve with decrease in torque as it approuches redline
	}};

	return profile;
}

float Engine::getTorque(float rpm)
{
	#define torqueCurve torqueCurveProfile.parameters
	return torque * ( rpm > maxRpm ? -rpm/maxRpm :
					  rpm < torqueCurve[0][PARAM_RPM] ? torqueCurve[0][PARAM_SLOPE]*rpm + torqueCurve[0][PARAM_INTERCEPT] :
					  rpm < torqueCurve[1][PARAM_RPM] ? torqueCurve[1][PARAM_SLOPE]*rpm + torqueCurve[1][PARAM_INTERCEPT] :
							  	  	  	  	  	  	    torqueCurve[2][PARAM_SLOPE]*rpm + torqueCurve[2][PARAM_INTERCEPT] );
	#undef torqueCurve
}

float Engine::getDriveForce()
{
	return this->getTorque(rpm) * gearRatio[gear] * gearRatio[differential] * transmissionEfficiency / tireRadius;
//	return this->getTorque(rpm) * gearRatio[gear] * gearRatio[differential] * 0.765 * tireRadius * 5000.0;
}

void Engine::update(float speed)
{
	const float wheelAngularSpeed = speed / tireRadius;  // fixme implement a better way to compute wheel angular speed as this formula assumes no wheel spin.

	rpm = wheelAngularSpeed * gearRatio[gear] * gearRatio[differential] * (30.0/M_PI);  // 60/2pi conversion to RPM
//	rpm = (speed/tireRadius) * gearRatio[gear] * gearRatio[differential] * (30.0f/M_PI) * 0.002;

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
