/*
 * mechanics.cpp
 *
 *  Created on: 9 de out de 2017
 *      Author: carlosfaruolo
 */

#include "mechanics.hpp"

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

const float Mechanics::GRAVITY_ACCELERATION = 9.8066; // standard gravity (actual value varies with altitude, from 9.7639 to 9.8337)

static const float AIR_DENSITY = 1.2041,  // air density at sea level, 20ºC (68ºF) (but actually varies significantly with altitude, temperature and humidity)
				   DEFAULT_FRONTAL_AREA_CAR  = 1.81,  // frontal area (in square-meters) of a Nissan 300ZX (Z32)
				   DEFAULT_FRONTAL_AREA_BIKE = 0.70;  // estimated frontal area (in square-meters) of a common sporty bike

static const float DEFAULT_CDA_CAR  = 0.31 * DEFAULT_FRONTAL_AREA_CAR,   // drag coefficient (Cd) of a Nissan 300ZX (Z32)
				   DEFAULT_CDA_BIKE = 0.60 * DEFAULT_FRONTAL_AREA_BIKE,  // estimated drag coefficient (Cd) of a common sporty bike
				   DEFAULT_CLA_CAR  = 0.20 * DEFAULT_FRONTAL_AREA_CAR,    // estimated lift coefficient (Cl) of a Nissan 300ZX (Z32)
				   DEFAULT_CLA_BIKE = 0.10 * DEFAULT_FRONTAL_AREA_BIKE;   // estimated lift coefficient (Cl) of a common sporty bike

Mechanics::Mechanics(const Engine& eng, VehicleType type)
: simulationType(SIMULATION_TYPE_FAKESLIP), engine(eng),
  automaticShiftingEnabled(),
  automaticShiftingLowerThreshold(0.5*engine.maximumPowerRpm/engine.maxRpm),
  automaticShiftingUpperThreshold(engine.maximumPowerRpm/engine.maxRpm),
  wheelCount(type == TYPE_CAR? 4 : type == TYPE_BIKE? 2 : 1),
  tireFrictionFactor(1.0),
  rollingResistanceFactor(0.2),
  airDragFactor((type == TYPE_CAR? DEFAULT_CDA_CAR : type == TYPE_BIKE? DEFAULT_CDA_BIKE : 0.5) * AIR_DENSITY),
  downforceFactor((type == TYPE_CAR? DEFAULT_CLA_CAR : type == TYPE_BIKE? DEFAULT_CLA_BIKE : 0.5) * AIR_DENSITY),
  rollingResistanceForce(), airDragForce(), brakingForce(), slopePullForce()
{
	engine.minRpm = 1000;
	setSuggestedWeightDistribuition();
	reset();
}

void Mechanics::reset()
{
	engine.gear = 1;
	engine.rpm = engine.minRpm;
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
}

void Mechanics::updatePowertrain(float delta)
{
	if(automaticShiftingEnabled and slipRatio < 0.1)
	{
		if(engine.gear < engine.gearCount and engine.rpm > automaticShiftingUpperThreshold*engine.maxRpm)
			engine.gear++;

		if(engine.gear > 1 and engine.rpm < automaticShiftingLowerThreshold*engine.maxRpm)
			engine.gear--;
	}

	switch(simulationType)
	{
		default:
		case SIMULATION_TYPE_SLIPLESS:
		case SIMULATION_TYPE_FAKESLIP:		updateBySimplifiedScheme(delta); break;
		case SIMULATION_TYPE_PACEJKA_BASED: updateByPacejkaScheme(delta); break;
	}

	brakingForce = brakePedalPosition * tireFrictionFactor * mass * GRAVITY_ACCELERATION * sgn(speed);  // a multiplier here could be added for stronger and easier braking
	rollingResistanceForce = rollingResistanceFactor * mass * GRAVITY_ACCELERATION * sgn(speed);  // rolling friction is independant on wheel count since the weight will be divided between them
	slopePullForce = mass * GRAVITY_ACCELERATION * sin(slopeAngle),
	airDragForce = 0.5 * airDragFactor * pow2(speed) * AIR_DRAG_ARBITRARY_ADJUST,
	downforce = 0.5 * downforceFactor * pow2(speed) * DOWNFORCE_ARBITRATY_ADJUST;

	// update acceleration
	acceleration = ((arbitraryForceFactor*getDriveForce() - slopePullForce - brakingForce - rollingResistanceForce) - airDragForce)/mass;

	// update speed
	speed += delta*acceleration;
}

