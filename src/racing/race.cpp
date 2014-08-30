/*
 * race.cpp
 *
 *  Created on: 25/08/2014
 *      Author: carlosfaruolo
 */

#include "race.hpp"

#include <cmath>
#include <Box2D/Box2D.h>

#include "../util.hpp"
#include "../util/b2Math_ex.hpp"
#include "../game_engine.hpp"
#include "elements.hpp"

using GameEngine::Image;

b2World* world;
Car* player;

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

	track_bg->draw(-camera.x, -camera.y);
	car_sprite->draw_rotated(10*player->m_body->GetPosition().x-camera.x, 10*player->m_body->GetPosition().y-camera.y, 23, 48, Math::PI - player->m_body->GetAngle());

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
	camera.x = 10*player->m_body->GetPosition().x - camera.w/2;
	camera.y = 10*player->m_body->GetPosition().y - camera.h/2;

	//prevent camera out of bounds
//	if(camera.x < 0)
//		camera.x = 0;
//	if(camera.y < 0)
//		camera.y = 0;
}
