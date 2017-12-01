/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "futil/string_actions.hpp"

#include <cstdlib>
#include <cmath>

using std::string;

Pseudo3DVehicle::Pseudo3DVehicle()
: spec(Pseudo3DVehicle::Spec()),
  body(Engine(), Mechanics::TYPE_OTHER),
  engineSoundProfile(), sprite()
{}

Pseudo3DVehicle::Pseudo3DVehicle(const Pseudo3DVehicle::Spec& s, int alternateSpriteIndex)
: spec(s),
  body(Engine(spec.engineMaximumRpm, spec.engineMaximumPower, spec.engineTorqueCurveProfile, spec.engineGearCount), spec.type, spec.dragArea, spec.liftArea),
  engineSoundProfile(spec.sound),
  sprite(alternateSpriteIndex == -1? spec.sprite : spec.alternateSprites[alternateSpriteIndex])
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
