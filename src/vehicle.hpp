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
#include "fgeal/extra/sprite.hpp"

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

	// physics simulation
	Mechanics body;

	// logic data
	float position, horizontalPosition, verticalPosition;
	float pseudoAngle, strafeSpeed, curvePull, corneringForceLeechFactor, corneringStiffness;
//	float verticalSpeed;
//	bool onAir, onLongAir;
	bool isBurningRubber;

	// sound data
	EngineSoundProfile engineSoundProfile;
	EngineSoundSimulator engineSound;

	// graphics data
	Pseudo3DVehicleAnimationSpec spriteSpec;
	std::vector<fgeal::Sprite*> sprites;

	Pseudo3DVehicle();  // zero constructor

	/** Creates a vehicle with the given specifications. The optional 'alternateSpriteIndex' argument specifies an alternate skin to use (-1 means use default sprite). */
	Pseudo3DVehicle(const Pseudo3DVehicle::Spec& spec, int alternateSpriteIndex=-1);

	~Pseudo3DVehicle();

	/** Disposes of dynamically loaded graphics and sounds. */
	void clearDynamicData();

	/** Loads dynamic graphics and sounds. */
	void setupDynamicData();
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
