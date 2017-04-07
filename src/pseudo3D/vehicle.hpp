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

#include "util/properties.hpp"

#include <map>
#include <string>

struct Vehicle
{
	std::string name, sheetFilename;

	std::map<short, std::string> soundsFilenames;
	bool isLastSoundRedline;

	Engine engine;
	float mass;

	// creates a empty vehicle object
	Vehicle();

	// creates a vehicle with definitions taken from the given properties
	Vehicle(const util::Properties& properties);
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
