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
#include <cfloat>

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

	const Course::Segment& segment = course.lines[((int)(position*coursePositionFactor/course.roadSegmentLength))%course.lines.size()];
	const float wheelAngleFactor = 1 - corneringForceLeechFactor*fabs(pseudoAngle)/PSEUDO_ANGLE_MAX;
	slopeAngle = atan2(segment.y - posY, course.roadSegmentLength);

	const float tireFriction = getTireKineticFrictionCoefficient() * vehicle.mass * GRAVITY_ACCELERATION * sgn(vehicle.speed);
	brakingFriction = vehicle.brakePedalPosition * tireFriction;  // a multiplier here could be added for stronger and easier braking
	rollingFriction = getTireRollingResistanceCoefficient() * vehicle.mass * GRAVITY_ACCELERATION * sgn(vehicle.speed) * 4;  // fixme this formula assumes 4 wheels,
	slopePull = vehicle.mass * GRAVITY_ACCELERATION * sin(slopeAngle),
	airFriction = 0.5 * AIR_DENSITY * AIR_FRICTION_COEFFICIENT * squared(vehicle.speed) * AIR_FRICTION_ARBITRARY_ADJUST;

	// update acceleration
	vehicle.acceleration = ((wheelAngleFactor*getDriveForce() - slopePull - brakingFriction - rollingFriction) - airFriction)/vehicle.mass;
//	vehicle.acceleration = ((onAir? 0 : slopeFactor*wheelAngleFactor*getDriveForce() - brakingFriction - rollingFriction) - airFriction)/vehicle.mass;

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
//	strafeSpeed = onAir? 0 : pseudoAngle * vehicle.speed * coursePositionFactor;

	// limit strafing speed by tire friction
//	if(strafeSpeed >  tireFriction) strafeSpeed = tireFriction;
//	if(strafeSpeed < -tireFriction) strafeSpeed =-tireFriction;

	// limit strafing speed by magic constant
	if(strafeSpeed >  MAXIMUM_STRAFE_SPEED) strafeSpeed = MAXIMUM_STRAFE_SPEED;
	if(strafeSpeed < -MAXIMUM_STRAFE_SPEED) strafeSpeed =-MAXIMUM_STRAFE_SPEED;

	// update curve pull
	curvePull = segment.curve * vehicle.speed * coursePositionFactor * CURVE_PULL_FACTOR;

	// update strafe position
	posX += (strafeSpeed - curvePull)*delta;

	// update vertical position
	posY = segment.y;

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
	bgParallax.x -= segment.curve*vehicle.speed*0.025;
	bgParallax.y = -segment.y*0.01;

	if(bgParallax.x < -(2.0f*bg->getWidth()-game.getDisplay().getWidth()))
		bgParallax.x += bg->getWidth();

	if(bgParallax.x > 0)
		bgParallax.x -= bg->getWidth();
}

static const float RAD_TO_RPM = (30.0/M_PI);  // 60/2pi conversion to RPM

void Pseudo3DRaceState::shiftGear(int gear)
{
	if(gear < 0 or gear > vehicle.engine.gearCount)
		return;

	const float angularSpeedDiscrepancy = vehicle.wheelAngularSpeed * vehicle.engine.gearRatio[gear-1] * vehicle.engine.differentialRatio * RAD_TO_RPM - vehicle.engine.rpm;
	vehicle.engine.rpm += 0.25*angularSpeedDiscrepancy;
	vehicle.engine.gear = gear;
}

