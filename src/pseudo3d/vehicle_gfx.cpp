/*
 * vehicle_gfx.cpp
 *
 *  Created on: 7 de ago de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle_gfx.hpp"

#include "futil/string_actions.hpp"
#include "futil/round.h"

#include <cstdlib>

#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

using futil::Properties;
using std::string;

// default uint constants
static const unsigned
	DEFAULT_SPRITE_WIDTH = 56,
	DEFAULT_SPRITE_HEIGHT = 36;

// default float constants
static const float
	DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE = 45, // 45 degrees, pi/4 radians
	DEFAULT_SPRITE_DEPICTED_VEHICLE_WIDTH_PROPORTION = 0.857142857143;  // ~0,857

VehicleGraphics::VehicleGraphics() {}   // @suppress("Class members should be properly initialized")

/** Creates a vehicle graphics profile from the given properties data. */
VehicleGraphics::VehicleGraphics(const Properties& prop)
{
	// aux. var
	string key;

	key = "sprite_sheet_file";
	sheetFilename = prop.containsKey(key)? prop.get(key) : "assets/car.png";

	key = "sprite_state_count";
	spriteStateCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 1;

	key = "sprite_frame_width";
	spriteWidth = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_SPRITE_WIDTH;

	key = "sprite_frame_height";
	spriteHeight = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_SPRITE_HEIGHT;

	key = "sprite_vehicle_width";
	spriteDepictedVehicleWidth = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : futil::round(spriteWidth*DEFAULT_SPRITE_DEPICTED_VEHICLE_WIDTH_PROPORTION);

	key = "sprite_scale";
	spriteScale.x = spriteScale.y = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : 1.0;

	key = "sprite_scale_y";
	spriteScale.y = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : spriteScale.y;

	key = "sprite_scale_x";
	spriteScale.x = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : spriteScale.x;

	key = "sprite_contact_offset";
	spriteContatctOffset = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 0;

	key = "sprite_max_depicted_turn_angle";
	const float absoluteTurnAngle = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE;
	spriteMaxDepictedTurnAngle = absoluteTurnAngle/DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE;

	key = "sprite_frame_duration";
	spriteFrameDuration = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : -1.0;

	for(unsigned stateNumber = 0; stateNumber < spriteStateCount; stateNumber++)
	{
		key = "sprite_state" + futil::to_string(stateNumber) + "_frame_count";
		const unsigned stateFrameCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 1;
		spriteStateFrameCount.push_back(stateFrameCount);
	}
}
