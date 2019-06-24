/*
 * vehicle.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_HPP_
#define PSEUDO3D_VEHICLE_HPP_
#include <ciso646>

#include "automotive/vehicle_spec.hpp"
#include "automotive/engine_sound.hpp"
#include "pseudo3d/vehicle_gfx.hpp"
#include "fgeal/extra/sprite.hpp"

#include <string>
#include <vector>

// forward declared
class CarseGameLogicInstance;

struct Pseudo3DVehicle
{
	struct Spec extends VehicleSpec
	{
		// graphics data
		Pseudo3DVehicleAnimationSpec sprite;

		// additional alternate graphics data (optional)
		std::vector<Pseudo3DVehicleAnimationSpec> alternateSprites;

		// sound data (optional)
		EngineSoundProfile soundProfile;

		/* Loads data from the given filename, parse its vehicle spec data and store in this object. */
		void loadFromFile(const std::string& filename, const CarseGameLogicInstance&);
	};

	// physics simulation
	Mechanics body;

	// logic data
	float position, horizontalPosition, verticalPosition;  // positions
	float strafeSpeed, verticalSpeed;  // velocities ("forward" speed is in body class)
	float pseudoAngle, corneringStiffness, curvePull;  // "curving" variables
	float virtualOrientation;  // "virtual" vehicle orientation angle in the map (which direction is pointing in map)
	bool onAir, onLongAir;
	bool isTireBurnoutOccurring, isCrashing;

	// sound data
	EngineSoundSimulator engineSound;

	// graphics data
	Pseudo3DVehicleAnimationSpec spriteSpec;
	std::vector<fgeal::Sprite*> sprites;
	fgeal::Sprite* brakelightSprite, *shadowSprite, *smokeSprite;

	Pseudo3DVehicle();

	~Pseudo3DVehicle();

	/** Sets the attributes of this vehicle according to the specifications given by the Spec argument.
	 *  The optional 'alternateSpriteIndex' argument specifies whether to use the standard sprite or an alternate one (-1 means use standard sprite). */
	void setSpec(const Spec&, int alternateSpriteIndex=-1);

	/** Loads the graphic assets' data from the filesystem. */
	void loadGraphicAssetsData();

	/** Loads the sound assets' data from the filesystem. */
	void loadSoundAssetsData();

	inline void loadAssetsData() {
		loadGraphicAssetsData();
		loadSoundAssetsData();
	}

	/** Loads the graphic assets' data from the 'baseVehicle' argument to use its sprite data in this vehicle as well. The 'baseVehicle' "owns" the resources, though.  */
	void loadGraphicAssetsData(const Pseudo3DVehicle* baseVehicle);

	/** Loads the sound assets' data from the 'baseVehicle' argument to use its sound data in this vehicle as well. The 'baseVehicle' "owns" the resources, though. */
	void loadSoundAssetsData(const Pseudo3DVehicle* baseVehicle);

	inline void loadAssetsData(const Pseudo3DVehicle* baseVehicle) {
		loadGraphicAssetsData(baseVehicle);
		loadSoundAssetsData(baseVehicle);
	}

	/** Draws this vehicle at the given position (x, y).
	 *  The 'angle' argument specifies the angle to be depicted.
	 *  The 'distanceScale' specifies how far the vehicle is depicted.
	 *  TThe 'cropY' parameter specifies how much to crop the sprite vertically (bottom-up). */
	void draw(float x, float y, float angle=0, float distanceScale=1.0, float cropY=0) const;

	private:
	bool spriteAssetsAreShared, soundAssetsAreShared;

	// Disposes of loaded graphics and sounds assets.
	void freeAssetsData();
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