Pseudo3DRaceState::SurfaceType Pseudo3DRaceState::getCurrentSurfaceType()
{
	if(fabs(posX) > 1.2*course.roadWidth)
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

/*
 * More info about car physics
 * http://www.asawicki.info/Mirror/Car%20Physics%20for%20Games/Car%20Physics%20for%20Games.html
 * http://vehiclephysics.com/advanced/misc-topics-explained/#tire-friction
 * https://en.wikipedia.org/wiki/Friction#Coefficient_of_friction
 * https://en.wikipedia.org/wiki/Downforce
 * http://www3.wolframalpha.com/input/?i=r(v,+w)+%3D+(w+-+v)%2Fv
 * http://www.edy.es/dev/2011/12/facts-and-myths-on-the-pacejka-curves/
 * http://white-smoke.wikifoundry.com/page/Tyre+curve+fitting+and+validation

 	What to do when slip ratio is unstable (low speeds)? Options include:
 	 - Use non-sliping, simplified formula.
 	 - Assume zero traction torque.
 	 - Use Vinicius Martins idea to tamper the speed to maintain it at a minimum stable value, when it's not zero.
 	 - ... (research another formula).

 	When is the slip ratio unstable, exactly?
 	- Low speed. See PACEJKA_MAGIC_FORMULA_LOWER_SPEED_THRESHOLD
 	- Low slip ratio. See LOWEST_STABLE_SLIP
 	- ... (don't know, really)

 	Needs neutral gear implementation, specially to avoid powershifting in slip ratio model.
 	Also needs a proper shift time. Specially when using slip ratio mode because, without it, the vehicle appears to be powershifting.
*/
static const float PACEJKA_MAGIC_FORMULA_LOWER_SPEED_THRESHOLD = 22;  // 22m/s == 79,2km/h
static const double LOWEST_STABLE_SLIP = 500.0*DBL_EPSILON;  // increasing this constant degrades simulation quality

//xxx An estimated wheel (tire+rim) density. (33cm radius or 660mm diameter tire with 75kg mass). Actual value varies by tire (brand, weight, type, etc) and rim (brand , weight, shape, material, etc)
static const float AVERAGE_WHEEL_DENSITY = 75.0/squared(3.3);  // d = m/r^2, assuming wheel width = 1/PI in the original formula d = m/(PI * r^2 * width)

// ==================================================================================================================================================
// simple model
#ifdef TIRE_MODEL_SIMPLE

void Pseudo3DRaceState::updateDrivetrain(float delta)
{
	// this formula assumes no wheel slipping.
	vehicle.wheelAngularSpeed = vehicle.speed/vehicle.tireRadius;  // set new wheel angular speed
	vehicle.engine.update(delta, vehicle.wheelAngularSpeed);
}

float Pseudo3DRaceState::getDriveForce()
{
	// all engine power
//	return vehicle.engine.getDriveTorque() / vehicle.tireRadius;

	// all engine power, up to the maximum allowed by tire grip (driven wheels load)
	return std::min(vehicle.engine.getDriveTorque() / vehicle.tireRadius, getDrivenWheelsTireLoad() * getTireKineticFrictionCoefficient());
}

#endif
// ==================================================================================================================================================
// slip ratio model
#ifdef TIRE_MODEL_SLIP_RATIO

void Pseudo3DRaceState::updateDrivetrain(float delta)
{
	const bool unstable = isSlipRatioUnstable();
	if(unstable)  // assume no tire slipping when unstable
	{
		// xxx this formula assumes no wheel slipping.
		vehicle.engine.update(delta, vehicle.speed/vehicle.tireRadius);  // set new wheel angular speed
		return;
	}

	const unsigned drivenWheelsCount = (vehicle.type == Vehicle::TYPE_CAR? 4 : vehicle.type == Vehicle::TYPE_BIKE? 2 : 1) * (vehicle.drivenWheels != Vehicle::DRIVEN_WHEELS_ALL? 0.5f : 1.0f);
	const float wheelMass = AVERAGE_WHEEL_DENSITY * squared(vehicle.tireRadius);  // m = d*r^2, assuming wheel width = 1/PI
	const float drivenWheelsInertia = drivenWheelsCount * wheelMass * squared(vehicle.tireRadius) * 0.5;  // I = (mr^2)/2

//	const float tractionForce = unstable? 0 : getNormalizedTractionForce() * getDrivenWheelsTireLoad();  // assume zero traction when unstable
	const float tractionForce = getNormalizedTractionForce() * getTireKineticFrictionCoefficient() * getDrivenWheelsTireLoad();
	const float tractionTorque = tractionForce * vehicle.tireRadius;

	//fixme how to do this formula right? remove from ingame state braking calculation
//	const float brakingTorque = -brakePedalPosition*30;
	const float brakingTorque = 0;

	const float totalTorque = vehicle.engine.getDriveTorque() - tractionTorque + brakingTorque;

	const float arbitraryAdjustmentFactor = 0.001;
	const float wheelAngularAcceleration = arbitraryAdjustmentFactor * (totalTorque / drivenWheelsInertia);  // xxx we're assuming no inertia from the engine components.

	vehicle.engine.update(delta, vehicle.engine.getAngularSpeed() + delta * wheelAngularAcceleration);  // set new wheel angular speed
}

float Pseudo3DRaceState::getDriveForce()
{
	if(isSlipRatioUnstable() or vehicle.speed == 0)
	{
		cout << "using drive torque: " << (vehicle.engine.getDriveTorque() / vehicle.tireRadius) << endl ;
		// returns the current drive force as the one directly from the engine, up to the driven wheels tire load (prevent tire slip altogether)
		return std::min(vehicle.engine.getDriveTorque() / vehicle.tireRadius, getDrivenWheelsTireLoad());
	}
	else
	{
		cout << "using traction torque. driven wheels tire load: " << getDrivenWheelsTireLoad() << endl ;
		// returns the current drive force as the longitudinal/traction force
		return getNormalizedTractionForce() * getDrivenWheelsTireLoad() * getTireKineticFrictionCoefficient();
	}
}

float Pseudo3DRaceState::getLongitudinalSlipRatio()
{
//	return fabs(speed)==0? 0 : (engine.getAngularSpeed()*tireRadius)/fabs(speed) - 1.0;
	return fabs(vehicle.speed)==0? 0 : ((double) vehicle.engine.getAngularSpeed() * (double) vehicle.tireRadius - (double) vehicle.speed)/fabs(vehicle.speed);
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

bool Pseudo3DRaceState::isSlipRatioUnstable()
{
	// assume unstable if speed is this slow (actual speed varies with vehicle's characteristics)
//	return (vehicle.speed < PACEJKA_MAGIC_FORMULA_LOWER_SPEED_THRESHOLD);

	// assume unstable if slip ratio is too low
	return (vehicle.engine.getAngularSpeed()*vehicle.tireRadius - vehicle.speed < LOWEST_STABLE_SLIP*fabs(vehicle.speed));
}

#endif
// ==================================================================================================================================================

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

	return load*cos(slopeAngle);
}
