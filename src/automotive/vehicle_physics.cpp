/*
 * vehicle_physics.cpp
 *
 *  Created on: 27 de jul de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle_physics.hpp"

#include <cmath>

#ifndef M_PI
	# define M_PI		3.14159265358979323846	/* pi */
#endif

#define differential 0

void VehicleBody::update(float speed)
{
	const float wheelAngularSpeed = speed / tireRadius;  // fixme implement a better way to compute wheel angular speed as this formula assumes no wheel spin.

	engine.rpm = wheelAngularSpeed * gearRatio[gear] * gearRatio[differential] * (30.0/M_PI);  // 60/2pi conversion to RPM

	if(engine.rpm < engine.minRpm)
		engine.rpm = engine.minRpm;

	if(automaticShiftingEnabled)
	{
		if(gear < gearCount and engine.rpm > automaticShiftingUpperThreshold*engine.maxRpm)
			gear++;

		if(gear > 1 and engine.rpm < automaticShiftingLowerThreshold*engine.maxRpm)
			gear--;
	}
}

float VehicleBody::getDriveForce()
{
	// fixme should calculate this using longitudinal slip ratio instead
	return engine.getCurrentTorque() * gearRatio[gear] * gearRatio[differential] * transmissionEfficiency / tireRadius;
}
