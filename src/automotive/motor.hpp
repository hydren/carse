/*
 * motor.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef MOTOR_HPP_
#define MOTOR_HPP_
#include <ciso646>

#include "futil/properties.hpp"

#include <string>
#include <vector>

struct Engine
{
	float maximumTorque;
	float rpm, maxRpm, minRpm;
	float throttlePosition;
	float transmissionEfficiency;
	int gear, gearCount;
	std::vector<float> gearRatio;
	float differentialRatio, reverseGearRatio;

	// read-only info fields!
	std::string configuration, aspiration, valvetrain;
	unsigned displacement, valveCount;
	float maximumPower, maximumPowerRpm, maximumTorqueRpm;

	struct TorqueCurveProfile
	{
		std::vector< std::vector<float> > parameters;

		/** Creates a torque curve with some hardcoded values and the given max. RPM and max. torque RPM (Optional) */
		static TorqueCurveProfile create(float maxRpm, float rpmMaxTorque=-1);

		/** Fraction of maximum torque that is available with 1000rpm and max.rpm, respectively. */
		static const float TORQUE_CURVE_INITIAL_VALUE, TORQUE_CURVE_FINAL_VALUE;
	};

	TorqueCurveProfile torqueCurveProfile;

	/** Empty constructor */
	Engine();

	/** Creates a engine profile from the given properties data. */
	Engine(const futil::Properties& properties);

	/** Returns this engine's torque in the given RPM. */
	float getCurrentTorque();

	/** Returns the current driving force. */
	float getDriveTorque();

	/** Returns the current engine angular speed (which derives from the current RPM). */
	float getAngularSpeed();

	/** Updates the engine's state (RPM, gear, etc), given the current wheel angular speed. */
	void update(float delta, float wheelAngularSpeed);
};

#endif /* MOTOR_HPP_ */
