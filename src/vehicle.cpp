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
#include <algorithm>

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
  approximatedCenterOfGravityHeight(0), approximatedWheelbase(0), acceleration(0),
  drivenWheels(DRIVEN_WHEELS_ON_REAR),
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
		string t = to_lower(prop.get(key));
		if(t == "car" or t == "default") type = TYPE_CAR;
		else if(t == "bike") type = TYPE_BIKE;
		else type = TYPE_OTHER;
	}
	else type = TYPE_CAR;

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

	key = "driven_wheels";
	if(isValueSpecified(prop, key))
	{
		const string value = prop.get(key);
		if(value == "all") drivenWheels = DRIVEN_WHEELS_ALL;
		else if(value == "front") drivenWheels = DRIVEN_WHEELS_ON_FRONT;
		else drivenWheels = DRIVEN_WHEELS_ON_REAR;
	}
	else
		drivenWheels = DRIVEN_WHEELS_ON_REAR;

	engine = Engine(prop);

	speed = 0;
	brakePedalPosition = 0;
	acceleration = 0;

	// sound data

	if(EngineSoundProfile::requestsPresetProfile(prop))
		engineSoundProfile = game.getPresetEngineSoundProfile(EngineSoundProfile::getSoundDefinitionFromProperties(prop));
	else
		engineSoundProfile = EngineSoundProfile::loadFromProperties(prop);

	// sprite data

	sprite = Pseudo3DVehicleAnimationProfile(prop);

	// needs to be loaded after sprite data to make sure that 'depictedVehicleWidth' was parsed
	key = "vehicle_height";
	if(isValueSpecified(prop, key))
		approximatedCenterOfGravityHeight = 0.5f*atof(prop.get(key).c_str());  // aprox. half the height
	else
		approximatedCenterOfGravityHeight = 0.3506f * sprite.depictedVehicleWidth * sprite.scale.x * 895.0/24.0;  // proportion aprox. of a fairlady z32


	// attempt to estimate wheelbase
	{
		key = "vehicle_wheelbase";
		if(isValueSpecified(prop, key))
			approximatedWheelbase = atof(prop.get(key).c_str());
		else
			approximatedWheelbase = -1;

		key = "vehicle_length";
		if(approximatedWheelbase == -1 and isValueSpecified(prop, key))
			approximatedWheelbase = atof(prop.get(key).c_str());

		key = "vehicle_width";
		if(approximatedWheelbase == -1 and isValueSpecified(prop, key))
			approximatedWheelbase = 2.5251f * atof(prop.get(key).c_str());  // proportion aprox. of a fairlady z32

		key = "vehicle_height";
		if(approximatedWheelbase == -1 and isValueSpecified(prop, key))
			approximatedWheelbase = 3.6016f * atof(prop.get(key).c_str());  // proportion aprox. of a fairlady z32

		if(approximatedWheelbase == -1)
		{
			approximatedWheelbase = 2.5251f * sprite.depictedVehicleWidth * sprite.scale.x * 895.0/24.0;  // proportion aprox. of a fairlady z32
		}
	}
}

static const float PACEJKA_MAGIC_FORMULA_LOWER_SPEED_THRESHOLD = 22;  // 22m/s == 79,2km/h

/** Returns the current driving force. */
float Vehicle::getDriveForce()
{
	if(speed < PACEJKA_MAGIC_FORMULA_LOWER_SPEED_THRESHOLD)
		return std::min(engine.getDriveTorque() / tireRadius, getDrivenWheelsTireLoad());
	else
		return engine.getDriveTorque() / tireRadius;
}

/** Updates the simulation state of this vehicle (RPM, gear, etc). */
void Vehicle::update(float delta)
{
//	resolveSimulationSimple(delta);
	resolveSimulationAdvanced(delta);
}

void Vehicle::resolveSimulationSimple(float delta)
{
	// this formula assumes no wheel spin.
	engine.update(speed/tireRadius);  // set new wheel angular speed
}

