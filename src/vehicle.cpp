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
using fgeal::Vector2D;
using fgeal::Point;
using fgeal::Image;
using fgeal::Sprite;

Pseudo3DVehicle::Pseudo3DVehicle()
: body(Engine(), Mechanics::TYPE_OTHER),
  position(), horizontalPosition(), verticalPosition(),
  pseudoAngle(), strafeSpeed(), curvePull(), corneringStiffness(),
  verticalSpeed(0), onAir(false), onLongAir(false),
  isTireBurnoutOccurring(false), isCrashing(false),
  engineSound(), spriteSpec(), sprites(), brakelightSprite(null), shadowSprite(null), smokeSprite(null),
  spriteAssetsAreShared(false), soundAssetsAreShared(false)
{}

Pseudo3DVehicle::~Pseudo3DVehicle()
{
	freeAssetsData();
}

void Pseudo3DVehicle::setSpec(const Spec& spec, int alternateSpriteIndex)
{
	freeAssetsData();
	body = Mechanics(Engine(spec.engineMaximumRpm, spec.engineMaximumPower, spec.enginePowerBand, spec.engineGearCount), spec.type, spec.dragArea, spec.liftArea);
	spriteSpec = alternateSpriteIndex == -1? spec.sprite : spec.alternateSprites[alternateSpriteIndex];
	brakelightSprite = shadowSprite = smokeSprite = null;
	spriteAssetsAreShared = soundAssetsAreShared = false;

	engineSound.setProfile(spec.soundProfile, spec.engineMaximumRpm);

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
	body.weightDistribution = spec.weightDistribution;
	body.centerOfGravityHeight = spec.centerOfGravityHeight;
	body.wheelbase = spec.wheelbase;
}

void Pseudo3DVehicle::loadGraphicAssetsData()
{
	fgeal::Image* sheet = new fgeal::Image(spriteSpec.sheetFilename);

	if(sheet->getWidth() < (int) spriteSpec.frameWidth)
		throw std::runtime_error("Invalid sprite width value. Value is smaller than sprite sheet width (no whole sprites could be draw)");

	for(unsigned i = 0; i < spriteSpec.stateCount; i++)
	{
		fgeal::Sprite* sprite = new fgeal::Sprite(sheet, spriteSpec.frameWidth, spriteSpec.frameHeight,
									spriteSpec.frameDuration, spriteSpec.stateFrameCount[i],
									0, i*spriteSpec.frameHeight);

		sprite->scale = spriteSpec.scale;
		sprites.push_back(sprite);
	}

	if(spriteSpec.asymmetrical) for(unsigned i = 1; i < spriteSpec.stateCount; i++)
	{
		fgeal::Sprite* sprite = new fgeal::Sprite(sheet, spriteSpec.frameWidth, spriteSpec.frameHeight,
									spriteSpec.frameDuration, spriteSpec.stateFrameCount[i],
									0, (spriteSpec.stateCount-1 + i)*spriteSpec.frameHeight);

		sprite->scale = spriteSpec.scale;
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
				brakelightSpriteImage->getHeight(),
				-1, true
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
	spriteAssetsAreShared = false;
}

void Pseudo3DVehicle::loadSoundAssetsData()
{
	engineSound.loadAssetsData();
	soundAssetsAreShared = false;
}

void Pseudo3DVehicle::loadGraphicAssetsData(const Pseudo3DVehicle* optionalBaseVehicle)
{
	if(optionalBaseVehicle == null)
		throw std::invalid_argument("Argument is NULL");
	else if(spriteSpec.sheetFilename != optionalBaseVehicle->spriteSpec.sheetFilename)
		throw std::invalid_argument("Sprite spec. of the passed argument does not match this vehicle's.");

	sprites = optionalBaseVehicle->sprites;
	brakelightSprite = optionalBaseVehicle->brakelightSprite;
	shadowSprite = optionalBaseVehicle->shadowSprite;
	smokeSprite = optionalBaseVehicle->smokeSprite;
	spriteAssetsAreShared = true;
}

void Pseudo3DVehicle::loadSoundAssetsData(const Pseudo3DVehicle* baseVehicle)
{
	if(baseVehicle == null)
		throw std::invalid_argument("Argument is NULL");

	engineSound = baseVehicle->engineSound;
	soundAssetsAreShared = true;
}

void Pseudo3DVehicle::freeAssetsData()
{
	if(not soundAssetsAreShared)
		engineSound.freeAssetsData();

	if(not spriteAssetsAreShared)
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

		if(smokeSprite != null)
			delete smokeSprite;
	}
}

