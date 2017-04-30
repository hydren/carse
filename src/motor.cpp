/*
 * motor.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "motor.hpp"

float Engine::getDriveForce()
{
	return (rpm < maxRpm? torque : 0) * gearRatio[gear] * gearRatio[0] * 0.765 * tireRadius * 5000.0;
}
