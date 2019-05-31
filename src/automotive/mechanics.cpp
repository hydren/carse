/*
 * mechanics.cpp
 *
 *  Created on: 9 de out de 2017
 *      Author: carlosfaruolo
 */

#include "mechanics.hpp"

#include "futil/math_constants.h"

#include <algorithm>

#include <cmath>
#include <cfloat>

#ifdef sgn
	#undef sgn
#endif
#define sgn(x) (x > 0 ? 1 : x < 0 ? -1 : 0)

#define pow2(x) ((x)*(x))

// air friction and downforce arbitrary multipliers. don't have foundation in physics, but serves as a fine-tuning for the gameplay
#define AIR_DRAG_ARBITRARY_ADJUST 0.8
#define DOWNFORCE_ARBITRATY_ADJUST 0.5
#define BRAKE_PAD_TORQUE_ARBITRARY_ADJUST 7500
#define ROLLING_RESISTANCE_TORQUE_ARBITRARY_ADJUST 0.25

const float Mechanics::GRAVITY_ACCELERATION = 9.8066, // standard gravity (actual value varies with altitude, from 9.7639 to 9.8337)
			Mechanics::RAD_TO_RPM = (30.0/M_PI),  // 60/2pi conversion to RPM
			Mechanics::AIR_DENSITY = 1.2041;  // air density at sea level, 20ºC (68ºF) (but actually varies significantly with altitude, temperature and humidity)

Mechanics::Mechanics(const Engine& eng, VehicleType type, float dragArea, float liftArea)
: simulationType(SIMULATION_TYPE_WHEEL_LOAD_CAP), vehicleType(type), engine(eng),
  automaticShiftingEnabled(), automaticShiftingLastTime(0),
  mass(1250), tireRadius(650), wheelCount(type == TYPE_CAR? 4 : type == TYPE_BIKE? 2 : 1),
  speed(), acceleration(),
  centerOfGravityHeight(500), wheelbase(1000), weightDistribution(0.5),
  slopeAngle(), wheelAngularSpeed(), brakePedalPosition(),
  tireFrictionFactor(1.0),
  rollingResistanceFactor(0.2),
  airDragFactor(dragArea*AIR_DENSITY),
  downforceFactor(liftArea*AIR_DENSITY),
  drivenWheelsType(DRIVEN_WHEELS_ON_REAR),
  engineLocation(ENGINE_LOCATION_ON_FRONT),
  rollingResistanceForce(), airDragForce(), brakingForce(), slopePullForce(), downforce(),
  arbitraryForceFactor(1.0),
  slipRatio(), differentialSlipRatio()
{}

void Mechanics::reset()
{
	engine.reset();
	slopeAngle = 0;
	arbitraryForceFactor = 1.0;
	acceleration = 0;
	speed = 0;
	wheelAngularSpeed = 0;
	brakePedalPosition = 0;
	slipRatio = 0;
	differentialSlipRatio = 0;
	brakingForce = 0;
	rollingResistanceForce = 0;
	slopePullForce = 0;
	airDragForce = 0;
	downforce = 0;
	slipRatio = differentialSlipRatio = 0;
}

void Mechanics::updatePowertrain(float delta)
{
	if(automaticShiftingEnabled and slipRatio < 0.1 and automaticShiftingLastTime > 1.0)
	{
		const int nextGear = engine.gear+1, prevGear = engine.gear-1;
		bool shifted = false, stagedThrottle = false;
		if(engine.throttlePosition == 0)
		{
			engine.throttlePosition = 1.0;
			stagedThrottle = true;
		}
		if(nextGear-1 < engine.gearCount)
		{
			const float nextGearRpm = wheelAngularSpeed * engine.gearRatio[nextGear-1] * engine.differentialRatio * RAD_TO_RPM;
			if(nextGearRpm > 0)
			{
				const float nextGearDriveTorque = engine.getTorqueAt(nextGearRpm) * engine.gearRatio[nextGear-1] * engine.differentialRatio * engine.transmissionEfficiency;
				if(engine.getDriveTorque() < nextGearDriveTorque)
				{
					if(stagedThrottle)
						engine.throttlePosition = 0;
					shiftGear(nextGear);
					automaticShiftingLastTime = 0;
					shifted = true;
				}
			}
		}
		if(not shifted and prevGear > 0)
		{
			const float prevGearRpm = wheelAngularSpeed * engine.gearRatio[prevGear-1] * engine.differentialRatio * RAD_TO_RPM;
			if(prevGearRpm < engine.maxRpm)
			{
				const float prevGearDriveTorque = engine.getTorqueAt(prevGearRpm) * engine.gearRatio[prevGear-1] * engine.differentialRatio * engine.transmissionEfficiency;
				if(engine.getDriveTorque() < 0.9*prevGearDriveTorque)
				{
					if(stagedThrottle)
						engine.throttlePosition = 0;
					shiftGear(prevGear);
					automaticShiftingLastTime = 0;
					shifted = true;
				}
			}
		}
		if(not shifted and stagedThrottle)
			engine.throttlePosition = 0;
	}
	automaticShiftingLastTime += delta;

	const float weight = mass * GRAVITY_ACCELERATION;
	brakingForce = brakePedalPosition * tireFrictionFactor * weight * sgn(speed);  // a multiplier here could be added for stronger and easier braking
	rollingResistanceForce = rollingResistanceFactor * weight * sgn(speed);  // rolling friction is independant on wheel count since the weight will be divided between them
	slopePullForce = weight * sin(slopeAngle);
	airDragForce = 0.5 * airDragFactor * pow2(speed) * AIR_DRAG_ARBITRARY_ADJUST;

	// update downforce
	downforce = 0.5 * downforceFactor * pow2(speed) * DOWNFORCE_ARBITRATY_ADJUST;

	// update drivetrain
	if(simulationType == SIMULATION_TYPE_PACEJKA_BASED)
		updateByPacejkaScheme(delta);
	else
		updateBySimplifiedScheme(delta);

	// compute total net force
	float totalForce = (
		// drive force is ready to be get AFTER updating drivetrain
		arbitraryForceFactor*getDriveForce()

		// discounts for slope gravity pull and air drag friction
		- (slopePullForce + airDragForce)

		// pacejka scheme already accounts for braking force and rolling resistance force
		- (simulationType != SIMULATION_TYPE_PACEJKA_BASED? (brakingForce + rollingResistanceForce) : 0)
	);

	// update acceleration
	acceleration = totalForce/mass;  // divide forces by mass

	// update speed
	speed += delta*acceleration;
}

