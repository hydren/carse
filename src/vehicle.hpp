/*
 * vehicle.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_HPP_
#define PSEUDO3D_VEHICLE_HPP_
#include <ciso646>

#include "automotive/vehicle_physics.hpp"
#include "automotive/engine_sound.hpp"

#include "futil/properties.hpp"

#include "carse_game.hpp"

#include <map>
#include <vector>
#include <string>

struct Pseudo3DCarseGame;  // foward declaration

struct Vehicle
{
	// todo support more types of vehicles (jetskis, motorboats, hovercrafts, hovercars, trikes, etc)
	enum Type { TYPE_CAR, TYPE_BIKE, TYPE_OTHER } type;

	std::string name, authors, credits, comments;

	std::string sheetFilename;
	unsigned spriteStateCount, spriteWidth, spriteHeight, offset;
	float spriteFrameDuration;
	std::vector<unsigned> spriteStateFrameCount;
	fgeal::Vector2D spriteScale;
	float spriteMaxDepictedTurnAngle;

	VehicleBody body;

	EngineSoundProfile engineSoundProfile;

	// creates a empty vehicle object
	Vehicle();

	// creates a vehicle with definitions taken from the given properties
	Vehicle(const futil::Properties& properties, Pseudo3DCarseGame& game);
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
