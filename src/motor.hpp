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
	float rpm, maxRpm;
	int gear, gearCount;
	float *gearRatio, reverseGearRatio; // fixme this leaks :)
	float wheelRadius;

	float getDriveForce();
};

#endif /* MOTOR_HPP_ */