void Mechanics::shiftGear(int gear)
{
	if(gear < 0 or gear > engine.gearCount)
		return;

	if(gear != 0 and engine.gear != 0)
	{
		const float driveshaftRpm = wheelAngularSpeed
									* engine.gearRatio[gear-1]
									* engine.differentialRatio * RAD_TO_RPM;

		// add a portion of the discrepancy between the driveshaft RPM and the engine RPM (simulate losses due to shift time)
		engine.rpm += 0.25*(driveshaftRpm - engine.rpm);
	}
	else if(engine.gear == 0)
	{
		const float engineAngularSpeed = engine.rpm
										/(engine.gearRatio[gear-1]
										* engine.differentialRatio * RAD_TO_RPM);

		wheelAngularSpeed += (engineAngularSpeed - wheelAngularSpeed);
	}

	engine.gear = gear;
}

float Mechanics::getDriveForce()
{
	switch(simulationType)
	{
		default:
		case SIMULATION_TYPE_SLIPLESS:			return getDriveForceBySimplifiedSchemeSlipless();
		case SIMULATION_TYPE_WHEEL_LOAD_CAP:	return getDriveForceBySimplifiedSchemeFakeSlip();
		case SIMULATION_TYPE_PACEJKA_BASED:		return getDriveForceByPacejkaScheme();
	}
}

float Mechanics::getDrivenWheelsWeightLoad()
{
	const float transferedWeightLoad =  mass * acceleration * (centerOfGravityHeight/wheelbase);
	const float weightLoad = mass*GRAVITY_ACCELERATION - downforce;

	// fixme when AWD, wheel weight load transfer are not being accounted for.
	// this causes inaccurate behavior because weight transfer induces different slip ratios in the front and rear tires'.
	if(drivenWheelsType == DRIVEN_WHEELS_ALL)
		return weightLoad;

	if(drivenWheelsType == DRIVEN_WHEELS_ON_REAR)
		return weightDistribution*weightLoad + transferedWeightLoad;

	if(drivenWheelsType == DRIVEN_WHEELS_ON_FRONT)
		return (1-weightDistribution)*weightLoad - transferedWeightLoad;

	// the execution should not get to this point
	return weightLoad;
}

// ------------------------------------------------------------------------------------------------
// ---- SIMPLIFIED SCHEME -------------------------------------------------------------------------

void Mechanics::updateBySimplifiedScheme(float delta)
{
	// this formula assumes no wheel slipping.
	wheelAngularSpeed = speed/tireRadius;  // set new wheel angular speed
	engine.update(delta, wheelAngularSpeed);
}

float Mechanics::getDriveForceBySimplifiedSchemeSlipless()
{
	// all engine power
	return engine.getDriveTorque() / tireRadius;
}

float Mechanics::getDriveForceBySimplifiedSchemeFakeSlip()
{
	// all engine power, up to the maximum allowed by tire grip (driven wheels load)
	return std::min(engine.getDriveTorque() / tireRadius, getDrivenWheelsWeightLoad() * tireFrictionFactor);
}

// ------------------------------------------------------------------------------------------------
// ----- PACEJKA SCHEME ---------------------------------------------------------------------------

