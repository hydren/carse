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
		enum PowerBandType { POWER_BAND_TORQUEY, POWER_BAND_PEAKY, POWER_BAND_FLEXIBLE };

		std::vector< std::vector<float> > parameters;  // todo this could be a map...

		float getTorqueFactor(float rpm);

		/** Returns the RPM which theorectically give the maximum torque of this curve. */
		float getRpmMaxTorque();

		/** Creates a torque curve with some hardcoded values and the given max. RPM and max. torque RPM (Optional) */
		static TorqueCurveProfile create(float maxRpm, float rpmMaxTorque=-1);

		/** Creates a torque curve with a simple quadratic equation shape, given the redline RPM and power band type.
		 *  If 'rpmMaxPowerPtr' is not null, stores the RPM of maximum power on the variable.
		 *  If 'maxPowerPtr' is not null, stores the maximum normalized power on the variable. */
		static TorqueCurveProfile createSimpleQuadratic(float maxRpm, PowerBandType powerBandtype, float* rpmMaxPowerPtr=NULL, float* maxNormPowerPtr=NULL);

		static const float TORQUE_CURVE_PEAKY_INITIAL_VALUE,    TORQUE_CURVE_PEAKY_REDLINE_VALUE,
						   TORQUE_CURVE_TORQUEY_INITIAL_VALUE,  TORQUE_CURVE_TORQUEY_REDLINE_VALUE,
						   TORQUE_CURVE_FLEXIBLE_INITIAL_VALUE, TORQUE_CURVE_FLEXIBLE_REDLINE_VALUE;
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
