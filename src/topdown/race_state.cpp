/*
 * race_state.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "race_state.hpp"

#include <iostream>
#include <cmath>
#include <cstdio>

using std::cout;
using std::endl;
using fgeal::Display;
using fgeal::Image;
using fgeal::Sound;
using fgeal::Music;
using fgeal::Font;
using fgeal::Event;
using fgeal::EventQueue;

TopDownRaceState::TopDownRaceState(CarseGame* game)
: State(*game)
{
	lockOn = false;
	showDebug = false;

	cameraAngle = 0;

	isKeyUpPressed = false;
	isKeyDownPressed = false;
	isKeyRightPressed = false;
	isKeyLeftPressed = false;

	camera.x = camera.y = camera.w = camera.h = 0;

	car_sprite = null;
	track_bg = null;

	car_sound_idle = null;
	car_sound_high = null;
	music_sample = null;

	font = null;
	font2 = null;

	world = null;
	player = null;
}

TopDownRaceState::~TopDownRaceState()
{
	if(car_sprite != null) delete car_sprite;
	if(track_bg != null) delete track_bg;

	if(car_sound_idle != null) delete car_sound_idle;
	if(car_sound_high != null) delete car_sound_high;
	if(music_sample != null) delete music_sample;

	if(world != null) delete world;
	if(player != null) delete player;
}

void TopDownRaceState::initialize()
{
	camera.w = Display::getInstance().getWidth();
	camera.h = Display::getInstance().getHeight();
	camera.x = camera.y = 0;

	car_sprite = new Image("topdown_car.png");
	track_bg = new Image("topdown_simple_track.jpg");

	car_sound_idle = new Sound("engine_idle.ogg");
	car_sound_high = new Sound("engine_high.ogg");
	music_sample = new Music("music_sample.ogg");

	font = new Font("font.ttf");
	font2 = new Font("font.ttf");

	world = new b2World(b2Vec2(0, 0));
	player = new Car(world);
}

void TopDownRaceState::onEnter()
{
	cout << "race start!" << endl;
	music_sample->loop();
	car_sound_idle->loop();
}

void TopDownRaceState::onLeave()
{
	cout << "race end!" << endl;
	music_sample->stop();
	car_sound_idle->stop();
}

void TopDownRaceState::render()
{
	Display::getInstance().clear();

	if(lockOn)
	{
		track_bg->drawRotated(camera.w/2, camera.h/2, -cameraAngle, camera.x, camera.y);
		car_sprite->drawRotated(0.1*convertToPixels(player->m_body->GetPosition().x)-camera.x, 0.1*convertToPixels(player->m_body->GetPosition().y)-camera.y, M_PI - player->m_body->GetAngle()-cameraAngle, 23, 48);
	}
	else
	{
		track_bg->draw(-camera.x, -camera.y);
		car_sprite->drawRotated(0.1*convertToPixels(player->m_body->GetPosition().x)-camera.x, 0.1*convertToPixels(player->m_body->GetPosition().y)-camera.y, M_PI - player->m_body->GetAngle(), 23, 48);
	}

	font->drawText(std::string("Using fgeal ")+fgeal::VERSION+" on "+fgeal::ADAPTED_LIBRARY_NAME+" "+fgeal::ADAPTED_LIBRARY_VERSION, 4, fgeal::Display::getInstance().getHeight() - font->getSize(), fgeal::Color::CREAM);

	if(showDebug)
	{
		font2->drawText("Linear velocity:", 25, 25, fgeal::Color::WHITE);
		sprintf(buffer, "% 5.2f, % 5.2f, % 5.2f", player->m_body->GetLinearVelocity().x, player->m_body->GetLinearVelocity().y, player->m_body->GetLinearVelocity().Length());
		font->drawText(std::string(buffer), 50, 50, fgeal::Color::WHITE);

		font2->drawText("Scalar velocity:", 25, 75, fgeal::Color::WHITE);
		sprintf(buffer, "% 5.2f", sqrt(pow(player->m_body->GetLinearVelocity().x, 2) + pow(player->m_body->GetLinearVelocity().y, 2)));
		font->drawText(std::string(buffer), 150, 75, fgeal::Color::WHITE);

		font2->drawText("Angle:", 25, 100, fgeal::Color::WHITE);
		sprintf(buffer, "% 5.2f", player->m_body->GetAngle());
		font->drawText(std::string(buffer), 65, 100, fgeal::Color::WHITE);

		font2->drawText("FPS:", 25, 125, fgeal::Color::WHITE);
		sprintf(buffer, "%d", game.getFpsCount());
		font->drawText(std::string(buffer), 55, 125, fgeal::Color::WHITE);
	}

	fgeal::rest(0.01);
}

void TopDownRaceState::handlePhysics(float delta)
{
	const double forceFactorAbs = 50;

	double forceFactor = 0;
	if(isKeyDownPressed)
		forceFactor = -forceFactorAbs/2;
	else if(isKeyUpPressed)
	{
		forceFactor = forceFactorAbs;
		int i;
		for(i=0; i<player->m_body->GetLinearVelocity().Length() / 50;i++)
			forceFactor *= 1.5;
	}

	float angle = 0;
	if(isKeyLeftPressed)
		angle = -M_PI/4;
	else if(isKeyRightPressed)
		angle = M_PI/4;

	player->update(delta, forceFactor, angle);

	//update world
	world->Step(delta, 10, 10);

	//update the camera
	camera.x = 0.1*convertToPixels(player->m_body->GetPosition().x) - camera.w/2;
	camera.y = 0.1*convertToPixels(player->m_body->GetPosition().y) - camera.h/2;

	if(lockOn)
	{
		float angleDiff = cameraAngle - (M_PI - player->m_body->GetAngle());
		cameraAngle -= angleDiff/10;
	}
}

void TopDownRaceState::handleInput()
{
	Event event;
	EventQueue& eventQueue = EventQueue::getInstance();
	while(not eventQueue.isEmpty())
	{
		eventQueue.waitNextEvent(&event);
		if(event.getEventType() == fgeal::Event::Type::DISPLAY_CLOSURE)
		{
			//game.enterState(CarseGame::MENU_STATE_ID);
			game.running = false;
		}
		else if(event.getEventType() == fgeal::Event::Type::KEY_PRESS)
		{
			switch(event.getEventKeyCode())
			{
			case fgeal::Keyboard::Key::ARROW_UP:
				isKeyUpPressed = true;
				car_sound_idle->stop();
				if(not car_sound_high->isPlaying())
					car_sound_high->loop();
				break;
			case fgeal::Keyboard::Key::ARROW_DOWN:
				isKeyDownPressed = true;
				break;
			case fgeal::Keyboard::Key::ARROW_RIGHT:
				isKeyRightPressed = true;
				break;
			case fgeal::Keyboard::Key::ARROW_LEFT:
				isKeyLeftPressed = true;
				break;
			case fgeal::Keyboard::Key::ESCAPE:
				break;
			case fgeal::Keyboard::Key::ENTER:
				break;
			case fgeal::Keyboard::Key::P:
				if(music_sample->isPlaying())
					music_sample->pause();
				else
					music_sample->resume();
				break;
			case fgeal::Keyboard::Key::L:
				lockOn = !lockOn;
				break;
			case fgeal::Keyboard::Key::D:
				showDebug = !showDebug;
				break;
			default:
				break;
			}
		}
		else if(event.getEventType() == fgeal::Event::Type::KEY_RELEASE)
		{
			switch(event.getEventKeyCode())
			{
			case fgeal::Keyboard::Key::ARROW_UP:
				isKeyUpPressed = false;
				car_sound_high->stop();
				if(not car_sound_idle->isPlaying())
				car_sound_idle->loop();
				break;
			case fgeal::Keyboard::Key::ARROW_DOWN:
				isKeyDownPressed = false;
				break;
			case fgeal::Keyboard::Key::ARROW_RIGHT:
				isKeyRightPressed = false;
				break;
			case fgeal::Keyboard::Key::ARROW_LEFT:
				isKeyLeftPressed = false;
				break;
			default:
				break;
			}
		}
	}
}

void TopDownRaceState::update(float delta)
{
	handleInput();
	handlePhysics(delta);
}
