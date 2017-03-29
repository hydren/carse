/*
 * vehicle.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_HPP_
#define PSEUDO3D_VEHICLE_HPP_
#include <ciso646>

#include "motor.hpp"

struct Vehicle
{
	Engine engine;
	float mass;
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
