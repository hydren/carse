/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "futil/string_actions.hpp"

#include <stdexcept>
#include <cstdlib>
#include <cmath>

using std::string;

Pseudo3DVehicle::Pseudo3DVehicle()
: body(Engine(), Mechanics::TYPE_OTHER),
  position(), horizontalPosition(), verticalPosition(),
  pseudoAngle(), strafeSpeed(), curvePull(), corneringStiffness(),
  /* verticalSpeed(0), onAir(false), onLongAir(false), */
  isBurningRubber(false),
  engineSoundProfile(), engineSound(),
  spriteSpec(), sprites(), brakelightSprite(null), shadowSprite(null)
{}

Pseudo3DVehicle::Pseudo3DVehicle(const Pseudo3DVehicle::Spec& spec, int alternateSpriteIndex)
: body(Engine(spec.engineMaximumRpm, spec.engineMaximumPower, spec.enginePowerBand, spec.engineGearCount), spec.type, spec.dragArea, spec.liftArea),
  position(), horizontalPosition(), verticalPosition(),
  pseudoAngle(), strafeSpeed(), curvePull(), corneringStiffness(),
  /* verticalSpeed(0), onAir(false), onLongAir(false), */
  isBurningRubber(false),
  engineSoundProfile(spec.soundProfile), engineSound(),
  spriteSpec(alternateSpriteIndex == -1? spec.sprite : spec.alternateSprites[alternateSpriteIndex]),
  sprites(), brakelightSprite(null), shadowSprite(null)
{
	// update engine info data (optional)
	body.engine.configuration = spec.engineConfiguration;
	body.engine.aspiration = spec.engineAspiration;
	body.engine.valvetrain = spec.engineValvetrain;
	body.engine.displacement = spec.engineDisplacement;
	body.engine.valveCount = spec.engineValveCount;

	// set custom gear ratios
	for(unsigned g = 0; g < spec.engineGearCount; g++)
		body.engine.gearRatio[g] = spec.engineGearRatio[g];

	body.engine.reverseGearRatio = spec.engineReverseGearRatio;
	body.engine.differentialRatio = spec.engineDifferentialRatio;
	body.engine.transmissionEfficiency = spec.engineTransmissionEfficiency;

	// set custom physics data
	body.mass = spec.mass;
	body.tireRadius = spec.tireRadius;
	body.engineLocation = spec.engineLocation;
	body.drivenWheelsType = spec.drivenWheelsType;
	body.weightDistribuition = spec.weightDistribuition;
	body.centerOfGravityHeight = spec.centerOfGravityHeight;
	body.wheelbase = spec.wheelbase;
}

Pseudo3DVehicle::~Pseudo3DVehicle()
{
	if(not engineSound.getSoundData().empty())
	{
		for(unsigned i = 0; i < engineSound.getSoundData().size(); i++)
			delete engineSound.getSoundData()[i];

		engineSound.getSoundData().clear();
	}

	clearDynamicData();
}

void Pseudo3DVehicle::clearDynamicData()
{
	if(not sprites.empty())
	{
		delete sprites[0]->image;

		for(unsigned i = 0; i < sprites.size(); i++)
			delete sprites[i];

		sprites.clear();
	}

	if(brakelightSprite != null)
		delete brakelightSprite;

	if(shadowSprite != null)
		delete shadowSprite;
}

void Pseudo3DVehicle::setupDynamicData()
{
	engineSound.setProfile(engineSoundProfile, body.engine.maxRpm);

	fgeal::Image* sheet = new fgeal::Image(spriteSpec.sheetFilename);

	if(sheet->getWidth() < (int) spriteSpec.frameWidth)
		throw std::runtime_error("Invalid sprite width value. Value is smaller than sprite sheet width (no whole sprites could be draw)");

	for(unsigned i = 0; i < spriteSpec.stateCount; i++)
	{
		fgeal::Sprite* sprite = new fgeal::Sprite(sheet, spriteSpec.frameWidth, spriteSpec.frameHeight,
									spriteSpec.frameDuration, spriteSpec.stateFrameCount[i],
									0, i*spriteSpec.frameHeight);

		sprite->scale = spriteSpec.scale;
		sprite->referencePixelY = - (int) spriteSpec.contactOffset;
		sprites.push_back(sprite);
	}

	if(spriteSpec.asymmetrical) for(unsigned i = 1; i < spriteSpec.stateCount; i++)
	{
		fgeal::Sprite* sprite = new fgeal::Sprite(sheet, spriteSpec.frameWidth, spriteSpec.frameHeight,
									spriteSpec.frameDuration, spriteSpec.stateFrameCount[i],
									0, (spriteSpec.stateCount-1 + i)*spriteSpec.frameHeight);

		sprite->scale = spriteSpec.scale;
		sprite->referencePixelY = - (int) spriteSpec.contactOffset;
		sprites.push_back(sprite);
	}

	if(not spriteSpec.brakelightsSheetFilename.empty())
	{
		fgeal::Image* brakelightSpriteImage = new fgeal::Image(spriteSpec.brakelightsSheetFilename);
		if(spriteSpec.brakelightsMultipleSprites)
			brakelightSprite = new fgeal::Sprite(
				brakelightSpriteImage,
				brakelightSpriteImage->getWidth(),
				brakelightSpriteImage->getHeight()/spriteSpec.stateCount,
				-1, spriteSpec.stateCount, 0, 0, true
			);
		else
			brakelightSprite = new fgeal::Sprite(
				brakelightSpriteImage,
				brakelightSpriteImage->getWidth(),
				brakelightSpriteImage->getHeight()
				-1, -1, 0, 0, true
			);

		brakelightSprite->scale = spriteSpec.brakelightsSpriteScale;
	}

	if(not spriteSpec.shadowSheetFilename.empty())
	{
		fgeal::Image* shadowSpriteImage = new fgeal::Image(spriteSpec.shadowSheetFilename);
		shadowSprite = new fgeal::Sprite(
			shadowSpriteImage,
			shadowSpriteImage->getWidth(),
			shadowSpriteImage->getHeight()/spriteSpec.stateCount,
			-1, spriteSpec.stateCount, 0, 0, true
		);

		shadowSprite->scale = spriteSpec.scale;
	}
}
