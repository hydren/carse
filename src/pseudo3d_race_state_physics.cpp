/*
 * race_state_physics.cpp
 *
 *  Created on: 24 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "pseudo3d_race_state.hpp"

#include "carse_game.hpp"

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T> T pow2(T val) {
	return val*val;
}

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

static const float TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT = 0.85,
				   TIRE_FRICTION_COEFFICIENT_GRASS = 0.42,
				   ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT = 0.013,
				   ROLLING_RESISTANCE_COEFFICIENT_GRASS = 0.100,
				   COLLISION_RESTITUTION_COEFFICIENT = 0.5,

				   PLAYER_VEHICLE_PROJECTION_OFFSET = 6.0,  // needed since the player vehicle sprite is projected "ahead" of its actual position

				   PSEUDO_ANGLE_MAX = 1.0,
				   STEERING_SPEED = 2.0,
				   MINIMUM_SPEED_ALLOW_TURN = 1.0/36.0;  // == 1kph

void Pseudo3DRaceState::handlePhysics(float delta)
{
	const unsigned courseSegmentIndex = static_cast<int>(playerVehicle.position * coursePositionFactor / course.spec.roadSegmentLength) % course.spec.lines.size();
	const CourseSpec::Segment& courseSegment = course.spec.lines[courseSegmentIndex];
	const float corneringForceLeechFactor = (playerVehicle.body.vehicleType == Mechanics::TYPE_BIKE? 0.25 : 0.5),
				wheelAngleFactor = 1 - corneringForceLeechFactor*fabs(playerVehicle.pseudoAngle)/PSEUDO_ANGLE_MAX,
				maxStrafeSpeed = MAXIMUM_STRAFE_SPEED_FACTOR * coursePositionFactor * playerVehicle.corneringStiffness;

	playerVehicle.body.tireFrictionFactor = getTireKineticFrictionCoefficient();
	playerVehicle.body.rollingResistanceFactor = getTireRollingResistanceCoefficient();
	playerVehicle.body.arbitraryForceFactor = wheelAngleFactor;
	playerVehicle.body.slopeAngle = atan2(courseSegment.y - playerVehicle.verticalPosition, course.spec.roadSegmentLength);

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
	playerVehicle.strafeSpeed = playerVehicle.pseudoAngle * playerVehicle.body.speed * coursePositionFactor;

//	if(onAir) playerVehicle.strafeSpeed = 0;

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
	playerVehicle.curvePull = coursePositionFactor * pow2(playerVehicle.body.speed) * curvatureFactor / course.spec.roadSegmentLength;

	// update strafe position
	playerVehicle.horizontalPosition += (playerVehicle.strafeSpeed - playerVehicle.curvePull)*delta;

	// update vertical position
	playerVehicle.verticalPosition = courseSegment.y;

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
	parallax.x -= courseSegment.curve*playerVehicle.body.speed*0.025;
	parallax.y -= 2*playerVehicle.body.slopeAngle;

	if(parallax.x < -(2.0f*imgBackground->getWidth()-game.getDisplay().getWidth()))
		parallax.x += imgBackground->getWidth();

	if(parallax.x > 0)
		parallax.x -= imgBackground->getWidth();

	foreach(Pseudo3DVehicle&, trafficVehicle, std::vector<Pseudo3DVehicle>, trafficVehicles)
	{
		trafficVehicle.body.rollingResistanceFactor = ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT;
		trafficVehicle.body.tireFrictionFactor = TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT;
		trafficVehicle.body.updatePowertrain(delta);
		trafficVehicle.position += coursePositionFactor*trafficVehicle.body.speed*delta;  // update position

		const unsigned trafficVehicleCourseSegmentIndex = static_cast<int>((trafficVehicle.position - PLAYER_VEHICLE_PROJECTION_OFFSET * coursePositionFactor) / course.spec.roadSegmentLength) % course.spec.lines.size();
		if(trafficVehicleCourseSegmentIndex == courseSegmentIndex)  // if on the same segment, check for collision
		{
			const float pw = playerVehicle.spriteSpec.depictedVehicleWidth * playerVehicle.sprites.back()->scale.x * 7,
						px = playerVehicle.horizontalPosition - 0.5f*pw,
						tw = trafficVehicle.spriteSpec.depictedVehicleWidth * trafficVehicle.sprites.back()->scale.x * 7,
						tx = trafficVehicle.horizontalPosition*coursePositionFactor - 0.5f*tw;

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
