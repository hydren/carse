/*
 * vehicle.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_HPP_
#define PSEUDO3D_VEHICLE_HPP_
#include <ciso646>

#include "automotive/motor.hpp"
#include "automotive/engine_sound.hpp"
#include "pseudo3d/vehicle_gfx.hpp"

#include "futil/properties.hpp"

#include "carse_game.hpp"

#include "automotive/mechanics.hpp"

#include <map>
#include <vector>
#include <string>

struct Pseudo3DCarseGame;  // foward declaration

struct Vehicle
{
	// todo support more types of vehicles (jetskis, motorboats, hovercrafts, hovercars, trikes, etc)
	enum Type { TYPE_CAR, TYPE_BIKE, TYPE_OTHER } type;

	// general information
	std::string name, authors, credits, comments;

	// physics simulation
	Mechanics body;

	// sound data
	EngineSoundProfile engineSoundProfile;

	// graphics data
	Pseudo3DVehicleAnimationProfile sprite;
	int activeSkin;  // the active skin (-1 means no skin; original sheet)

	/** Creates a empty vehicle object. */
	Vehicle();

	/** Creates a vehicle with definitions taken from the given properties. */
	Vehicle(const futil::Properties& properties, Pseudo3DCarseGame& game);
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
