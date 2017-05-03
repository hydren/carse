/*
 * motor.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef MOTOR_HPP_
#define MOTOR_HPP_
#include <ciso646>

struct Engine
{
	float torque;
	float rpm, maxRpm, minRpm;
	int gear, gearCount;
	float *gearRatio, reverseGearRatio; // fixme this leaks :)
	float tireRadius;

	bool automaticShiftingEnabled;
	float automaticShiftingLowerThreshold;
	float automaticShiftingUpperThreshold;

	/** Returns the current driving force. */
	float getDriveForce();

	/** Updates the engine's state (RPM, gear, etc), given the current speed. */
	void update(float currentSpeed);
};

#endif /* MOTOR_HPP_ */
