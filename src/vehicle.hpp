/*
 * vehicle.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_HPP_
#define PSEUDO3D_VEHICLE_HPP_
#include <ciso646>

#include "pseudo3d/vehicle_gfx.hpp"
#include "automotive/vehicle.hpp"
#include "futil/properties.hpp"

#include <string>
#include <vector>
#include <map>

struct Pseudo3DVehicle
{
	struct Spec extends VehicleSpec
	{
		// graphics data
		Pseudo3DVehicleAnimationProfile sprite;

		// additional alternate graphics data (optional)
		std::map<std::string, Pseudo3DVehicleAnimationProfile> alternateSprites;
	};

	Mechanics::VehicleType type;

	// general information
	std::string name, authors, credits, comments;

	// physics simulation
	Mechanics body;

	// sound data
	EngineSoundProfile engineSoundProfile;

	// graphics data
	Pseudo3DVehicleAnimationProfile sprite;

	Pseudo3DVehicle();  // zero constructor

	/** Creates a vehicle with the given specifications. The optional 'skin' argument specifies which skin to use (other than the default). */
	Pseudo3DVehicle(const Pseudo3DVehicle::Spec& spec, const std::string& skin="default");
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
