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
#include <stdexcept>

#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

using std::string;
using futil::Properties;
using futil::to_lower;
using futil::trim;

// default uint constants
static const unsigned
	DEFAULT_SPRITE_WIDTH = 56,
	DEFAULT_SPRITE_HEIGHT = 36;

// default float constants
static const float
	DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE = 45, // 45 degrees, pi/4 radians
	DEFAULT_SPRITE_DEPICTED_VEHICLE_WIDTH_PROPORTION = 0.857142857143;  // ~0,857

Pseudo3DVehicleAnimationProfile::Pseudo3DVehicleAnimationProfile() {}   // @suppress("Class members should be properly initialized")

/** Creates a vehicle graphics profile from the given properties data. */
Pseudo3DVehicleAnimationProfile::Pseudo3DVehicleAnimationProfile(const Properties& prop)
{
	// aux. vars
	string key, key2;

	key = "sprite_sheet_file";
	sheetFilename = prop.containsKey(key)? prop.get(key) : "assets/car.png";

	key = "sprite_state_count";
	stateCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 1;

	key = "sprite_frame_width";
	frameWidth = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_SPRITE_WIDTH;

	key = "sprite_frame_height";
	frameHeight = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : DEFAULT_SPRITE_HEIGHT;

	key = "sprite_vehicle_width";
	depictedVehicleWidth = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : futil::round(frameWidth*DEFAULT_SPRITE_DEPICTED_VEHICLE_WIDTH_PROPORTION);

	// default scale
	scale.x = scale.y = 1.0;

	key = "vehicle_width";
	if(isValueSpecified(prop, key))  // if vehicle width is available, compute recommended scale factor
	{
		const float vehicleWidth = atoi(prop.get(key).c_str());  // the real-life vehicle width, in mm

		key = "sprite_vehicle_height"; key2 = "vehicle_height";
		if(isValueSpecified(prop, key) and isValueSpecified(prop, key2))  // if vehicle height (both real-life and in sprite) are available, adjust scale factor
		{
			// adjust scale factor to account for width/height ratio discrepancies
			const float spriteVehicleHeight = atoi(prop.get(key).c_str()),  // the vehicle width on the sprite, in pixels
						spriteWHRatio = ((float) depictedVehicleWidth) / spriteVehicleHeight;  // sprite width/height (WH) ratio

			// the real-life vehicle height (if available), in mm.
			const float vehicleHeight = isValueSpecified(prop, key2)? atoi(prop.get(key2).c_str()) : -1;

			if(vehicleHeight == 0)
				throw std::invalid_argument("vehicle height is zero!");

			const float vehicleWHRatio = (vehicleWidth / vehicleHeight);  // calculate it from the available real-life width and height

			const float ratioFixFactor = vehicleWHRatio / spriteWHRatio,  // multiplier that makes the sprite width/height ratio match the real-life width/height ratio
						fixedDepictedVehicleWidth = depictedVehicleWidth * ratioFixFactor;  // corrected in-sprite width

			// recommended scale factors, making sprite width/height ratio match the real-life width/height ratio
			scale.y = (vehicleWidth / fixedDepictedVehicleWidth) * (24.0/895.0);
			scale.x = scale.y * ratioFixFactor;
		}
		else  // no data about vehicle height or width/height ratio given; assume no no width/height ratio discrepancies between real-life
		{
			scale.x = scale.y = (vehicleWidth /(float) depictedVehicleWidth) * (24.0/895.0);  // recommended scale factor assuming no width/height ratio discrepancies
		}
	}

	key = "sprite_scale";
	if(isValueSpecified(prop, key))  // if scale factor is available, override previous definitions
	{
		scale.x = scale.y = atof(prop.get(key).c_str());
	}

	key = "sprite_scale_x";
	scale.x = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : scale.x;  // if x-scale factor is available, override previous definition

	key = "sprite_scale_y";
	scale.y = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : scale.y;  // if y-scale factor is available, override previous definition

	key = "sprite_contact_offset";
	contactOffset = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 0;

	asymmetrical = false;
	key = "sprite_asymmetric";
	if(isValueSpecified(prop, key))
	{
		const string value = to_lower(trim(prop.get(key)));
		if(value == "true" or value == "yes")
			asymmetrical = true;
	}

	key = "sprite_max_depicted_turn_angle";
	const float absoluteTurnAngle = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE;
	maxDepictedTurnAngle = absoluteTurnAngle/DEFAULT_SPRITE_MAX_DEPICTED_TURN_ANGLE;

	key = "sprite_frame_duration";
	frameDuration = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : -1.0;

	for(unsigned stateNumber = 0; stateNumber < stateCount; stateNumber++)
	{
		key = "sprite_state" + futil::to_string(stateNumber) + "_frame_count";
		const unsigned frameCount = isValueSpecified(prop, key)? atoi(prop.get(key).c_str()) : 1;
		stateFrameCount.push_back(frameCount);
	}
}
