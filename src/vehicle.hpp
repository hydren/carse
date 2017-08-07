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

	// physics data and simulation
	float mass;
	float tireRadius;
	Engine engine;
	float speed;

	// sound data
	EngineSoundProfile engineSoundProfile;

	// graphics data
	VehicleGraphics gfx;

	/** Creates a empty vehicle object. */
	Vehicle();

	/** Creates a vehicle with definitions taken from the given properties. */
	Vehicle(const futil::Properties& properties, Pseudo3DCarseGame& game);

	/** Returns the current driving force. */
	float getDriveForce();

	/** Updates the simulation state of this vehicle (engine, speed, etc). */
	void update(float delta);
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
