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
	float *gearRatio, reverseGearRatio;
	float wheelRadius;

	float getDriveForce();
	float getTorque();
};

#endif /* MOTOR_HPP_ */
