/*
 * motor.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "motor.hpp"

#include <cmath>

#define differential 0

#ifndef M_PI
	# define M_PI		3.14159265358979323846	/* pi */
#endif

float Engine::getTorque(float rpm)
{
	return (rpm < maxRpm? torque : 0);
}

float Engine::getDriveForce()
{
	return this->getTorque(rpm) * gearRatio[gear] * gearRatio[differential] * 0.765 * tireRadius * 5000.0;
}

void Engine::update(float speed)
{
	rpm = (speed/tireRadius) * gearRatio[gear] * gearRatio[differential] * (30.0f/M_PI) * 0.002;

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
