/*
 * race_state_physics.cpp
 *
 *  Created on: 24 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "race_state.hpp"

/* Tire coefficients
 *
 *          Rolling resist. | Peak static frict. | Kinetic frict.
 * dry alphalt - 0.013 ----------- 0.85 --------------- 0.75
 * wet asphalt - 0.013 ----------- 0.65 --------------- 0.52
 * concrete ---- 0.013 ----------- 0.85 --------------- 0.75
 * gravel ------ 0.020 ----------- 0.60 --------------- 0.55
 * grass ------- 0.100 ----------- 0.42 --------------- 0.35
 * dirt -------- 0.050 ----------- 0.68 --------------- 0.65
 * mud --------- 0.080 ----------- 0.55 --------------- 0.45
 * sand -------- 0.300 ----------- 0.60 --------------- 0.55
 * snow -------- 0.016 ----------- 0.20 --------------- 0.15
 * ice --------- 0.013 ----------- 0.10 --------------- 0.07
 *
 * (extrapolated)
 * water ------- 0.750 ----------- 2.00 --------------- 0.40
 *
 * */

static const float TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT = 0.85;
static const float TIRE_FRICTION_COEFFICIENT_GRASS = 0.42;
static const float ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT = 0.013;
static const float ROLLING_RESISTANCE_COEFFICIENT_GRASS = 0.100;

static const float PSEUDO_ANGLE_MAX = 1.0;
static const float CURVE_PULL_FACTOR = 0.2;
static const float STEERING_SPEED = 2.0;
static const float MINIMUM_SPEED_ALLOW_TURN = 1.0/36.0;  // == 1kph

void Pseudo3DRaceState::handlePhysics(float delta)
{
	const CourseSpec::Segment& segment = course.spec.lines[((int)(playerVehicle.position*coursePositionFactor/course.spec.roadSegmentLength))%course.spec.lines.size()];
	const float wheelAngleFactor = 1 - playerVehicle.corneringForceLeechFactor*fabs(playerVehicle.pseudoAngle)/PSEUDO_ANGLE_MAX;

	playerVehicle.body.tireFrictionFactor = getTireKineticFrictionCoefficient();
	playerVehicle.body.rollingResistanceFactor = getTireRollingResistanceCoefficient();
	playerVehicle.body.arbitraryForceFactor = wheelAngleFactor;
	playerVehicle.body.slopeAngle = atan2(segment.y - playerVehicle.verticalPosition, course.spec.roadSegmentLength);

	if(onSceneIntro)
		playerVehicle.body.engine.gear = 0;

	playerVehicle.body.engine.throttlePosition = isPlayerAccelerating()? 1.0 : 0.0;
	playerVehicle.body.brakePedalPosition = isPlayerBraking()? 1.0 : 0.0;
	playerVehicle.body.updatePowertrain(delta);

	// update position
	playerVehicle.position += playerVehicle.body.speed*delta;

	// update steering
	if(isPlayerSteeringRight() and fabs(playerVehicle.body.speed) >= MINIMUM_SPEED_ALLOW_TURN)
	{
		if(playerVehicle.pseudoAngle < 0) playerVehicle.pseudoAngle *= 1/(1+5*delta);
		playerVehicle.pseudoAngle += delta * STEERING_SPEED;
	}
	else if(isPlayerSteeringLeft() and fabs(playerVehicle.body.speed) >= MINIMUM_SPEED_ALLOW_TURN)
	{
		if(playerVehicle.pseudoAngle > 0) playerVehicle.pseudoAngle *= 1/(1+5*delta);
		playerVehicle.pseudoAngle -= delta * STEERING_SPEED;
	}
	else playerVehicle.pseudoAngle *= 1/(1+5*delta);

	if(playerVehicle.pseudoAngle > PSEUDO_ANGLE_MAX) playerVehicle.pseudoAngle = PSEUDO_ANGLE_MAX;
	if(playerVehicle.pseudoAngle <-PSEUDO_ANGLE_MAX) playerVehicle.pseudoAngle =-PSEUDO_ANGLE_MAX;

	// update strafing
	playerVehicle.strafeSpeed = playerVehicle.pseudoAngle * playerVehicle.body.speed * playerVehicle.corneringStiffness * coursePositionFactor;
//	vehicle.strafeSpeed = onAir? 0 : vehicle.pseudoAngle * vehicle.body.speed * coursePositionFactor;

	// limit strafing speed by magic constant
	if(playerVehicle.strafeSpeed >  MAXIMUM_STRAFE_SPEED * playerVehicle.corneringStiffness) playerVehicle.strafeSpeed = MAXIMUM_STRAFE_SPEED * playerVehicle.corneringStiffness;
	if(playerVehicle.strafeSpeed < -MAXIMUM_STRAFE_SPEED * playerVehicle.corneringStiffness) playerVehicle.strafeSpeed =-MAXIMUM_STRAFE_SPEED * playerVehicle.corneringStiffness;

	// update curve pull
	//curvePull = segment.curve * vehicle.body.speed * coursePositionFactor * CURVE_PULL_FACTOR;
	playerVehicle.curvePull = sin(atan2(segment.curve*50, course.spec.roadSegmentLength));
	playerVehicle.curvePull *= playerVehicle.body.speed * coursePositionFactor;

	// update strafe position
	playerVehicle.horizontalPosition += (playerVehicle.strafeSpeed - playerVehicle.curvePull)*delta;

	// update vertical position
	playerVehicle.verticalPosition = segment.y;

	/*
	if(segment.y >= posY)
	{
		verticalSpeed = (segment.y - posY)/delta;
		posY = segment.y;
		if(onLongAir)
		{
			onLongAir = false;
			sndJumpImpact->stop();
			sndJumpImpact->play();
		}
		onAir = false;
	}
	else if(segment.y < posY)
	{
		verticalSpeed += 2500*GRAVITY_ACCELERATION * delta;
		if(verticalSpeed > 1000) onAir = true;
		if(verticalSpeed > 6000) onLongAir = true;
		posY -= verticalSpeed*delta;
	}
	*/

	// update bg parallax
	parallax.x -= segment.curve*playerVehicle.body.speed*0.025;
	parallax.y -= 2*playerVehicle.body.slopeAngle;

	if(parallax.x < -(2.0f*imgBackground->getWidth()-game.getDisplay().getWidth()))
		parallax.x += imgBackground->getWidth();

	if(parallax.x > 0)
		parallax.x -= imgBackground->getWidth();
}

void Pseudo3DRaceState::shiftGear(int gear)
{
	// todo play gear shift sound
	playerVehicle.body.shiftGear(gear);
}

Pseudo3DRaceState::SurfaceType Pseudo3DRaceState::getCurrentSurfaceType()
{
	if(fabs(playerVehicle.horizontalPosition) > 1.2*course.spec.roadWidth)
		return SURFACE_TYPE_GRASS;
	else
		return SURFACE_TYPE_DRY_ASPHALT;
}

float Pseudo3DRaceState::getTireKineticFrictionCoefficient()
{
	switch(getCurrentSurfaceType())
	{
		default:
		case SURFACE_TYPE_DRY_ASPHALT: return TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT;
		case SURFACE_TYPE_GRASS:       return TIRE_FRICTION_COEFFICIENT_GRASS;
	}
}

float Pseudo3DRaceState::getTireRollingResistanceCoefficient()
{
	switch(getCurrentSurfaceType())
	{
		default:
		case SURFACE_TYPE_DRY_ASPHALT: return ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT;
		case SURFACE_TYPE_GRASS:       return ROLLING_RESISTANCE_COEFFICIENT_GRASS;
	}
}
