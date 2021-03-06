/*
 * vehicle_spec.hpp
 *
 *  Created on: 30 de nov de 2017
 *      Author: carlosfaruolo
 */

#ifndef AUTOMOTIVE_VEHICLE_SPEC_HPP_
#define AUTOMOTIVE_VEHICLE_SPEC_HPP_
#include <ciso646>

#include "motor.hpp"
#include "mechanics.hpp"

struct VehicleSpec
{
	// general information
	std::string name, authors, credits, comments;

	// the type of vehicle
	Mechanics::VehicleType type;

	// the vehicle's mass, in Kg
	float mass;

	// the tire radius, in mm
	float tireRadius;

	// the distance between the centers of the front and rear wheels.
	float wheelbase;

	// the weight distribution of the rear wheels; the front wheels distribution are, implicitly, 1 minus this value.
	float weightDistribution;

	// the height of the vehicle's center of gravity. its aproximated as half the height (if it's available; otherwise it's hardcoded)
	float centerOfGravityHeight;

	// the product of the vehicle's drag coefficient (Cd) and its reference area (frontal, cross-sectional area)
	float dragArea;

	// the product of the vehicle's lift coefficient (Cl) and its reference area (frontal, cross-sectional area)
	float liftArea;

	// the maximum torque output of the engine
	float engineMaximumTorque;

	// the maximum and minimum RPM
	float engineMaximumRpm, engineMinimumRpm;

	// the mean transmission efficiency of the engine
	float engineTransmissionEfficiency;

	// the number of gears on the transmission (excluding the reverse gear)
	float engineGearCount;

	// the transmission's gear ratios
	std::vector<float> engineGearRatio;

	// the transmission's reverse gear ratio
	float engineReverseGearRatio;

	// the transmission's differential ratio
	float engineDifferentialRatio;

	// the torque curve data
	Engine::TorqueCurveProfile::PowerBandType enginePowerBand;

	// the drivetrain type of the vehicle
	Mechanics::DrivenWheelsType drivenWheelsType;

	// the approximate engine location
	Mechanics::EngineLocation engineLocation;

	// informative-only fields
	std::string engineConfiguration, engineAspiration, engineValvetrain;
	unsigned engineDisplacement, engineValveCount;
	float engineMaximumPower, engineMaximumPowerRpm, engineMaximumTorqueRpm;
};

#endif /* AUTOMOTIVE_VEHICLE_SPEC_HPP_ */