/*
 * More info about car physics
 * http://www.asawicki.info/Mirror/Car%20Physics%20for%20Games/Car%20Physics%20for%20Games.html
 * http://vehiclephysics.com/advanced/misc-topics-explained/#tire-friction
 * https://en.wikipedia.org/wiki/Friction#Coefficient_of_friction
 * https://en.wikipedia.org/wiki/Downforce
 * http://www3.wolframalpha.com/input/?i=r(v,+w)+%3D+(w+-+v)%2Fv
 * http://www.edy.es/dev/2011/12/facts-and-myths-on-the-pacejka-curves/
 * http://white-smoke.wikifoundry.com/page/Tyre+curve+fitting+and+validation
 * http://web.archive.org/web/20050308061534/home.planet.nl/~monstrous/tutstab.html
 * http://hpwizard.com/aerodynamics.html
 * http://www.vespalabs.org/projects/vespa-cfd-3d-model/openfoam-motorbike-tutorial

*/

// An estimated wheel (tire+rim) density. (33cm radius or 660mm diameter tire with 75kg mass). Actual value varies by tire (brand, weight, type, etc) and rim (brand , weight, shape, material, etc)
static const float AVERAGE_WHEEL_DENSITY = 75.0/pow2(3.3);  // d = m/r^2, assuming wheel width = 1/PI in the original formula d = m/(PI * r^2 * width)

void Mechanics::updateByPacejkaScheme(float delta)
{
	updateSlipRatio(delta);
	const unsigned drivenWheelsCount = wheelCount / (drivenWheelsType == DRIVEN_WHEELS_ALL? 1 : 2);
	const float wheelMass = AVERAGE_WHEEL_DENSITY * pow2(tireRadius);  // m = d*r^2, assuming wheel width = 1/PI
	const float drivenWheelsInertia = drivenWheelsCount * wheelMass * pow2(tireRadius) * 0.5;  // I = (mr^2)/2

	const float tractionForce = getNormalizedTractionForce() * tireFrictionFactor * getDrivenWheelsWeightLoad();
	const float tractionTorque = tractionForce * tireRadius;

	// todo braking torque calculation should have a brake pad slip ratio on its on
	const float brakingTorque = brakePedalPosition * sgn(wheelAngularSpeed) * BRAKE_PAD_TORQUE_ARBITRARY_ADJUST;
	const float rollingResistanceTorque = rollingResistanceForce * tireRadius * ROLLING_RESISTANCE_TORQUE_ARBITRARY_ADJUST;

	const float totalTorque = engine.getDriveTorque() - tractionTorque - brakingTorque - rollingResistanceTorque;

	const float arbitraryAdjustmentFactor = 0.002;
	const float wheelAngularAcceleration = arbitraryAdjustmentFactor * (totalTorque / drivenWheelsInertia);  // XXX we're assuming no inertia from the engine components.

	wheelAngularSpeed += delta * wheelAngularAcceleration;  // update wheel angular speed
	engine.update(delta, wheelAngularSpeed);  // updated engine RPM based on the wheel angular speed
}

float Mechanics::getDriveForceByPacejkaScheme()
{
	return getNormalizedTractionForce() * getDrivenWheelsWeightLoad() * tireFrictionFactor;
}

void Mechanics::updateSlipRatio(float delta)
{
	// slip ratio computation don't work properly on low speeds due to numerical instability when dividing by values closer and closer to zero
	// in an attempt to attenuate the issue, we follow an approach suggested by Bernard and Clover in [SAE950311]
	static const double B_CONSTANT = 0.91,     // constant
						TAU_CONSTANT = 0.02,   // oscillation period (experimental)
						LOWEST_STABLE_SPEED = 5.0f;

	// approach suggested by Bernard and Clover in [SAE950311].
	double deltaRatio = ((double) wheelAngularSpeed * (double) tireRadius - (double) speed) - fabs((double) speed) * differentialSlipRatio;
	deltaRatio /= B_CONSTANT;
	differentialSlipRatio += deltaRatio * delta;

	// The differential equation tends to oscillate at low speeds.
	// To counter this, use a derived value in the force equations.
	//
	//   SR = SR + Tau * d SR/dt, where Tau is close to the oscillation period
	//
	// (Thanks to Gregor Veble in rec.autos.simulators)
	if(speed < LOWEST_STABLE_SPEED)
		slipRatio = differentialSlipRatio + TAU_CONSTANT * deltaRatio;
	else
		slipRatio = differentialSlipRatio;
}

float Mechanics::getNormalizedTractionForce()
{
	// approximation/simplification based on a simplified Pacejka's formula from Marco Monster's website "Car Physics for Games".
	return slipRatio < 0.06? (20.0*slipRatio)  // 0 to 6% slip ratio gives traction from 0 up to 120%
		 : slipRatio < 0.20? (9.0 - 10.0*slipRatio)/7.0  // 6 to 20% slip ratio gives traction from 120% up to 100%
		 : slipRatio < 1.00? (1.075 - 0.375*slipRatio)  // 20% to 100% slip ratio gives traction from 100 down to 70%
				 	 	 	 : 0.7;  // over 100% slip ratio gives traction 70%
}
