/*
 * vehicle.hpp
 *
 *  Created on: 29/08/2014
 *      Author: felipe
 */

#ifndef ELEMENTS_HPP_
#define ELEMENTS_HPP_

#include <Box2D/Box2D.h>

#ifndef DEGTORAD
#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#endif

struct Tire
{
	b2Body* m_body;
	b2Fixture* fixture;

	float m_currentTraction;

	float m_maxDriveForce, m_maxLateralImpulse;

	Tire(b2World* world, float maxDriveForce, float maxLateralImpulse);

    b2Vec2 getLateralVelocity() {
        b2Vec2 currentRightNormal = m_body->GetWorldVector( b2Vec2(1,0) );
        return b2Dot( currentRightNormal, m_body->GetLinearVelocity() ) * currentRightNormal;
    }

    b2Vec2 getForwardVelocity() {
        b2Vec2 currentForwardNormal = m_body->GetWorldVector( b2Vec2(0,1) );
        return b2Dot( currentForwardNormal, m_body->GetLinearVelocity() ) * currentForwardNormal;
    }

    void updateFriction();

    void updateDrive(float desiredSpeed);

    void updateTurn(float desiredTorque) {
		m_body->ApplyTorque( desiredTorque , true);
	}
};


struct Car
{
	b2Body* m_body;
	b2Fixture* fixture;

    b2RevoluteJoint *flJoint, *frJoint;

	Tire *tireFrontLeft, *tireFrontRight, *tireRearLeft, *tireRearRight;

	Car(b2World* world);

    ~Car() {
        delete tireFrontLeft;
        delete tireFrontRight;
        delete tireRearLeft;
        delete tireRearRight;
    }

    void update(float throtle, float desiredAngle);
};





#endif /* ELEMENTS_HPP_ */
