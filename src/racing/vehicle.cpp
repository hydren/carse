/*
 * elements.cpp
 *
 *  Created on: 29/08/2014
 *      Author: felipe
 */

#include "vehicle.hpp"


Tire::Tire(b2World* world, float maxDriveForce, float maxLateralImpulse)
: m_maxDriveForce(maxDriveForce), m_maxLateralImpulse(maxLateralImpulse)
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	m_body = world->CreateBody(&bodyDef);

	b2PolygonShape polygonShape;
	polygonShape.SetAsBox( 0.5f, 1.25f );
	fixture = m_body->CreateFixture(&polygonShape, 1);//shape, density
	//		fixture->SetUserData( new CarTireFUD() );

	m_body->SetUserData( this );

	m_currentTraction = 1;
}


void Tire::updateFriction() {
	//lateral linear velocity
	b2Vec2 impulse = m_body->GetMass() * -getLateralVelocity();
	if ( impulse.Length() > m_maxLateralImpulse )
		impulse *= m_maxLateralImpulse / impulse.Length();
	m_body->ApplyLinearImpulse( m_currentTraction * impulse, m_body->GetWorldCenter() , true);

	//angular velocity
	m_body->ApplyAngularImpulse( m_currentTraction * 0.1f * m_body->GetInertia() * -m_body->GetAngularVelocity() , true);

	//forward linear velocity
	b2Vec2 currentForwardNormal = getForwardVelocity();
	float currentForwardSpeed = currentForwardNormal.Normalize();
	float dragForceMagnitude = -2 * currentForwardSpeed;
	m_body->ApplyForce( m_currentTraction * dragForceMagnitude * currentForwardNormal, m_body->GetWorldCenter() , true);
}

void Tire::updateDrive(float desiredSpeed) {

	//find current speed in forward direction
	b2Vec2 currentForwardNormal = m_body->GetWorldVector( b2Vec2(0,1) );
	float currentSpeed = b2Dot( getForwardVelocity(), currentForwardNormal );

	//apply necessary force
	float force = 0;
	if ( desiredSpeed > currentSpeed )
		force = m_maxDriveForce;
	else if ( desiredSpeed < currentSpeed )
		force = -m_maxDriveForce;
	else
		return;
	m_body->ApplyForce( m_currentTraction * force * currentForwardNormal, m_body->GetWorldCenter() , true);
}

Car::Car(b2World* world, float x, float y, float angle) {
//TODO adapt the code to specify position and angle
	//create car body
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	m_body = world->CreateBody(&bodyDef);
	m_body->SetAngularDamping(3);

	b2Vec2 vertices[8];
	vertices[0].Set( 1.5,   0);
	vertices[1].Set(   3, 2.5);
	vertices[2].Set( 2.8, 5.5);
	vertices[3].Set(   1,  10);
	vertices[4].Set(  -1,  10);
	vertices[5].Set(-2.8, 5.5);
	vertices[6].Set(  -3, 2.5);
	vertices[7].Set(-1.5,   0);
	b2PolygonShape polygonShape;
	polygonShape.Set( vertices, 8 );
	fixture = m_body->CreateFixture(&polygonShape, 0.1f);//shape, density

	//prepare common joint parameters
	b2RevoluteJointDef jointDef;
	jointDef.bodyA = m_body;
	jointDef.enableLimit = true;
	jointDef.lowerAngle = 0;
	jointDef.upperAngle = 0;
	jointDef.localAnchorB.SetZero();//center of tire

//		float maxForwardSpeed = 250;
//		float maxBackwardSpeed = -40;
	float backTireMaxDriveForce = 300;
	float frontTireMaxDriveForce = 500;
	float backTireMaxLateralImpulse = 8.5;
	float frontTireMaxLateralImpulse = 7.5;

	//back left tire
	tireRearLeft = new Tire(world, backTireMaxDriveForce, backTireMaxLateralImpulse);
	jointDef.bodyB = tireRearLeft->m_body;
	jointDef.localAnchorA.Set( -3, 0.75f );
	world->CreateJoint( &jointDef );

	//back right tire
	tireRearRight = new Tire(world, backTireMaxDriveForce, backTireMaxLateralImpulse);
	jointDef.bodyB = tireRearRight->m_body;
	jointDef.localAnchorA.Set( 3, 0.75f );
	world->CreateJoint( &jointDef );

	//front left tire
	tireFrontLeft = new Tire(world, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
	jointDef.bodyB = tireFrontLeft->m_body;
	jointDef.localAnchorA.Set( -3, 8.5f );
	flJoint = (b2RevoluteJoint*)world->CreateJoint( &jointDef );

	//front right tire
	tireFrontRight = new Tire(world, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
	jointDef.bodyB = tireFrontRight->m_body;
	jointDef.localAnchorA.Set( 3, 8.5f );
	frJoint = (b2RevoluteJoint*)world->CreateJoint( &jointDef );
}

void Car::update(float delta, float throtle, float desiredAngle) {

   	tireFrontLeft->updateFriction();
   	tireFrontRight->updateFriction();
   	tireRearLeft->updateFriction();
   	tireRearRight->updateFriction();

   	tireFrontLeft->updateDrive(throtle);
   	tireFrontRight->updateDrive(throtle);
   	tireRearLeft->updateDrive(throtle);
   	tireRearRight->updateDrive(throtle);

   	//control steering
   	float turnSpeedPerSec = 160 * DEGTORAD;//from lock to lock in 0.5 sec
   	float turnPerTimeStep = turnSpeedPerSec * delta;
   	float angleNow = flJoint->GetJointAngle();
   	float angleToTurn = desiredAngle - angleNow;
   	angleToTurn = b2Clamp( angleToTurn, -turnPerTimeStep, turnPerTimeStep );
   	float newAngle = angleNow + angleToTurn;
   	flJoint->SetLimits( newAngle, newAngle );
   	frJoint->SetLimits( newAngle, newAngle );
}

