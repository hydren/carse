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
#include <cmath>

#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

using std::string;
using fgeal::Image;
using fgeal::Sprite;
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
	string key, key2, key3;

	key = "sprite_sheet_file";
	sheetFilename = isValueSpecified(prop, key)? prop.get(key) : "DEFAULT";

	// attempt to read up to 9 alternative sheets
	for(unsigned i = 2; i <= 9; i++)
	{
		key = "sprite_sheet" + futil::to_string(i) + "_file";
		if(isValueSpecified(prop, key))
			sheetFilenameExtra.push_back(prop.get(key));
	}

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

	bool keepAspectRatio = false;
	key = "sprite_keep_aspect_ratio";
	if(isValueSpecified(prop, key))
	{
		const string value = to_lower(trim(prop.get(key)));
		if(value == "true" or value == "yes")
			keepAspectRatio = true;
	}

	key = "vehicle_width";
	if(isValueSpecified(prop, key))  // if vehicle width is available, compute recommended scale factor
	{
		const float vehicleWidth = atoi(prop.get(key).c_str());  // the real-life vehicle width, in mm

		key = "sprite_vehicle_height"; key2 = "vehicle_height"; key3 = "vehicle_width_height_ratio";
		if(not keepAspectRatio and isValueSpecified(prop, key)  // if vehicle height (both real-life and in sprite) are available, adjust scale factor (if allowed)
		and (isValueSpecified(prop, key2) or isValueSpecified(prop, key3)))  // ratios can be obtained by specifing height or ratio itself
		{
			// adjust scale factor to account for width/height ratio discrepancies
			const float spriteVehicleHeight = atoi(prop.get(key).c_str()),  // the vehicle width on the sprite, in pixels
						spriteWHRatio = ((float) depictedVehicleWidth) / spriteVehicleHeight;  // sprite width/height (WH) ratio

			// the real-life vehicle height (if available), in mm.
			const float vehicleHeight = isValueSpecified(prop, key2)? atoi(prop.get(key2).c_str()) : -1;

			if(vehicleHeight == 0)
				throw std::invalid_argument("vehicle height is zero!");

			const float vehicleWHRatio = isValueSpecified(prop, key3)? atof(prop.get(key3).c_str())  // if real-life width/height ratio is available, prefer to use it
															   : (vehicleWidth / vehicleHeight);  // otherwise calculate it from the available real-life width and height

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

void Pseudo3DVehicleAnimation::setProfile(const Pseudo3DVehicleAnimationProfile& profile, float scaleFactor, int skin)
{
	this->profile = profile;

	// delete previous sprites
	clearSprites();

	Image* sheet = new Image(skin == -1? profile.sheetFilename : profile.sheetFilenameExtra[skin]);

	if(sheet->getWidth() < (int) profile.frameWidth)
		throw std::runtime_error("Invalid sprite width value. Value is smaller than sprite sheet width (no whole sprites could be draw)");

	for(unsigned i = 0; i < profile.stateCount; i++)
	{
		Sprite* sprite = new Sprite(sheet, profile.frameWidth, profile.frameHeight,
									profile.frameDuration, profile.stateFrameCount[i],
									0, i*profile.frameHeight);

		sprite->scale = profile.scale * scaleFactor;
		sprite->referencePixelY = - (int) profile.contactOffset;
		sprites.push_back(sprite);
	}

	if(profile.asymmetrical) for(unsigned i = 1; i < profile.stateCount; i++)
	{
		Sprite* sprite = new Sprite(sheet, profile.frameWidth, profile.frameHeight,
									profile.frameDuration, profile.stateFrameCount[i],
									0, (profile.stateCount-1 + i)*profile.frameHeight);

		sprite->scale = profile.scale * scaleFactor;
		sprite->referencePixelY = - (int) profile.contactOffset;
		sprites.push_back(sprite);
	}
}

void Pseudo3DVehicleAnimation::setFrameDuration(float duration)
{
	for(unsigned i = 0; i < sprites.size(); i++)
		sprites[i]->duration = duration;
}

static const float LEAN_RATIO_MAGNITUDE_THRESHOLD = 0.1f,
				   LEAN_RATIO_MAX_MAGNITUDE = 1.0f;

void Pseudo3DVehicleAnimation::draw(float cx, float cy, float leanRatio)
{
	// the ammount of pseudo angle that will trigger the last sprite
//	const float PSEUDO_ANGLE_LAST_STATE = LEAN_RATIO_MAX_MAGNITUDE;  // show last sprite when the lean angle is at its max
	const float LEAN_RATIO_FINAL_STATE = profile.maxDepictedTurnAngle;  // show last sprite when the lean angle is at the specified ammount in the .properties

	// linear sprite progression
//	const unsigned animationIndex = (profile.stateCount-1)*fabs(leanRatio)/LEAN_RATIO_FINAL_STATE;

	// exponential sprite progression. may be slower.
//	const unsigned animationIndex = (profile.stateCount-1)*(exp(fabs(leanRatio))-1)/(exp(LEAN_RATIO_FINAL_STATE)-1);

	// linear sprite progression with 1-index advance at threshold angle
	unsigned animationIndex = 0;
	if(profile.stateCount > 1 and fabs(leanRatio) > LEAN_RATIO_MAGNITUDE_THRESHOLD)
		animationIndex = 1 + (profile.stateCount-2)*(fabs(leanRatio) - LEAN_RATIO_MAGNITUDE_THRESHOLD)/(LEAN_RATIO_FINAL_STATE - LEAN_RATIO_MAGNITUDE_THRESHOLD);

	// cap index to max possible
	if(animationIndex > profile.stateCount - 1)
		animationIndex = profile.stateCount - 1;

	const bool isLeanRight = (leanRatio > 0 and animationIndex != 0);

	// if asymmetrical, right-leaning sprites are after all left-leaning ones
	if(isLeanRight and profile.asymmetrical)
		animationIndex += (profile.stateCount-1);

	Sprite& sprite = *sprites[animationIndex];
	sprite.flipmode = isLeanRight and not profile.asymmetrical? Image::FLIP_HORIZONTAL : Image::FLIP_NONE;
	sprite.computeCurrentFrame();

	sprite.draw(cx - 0.5*(sprite.scale.x*profile.frameWidth), cy - 0.5*(sprite.scale.y*profile.frameHeight) - sprite.scale.y*profile.contactOffset);
}

Pseudo3DVehicleAnimation::~Pseudo3DVehicleAnimation()
{
	clearSprites();
}

void Pseudo3DVehicleAnimation::clearSprites()
{
	if(not sprites.empty())
	{
		delete sprites[0]->image;

		for(unsigned i = 0; i < sprites.size(); i++)
			delete sprites[i];

		sprites.clear();
	}
}
