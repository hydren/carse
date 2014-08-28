/*
 * race.cpp
 *
 *  Created on: 25/08/2014
 *      Author: carlosfaruolo
 */

#include "race.hpp"

#include "../util.hpp"
#include "../game_engine.hpp"
#include <cmath>
#include <Box2D/Box2D.h>
#include "../util/b2Math_ex.hpp"

using GameEngine::Image;


class TDTire {
  public:
  b2Body* m_body;

  TDTire(b2World* world) {
	  b2BodyDef bodyDef;
	  bodyDef.type = b2_dynamicBody;
	  m_body = world->CreateBody(&bodyDef);

	  b2PolygonShape polygonShape;
	  polygonShape.SetAsBox( 0.5f, 1.25f );
	  m_body->CreateFixture(&polygonShape, 1);//shape, density

	  m_body->SetUserData( this );
  }

  ~TDTire() {
	  m_body->GetWorld()->DestroyBody(m_body);
  }

  b2Vec2 getLateralVelocity() {
	b2Vec2 currentRightNormal = m_body->GetWorldVector( b2Vec2(1,0) );
	return b2Dot( currentRightNormal, m_body->GetLinearVelocity() ) * currentRightNormal;
  }
  void updateFriction() {
     b2Vec2 impulse = m_body->GetMass() * -getLateralVelocity();
     m_body->ApplyLinearImpulse( impulse, m_body->GetWorldCenter() , true);
//     m_body->ApplyAngularImpulse( 0.1f * m_body->GetInertia() * -m_body->GetAngularVelocity() , true);
  }
};

TDTire* tire;
b2World* world;



//the race camera
Rect camera;
bool running = true;

Image* car_sprite, *track_bg;

double angle = 0;
b2Vec2 car_pos(200, 200);
b2Vec2 car_speed;

GameEngine::EventQueue* eventQueue;
GameEngine::Event* ev;

bool isKeyUpPressed = false,
	isKeyDownPressed = false,
	isKeyRightPressed = false,
	isKeyLeftPressed = false;

Race::Race()
{
	camera.w = GameEngine::display->getWidth();
	camera.h = GameEngine::display->getHeight();
	camera.x = camera.y = 0;

	car_sprite = new Image("car-delorean-dmc12.png");
	track_bg = new Image("simple_track.jpg");
	eventQueue = new GameEngine::EventQueue;
	world = new b2World(b2Vec2(0, 0));
	tire = new TDTire(world);
}

Race::~Race()
{
	delete car_sprite;
}

void Race::start()
{
	cout << "race start!" << endl;
	do
	{
		handleInput();
		handlePhysics();
		handleRender();
	}
	while(running);
}

void Race::handleInput()
{
	while(not eventQueue->isEmpty())
	{
		ev = eventQueue->waitForEvent();

		if(ev->getEventType() == GameEngine::Event::Type::DISPLAY_CLOSURE)
		{
			running=false;
		}
		else if(ev->getEventType() == GameEngine::Event::Type::KEY_PRESS)
		{
			switch(ev->getEventKeyCode())
			{
			case GameEngine::Event::Key::ARROW_UP:
				isKeyUpPressed = true;
				break;
			case GameEngine::Event::Key::ARROW_DOWN:
				isKeyDownPressed = true;
				break;
			case GameEngine::Event::Key::ARROW_RIGHT:
				isKeyRightPressed = true;
				break;
			case GameEngine::Event::Key::ARROW_LEFT:
				isKeyLeftPressed = true;
				break;
			case GameEngine::Event::Key::ESCAPE:
				break;
			case GameEngine::Event::Key::ENTER:
				break;
			default:
				break;
			}
		}
		else if(ev->getEventType() == GameEngine::Event::Type::KEY_RELEASE)
		{
			switch(ev->getEventKeyCode())
			{
			case GameEngine::Event::Key::ARROW_UP:
				isKeyUpPressed = false;
				break;
			case GameEngine::Event::Key::ARROW_DOWN:
				isKeyDownPressed = false;
				break;
			case GameEngine::Event::Key::ARROW_RIGHT:
				isKeyRightPressed = false;
				break;
			case GameEngine::Event::Key::ARROW_LEFT:
				isKeyLeftPressed = false;
				break;
			default:
				break;
			}
		}

	}
}

void Race::handleRender()
{
	GameEngine::display->clear();

	track_bg->draw(-camera.x, -camera.y);
//	car_sprite->draw_rotated(car_pos.x-camera.x, car_pos.y-camera.y, 23, 48, angle);
	car_sprite->draw_rotated(tire->m_body->GetPosition().x-camera.x, tire->m_body->GetPosition().y-camera.y, 23, 48, tire->m_body->GetAngle());

	GameEngine::rest(0.01);
	GameEngine::display->refresh();
}

void Race::handlePhysics()
{
	const double forceFactorAbs = 50000000;
	if(isKeyLeftPressed)
	{
		tire->m_body->ApplyAngularImpulse(+Math::PI/32, true);
	}
	else if(isKeyRightPressed)
	{
		tire->m_body->ApplyAngularImpulse(-Math::PI/32, true);
	}

	double forceFactor = 0;
	if(isKeyDownPressed)
		forceFactor = forceFactorAbs;
	else if(isKeyUpPressed)
		forceFactor = -forceFactorAbs;

	b2Vec2 force(sin(tire->m_body->GetAngle()), cos(tire->m_body->GetAngle()));
	force *= forceFactor;
	tire->m_body->ApplyForceToCenter(force, true);
	tire->m_body->ApplyAngularImpulse( 0.1f * tire->m_body->GetInertia() * -tire->m_body->GetAngularVelocity() , true);

	b2Vec2 forceDrag = tire->m_body->GetLinearVelocity();
//	forceDrag *= 0.1;
	tire->m_body->ApplyForceToCenter(-forceDrag, true);

	world->Step((1.0f / 60.0f), 10, 10);
	cout << tire->m_body->GetLinearVelocity().x << " " << tire->m_body->GetLinearVelocity().y << " " << tire->m_body->GetLinearVelocity().Length() << endl;

	//update the camera
	camera.x = tire->m_body->GetPosition().x - camera.w/2;
	camera.y = tire->m_body->GetPosition().y - camera.h/2;
//	if(camera.x < 0)
//		camera.x = 0;
//	if(camera.y < 0)
//		camera.y = 0;
}