static const float GRAVITY_ACCELERATION = 9.8066; // standard gravity (actual value varies with altitude, from 9.7639 to 9.8337)
float Vehicle::getDrivenWheelsTireLoad()
{
	float load = mass*GRAVITY_ACCELERATION;

	if(drivenWheels != DRIVEN_WHEELS_ALL)
		load = 0.5 * load + (drivenWheels == DRIVEN_WHEELS_ON_REAR? 1.0 : -1.0) * (approximatedCenterOfGravityHeight/approximatedWheelbase) * mass * acceleration;

	return load;

//	return (mass * GRAVITY_ACCELERATION) / (float) (drivenWheels != DRIVEN_WHEELS_ALL? 2 : 1);
}

float Vehicle::getLongitudinalSlipRatio()
{
//	return fabs(speed)==0? 0 : (engine.getAngularSpeed()*tireRadius)/fabs(speed) - 1.0;
	return fabs(speed)==0? 0 : (engine.getAngularSpeed()*tireRadius - speed)/fabs(speed);
}

/*
 * More info
 * http://www.asawicki.info/Mirror/Car%20Physics%20for%20Games/Car%20Physics%20for%20Games.html
 * http://vehiclephysics.com/advanced/misc-topics-explained/#tire-friction
 * https://en.wikipedia.org/wiki/Friction#Coefficient_of_friction
 * https://en.wikipedia.org/wiki/Downforce
 * http://www3.wolframalpha.com/input/?i=r(v,+w)+%3D+(w+-+v)%2Fv
 * http://www.edy.es/dev/2011/12/facts-and-myths-on-the-pacejka-curves/
 * http://white-smoke.wikifoundry.com/page/Tyre+curve+fitting+and+validation
*/

float Vehicle::getNormalizedTractionForce()
{
	// based on a simplified Pacejka's formula from Marco Monster's website "Car Physics for Games".
	// this formula don't work properly on low speeds (numerical instability)
	const float longitudinalSlipRatio = getLongitudinalSlipRatio();
	return longitudinalSlipRatio < 0.06? (20.0*longitudinalSlipRatio)  // 0 to 6% slip ratio gives traction from 0 up to 120%
			: longitudinalSlipRatio < 0.20? (7.2 - longitudinalSlipRatio)/7.0  // 6 to 20% slip ratio gives traction from 120% up to 100%
					: longitudinalSlipRatio < 1.00? (1.075 - 0.375*longitudinalSlipRatio)  // 20% to 100% slip ratio gives traction from 100 down to 70%
							: 0.7;  // over 100% slip ratio gives traction 70%
}

//xxx An estimated wheel (tire+rim) density. (33cm radius or 660mm diameter tire with 75kg mass). Actual value varies by tire (brand, weight, type, etc) and rim (brand , weight, shape, material, etc)
static const float AVERAGE_WHEEL_DENSITY = 75.0/squared(3.3);  // d = m/r^2, assuming wheel width = 1/PI in the original formula d = m/(PI * r^2 * width)

void Vehicle::resolveSimulationAdvanced(float delta)
{
	if(speed < PACEJKA_MAGIC_FORMULA_LOWER_SPEED_THRESHOLD)
	{
		resolveSimulationSimple(delta);
		return;
	}

	float wheelAngularAcceleration = 0;

	const unsigned drivenWheelsCount = (type == TYPE_CAR? 4 : type == TYPE_BIKE? 2 : 1) * (drivenWheels != DRIVEN_WHEELS_ALL? 0.5f : 1.0f);
	const float wheelMass = AVERAGE_WHEEL_DENSITY * squared(tireRadius);  // m = d*r^2, assuming wheel width = 1/PI
	const float drivenWheelsInertia = drivenWheelsCount * wheelMass * squared(tireRadius) * 0.5;  // I = (mr^2)/2

	const float tractionForce = getNormalizedTractionForce() * getDrivenWheelsTireLoad();
	const float tractionTorque = tractionForce / tireRadius;

	//fixme how to do this formula right? remove from ingame state braking calculation
//	const float brakingTorque = -brakePedalPosition*30;
	const float brakingTorque = 0;

	const float totalTorque = engine.getDriveTorque() - tractionTorque + brakingTorque;

	const float arbitraryAdjustmentFactor = 0.001;
	wheelAngularAcceleration = arbitraryAdjustmentFactor * (totalTorque / drivenWheelsInertia);  // xxx we're assuming no inertia from the engine components.

	engine.update(engine.getAngularSpeed() + delta * wheelAngularAcceleration);  // set new wheel angular speed
}
