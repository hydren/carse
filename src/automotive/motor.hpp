/*
 * motor.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef MOTOR_HPP_
#define MOTOR_HPP_
#include <ciso646>

#include "futil/language.hpp"
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

	/** A class meant to hold data related to the engine's torque curve.
	 * 	Data is held as a vector of line sections that interpolate a data table.
	 * 	There are 2 static methods that work as proper constructors for this class
	 * */
	struct TorqueCurveProfile
	{
		std::vector< std::vector<float> > parameters;  // todo this could be a map...

		enum PowerBandType {
			POWER_BAND_TYPICAL,       // peak power at 91.5% of RPM range / peak torque at 55.8% of RPM range
			POWER_BAND_PEAKY,         // peak power at 94.0% of RPM range / peak torque at 63.4% of RPM range
			POWER_BAND_TORQUEY,       // peak power at 73.2% of RPM range / peak torque at 55.1% of RPM range
			POWER_BAND_SEMI_TORQUEY,  // peak power at 84.3% of RPM range / peak torque at 54.4% of RPM range
			POWER_BAND_WIDE,          // peak power at 97.6% of RPM range / peak torque at 55.1% of RPM range
			//POWER_BAND_QUASIFLAT      // peak power at 100% of RPM range / constant torque from 1000 RPM on
			//POWER_BAND_TYPICAL_ELECTRIC
		};

		float getTorqueFactor(float rpm);

		/** Returns the RPM which theorectically give the maximum torque of this curve. */
		float getRpmMaxTorque();

		/** Creates a torque curve as two linear functions (increasing then decreasing), given the redline RPM, power band type and (optional) RPM of maximum torque.
		 *  If 'rpmMaxPowerPtr' is not null, stores the RPM of maximum power on the variable.
		 *  If 'maxPowerPtr' is not null, stores the maximum normalized power on the variable. */
		static TorqueCurveProfile createAsDualLinear(float maxRpm, PowerBandType powerBandtype, float rpmMaxTorque=-1, float* rpmMaxPowerPtr=null, float* maxNormPowerPtr=null);

		/** Creates a torque curve as a simple quadratic function (downward openning), given the redline RPM and power band type.
		 *  If 'rpmMaxPowerPtr' is not null, stores the RPM of maximum power on the variable.
		 *  If 'maxPowerPtr' is not null, stores the maximum normalized power on the variable. */
		static TorqueCurveProfile createAsSingleQuadratic(float maxRpm, PowerBandType powerBandtype, float* rpmMaxPowerPtr=null, float* maxNormPowerPtr=null);

		private:
		static void queryParameters(Engine::TorqueCurveProfile::PowerBandType type, float& initialTorqueFactor, float& redlineTorqueFactor);
	};

	Engine(float maxRpm=7000, float maxPower=300, TorqueCurveProfile::PowerBandType powerBand=TorqueCurveProfile::POWER_BAND_TYPICAL, unsigned gearCount=5);

	TorqueCurveProfile torqueCurveProfile;

	/** Reset the engine's state. */
	void reset();

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
