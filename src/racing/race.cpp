/*
 * race.cpp
 *
 *  Created on: 25/08/2014
 *      Author: carlosfaruolo
 */

#include "race.hpp"

#include "../util.hpp"
#include "../physics/vector2d.hpp"
#include "../game_engine.hpp"
#include <cmath>

using GameEngine::Image;

//the race camera
Rect camera;
bool running = true;

Image* car_sprite, *track_bg;

double angle = 0;
vector2d car_pos(200, 200);
vector2d car_speed;



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
	car_sprite->draw_rotated(car_pos.x-camera.x, car_pos.y-camera.y, 23, 48, angle);

	GameEngine::rest(0.01);
	GameEngine::display->refresh();
}

void Race::handlePhysics()
{
//	const double speed = 10;
//	double speed_abs = 0;
//
//	if(isKeyDownPressed || isKeyUpPressed)
//	{
//		if(isKeyDownPressed)
//			speed_abs = speed;
//		else if(isKeyUpPressed)
//			speed_abs = -speed;
//
//		if(isKeyLeftPressed)
//		{
//			angle += (car_speed.magnitude()/speed)*Math::PI/64;
//		}
//		else if(isKeyRightPressed)
//		{
//			angle -= (car_speed.magnitude()/speed)*Math::PI/64;
//		}
//
//		car_speed.x = speed_abs*sin(angle);
//		car_speed.y = speed_abs*cos(angle);
//	}
//	else
//	{
//		if(isKeyLeftPressed)
//		{
//			angle += (car_speed.magnitude()/speed)*Math::PI/64;
//		}
//		else if(isKeyRightPressed)
//		{
//			angle -= (car_speed.magnitude()/speed)*Math::PI/64;
//		}
//
//		car_speed.x *= 0.9;
//		car_speed.y *= 0.9;
//	}

	const double speed = 10;
	if(isKeyLeftPressed)
	{
		angle -= (car_speed.unit().innerProduct(vector2d(sin(angle), cos(angle))))
		* atan(car_speed.magnitude()) * Math::PI/64;
	}
	else if(isKeyRightPressed)
	{
		angle += (car_speed.unit().innerProduct(vector2d(sin(angle), cos(angle))))
		* atan(car_speed.magnitude()) * Math::PI/64;
	}
	double speed_abs = 0;
	if(isKeyDownPressed)
		speed_abs = speed;
	else if(isKeyUpPressed)
		speed_abs = -speed;

	if(speed_abs == 0)
	{
		car_speed.x *= 0.99;
		car_speed.y *= 0.99;
	}
	else
	{
		car_speed.x = speed_abs*sin(angle);
		car_speed.y = speed_abs*cos(angle);
	}

	car_pos.x += car_speed.x;
	car_pos.y += car_speed.y;

	//update the camera
	camera.x = car_pos.x - camera.w/2;
	camera.y = car_pos.y - camera.h/2;
//	if(camera.x < 0)
//		camera.x = 0;
//	if(camera.y < 0)
//		camera.y = 0;
}
