/*
 * race_state_physics.cpp
 *
 *  Created on: 24 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "pseudo3d_race_state.hpp"

#include "carse_game.hpp"
#include "util.hpp"

using std::vector;

#include <iostream>
using std::cout; using std::endl;
// XXX DEBUG

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

#define GRAVITY_ACCELERATION Mechanics::GRAVITY_ACCELERATION

static const float TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT = 0.85,
				   TIRE_FRICTION_COEFFICIENT_GRASS = 0.42,
				   ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT = 0.013,
				   ROLLING_RESISTANCE_COEFFICIENT_GRASS = 0.100,
				   COLLISION_RESTITUTION_COEFFICIENT = 0.5,

				   PSEUDO_ANGLE_MAX = 1.0,
				   STEERING_SPEED = 2.0,
				   MINIMUM_SPEED_ALLOW_TURN = 1.0/36.0,  // == 1kph
				   MINIMUM_SPEED_CORNERING_LEECH = 10;  // == 36kph

void Pseudo3DRaceState::handlePhysics(float delta)
{
	const unsigned courseSegmentIndex = static_cast<int>(playerVehicle.position * coursePositionFactor / course.spec.roadSegmentLength) % course.spec.lines.size();
	const CourseSpec::Segment& courseSegment = course.spec.lines[courseSegmentIndex];
	const float corneringForceLeechFactor = playerVehicle.body.speed > MINIMUM_SPEED_CORNERING_LEECH? (playerVehicle.body.vehicleType == Mechanics::TYPE_BIKE? 0.4 : 0.5) : 0,
				wheelAngleFactor = 1 - corneringForceLeechFactor*fabs(playerVehicle.pseudoAngle)/PSEUDO_ANGLE_MAX,
				maxStrafeSpeed = MAXIMUM_STRAFE_SPEED_FACTOR * playerVehicle.corneringStiffness;

	playerVehicle.body.tireFrictionFactor = playerVehicle.onAir? 0 : getTireKineticFrictionCoefficient();
	playerVehicle.body.rollingResistanceFactor = playerVehicle.onAir? 0 : getTireRollingResistanceCoefficient();
	playerVehicle.body.arbitraryForceFactor = wheelAngleFactor;
	playerVehicle.body.slopeAngle = playerVehicle.onAir? 0 : atan2(courseSegment.y - playerVehicle.verticalPosition, course.spec.roadSegmentLength);

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

	if(fabs(playerVehicle.pseudoAngle) > PSEUDO_ANGLE_MAX)
		playerVehicle.pseudoAngle = PSEUDO_ANGLE_MAX * sgn(playerVehicle.pseudoAngle);

	// update strafing
	playerVehicle.strafeSpeed = playerVehicle.pseudoAngle * playerVehicle.body.speed;

	if(playerVehicle.onLongAir) playerVehicle.strafeSpeed = 0;

	// limit strafing speed by magic constant
	if(fabs(playerVehicle.strafeSpeed) > maxStrafeSpeed)
		playerVehicle.strafeSpeed = maxStrafeSpeed * sgn(playerVehicle.strafeSpeed);

	// update curve pull
//	const float curvatureFactor = 2 * sin(0.5 * courseSegment.curve);  // correct formula according to theory, assuming 'courseSegment.curve' is the degree of curvature, in radians. however it does not behave nicely...
//	const float curvatureFactor = 2 * sin(atan(0.5 * courseSegment.curve));
//	const float curvatureFactor = 2 * sin(atan(0.4 * pow2(courseSegment.curve))) * sgn(courseSegment.curve);
//	const float curvatureFactor = sqrt(2 * courseSegment.curve) * atan(0.5 * courseSegment.curve);
//	const float curvatureFactor = sqrt(2 * fabs(courseSegment.curve)) * atan(0.25 * pow2(courseSegment.curve)) * sgn(courseSegment.curve);
//	const float curvatureFactor = 1.25 * courseSegment.curve - atan(courseSegment.curve);
	const float curvatureFactor = 1.25 * courseSegment.curve;
	playerVehicle.curvePull = pow2(playerVehicle.body.speed) * curvatureFactor / course.spec.roadSegmentLength;

	// update strafe position
	playerVehicle.horizontalPosition += (playerVehicle.strafeSpeed - playerVehicle.curvePull)*delta;

	// update "virtual" orientation
	playerVehicle.virtualOrientation += courseSegment.curve * playerVehicle.body.speed * delta;  // FIXME get a proper formula for this, instead of this rough approximation

	if(enableJumpSimulation)
	{
		if(playerVehicle.onAir)
			playerVehicle.verticalSpeed -= 10 * GRAVITY_ACCELERATION * delta;
		else
			playerVehicle.verticalSpeed = fabs(playerVehicle.body.speed) * sin(playerVehicle.body.slopeAngle);

		playerVehicle.verticalPosition += coursePositionFactor * playerVehicle.verticalSpeed * delta;

		if(courseSegment.y >= playerVehicle.verticalPosition)
		{
			playerVehicle.verticalPosition = courseSegment.y;
			if(playerVehicle.onLongAir)
				sndJumpImpact->play();
			playerVehicle.onAir = playerVehicle.onLongAir = false;
		}
		else
		{
			playerVehicle.onAir = true;
			if(playerVehicle.verticalPosition - courseSegment.y > 500)
				playerVehicle.onLongAir = true;
		}
	}
	else
	{
		// update vertical position
		playerVehicle.verticalPosition = courseSegment.y;
	}

	// verify for prop collision
	if(courseSegment.propIndex != -1)
	{
		const CourseSpec::Prop& prop = course.spec.props[courseSegment.propIndex];
		if(prop.blocking)
		{
			const float pw = playerVehicle.spriteSpec.depictedVehicleWidth * playerVehicle.sprites.back()->scale.x * 7,
						px = playerVehicle.horizontalPosition * coursePositionFactor - 0.5f*pw,
						tx = courseSegment.propX * coursePositionFactor * 10;  // FIXME fix this formula because it does not behave correctly for different sized props

			cout << "pw=" << pw << " px=" << px << " tx=" << tx << endl;
			if(not (px + pw < tx or px > tx))
			{
				cout << "collided with prop" << endl;
				playerVehicle.position += (1.f - playerVehicle.body.speed) * delta;  // revert progress and pushes back the car a little bit
				playerVehicle.body.speed = -1;
				playerVehicle.isCrashing = true;
			}
		}
	}

	// verify for traffic collision
	foreach(Pseudo3DVehicle&, trafficVehicle, vector<Pseudo3DVehicle>, trafficVehicles)
	{
		trafficVehicle.body.rollingResistanceFactor = ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT;
		trafficVehicle.body.tireFrictionFactor = TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT;
		trafficVehicle.body.updatePowertrain(delta);
		trafficVehicle.position += trafficVehicle.body.speed*delta;  // update position

		const unsigned trafficVehicleCourseSegmentIndex = static_cast<int>(trafficVehicle.position * coursePositionFactor / course.spec.roadSegmentLength) % course.spec.lines.size();
		if(trafficVehicleCourseSegmentIndex == courseSegmentIndex)  // if on the same segment, check for collision
		{
			const float pw = playerVehicle.spriteSpec.depictedVehicleWidth * playerVehicle.sprites.back()->scale.x * 7,
						px = playerVehicle.horizontalPosition * coursePositionFactor - 0.5f*pw,
						tw = trafficVehicle.spriteSpec.depictedVehicleWidth * trafficVehicle.sprites.back()->scale.x * 7,
						tx = trafficVehicle.horizontalPosition * coursePositionFactor - 0.5f*tw;

			if(not (px + pw < tx or px > tx + tw))
			{
				// slow player's vehicle down using collision formula (but not applying to traffic vehicle, though)
				playerVehicle.body.speed = (COLLISION_RESTITUTION_COEFFICIENT * trafficVehicle.body.mass * (trafficVehicle.body.speed - playerVehicle.body.speed)
											+ playerVehicle.body.mass * playerVehicle.body.speed + trafficVehicle.body.mass * trafficVehicle.body.speed)
													/(playerVehicle.body.mass + trafficVehicle.body.mass);
				playerVehicle.isCrashing = true;
			}
		}
	}
}

void Pseudo3DRaceState::shiftGear(int gear)
{
	// todo play gear shift sound
	playerVehicle.body.shiftGear(gear);
}

Pseudo3DRaceState::SurfaceType Pseudo3DRaceState::getCurrentSurfaceType()
{
	if(fabs(playerVehicle.horizontalPosition * coursePositionFactor) > 1.2*course.spec.roadWidth)
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
