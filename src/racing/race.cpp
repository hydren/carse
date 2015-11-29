/*
 * race.cpp
 *
 *  Created on: 25/08/2014
 *      Author: carlosfaruolo
 */

#include "race.hpp"

#include <cmath>
#include <Box2D/Box2D.h>
#include "fgeal.hpp"

#include <iostream>
#include "../util/b2Math_ex.hpp"
#include "vehicle.hpp"

//#define LOCK_ON

using GameEngine::Image;
using Math::convertToPixels;
using std::cout; using std::endl;

b2World* world;
Car* player;

bool running = true;

//the race camera
Rect camera;
double cameraAngle = 0;

Image* car_sprite, *track_bg;

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

	car_sprite = new Image("car.png");
	track_bg = new Image("simple_track.jpg");
	eventQueue = new GameEngine::EventQueue;
	world = new b2World(b2Vec2(0, 0));
	player = new Car(world);
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

#ifdef LOCK_ON

	track_bg->draw_rotated(camera.w/2, camera.h/2, camera.x, camera.y, -cameraAngle);
	car_sprite->draw_rotated(0.1*convertToPixels(player->m_body->GetPosition().x)-camera.x, 0.1*convertToPixels(player->m_body->GetPosition().y)-camera.y, 23, 48, Math::PI - player->m_body->GetAngle()-cameraAngle);

#endif

#ifndef LOCK_ON

	track_bg->draw(-camera.x, -camera.y);
	car_sprite->draw_rotated(0.1*convertToPixels(player->m_body->GetPosition().x)-camera.x, 0.1*convertToPixels(player->m_body->GetPosition().y)-camera.y, 23, 48, Math::PI - player->m_body->GetAngle());

#endif

	GameEngine::rest(0.01);
	GameEngine::display->refresh();
}

void Race::handlePhysics()
{
	const double forceFactorAbs = 50;

	double forceFactor = 0;
	if(isKeyDownPressed)
		forceFactor = -forceFactorAbs/2;
	else if(isKeyUpPressed)
		forceFactor = forceFactorAbs;

	float angle = 0;
	if(isKeyLeftPressed)
		angle = -Math::PI/4;
	else if(isKeyRightPressed)
		angle = Math::PI/4;

	player->update(forceFactor, angle);

	//update world
	world->Step((1.0f / 60.0f), 10, 10);
	cout << player->m_body->GetLinearVelocity().x << " " << player->m_body->GetLinearVelocity().y << " " << player->m_body->GetLinearVelocity().Length() << endl;

	//update the camera

#ifdef LOCK_ON
	camera.x = 0.1*convertToPixels(player->m_body->GetPosition().x) - camera.w/2;
	camera.y = 0.1*convertToPixels(player->m_body->GetPosition().y) - camera.h/2;

	float angleDiff = cameraAngle - (Math::PI - player->m_body->GetAngle());
	cameraAngle -= angleDiff/10;
#endif

#ifndef LOCK_ON
	camera.x = 0.1*convertToPixels(player->m_body->GetPosition().x) - camera.w/2;
	camera.y = 0.1*convertToPixels(player->m_body->GetPosition().y) - camera.h/2;
	//prevent camera out of bounds
//	if(camera.x < 0)
//		camera.x = 0;
//	if(camera.y < 0)
//		camera.y = 0;
#endif

}
