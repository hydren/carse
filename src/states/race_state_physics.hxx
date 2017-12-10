/*
 * race_state_physics.hxx
 *
 *  Created on: 29 de ago de 2017
 *      Author: carlosfaruolo
 */

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
	const Course::Segment& segment = course.lines[((int)(vehicle.position*coursePositionFactor/course.roadSegmentLength))%course.lines.size()];
	const float wheelAngleFactor = 1 - vehicle.corneringForceLeechFactor*fabs(vehicle.pseudoAngle)/PSEUDO_ANGLE_MAX;

	vehicle.body.tireFrictionFactor = getTireKineticFrictionCoefficient();
	vehicle.body.rollingResistanceFactor = getTireRollingResistanceCoefficient();
	vehicle.body.arbitraryForceFactor = wheelAngleFactor;
	vehicle.body.slopeAngle = atan2(segment.y - vehicle.verticalPosition, course.roadSegmentLength);

	vehicle.body.engine.throttlePosition = isPlayerAccelerating()? 1.0 : 0.0;
	vehicle.body.brakePedalPosition = isPlayerBraking()? 1.0 : 0.0;
	vehicle.body.updatePowertrain(delta);

	// update position
	vehicle.position += vehicle.body.speed*delta;

	// update steering
	if(isPlayerSteeringRight() and fabs(vehicle.body.speed) >= MINIMUM_SPEED_ALLOW_TURN)
	{
		if(vehicle.pseudoAngle < 0) vehicle.pseudoAngle *= 1/(1+5*delta);
		vehicle.pseudoAngle += delta * STEERING_SPEED;
	}
	else if(isPlayerSteeringLeft() and fabs(vehicle.body.speed) >= MINIMUM_SPEED_ALLOW_TURN)
	{
		if(vehicle.pseudoAngle > 0) vehicle.pseudoAngle *= 1/(1+5*delta);
		vehicle.pseudoAngle -= delta * STEERING_SPEED;
	}
	else vehicle.pseudoAngle *= 1/(1+5*delta);

	if(vehicle.pseudoAngle > PSEUDO_ANGLE_MAX) vehicle.pseudoAngle = PSEUDO_ANGLE_MAX;
	if(vehicle.pseudoAngle <-PSEUDO_ANGLE_MAX) vehicle.pseudoAngle =-PSEUDO_ANGLE_MAX;

	// update strafing
	vehicle.strafeSpeed = vehicle.pseudoAngle * vehicle.body.speed * vehicle.corneringStiffness * coursePositionFactor;
//	vehicle.strafeSpeed = onAir? 0 : vehicle.pseudoAngle * vehicle.body.speed * coursePositionFactor;

	// limit strafing speed by magic constant
	if(vehicle.strafeSpeed >  MAXIMUM_STRAFE_SPEED * vehicle.corneringStiffness) vehicle.strafeSpeed = MAXIMUM_STRAFE_SPEED * vehicle.corneringStiffness;
	if(vehicle.strafeSpeed < -MAXIMUM_STRAFE_SPEED * vehicle.corneringStiffness) vehicle.strafeSpeed =-MAXIMUM_STRAFE_SPEED * vehicle.corneringStiffness;

	// update curve pull
	//curvePull = segment.curve * vehicle.body.speed * coursePositionFactor * CURVE_PULL_FACTOR;
	vehicle.curvePull = sin(atan2(segment.curve*50, course.roadSegmentLength));
	vehicle.curvePull *= vehicle.body.speed * coursePositionFactor;

	// update strafe position
	vehicle.horizontalPosition += (vehicle.strafeSpeed - vehicle.curvePull)*delta;

	// update vertical position
	vehicle.verticalPosition = segment.y;

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
	parallax.x -= segment.curve*vehicle.body.speed*0.025;
	parallax.y -= 2*vehicle.body.slopeAngle;

	if(parallax.x < -(2.0f*imgBackground->getWidth()-game.getDisplay().getWidth()))
		parallax.x += imgBackground->getWidth();

	if(parallax.x > 0)
		parallax.x -= imgBackground->getWidth();
}

static const float RAD_TO_RPM = (30.0/M_PI);  // 60/2pi conversion to RPM

void Pseudo3DRaceState::shiftGear(int gear)
{
	if(gear < 0 or gear > vehicle.body.engine.gearCount)
		return;

	if(gear != 0 and vehicle.body.engine.gear != 0)
	{
		const float driveshaftRpm = vehicle.body.wheelAngularSpeed
									* vehicle.body.engine.gearRatio[gear-1]
									* vehicle.body.engine.differentialRatio * RAD_TO_RPM;

		// add a portion of the discrepancy between the driveshaft RPM and the engine RPM (simulate losses due to shift time)
		vehicle.body.engine.rpm += 0.25*(driveshaftRpm - vehicle.body.engine.rpm);
	}

	vehicle.body.engine.gear = gear;
}

Pseudo3DRaceState::SurfaceType Pseudo3DRaceState::getCurrentSurfaceType()
{
	if(fabs(vehicle.horizontalPosition) > 1.2*course.roadWidth)
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
