/*
 * box2d_vehicle.hpp
 *
 *  Created on: 19 de jun de 2017
 *      Author: carlosfaruolo
 */

#ifndef BOX2D_BOX2D_VEHICLE_HPP_
#define BOX2D_BOX2D_VEHICLE_HPP_
#include <ciso646>

#include <Box2D/Box2D.h>

#ifndef DEGTORAD
	#define DEGTORAD 0.0174532925199432957f
	#define RADTODEG 57.295779513082320876f
#endif

//XXX currently works only as 4-wheeled vehicles
struct Box2DVehicleBody
{
	struct Tire
	{
		b2Body* m_body;
		b2Fixture* fixture;

		float m_currentTraction;

		float m_maxDriveForce, m_maxLateralImpulse;

		Tire(b2World* world, float maxDriveForce, float maxLateralImpulse);

	    b2Vec2 getLateralVelocity();

	    b2Vec2 getForwardVelocity();

	    void updateFriction();

	    void updateDrive(float desiredSpeed);

	    void updateTurn(float desiredTorque);
	};

	b2Body* m_body;
	b2Fixture* fixture;

    b2RevoluteJoint *flJoint, *frJoint;

    Tire* tireFrontLeft, *tireFrontRight, *tireRearLeft, *tireRearRight;

    Box2DVehicleBody(b2World* world, float x=0, float y=0, float angle=0);
    ~Box2DVehicleBody();

    void update(float delta, float throtle, float desiredAngle);
};

#endif /* BOX2D_BOX2D_VEHICLE_HPP_ */
