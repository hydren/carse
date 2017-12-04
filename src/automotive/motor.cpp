/*
 * motor.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "motor.hpp"

#include <cmath>
#include <cstdlib>

#include "futil/string_actions.hpp"

using std::string;
using std::vector;

#define PARAM_RPM 0
#define PARAM_SLOPE 1
#define PARAM_INTERCEPT 2

#ifndef M_PI
	# define M_PI		3.14159265358979323846	/* pi */
#endif

static const float ENGINE_FRICTION_COEFFICIENT = 0.2 * 30.0,
		           TORQUE_POWER_CONVERSION_FACTOR = 5252.0 * 1.355818,
                   RAD_TO_RPM = (30.0/M_PI);  // 60/2pi conversion to RPM

#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

/* todo revise TorqueCurveProfile to be able to specify specific RPMs for max. power or/and max. torque.
 * The calculation of the torque curves could be more specific to the point that one could specify a torque
 * curve from a data set, but a proper interface to do this needs to be done; nevertheless, the current
 * model supports it.
 *
 * Also, there is a way to specify a RPM value of the maximum power output and adjust the torque curve
 * parameters accordingly, to match that specific characteristic, but the formula gets too complicated. */

void Engine::TorqueCurveProfile::queryParameters(PowerBandType type, float& initialTorqueFactor, float& redlineTorqueFactor)
{
	float& l = initialTorqueFactor, &u = redlineTorqueFactor;
	switch(type)
	{
		default:
		case POWER_BAND_TYPICAL:      l = 0.60; u = 0.75; break;  // peak power at 91.5% of RPM range / peak torque at 55.8% of RPM range
		case POWER_BAND_PEAKY:        l = 0.40; u = 0.80; break;  // peak power at 94.0% of RPM range / peak torque at 63.4% of RPM range
		case POWER_BAND_TORQUEY:      l = 0.80; u = 0.40; break;  // peak power at 73.2% of RPM range / peak torque at 55.1% of RPM range
		case POWER_BAND_SEMI_TORQUEY: l = 0.50; u = 0.65; break;  // peak power at 84.3% of RPM range / peak torque at 54.4% of RPM range
		case POWER_BAND_WIDE:         l = 0.70; u = 0.80; break;  // peak power at 97.6% of RPM range / peak torque at 55.1% of RPM range
	}
}

/// Create a linear "curve".
static vector<float> createLinearSegment(float rpm_i, float torque_i, float rpm_f, float torque_f)
{
	vector<float> param(3);
	param[0] = rpm_f;  // RPM range threshold or right bound.
	param[1] = (torque_f - torque_i)/(rpm_f - rpm_i);  // line slope
	param[2] = (rpm_f * torque_i - rpm_i * torque_f)/(rpm_f - rpm_i);  // line intercept
	return param;
}

Engine::TorqueCurveProfile Engine::TorqueCurveProfile::createAsDualLinear(float maxRpm, PowerBandType type, float rpmMaxTorque, float* rpmMaxPowerPtr, float* maxNormPowerPtr)
{
	float torque_i, torque_f;
	TorqueCurveProfile::queryParameters(type, torque_i, torque_f);

	if(rpmMaxTorque == -1)
		rpmMaxTorque = TorqueCurveProfile::createAsSingleQuadratic(maxRpm, type, null, null).getRpmMaxTorque();

	TorqueCurveProfile profile;
	profile.parameters.push_back(createLinearSegment(           1,        0,         1000, torque_i));  // hardcoded linear warmup torque curve in 1-1000rpm range
	profile.parameters.push_back(createLinearSegment(        1000, torque_i, rpmMaxTorque,      1.0));  // curve that leads to the highest torque point (from 1001rpm to rpmMaxTorque)
	profile.parameters.push_back(createLinearSegment(rpmMaxTorque,      1.0,       maxRpm, torque_f));  // curve with decrease in torque as it approuches redline (from rpmMaxTorque to maxRpm)

	float xm = -profile.parameters[2][PARAM_INTERCEPT]/(2*profile.parameters[2][PARAM_SLOPE]);
	if(xm < rpmMaxTorque)
		xm = rpmMaxTorque;
	else if(xm > maxRpm)
		xm = maxRpm;

	if(rpmMaxPowerPtr != null)
		*rpmMaxPowerPtr = xm;

	if(maxNormPowerPtr != null)
		*maxNormPowerPtr = profile.getTorqueFactor(xm)*xm/TORQUE_POWER_CONVERSION_FACTOR;

	return profile;
}

