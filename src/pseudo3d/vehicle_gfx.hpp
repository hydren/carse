/*
 * vehicle_gfx.hpp
 *
 *  Created on: 7 de ago de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_GFX_HPP_
#define PSEUDO3D_VEHICLE_GFX_HPP_
#include <ciso646>

#include <string>
#include <vector>

#include "fgeal/fgeal.hpp"

/** A class containing data used to draw pseudo-3D vehicle animations. */
struct Pseudo3DVehicleAnimationSpec
{
	// Main vehicle sprites

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

	// Brakelights sprites

	/** The filename of the image containing the sprite for the optional brakelights overlay.
	 *  Note that the actual brakelight image portraited in the sheet should be in the same scale as
	 *  the vehicle sprite sheet.*/
	std::string brakelightsSheetFilename;

	/** The scaling factor of the brakelights overlay. Applies to all frames. */
	fgeal::Vector2D brakelightsSpriteScale;

	/** The positions of the brakelights within the animation's coordinates.
	 *  Note that this is a vector because each of the vehicle animation's states may have the
	 *  brakelights located at different coordinates. */
	std::vector<fgeal::Point> brakelightsPositions;

	/** An optional offset applied to the brakelights sprites' positions, meant to center them when
	 *  necessary, i.e. mirrowed sprites with surrounding alpha regions.  */
	fgeal::Vector2D brakelightsOffset;

	/** If true (default), a mirrowed version of the brakeligthts animation is drawn as well.
	 *  The position of the mirrowed brakelights is a mirrowed version of the 'brakelightsPositions',
	 *  minus the animation width. */
	bool brakelightsMirrowed;

	/** If false, it's assumed that there only a single brakelight sprite, and it will be used
	 *  for all animation states. If true, the brakelight animation is assumed instead to have
	 *  multiple sprites, one per each animation state; the brakelight sheet is also assumed to contain
	 *  N equally sized frames, where N is the number of states of this animation. */
	bool brakelightsMultipleSprites;

	// Shadow layer/sprites

	/** The filename of the image containing the sprite for the optional shadow overlay.
	 *  If this field is specified, the shadow sprite will be drawn before the vehicle sprite.
	 *  Note that this sprites should depict the vehicle's shadow as it was being cast from
	 *  top-down position. Also, the shadow image should be in the same scale as the vehicle sprite.
	 *  If this field is assigned 'none' or is left unspecified, it's assumed that there is no shadow
	 *  to draw/cast. If 'default' or 'builtin' is specified, a black, translucent ellipsis is drawn
	 *  below the vehicle, with a width equivalent to 95% of the depicted vehicle width. */
	std::string shadowSheetFilename;

	/** The positions of the shadows within the animation's coordinates.
	 *  Note that this is a vector because each of the vehicle animation's states may have the
	 *  shadow located at different coordinates. By default, (0, 0) coordinates are assumed. */
	std::vector<fgeal::Point> shadowPositions;
};

#endif /* PSEUDO3D_VEHICLE_GFX_HPP_ */
