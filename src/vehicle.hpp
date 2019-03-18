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
#include "automotive/vehicle_spec.hpp"
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

		/* Loads data from the given filename, parse its vehicle spec data and store in this object. */
		void loadFromFile(const std::string& filename);

		/* Creates a vehicle spec. by loading and parsing the data in the given filename. */
		inline static Spec createFromFile(const std::string& filename) { Spec spec; spec.loadFromFile(filename); return spec; }

		/* Creates a engine sound profile by loading and parsing the data in the given filename. */
		static EngineSoundProfile createEngineSoundProfileFromFile(const std::string& filename);
	};

	// physics simulation
	Mechanics body;

	// logic data
	float position, horizontalPosition, verticalPosition;
	float pseudoAngle, strafeSpeed, curvePull, corneringStiffness;
//	float verticalSpeed;
//	bool onAir, onLongAir;
	bool isTireBurnoutOccurring;

	// sound data
	EngineSoundSimulator engineSound;

	// graphics data
	Pseudo3DVehicleAnimationSpec spriteSpec;
	std::vector<fgeal::Sprite*> sprites;
	fgeal::Sprite* brakelightSprite, *shadowSprite, *smokeSprite;

	Pseudo3DVehicle();  // zero constructor

	/** Creates a vehicle with the given specifications. The optional 'alternateSpriteIndex' argument specifies an alternate skin to use (-1 means use default sprite). */
	Pseudo3DVehicle(const Pseudo3DVehicle::Spec& spec, int alternateSpriteIndex=-1);

	~Pseudo3DVehicle();

	/** Loads the graphic assets' data from the filesystem. The 'optionalBaseVehicle' argument can be passed to use its sprite data instead of loading the assets again.  */
	void loadGraphicAssetsData(const Pseudo3DVehicle* optionalBaseVehicle=null);

	/** Loads the sound assets' data from the filesystem. The 'optionalBaseVehicle' argument can be passed to use its sound data instead of loading the assets again.  */
	void loadSoundAssetsData(const Pseudo3DVehicle* optionalBaseVehicle=null);

	inline void loadAssetsData(const Pseudo3DVehicle* optionalBaseVehicle=null) {
		loadGraphicAssetsData(optionalBaseVehicle);
		loadSoundAssetsData(optionalBaseVehicle);
	}

	/** Disposes of loaded graphics and sounds assets. */
	void freeAssetsData();

	/** Draws this vehicle at the given position (x, y).
	 *  The 'angle' argument specifies the angle to be depicted.
	 *  The 'distanceScale' specifies how far the vehicle is depicted.
	 *  TThe 'cropY' parameter specifies how much to crop the sprite vertically (bottom-up). */
	void draw(float x, float y, float angle=0, float distanceScale=1.0, float cropY=0);

	private:
	bool spriteAssetsAreShared, soundAssetsAreShared;
};

#endif /* PSEUDO3D_VEHICLE_HPP_ */