Engine::TorqueCurveProfile Engine::TorqueCurveProfile::createAsSingleQuadratic(float maxRpm, PowerBandType type, float* rpmMaxPowerPtr, float* maxNormPowerPtr)
{
	float l;  // the fraction of the engine's maximum torque that is available at 1000RPM
	float u;  // the fraction of the engine's maximum torque that is available at the redline RPM.
	switch(type)
	{
		default:
		case POWER_BAND_TYPICAL:      l = 0.60; u = 0.75; break;  // peak power at 91.5% of RPM range / peak torque at 55.8% of RPM range
		case POWER_BAND_PEAKY:        l = 0.40; u = 0.80; break;  // peak power at 94.0% of RPM range / peak torque at 63.4% of RPM range
		case POWER_BAND_TORQUEY:      l = 0.80; u = 0.40; break;  // peak power at 73.2% of RPM range / peak torque at 55.1% of RPM range
		case POWER_BAND_SEMI_TORQUEY: l = 0.50; u = 0.65; break;  // peak power at 84.3% of RPM range / peak torque at 54.4% of RPM range
		case POWER_BAND_WIDE:         l = 0.70; u = 0.80; break;  // peak power at 97.6% of RPM range / peak torque at 55.1% of RPM range
	}

	//a = (l+u-2)-2sqrt((l-1)(u-1)), b =(2-2l)+2sqrt((l-1)(u-1)), c=l
	const float a = (l+u-2)-2*sqrt((l-1)*(u-1)), b = (2-2*l)+2*sqrt((l-1)*(u-1)), c = l;

	if(rpmMaxPowerPtr != null)
	{
		*rpmMaxPowerPtr = ((- b - sqrt(b*b - 3*a*c))/(3*a))*maxRpm;
	}

	if(maxNormPowerPtr != null)
	{
		const float phi = sqrt(b*b - 3*a*c);
		*maxNormPowerPtr = (phi + b)*(b*b + phi*b - 6*a*c)/(27*a*a);
	}

	TorqueCurveProfile profile;
	profile.parameters.push_back(createLinearSegment(0001, 0.0, 1000, l));  // hardcoded linear warmup torque curve in 1-1000rpm range

	// create piecewise linear curves interpolating the quadratic.
	float previousTorqueFactor = l, rpmStep = 100;
	for(float rpm = 1000+rpmStep; rpm < maxRpm; rpm += rpmStep)
	{
		const float rpmFactor = (rpm-1000)/maxRpm, torqueFactor = a*pow(rpmFactor, 2) + b*rpmFactor + c;
		profile.parameters.push_back(createLinearSegment(rpm-rpmStep, previousTorqueFactor, rpm, torqueFactor));
		previousTorqueFactor = torqueFactor;
	}

	return profile;
}

float Engine::TorqueCurveProfile::getTorqueFactor(float rpm)
{
	if(rpm < 1)
		return -1;

	for(unsigned i = 0; i < parameters.size(); i++)
		if(rpm < parameters[i][PARAM_RPM])
			return parameters[i][PARAM_SLOPE]*rpm + parameters[i][PARAM_INTERCEPT];

	// if we get here, RPM is over the redline
	return parameters[parameters.size()-1][PARAM_SLOPE]*rpm + parameters[parameters.size()-1][PARAM_INTERCEPT];
}

float Engine::TorqueCurveProfile::getRpmMaxTorque()
{
	float maxTorqueRpm = parameters[0][PARAM_RPM];  // initialize with first range RPM
	float maxTorque = maxTorqueRpm * parameters[0][PARAM_SLOPE] + parameters[0][PARAM_INTERCEPT];
	for(unsigned i = 0; i < parameters.size(); i++)
	{
		const float rpm = parameters[i][PARAM_RPM];
		const float torque = rpm * parameters[i][PARAM_SLOPE] + parameters[i][PARAM_INTERCEPT];
		if(torque > maxTorque)
		{
			maxTorque = torque;
			maxTorqueRpm = rpm;
		}
	}
	return maxTorqueRpm;
}

Engine::Engine(float maxRpm, float maxPower, TorqueCurveProfile::PowerBandType curve, unsigned gearCount)
: maximumTorque(), rpm(), maxRpm(maxRpm), minRpm(1000), throttlePosition(), transmissionEfficiency(0.7),
  gear(), gearCount(gearCount), gearRatio(gearCount), differentialRatio(), reverseGearRatio(),
  configuration(), aspiration(), valvetrain(), displacement(3000), valveCount(),
  maximumPower(maxPower), maximumPowerRpm(), maximumTorqueRpm(), torqueCurveProfile()
{
	float maxPowerNormalized;
	torqueCurveProfile = Engine::TorqueCurveProfile::createAsSingleQuadratic(maxRpm, curve, &maximumPowerRpm, &maxPowerNormalized);
	maximumTorqueRpm = torqueCurveProfile.getRpmMaxTorque();
	maximumTorque = ((maximumPower * TORQUE_POWER_CONVERSION_FACTOR)/maximumPowerRpm)/torqueCurveProfile.getTorqueFactor(maximumPowerRpm);

	// default gear ratios
	reverseGearRatio = 3.25;
	differentialRatio = 4.0;
	for(unsigned g = 0; g < gearCount; g++)
		gearRatio[g] = 3.0 + g*2.0/(1.0 - gearCount);  // generic gear ratio

	rpm = 0;
	gear = 1;
	throttlePosition = 0;
}

void Engine::reset()
{
	rpm = minRpm;
	gear = 1;
	throttlePosition = 0;
}

float Engine::getCurrentTorque()
{
	return throttlePosition * maximumTorque * ( rpm > maxRpm ? -rpm/maxRpm : torqueCurveProfile.getTorqueFactor(rpm));
}

float Engine::getDriveTorque()
{
	return gear==0? 0 : this->getCurrentTorque() * gearRatio[gear-1] * differentialRatio * transmissionEfficiency;
}

float Engine::getAngularSpeed()
{
	return rpm / ((gear==0? 1.0 : gearRatio[gear-1]) * differentialRatio * RAD_TO_RPM);
}

void Engine::update(float delta, float wheelAngularSpeed)
{
	if(gear == 0)
	{
		const float rpmRatio = rpm/maxRpm;
		rpm += (getCurrentTorque()*RAD_TO_RPM - (1-throttlePosition)*rpmRatio*rpmRatio*maximumTorque*ENGINE_FRICTION_COEFFICIENT)*delta;
	}
	else
	{
		//synchronize both rotations  (acts like a synchromesh)
		const float rpmDiff = (wheelAngularSpeed * gearRatio[gear-1] * differentialRatio * RAD_TO_RPM) - rpm, synchonizationFactor = 50.0;
		rpm += delta * synchonizationFactor * rpmDiff;
		// fixme the wheel angular speed should change as well when syncing, but previous attempts got strange behavior
	}

	if(rpm < minRpm)
		rpm = minRpm;

	if(rpm > maxRpm+100)
		rpm = maxRpm+100;
}
