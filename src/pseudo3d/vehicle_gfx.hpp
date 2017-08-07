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

struct VehicleGraphics
{
	std::string sheetFilename;
	unsigned spriteStateCount, spriteWidth, spriteHeight, spriteContatctOffset;
	fgeal::Vector2D spriteScale;
	float spriteFrameDuration;
	std::vector<unsigned> spriteStateFrameCount;
	float spriteMaxDepictedTurnAngle;
	unsigned spriteDepictedVehicleWidth;

	/** Empty constructor */
	VehicleGraphics();

	/** Creates a vehicle graphics profile from the given properties data. */
	VehicleGraphics(const futil::Properties& properties);
};

#endif /* PSEUDO3D_VEHICLE_GFX_HPP_ */
