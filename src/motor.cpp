/*
 * motor.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "motor.hpp"

float Engine::getDriveForce()
{
	return this->getTorque() * gearRatio[gear] * gearRatio[0] * 0.765 * wheelRadius * 5000.0;
}

float Engine::getTorque()
{
	return torque*
		   (rpm < 3000? rpm/3000.0
		  : rpm < 6750? 1725.0/1125.0 - rpm/5625.0
		              : (7000.0-rpm)/750.0 );
}
