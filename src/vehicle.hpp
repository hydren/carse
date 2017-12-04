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

struct Pseudo3DVehicle
{
	struct Spec extends VehicleSpec
	{
		// graphics data
		Pseudo3DVehicleAnimationSpec sprite;

		// additional alternate graphics data (optional)
		std::vector<Pseudo3DVehicleAnimationSpec> alternateSprites;
	};

	const Spec* spec;

	// physics simulation
	Mechanics body;

	// sound data
	EngineSoundProfile engineSoundProfile;

	// graphics data
	Pseudo3DVehicleAnimationSpec sprite;

	Pseudo3DVehicle();  // zero constructor

	/** Creates a vehicle with the given specifications. The optional 'alternateSpriteIndex' argument specifies an alternate skin to use (-1 means use default sprite). */
	Pseudo3DVehicle(const Pseudo3DVehicle::Spec& spec, int alternateSpriteIndex=-1);
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
