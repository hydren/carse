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
#include "automotive/engine_sound.hpp"

#include "util/properties.hpp"

#include "carse_game.hpp"

#include <map>
#include <vector>
#include <string>

struct Vehicle
{
	std::string name, sheetFilename;

	unsigned spriteStateCount, spriteWidth, spriteHeight;
	float spriteFrameDuration;
	std::vector<unsigned> spriteStateFrameCount;
	float spriteScale;

	EngineSoundProfile engineSoundProfile;

	Engine engine;
	float mass;

	// creates a empty vehicle object
	Vehicle();

	// creates a vehicle with definitions taken from the given properties
	Vehicle(const util::Properties& properties, CarseGame& game);
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
