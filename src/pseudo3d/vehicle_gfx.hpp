/*
 * vehicle_gfx.hpp
 *
 *  Created on: 7 de ago de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_GFX_HPP_
#define PSEUDO3D_VEHICLE_GFX_HPP_
#include <ciso646>

#include <map>
#include <vector>
#include <string>

#include "fgeal/fgeal.hpp"
#include "futil/properties.hpp"

/** A class containing data used to draw pseudo-3D vehicle animations. */
struct Pseudo3DVehicleAnimationProfile
{
	/** The filename of the image containing the sprite sheet. */
	std::string sheetFilename;

	/** The amount of states of this animation. */
	unsigned stateCount;

	/** The width of the sprite frame. */
	unsigned frameWidth;

	/** The width of the sprite frame. */
	unsigned frameHeight;

	/** The offset between the sprite's bottom and the depicted contact point of the vehicle (i.e the
	 *  distance between the car tires' bottom and the sprite's bottom). */
	unsigned contactOffset;

	/** The scaling factor of this animation. Applies to all frames. */
	fgeal::Vector2D scale;

	/** The time duration of each frame. Applies to all frames. */
	float frameDuration;

	/** A vector containing the amount of frames of each state. Each index corresponds to each state. */
	std::vector<unsigned> stateFrameCount;

	/** If true, the sprite is not horizontally symmetrical and, therefore, includes right-leaning
	 *  versions of each state. */
	bool asymmetrical;

	/** The maximum turning angle depicted on the sprite. This is used to adjust how quickly the
	 *  animation will switch states depending on the vehicle's pseudo angle. */
	float maxDepictedTurnAngle;

	/** The width of the vehicle as depicted in the sprite (in pixels). This is used to align animation
	 *  effects, such as burning rubber's smoking animation, etc. */
	unsigned depictedVehicleWidth;

	/** Empty constructor */
	Pseudo3DVehicleAnimationProfile();

	/** Creates a vehicle graphics profile from the given properties data. */
	Pseudo3DVehicleAnimationProfile(const futil::Properties& properties);
};

#endif /* PSEUDO3D_VEHICLE_GFX_HPP_ */
