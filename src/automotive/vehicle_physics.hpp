/*
 * vehicle_physics.hpp
 *
 *  Created on: 27 de jul de 2017
 *      Author: carlosfaruolo
 */

#ifndef AUTOMOTIVE_VEHICLE_PHYSICS_HPP_
#define AUTOMOTIVE_VEHICLE_PHYSICS_HPP_
#include <ciso646>

#include "motor.hpp"

struct VehicleBody
{
	Engine engine;

	float mass;
	float tireRadius;

	float transmissionEfficiency;
	int gear, gearCount;
	float *gearRatio, reverseGearRatio; // fixme this leaks :)

	bool automaticShiftingEnabled;
	float automaticShiftingLowerThreshold;
	float automaticShiftingUpperThreshold;

	/** Updates the engine's state (RPM, gear, etc), given the current speed. */
	void update(float currentSpeed);  /// fixme todo change parameter to time step and stop relying on vehicle speed to get RPM.

	/** Returns the current resulting driving force. */
	float getDriveForce();
};

#endif /* AUTOMOTIVE_VEHICLE_PHYSICS_HPP_ */
