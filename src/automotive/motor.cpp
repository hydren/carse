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

const float Engine::TorqueCurveProfile::TORQUE_CURVE_INITIAL_VALUE = 0.55f,
			Engine::TorqueCurveProfile::TORQUE_CURVE_FINAL_VALUE =   1.f/3.f;  // 0.33333... (one third)

//xxx An estimated wheel (tire+rim) density. (33cm radius or 660mm diameter tire with 75kg mass). Actual value varies by tire (brand, weight, type, etc) and rim (brand , weight, shape, material, etc)
const float AVERAGE_WHEEL_DENSITY = 75/pow(330, 2);  // d = m/r^2, assuming wheel width = 1/PI in the original formula d = m/(PI * r^2 * width)

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

float Engine::getCurrentTorque()
{
	#define torqueCurve torqueCurveProfile.parameters
	return throttlePosition * maximumTorque *
			( rpm > maxRpm ? -rpm/maxRpm :
			  rpm < torqueCurve[0][PARAM_RPM] ? torqueCurve[0][PARAM_SLOPE]*rpm + torqueCurve[0][PARAM_INTERCEPT] :
			  rpm < torqueCurve[1][PARAM_RPM] ? torqueCurve[1][PARAM_SLOPE]*rpm + torqueCurve[1][PARAM_INTERCEPT] :
												torqueCurve[2][PARAM_SLOPE]*rpm + torqueCurve[2][PARAM_INTERCEPT] );
	#undef torqueCurve
}

float Engine::getDriveForce()
{
	return this->getCurrentTorque() * gearRatio[gear] * gearRatio[differential] * transmissionEfficiency / tireRadius;
//	return this->getCurrentTorque() * gearRatio[gear] * gearRatio[differential] * 0.765 * tireRadius * 5000.0;
}

void Engine::update(float delta, float currentSpeed)
{
	const float rpmWheelAngularSpeedRatio = gearRatio[gear] * gearRatio[differential] * (30.0/M_PI);  // 60/2pi conversion to RPM

//	const float wheelAngularSpeed = speed / tireRadius;  // this formula assumes no wheel spin.
	float wheelAngularSpeed = rpm / rpmWheelAngularSpeedRatio;

	const float driveTorque = this->getCurrentTorque() * gearRatio[gear] * gearRatio[differential] * transmissionEfficiency;

	const float slipRatio = (wheelAngularSpeed*tireRadius - currentSpeed)/fabs(currentSpeed);
	const float tractionForce = 30*slipRatio; // fixme how to compute a reasonable curve? we know weight load can be computed from weight distribuition.
	const float tractionTorque = tractionForce / tireRadius;

	const float brakingTorque = brakePosition*30; //fixme how to do this formula right? remove from ingame state braking calculation

	const unsigned drivenWheelsCount = 2;  // fixme driven wheels count should be retrieved by vehicle specification.

	const float totalTorque = driveTorque + tractionTorque*drivenWheelsCount + brakingTorque;

	const float wheelMass = AVERAGE_WHEEL_DENSITY * tireRadius*tireRadius;  // m = d*r^2, assuming wheel width = 1/PI
	const float drivenWheelsInertia = drivenWheelsCount * wheelMass * tireRadius*tireRadius * 0.5;  // I = (mr^2)/2

	wheelAngularSpeed += (totalTorque / drivenWheelsInertia)*delta;  // xxx we're assuming no inertia from the engine components.

	rpm = wheelAngularSpeed * rpmWheelAngularSpeedRatio;

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
