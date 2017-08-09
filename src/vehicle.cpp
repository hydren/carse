/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "futil/string_actions.hpp"

#include <cstdlib>
#include <cmath>

#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

#ifdef squared
	#undef squared
#endif
#define squared(x) (x*x)

using futil::Properties;
using futil::to_lower;
using std::string;

// default float constants
static const float
	DEFAULT_VEHICLE_MASS = 1250,  // kg
	DEFAULT_TIRE_DIAMETER = 678;  // mm

Vehicle::Vehicle()
: type(TYPE_CAR), name(), authors(), credits(), comments(),
  mass(0), tireRadius(0), engine(), speed(0), brakePedalPosition(0),
  engineSoundProfile(), sprite()
{}

Vehicle::Vehicle(const Properties& prop, Pseudo3DCarseGame& game)
{
	// aux. var
	string key;

	// logic data

	key = "vehicle_type";
	if(prop.containsKey(key))
	{
		string t = prop.get(key);
		if(to_lower(t) == "car") type = TYPE_CAR;
		else if(to_lower(t) == "bike") type = TYPE_BIKE;
		else type = TYPE_OTHER;
	}

	// info data

	key = "vehicle_name";
	name = prop.containsKey(key)? prop.get(key) : "unnamed";

	key = "authors";
	authors = prop.containsKey(key)? prop.get(key) : "unknown";

	key = "credits";
	credits = prop.containsKey(key)? prop.get(key) : "";

	key = "comments";
	comments = prop.containsKey(key)? prop.get(key) : "";

	// todo read more data from properties

	// physics data

	key = "vehicle_mass";
	mass = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_VEHICLE_MASS;

	key = "tire_diameter";
	tireRadius = (isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_TIRE_DIAMETER) * 0.0005;

	engine = Engine(prop);

	speed = 0;
	brakePedalPosition = 0;

	// sound data

	if(EngineSoundProfile::requestsPresetProfile(prop))
		engineSoundProfile = game.getPresetEngineSoundProfile(EngineSoundProfile::getSoundDefinitionFromProperties(prop));
	else
		engineSoundProfile = EngineSoundProfile::loadFromProperties(prop);

	// sprite data

	sprite = Pseudo3DVehicleAnimationProfile(prop);
}

/** Returns the current driving force. */
float Vehicle::getDriveForce()
{
	return engine.getDriveTorque() / tireRadius;
}

/** Updates the simulation state of this vehicle (RPM, gear, etc). */
void Vehicle::update(float delta)
{
	//fixme this formula assumes no wheel spin.
	const float wheelAngularSpeed = speed/tireRadius;
	engine.update(wheelAngularSpeed);  // set new wheel angular speed
}

/*
//xxx An estimated wheel (tire+rim) density. (33cm radius or 660mm diameter tire with 75kg mass). Actual value varies by tire (brand, weight, type, etc) and rim (brand , weight, shape, material, etc)
static const float AVERAGE_WHEEL_DENSITY = 75.0/squared(3.3);  // d = m/r^2, assuming wheel width = 1/PI in the original formula d = m/(PI * r^2 * width)
static const float GRAVITY_ACCELERATION = 9.8066; // standard gravity (actual value varies with altitude, from 9.7639 to 9.8337)

void Vehicle::update(float delta)
{
	float wheelAngularSpeed = engine.getAngularSpeed();

	const float tireWeightLoad = (mass * GRAVITY_ACCELERATION)/4;  // xxx this formula assumes 4 wheels. this is not always the case (ex: bikes).

	//fixme this Pacejka's formula still don't work properly
	// based on a simplified Pacejka's formula from Marco Monster's website "Car Physics for Games".
	const float longitudinalSlipRatio = fabs(speed)==0? 0 : (wheelAngularSpeed*tireRadius - speed)/fabs(speed);
	const float normalizedTractionForce = longitudinalSlipRatio < 0.06? (      20.0*longitudinalSlipRatio)      // 0 to 6% slip ratio gives traction from 0 up to 120%
										: longitudinalSlipRatio < 0.90? (9.0 - 10.0*longitudinalSlipRatio)/7.0  // 6% to 90% slip ratio gives traction from 120 down to 0%
										: 0;  // over 90% slip ratio gives no traction at all

	const float tractionForce = normalizedTractionForce * tireWeightLoad;
	const float tractionTorque = tractionForce / tireRadius;

	//fixme how to do this formula right? remove from ingame state braking calculation
//	const float brakingTorque = -brakePedalPosition*30;
	const float brakingTorque = 0;

	const unsigned drivenWheelsCount = 2;  // fixme driven wheels count should be retrieved by vehicle specification.

	const float totalTorque = engine.getDriveTorque() + tractionTorque*drivenWheelsCount + brakingTorque;

	const float wheelMass = AVERAGE_WHEEL_DENSITY * squared(tireRadius);  // m = d*r^2, assuming wheel width = 1/PI
	const float drivenWheelsInertia = drivenWheelsCount * wheelMass * squared(tireRadius) * 0.5;  // I = (mr^2)/2

	wheelAngularSpeed += (totalTorque / drivenWheelsInertia)*delta;  // xxx we're assuming no inertia from the engine components.

	engine.update(wheelAngularSpeed);  // set new wheel angular speed
}
*/