float Mechanics::getDriveForce()
{
	switch(simulationType)
	{
		default:
		case SIMULATION_TYPE_SLIPLESS:  	return getDriveForceBySimplifiedSchemeSlipless();
		case SIMULATION_TYPE_FAKESLIP:		return getDriveForceBySimplifiedSchemeFakeSlip();
		case SIMULATION_TYPE_PACEJKA_BASED: return getDriveForceByPacejkaScheme();
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
		return weightDistribuition*weightLoad + transferedWeightLoad;

	if(drivenWheelsType == DRIVEN_WHEELS_ON_FRONT)
		return (1-weightDistribuition)*weightDistribuition - transferedWeightLoad;

	// the execution should not get to this point
	return weightLoad;
}

void Mechanics::setSuggestedWeightDistribuition()
{
	static const float FR_WEIGHT_DISTRIBUITION = 0.45,
					   MR_WEIGHT_DISTRIBUITION = 0.55,
					   RR_WEIGHT_DISTRIBUITION = 0.65,
					   FF_WEIGHT_DISTRIBUITION = 0.40;

	switch(engineLocation)
	{
		default:
		case ENGINE_LOCATION_ON_MIDDLE:		weightDistribuition = MR_WEIGHT_DISTRIBUITION; break;
		case ENGINE_LOCATION_ON_REAR:		weightDistribuition = RR_WEIGHT_DISTRIBUITION; break;
		case ENGINE_LOCATION_ON_FRONT:
			if(drivenWheelsType == DRIVEN_WHEELS_ON_FRONT)
				weightDistribuition = FF_WEIGHT_DISTRIBUITION;
			if(drivenWheelsType == DRIVEN_WHEELS_ON_REAR)
				weightDistribuition = FR_WEIGHT_DISTRIBUITION;
			break;
	}
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

//	const float tractionForce = unstable? 0 : getNormalizedTractionForce() * getDrivenWheelsTireLoad();  // assume zero traction when unstable
	const float tractionForce = getNormalizedTractionForce() * tireFrictionFactor * getDrivenWheelsWeightLoad();
	const float tractionTorque = tractionForce * tireRadius;

	//fixme how to do this formula right? remove from ingame state braking calculation
//	const float brakingTorque = -brakePedalPosition*30;
	const float brakingTorque = 0;

	const float totalTorque = engine.getDriveTorque() - tractionTorque + brakingTorque;

	const float arbitraryAdjustmentFactor = 0.001;
	const float wheelAngularAcceleration = arbitraryAdjustmentFactor * (totalTorque / drivenWheelsInertia);  // xxx we're assuming no inertia from the engine components.

	wheelAngularSpeed += delta * wheelAngularAcceleration;  // update wheel angular speed
	engine.update(delta, wheelAngularSpeed);  // updated engine RPM based on the wheel angular speed
}

float Mechanics::getDriveForceByPacejkaScheme()
{
	return getNormalizedTractionForce() * getDrivenWheelsWeightLoad() * tireFrictionFactor;
}

static const double LOWEST_STABLE_SLIP = 500.0*DBL_EPSILON;  // increasing this constant degrades simulation quality

void Mechanics::updateSlipRatio(float delta)
{
	static const double B_CONSTANT = 0.91,     // constant
						TAU_CONSTANT = 0.02,   // oscillation period (experimental)
						LOWEST_STABLE_SPEED = 5.0f;

	if(engine.gear == 0)
		engine.gear = 0;

	// approach suggested by Bernard and Clover in [SAE950311].
	double deltaRatio = ((double) wheelAngularSpeed * (double) tireRadius - (double) speed) - fabs((double) speed)* differentialSlipRatio;
	delta /= B_CONSTANT;
	differentialSlipRatio += deltaRatio * delta;

	// The differential equation tends to oscillate at low speeds.
	// To counter this, use a derived value in the force equations.
	//
	//   SR = SR + Tau * d SR/dt, where Tau is close to the oscillation period
	//
	// (Thanks to Gregor Veble in r.a.s.)
	if(speed < LOWEST_STABLE_SPEED)
		slipRatio = differentialSlipRatio + TAU_CONSTANT * deltaRatio;
	else
		slipRatio = differentialSlipRatio;

//	slipRatio = fabs(speed)==0? 0 : (engine.getAngularSpeed()*tireRadius)/fabs(speed) - 1.0;
//	slipRatio = fabs(speed)==0? 0 : ((double) engine.getAngularSpeed() * (double) tireRadius - (double) speed)/fabs(speed);
}

float Mechanics::getNormalizedTractionForce()
{
	// based on a simplified Pacejka's formula from Marco Monster's website "Car Physics for Games".
	// this formula don't work properly on low speeds (numerical instability)
	return slipRatio < 0.06? (20.0*slipRatio)  // 0 to 6% slip ratio gives traction from 0 up to 120%
		 : slipRatio < 0.20? (9.0 - 10.0*slipRatio)/7.0  // 6 to 20% slip ratio gives traction from 120% up to 100%
		 : slipRatio < 1.00? (1.075 - 0.375*slipRatio)  // 20% to 100% slip ratio gives traction from 100 down to 70%
				 	 	 	 : 0.7;  // over 100% slip ratio gives traction 70%
}
