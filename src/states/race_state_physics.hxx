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

static const float FR_DRIVEN_WHEELS_LOAD = 0.45,
				   MR_DRIVEN_WHEELS_LOAD = 0.55,
				   RR_DRIVEN_WHEELS_LOAD = 0.65,
				   FF_DRIVEN_WHEELS_LOAD = 0.60;

void Pseudo3DRaceState::handlePhysics(float delta)
{
	vehicle.engine.throttlePosition = Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP)? 1.0 : 0.0;
	vehicle.brakePedalPosition =  Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN)? 1.0 : 0.0;
	updateDrivetrain(delta);

	const float wheelAngleFactor = 1 - corneringForceLeechFactor*fabs(pseudoAngle)/PSEUDO_ANGLE_MAX;

	const bool onGrass = (fabs(posX) > 1.2*course.roadWidth);
	const float tireFrictionCoefficient = onGrass? TIRE_FRICTION_COEFFICIENT_GRASS : TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT,
				rollingResistanceCoefficient = onGrass? ROLLING_RESISTANCE_COEFFICIENT_GRASS : ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT;

	const float tireFriction = tireFrictionCoefficient * vehicle.mass * GRAVITY_ACCELERATION * sgn(vehicle.speed);
	brakingFriction = vehicle.brakePedalPosition * tireFriction;  // a multiplier here could be added for stronger and easier braking
	rollingFriction = rollingResistanceCoefficient * vehicle.mass * GRAVITY_ACCELERATION * sgn(vehicle.speed) * 4;  // fixme this formula assumes 4 wheels
	airFriction = 0.5 * AIR_DENSITY * AIR_FRICTION_COEFFICIENT * squared(vehicle.speed) * AIR_FRICTION_ARBITRARY_ADJUST;

	// update acceleration
	vehicle.acceleration = (wheelAngleFactor*getDriveForce() - brakingFriction - rollingFriction - airFriction)/vehicle.mass;

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

	const Course::Segment& segment = course.lines[((int)(position*coursePositionFactor/course.roadSegmentLength))%course.lines.size()];

	// update curve pull
	curvePull = segment.curve * vehicle.speed * coursePositionFactor * CURVE_PULL_FACTOR;

	// update strafe position
	posX += (strafeSpeed - curvePull)*delta;

	// update bg parallax
	bgParallax.x += segment.curve*vehicle.speed*0.025;
	bgParallax.y = -segment.y*0.01;

	if(bgParallax.x < -(2.0f*bg->getWidth()-game.getDisplay().getWidth()))
		bgParallax.x += bg->getWidth();

	if(bgParallax.x > 0)
		bgParallax.x -= bg->getWidth();
}

/*
 * More info
 * http://www.asawicki.info/Mirror/Car%20Physics%20for%20Games/Car%20Physics%20for%20Games.html
 * http://vehiclephysics.com/advanced/misc-topics-explained/#tire-friction
 * https://en.wikipedia.org/wiki/Friction#Coefficient_of_friction
 * https://en.wikipedia.org/wiki/Downforce
 * http://www3.wolframalpha.com/input/?i=r(v,+w)+%3D+(w+-+v)%2Fv
 * http://www.edy.es/dev/2011/12/facts-and-myths-on-the-pacejka-curves/
 * http://white-smoke.wikifoundry.com/page/Tyre+curve+fitting+and+validation
*/
static const float PACEJKA_MAGIC_FORMULA_LOWER_SPEED_THRESHOLD = 22;  // 22m/s == 79,2km/h

//xxx An estimated wheel (tire+rim) density. (33cm radius or 660mm diameter tire with 75kg mass). Actual value varies by tire (brand, weight, type, etc) and rim (brand , weight, shape, material, etc)
static const float AVERAGE_WHEEL_DENSITY = 75.0/squared(3.3);  // d = m/r^2, assuming wheel width = 1/PI in the original formula d = m/(PI * r^2 * width)

