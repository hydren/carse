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
using fgeal::Keyboard;

using util::toPixels;
using util::toMeters;

#ifndef M_PI
	# define M_PI		3.14159265358979323846	/* pi */
#endif

TopDownRaceState::TopDownRaceState(CarseGame* game)
: State(*game)
{
	lockOn = false;
	showDebug = true;

	cameraAngle = 0;

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

	car_sprite = new Image("assets/topdown_car.png");
	track_bg = new Image("assets/topdown_simple_track.jpg");

	car_sound_idle = new Sound("assets/sound/engine/default_engine_idle.ogg");
	car_sound_high = new Sound("assets/sound/engine/default_engine_rev.ogg");
	music_sample = new Music("assets/music_sample.ogg");

	font = new Font("assets/font.ttf");
	font2 = new Font("assets/font.ttf");

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
		car_sprite->drawRotated(0.1*toPixels(player->m_body->GetPosition().x)-camera.x, 0.1*toPixels(player->m_body->GetPosition().y)-camera.y, M_PI - player->m_body->GetAngle()-cameraAngle, 23, 48);
	}
	else
	{
		track_bg->draw(-camera.x, -camera.y);
		car_sprite->drawRotated(0.1*toPixels(player->m_body->GetPosition().x)-camera.x, 0.1*toPixels(player->m_body->GetPosition().y)-camera.y, M_PI - player->m_body->GetAngle(), 23, 48);
	}

	if(showDebug)
	{
		font->drawText(std::string("Using fgeal ")+fgeal::VERSION+" on "+fgeal::ADAPTED_LIBRARY_NAME+" "+fgeal::ADAPTED_LIBRARY_VERSION, 4, fgeal::Display::getInstance().getHeight() - font->getHeight(), fgeal::Color::CREAM);

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
	if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN))
		forceFactor = -forceFactorAbs/2;
	else if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP))
	{
		forceFactor = forceFactorAbs;
		int i;
		for(i=0; i<player->m_body->GetLinearVelocity().Length() / 50;i++)
			forceFactor *= 1.5;
	}

	float angle = 0;
	if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_LEFT))
		angle = -M_PI/4;
	else if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_RIGHT))
		angle = M_PI/4;

	player->update(delta, forceFactor, angle);

	//update world
	world->Step(delta, 10, 10);

	//update the camera
	camera.x = 0.1*toPixels(player->m_body->GetPosition().x) - camera.w/2;
	camera.y = 0.1*toPixels(player->m_body->GetPosition().y) - camera.h/2;

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
		if(event.getEventType() == fgeal::Event::TYPE_DISPLAY_CLOSURE)
		{
			//game.enterState(CarseGame::MENU_STATE_ID);
			game.running = false;
		}
		else if(event.getEventType() == fgeal::Event::TYPE_KEY_PRESS)
		{
			switch(event.getEventKeyCode())
			{
				case fgeal::Keyboard::KEY_ARROW_UP:
					car_sound_idle->stop();
					if(not car_sound_high->isPlaying())
						car_sound_high->loop();
					break;
				case fgeal::Keyboard::KEY_P:
					if(music_sample->isPlaying())
						music_sample->pause();
					else
						music_sample->resume();
					break;
				case fgeal::Keyboard::KEY_L:
					lockOn = !lockOn;
					break;
				case fgeal::Keyboard::KEY_D:
					showDebug = !showDebug;
					break;
				default: break;
			}
		}
		else if(event.getEventType() == fgeal::Event::TYPE_KEY_RELEASE)
		{
			switch(event.getEventKeyCode())
			{
				case fgeal::Keyboard::KEY_ARROW_UP:
					car_sound_high->stop();
					if(not car_sound_idle->isPlaying())
					car_sound_idle->loop();
					break;
				default: break;
			}
		}
	}
}

void TopDownRaceState::update(float delta)
{
	handleInput();
	handlePhysics(delta);
}