void Pseudo3DVehicle::draw(float x, float y, float angle, float distanceScale, float cropY) const
{
	unsigned animationIndex = 0;
	for(unsigned i = 1; i < spriteSpec.stateCount; i++)
		if(fabs(angle) >= spriteSpec.depictedTurnAngle[i])
			animationIndex = i;

	const bool isLeanRight = (angle > 0 and animationIndex != 0);

	// if asymmetrical, right-leaning sprites are after all left-leaning ones
	if(isLeanRight and spriteSpec.asymmetrical)
		animationIndex += (spriteSpec.stateCount-1);

	Sprite& sprite = *sprites[animationIndex];
	sprite.flipmode = isLeanRight and not spriteSpec.asymmetrical? Image::FLIP_HORIZONTAL : Image::FLIP_NONE;
//	sprite.duration = body.speed != 0? 0.1*400.0/(body.speed*sprite.numberOfFrames) : 999;  // sometimes work, sometimes don't
	sprite.duration = spriteSpec.frameDuration / sqrt(body.speed);  // this formula doesn't present good tire animation results.
//	sprite.duration = body.speed != 0? 2.0*M_PI*body.tireRadius/(body.speed*sprite.numberOfFrames) : -1;  // this formula should be the physically correct, but still not good visually.
	sprite.computeCurrentFrame();

	const Vector2D originalSpriteScale = sprite.scale;
	sprite.scale *= distanceScale;

	const Point vehicleSpritePosition = {
		x - sprite.scale.x * 0.5f * spriteSpec.frameWidth,
		y - sprite.scale.y * (spriteSpec.frameHeight - spriteSpec.contactOffset)
	};

	if(shadowSprite != null)
	{
		const Vector2D originalScale = shadowSprite->scale;
		shadowSprite->scale *= distanceScale;

		const Point& shadowPosition = spriteSpec.shadowPositions[animationIndex];
		shadowSprite->currentFrameSequenceIndex = animationIndex;
		shadowSprite->flipmode = sprite.flipmode;

		shadowSprite->draw(
			vehicleSpritePosition.x + shadowPosition.x * sprite.scale.x,
			vehicleSpritePosition.y + shadowPosition.y * sprite.scale.y
		);

		shadowSprite->scale = originalScale;
	}

	sprite.croppingArea.h = cropY;
	sprite.draw(vehicleSpritePosition.x, vehicleSpritePosition.y);
	sprite.croppingArea.h = 0;

	if(body.brakePedalPosition > 0 and brakelightSprite != null)
	{
		const Vector2D originalScale = brakelightSprite->scale;
		brakelightSprite->scale *= distanceScale;

		if(spriteSpec.brakelightsMultipleSprites)
			brakelightSprite->currentFrameSequenceIndex = animationIndex;

		const float scaledBrakelightPositionX = spriteSpec.brakelightsPositions[animationIndex].x * sprite.scale.x,
					scaledBrakelightPositionY = spriteSpec.brakelightsPositions[animationIndex].y * sprite.scale.y,
					scaledBrakelightOffsetX = spriteSpec.brakelightsOffset.x * brakelightSprite->scale.x,
					scaledBrakelightOffsetY = spriteSpec.brakelightsOffset.y * brakelightSprite->scale.y,
					scaledTurnOffset = (spriteSpec.brakelightsPositions[animationIndex].x - spriteSpec.brakelightsPositions[0].x)*sprite.scale.x,
					scaledFlipOffset = sprite.flipmode != Image::FLIP_HORIZONTAL? 0 : 2*scaledTurnOffset;

		if(not spriteSpec.brakelightsMirrowed)
			brakelightSprite->flipmode = sprite.flipmode;

		brakelightSprite->draw(
			vehicleSpritePosition.x + scaledBrakelightPositionX - scaledFlipOffset + scaledBrakelightOffsetX,
			vehicleSpritePosition.y + scaledBrakelightPositionY + scaledBrakelightOffsetY
		);

		if(spriteSpec.brakelightsMirrowed)
		{
			const float scaledFrameWidth = spriteSpec.frameWidth*sprite.scale.x;

			brakelightSprite->draw(
				vehicleSpritePosition.x + scaledFrameWidth - scaledBrakelightPositionX - scaledFlipOffset + scaledBrakelightOffsetX + 2*scaledTurnOffset,
				vehicleSpritePosition.y + scaledBrakelightPositionY + scaledBrakelightOffsetY
			);
		}

		brakelightSprite->scale = originalScale;
	}

	if(isTireBurnoutOccurring and smokeSprite != null)
	{
		const Vector2D originalScale = smokeSprite->scale;
		smokeSprite->scale *= distanceScale;

		const float maxDepictedTurnAngle = spriteSpec.depictedTurnAngle.size() < 2? 0 : spriteSpec.depictedTurnAngle.back();

		const Point smokeSpritePosition = {
				vehicleSpritePosition.x + 0.5f*(sprite.scale.x*(sprite.width - spriteSpec.depictedVehicleWidth) - smokeSprite->width*smokeSprite->scale.x)
				+ ((angle > 0? -1.f : 1.f)*10.f*animationIndex*maxDepictedTurnAngle),
				vehicleSpritePosition.y + sprite.height*sprite.scale.y - smokeSprite->height*smokeSprite->scale.y  // should have included ` - sprite.offset*sprite.scale.x`, but don't look good
		};

		// left smoke
		smokeSprite->computeCurrentFrame();
		smokeSprite->flipmode = Image::FLIP_NONE;
		smokeSprite->draw(smokeSpritePosition.x, smokeSpritePosition.y);

		// right smoke
		smokeSprite->computeCurrentFrame();
		smokeSprite->flipmode = Image::FLIP_HORIZONTAL;
		smokeSprite->draw(smokeSpritePosition.x + spriteSpec.depictedVehicleWidth*sprite.scale.x, smokeSpritePosition.y);

		smokeSprite->scale = originalScale;
	}

	sprite.scale = originalSpriteScale;
}
