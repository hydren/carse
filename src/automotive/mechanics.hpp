/*
 * mechanics.hpp
 *
 *  Created on: 9 de out de 2017
 *      Author: carlosfaruolo
 */

#ifndef AUTOMOTIVE_MECHANICS_HPP_
#define AUTOMOTIVE_MECHANICS_HPP_
#include <ciso646>

#include "motor.hpp"

/** Class that performs simulations of vehicle powertrains and physics.
 *
 * Limitations:
 *
 * - Doesn't do turning physics.
 * - Supports only vehicles with wheels divided in 2 clusters: front wheel(s) and rear wheel(s). (transversally bipartite wheels)
 *   Example: cars, bikes, trikes. A 6-wheeled vehicle, for example, is not supported.
 *
 *  */
struct Mechanics
{
	static const float GRAVITY_ACCELERATION;

	enum SimulationType
	{
		SIMULATION_TYPE_SLIPLESS,  // no slip ratio, no fake slip
		SIMULATION_TYPE_FAKESLIP,  // no slip ratio, fake slip that limits power output to driven tires' load
		SIMULATION_TYPE_PACEJKA_BASED  // slip ratio simulation, Pacejka friction, scheme based on suggestions from SAE950311 and Gregor Veble
	}
	simulationType;

	enum VehicleType { TYPE_CAR, TYPE_BIKE, TYPE_OTHER } vehicleType;

	Engine engine;

	bool automaticShiftingEnabled;
	float automaticShiftingLowerThreshold;
	float automaticShiftingUpperThreshold;

	float mass;
	float tireRadius;
	unsigned wheelCount;

	float speed, acceleration;
	float centerOfGravityHeight, wheelbase;
	float weightDistribuition;  // the weight distribuition of the rear wheels; the front wheels dist. are, implicitly, 1 minus this value.
	float slopeAngle;  // the angle of the current slope which the vehicle is above.

	float wheelAngularSpeed;
	float brakePedalPosition;

	float tireFrictionFactor,       // tire friction coefficient
		  rollingResistanceFactor,  // rolling resistance or rolling friction coefficient (Crr) of the tires
		  airDragFactor,	// the vehicle's CdA (drag coefficient (Cd) * surface area) multiplied by air density.
		  downforceFactor;  // the vehicle's ClA (lift coefficient (Cl) * surface area) multiplied by air density.

	enum DrivenWheelsType {
		DRIVEN_WHEELS_ON_FRONT,
		DRIVEN_WHEELS_ON_REAR,
		DRIVEN_WHEELS_ALL
	} drivenWheelsType;

	enum EngineLocation {
		ENGINE_LOCATION_ON_FRONT,
		ENGINE_LOCATION_ON_MIDDLE,
		ENGINE_LOCATION_ON_REAR
	} engineLocation;

	float rollingResistanceForce, airDragForce, brakingForce, slopePullForce, downforce;

	/** An arbitrary force factor applyied to the net drive force when computing acceleration. Default is 1.0. */
	float arbitraryForceFactor;

	// todo support more types of vehicles (jetskis, motorboats, hovercrafts, hovercars, trikes, etc)
	Mechanics(const Engine& engine, VehicleType, float dragArea=0, float liftArea=0);

	/** Resets the powertrain state to idle. */
	void reset();

	/** Updates the powertrain, given the time step. */
	void updatePowertrain(float timeStep);


	void shiftGear(int gear);

	/** Returns the current resulting force coming from the powertrain that accelerates the car. */
	float getDriveForce();

	/** Returns the current total weight load on the driven wheels. */
	float getDrivenWheelsWeightLoad();

	private:

	// Simplified scheme-related
	void updateBySimplifiedScheme(float);
	float getDriveForceBySimplifiedSchemeSlipless();
	float getDriveForceBySimplifiedSchemeFakeSlip();

	public:  // make these public for debugging when using pacejka scheme

	// Pacejka scheme-related
	void updateByPacejkaScheme(float);
	float getDriveForceByPacejkaScheme();
	float getNormalizedTractionForce();

	// these refer to the longitudinal slip ratio
	double slipRatio, differentialSlipRatio;
	void updateSlipRatio(float delta);
};

#endif /* AUTOMOTIVE_MECHANICS_HPP_ */