void Pseudo3DRaceState::updateDrivetrain(float delta)
{
	if(vehicle.speed < PACEJKA_MAGIC_FORMULA_LOWER_SPEED_THRESHOLD)
	{
		// xxx this formula assumes no wheel spin.
		vehicle.engine.update(vehicle.speed/vehicle.tireRadius);  // set new wheel angular speed
		return;
	}

	const unsigned drivenWheelsCount = (vehicle.type == Vehicle::TYPE_CAR? 4 : vehicle.type == Vehicle::TYPE_BIKE? 2 : 1) * (vehicle.drivenWheels != Vehicle::DRIVEN_WHEELS_ALL? 0.5f : 1.0f);
	const float wheelMass = AVERAGE_WHEEL_DENSITY * squared(vehicle.tireRadius);  // m = d*r^2, assuming wheel width = 1/PI
	const float drivenWheelsInertia = drivenWheelsCount * wheelMass * squared(vehicle.tireRadius) * 0.5;  // I = (mr^2)/2

	const float tractionForce = getNormalizedTractionForce() * getDrivenWheelsTireLoad();
	const float tractionTorque = tractionForce / vehicle.tireRadius;

	//fixme how to do this formula right? remove from ingame state braking calculation
//	const float brakingTorque = -brakePedalPosition*30;
	const float brakingTorque = 0;

	const float totalTorque = vehicle.engine.getDriveTorque() - tractionTorque + brakingTorque;

	const float arbitraryAdjustmentFactor = 0.001;
	const float wheelAngularAcceleration = arbitraryAdjustmentFactor * (totalTorque / drivenWheelsInertia);  // xxx we're assuming no inertia from the engine components.

	vehicle.engine.update(vehicle.engine.getAngularSpeed() + delta * wheelAngularAcceleration);  // set new wheel angular speed
}

float Pseudo3DRaceState::getLongitudinalSlipRatio()
{
//	return fabs(speed)==0? 0 : (engine.getAngularSpeed()*tireRadius)/fabs(speed) - 1.0;
	return fabs(vehicle.speed)==0? 0 : (vehicle.engine.getAngularSpeed()*vehicle.tireRadius - vehicle.speed)/fabs(vehicle.speed);
}

float Pseudo3DRaceState::getNormalizedTractionForce()
{
	// based on a simplified Pacejka's formula from Marco Monster's website "Car Physics for Games".
	// this formula don't work properly on low speeds (numerical instability)
	const float longitudinalSlipRatio = getLongitudinalSlipRatio();
	return longitudinalSlipRatio < 0.06? (20.0*longitudinalSlipRatio)  // 0 to 6% slip ratio gives traction from 0 up to 120%
			: longitudinalSlipRatio < 0.20? (9.0 - 10.0*longitudinalSlipRatio)/7.0  // 6 to 20% slip ratio gives traction from 120% up to 100%
					: longitudinalSlipRatio < 1.00? (1.075 - 0.375*longitudinalSlipRatio)  // 20% to 100% slip ratio gives traction from 100 down to 70%
							: 0.7;  // over 100% slip ratio gives traction 70%
}

/** Returns the current driving force. */
float Pseudo3DRaceState::getDriveForce()
{
	if(vehicle.speed < PACEJKA_MAGIC_FORMULA_LOWER_SPEED_THRESHOLD)
		return std::min(vehicle.engine.getDriveTorque() / vehicle.tireRadius, getDrivenWheelsTireLoad());
	else
		return vehicle.engine.getDriveTorque() / vehicle.tireRadius;
}

static const float engineLocationFactorRWD(Vehicle::EngineLocation loc)
{
	if(loc == Vehicle::ENGINE_LOCATION_ON_REAR) return RR_DRIVEN_WHEELS_LOAD;
	if(loc == Vehicle::ENGINE_LOCATION_ON_MIDDLE) return MR_DRIVEN_WHEELS_LOAD;
	else return FR_DRIVEN_WHEELS_LOAD;
}

float Pseudo3DRaceState::getDrivenWheelsTireLoad()
{
	const float loadTransfered =  vehicle.mass * vehicle.acceleration * (vehicle.approximatedCenterOfGravityHeight/vehicle.approximatedWheelbase);
	float load = vehicle.mass*GRAVITY_ACCELERATION;

	if(vehicle.drivenWheels == Vehicle::DRIVEN_WHEELS_ON_REAR)
		load = engineLocationFactorRWD(vehicle.engineLocation)*load + loadTransfered;

	else if(vehicle.drivenWheels == Vehicle::DRIVEN_WHEELS_ON_FRONT)
		load = FF_DRIVEN_WHEELS_LOAD*load - loadTransfered;

	return load;
}
