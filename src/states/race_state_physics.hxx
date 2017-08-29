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

#ifdef sgn
	#undef sgn
#endif
#define sgn(x) (x > 0 ? 1 : x < 0 ? -1 : 0)

#ifdef squared
	#undef squared
#endif
#define squared(x) ((x)*(x))

// these arbitrary multipliers don't have foundation in physics, but serves as a fine-tuning for the gameplay
#define AIR_FRICTION_ARBITRARY_ADJUST 0.8
#define ROLLING_FRICTION_ARBITRARY_ADJUST 2.5

static const float GRAVITY_ACCELERATION = 9.8066; // standard gravity (actual value varies with altitude, from 9.7639 to 9.8337)
static const float AIR_DENSITY = 1.2041;  // at sea level, 20ºC (68ºF) (but actually varies significantly with altitude, temperature and humidity)
static const float AIR_FRICTION_COEFFICIENT = 0.31 * 1.81;  // CdA. Hardcoded values are: 0.31 drag coefficient (Cd) and 1.81m2 reference/frontal area (A) of a Nissan 300ZX (Z32)
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
	vehicle.engine.throttlePosition = Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP)? 1.0 : 0.0;
	vehicle.brakePedalPosition =  Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN)? 1.0 : 0.0;
	vehicle.update(delta);

	const float wheelAngleFactor = 1 - corneringForceLeechFactor*fabs(pseudoAngle)/PSEUDO_ANGLE_MAX;

	const bool onGrass = (fabs(posX) > 1.2*course.roadWidth);
	const float tireFrictionCoefficient = onGrass? TIRE_FRICTION_COEFFICIENT_GRASS : TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT,
				rollingResistanceCoefficient = onGrass? ROLLING_RESISTANCE_COEFFICIENT_GRASS : ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT;

	const float tireFriction = tireFrictionCoefficient * vehicle.mass * GRAVITY_ACCELERATION * sgn(vehicle.speed);
	brakingFriction = vehicle.brakePedalPosition * tireFriction;  // a multiplier here could be added for stronger and easier braking
	rollingFriction = rollingResistanceCoefficient * vehicle.mass * GRAVITY_ACCELERATION * sgn(vehicle.speed) * 4;  // fixme this formula assumes 4 wheels
	airFriction = 0.5 * AIR_DENSITY * AIR_FRICTION_COEFFICIENT * squared(vehicle.speed) * AIR_FRICTION_ARBITRARY_ADJUST;

	// update acceleration
	vehicle.acceleration = (wheelAngleFactor*vehicle.getDriveForce() - brakingFriction - rollingFriction - airFriction)/vehicle.mass;

	// update speed
	vehicle.speed += delta*vehicle.acceleration;

	// update position
	position += vehicle.speed*delta;

	// update steering
	if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_RIGHT) and fabs(vehicle.speed) >= MINIMUM_SPEED_ALLOW_TURN)
	{
		if(pseudoAngle < 0) pseudoAngle *= 1/(1+5*delta);
		pseudoAngle += delta * STEERING_SPEED;
	}
	else if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_LEFT) and fabs(vehicle.speed) >= MINIMUM_SPEED_ALLOW_TURN)
	{
		if(pseudoAngle > 0) pseudoAngle *= 1/(1+5*delta);
		pseudoAngle -= delta * STEERING_SPEED;
	}
	else pseudoAngle *= 1/(1+5*delta);

	if(pseudoAngle > PSEUDO_ANGLE_MAX) pseudoAngle = PSEUDO_ANGLE_MAX;
	if(pseudoAngle <-PSEUDO_ANGLE_MAX) pseudoAngle =-PSEUDO_ANGLE_MAX;

	// update strafing
	strafeSpeed = pseudoAngle * vehicle.speed * coursePositionFactor;

	// limit strafing speed by tire friction
//	if(strafeSpeed >  tireFriction) strafeSpeed = tireFriction;
//	if(strafeSpeed < -tireFriction) strafeSpeed =-tireFriction;

	// limit strafing speed by magic constant
	if(strafeSpeed >  MAXIMUM_STRAFE_SPEED) strafeSpeed = MAXIMUM_STRAFE_SPEED;
	if(strafeSpeed < -MAXIMUM_STRAFE_SPEED) strafeSpeed =-MAXIMUM_STRAFE_SPEED;

	const unsigned N = course.lines.size();
	const Course::Segment& segment = course.lines[((int)(position*coursePositionFactor/course.roadSegmentLength))%N];

	// update curve pull
	curvePull = segment.curve * vehicle.speed * coursePositionFactor * CURVE_PULL_FACTOR;

	// update strafe position
	posX += (strafeSpeed - curvePull)*delta;

	// update bg parallax
	bgParallax.x += segment.curve*vehicle.speed*0.025;
	bgParallax.y = -segment.y*0.01;

	if(bgParallax.x < -(2.0f*bg->getWidth()-Display::getInstance().getWidth()))
		bgParallax.x += bg->getWidth();

	if(bgParallax.x > 0)
		bgParallax.x -= bg->getWidth();

	// course looping control
	while(position * coursePositionFactor >= N*course.roadSegmentLength) position -= N*course.roadSegmentLength / coursePositionFactor;
	while(position < 0) position += N*course.roadSegmentLength / coursePositionFactor;
}
