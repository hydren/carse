/*
 * mechanics.hpp
 *
 *  Created on: 9 de out de 2017
 *      Author: carlosfaruolo
 */

#ifndef AUTOMOTIVE_MECHANICS_HPP_
#define AUTOMOTIVE_MECHANICS_HPP_
#include <ciso646>

//#define USE_PACEJKA_SCHEME

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
	Engine engine;

	float mass;
	float tireRadius;
	unsigned wheelCount;

	float speed, acceleration;
	float centerOfGravityHeight, wheelbase;
	float weightDistribuition;  // the weight distribuition of the rear wheels; the front wheels dist. are, implicitly, 1 minus this value.
	float slopeAngle;  // the angle of the current slope which the vehicle is above.

	float wheelAngularSpeed;
	float brakePedalPosition;

	float surfaceTireFrictionCoefficient,
		  surfaceTireRollingResistanceCoefficient,
		  airFrictionCoefficient;  // the vehicle's CdA (Cd * surface area) multiplied by air density.

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

	float rollingFriction, airFriction, brakingFriction, slopePull;

	/** An arbitrary force factor applyied to the net drive force when computing acceleration. Default is 1.0. */
	float arbitraryForceFactor;

	Mechanics(const Engine& engine);

	/** Resets the powertrain state to idle. */
	void reset();

	/** Updates the powertrain, given the time step. */
	void updatePowertrain(float timeStep);

	/** Returns the current resulting force coming from the powertrain that accelerates the car. */
	float getDriveForce();

	/** Returns the current total weight load on the driven wheels. */
	float getDrivenWheelsWeightLoad();

	/** Sets a suggested value for the rear weight distribuition, accounting for the engine location within the vehicle. */
	void setSuggestedWeightDistribuition();

	private:

	// Simplified scheme-related
	void updateBySimplifiedScheme(float);
	float getDriveForceBySimplifiedScheme();

	// Pacejka scheme-related
	#ifdef USE_PACEJKA_SCHEME
	public:  // make these public when using pacejka scheme, for debugging
	#endif
	void updateByPacejkaScheme(float);
	float getDriveForceByPacejkaScheme();
	float getNormalizedTractionForce();
	// these refer to the longitudinal slip ratio
	double slipRatio, differentialSlipRatio;
	void updateSlipRatio(float delta);
	private:
};

#endif /* AUTOMOTIVE_MECHANICS_HPP_ */
