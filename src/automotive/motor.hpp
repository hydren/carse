/*
 * motor.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef MOTOR_HPP_
#define MOTOR_HPP_
#include <ciso646>

#include <string>

struct Engine
{
	float maximumTorque;
	float rpm, maxRpm, minRpm;

	float transmissionEfficiency;
	int gear, gearCount;
	float *gearRatio, reverseGearRatio; // fixme this leaks :)

	float tireRadius;

	float throttlePosition, brakePosition;

	// read-only info fields!
	std::string configuration, aspiration, valvetrain;
	unsigned displacement, valveCount;
	float maximumPower, maximumPowerRpm, maximumTorqueRpm;

	struct TorqueCurveProfile
	{
		float parameters[3][3];

		/** Creates a torque curve with some hardcoded values and the given max. RPM and max. torque RPM (Optional) */
		static TorqueCurveProfile create(float maxRpm, float rpmMaxTorque=-1);

		/** Fraction of maximum torque that is available with 1000rpm and max.rpm, respectively. */
		static const float TORQUE_CURVE_INITIAL_VALUE, TORQUE_CURVE_FINAL_VALUE;
	};

	TorqueCurveProfile torqueCurveProfile;

	bool automaticShiftingEnabled;
	float automaticShiftingLowerThreshold;
	float automaticShiftingUpperThreshold;

	/** Returns this engine's torque in the given RPM. */
	float getCurrentTorque();

	/** Returns the current driving force. */
	float getDriveForce();

	/** Updates the engine's state (RPM, gear, etc), given the time step and current speed. */
	void update(float delta, float speed);
};

#endif /* MOTOR_HPP_ */
