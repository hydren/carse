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
using fgeal::Image;
using fgeal::Sound;
using fgeal::Music;
using fgeal::Font;
using fgeal::EventQueue;

RaceState::RaceState(CarseGame* game)
: State(*game)
{
	lockOn = false;  // glitches on SDL adapters
	showDebug = false; // crashes on allegro adapter

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

	eventQueue = null;

	world = null;
	player = null;
}

RaceState::~RaceState()
{
	delete car_sprite;
	delete track_bg;

	delete car_sound_idle;
	delete car_sound_high;
	delete music_sample;

	delete eventQueue;

	delete world;
	delete player;
}

void RaceState::initialize()
{
	camera.w = fgeal::display->getWidth();
	camera.h = fgeal::display->getHeight();
	camera.x = camera.y = 0;

	car_sprite = new Image("car.png");
	track_bg = new Image("simple_track.jpg");

	car_sound_idle = new Sound("engine_idle.ogg");
	car_sound_high = new Sound("engine_high.ogg");
	music_sample = new Music("music_sample.ogg");

	font = new Font("font.ttf");
	font2 = new Font("font.ttf");

	eventQueue = new fgeal::EventQueue;

	world = new b2World(b2Vec2(0, 0));
	player = new Car(world);
}

void RaceState::onEnter()
{
	cout << "race start!" << endl;
	music_sample->loop();
	car_sound_idle->loop();
}

void RaceState::onLeave()
{
	cout << "race end!" << endl;
	music_sample->stop();
	car_sound_idle->stop();
}

void RaceState::render()
{
	fgeal::display->clear();

	if(lockOn)
	{
		track_bg->drawRotated(camera.w/2, camera.h/2, camera.x, camera.y, -cameraAngle);
		car_sprite->drawRotated(0.1*convertToPixels(player->m_body->GetPosition().x)-camera.x, 0.1*convertToPixels(player->m_body->GetPosition().y)-camera.y, 23, 48, M_PI - player->m_body->GetAngle()-cameraAngle);
	}
	else
	{
		track_bg->draw(-camera.x, -camera.y);
		car_sprite->drawRotated(0.1*convertToPixels(player->m_body->GetPosition().x)-camera.x, 0.1*convertToPixels(player->m_body->GetPosition().y)-camera.y, 23, 48, M_PI - player->m_body->GetAngle());
	}


	if(showDebug)
	{
		font2->drawText("Linear velocity: ", 25, 25, fgeal::Color::WHITE);
		sprintf(buffer, "% 5.2f, % 5.2f, % 5.2f", player->m_body->GetLinearVelocity().x, player->m_body->GetLinearVelocity().y, player->m_body->GetLinearVelocity().Length());
		font->drawText(std::string(buffer), 50, 50, fgeal::Color::WHITE);

		font2->drawText("Angle: ", 25, 25, fgeal::Color::WHITE);
		sprintf(buffer, "% 5.2f", player->m_body->GetAngle());
		font->drawText(std::string(buffer), 50, 75, fgeal::Color::WHITE);
	}

	fgeal::rest(0.01);
	fgeal::display->refresh();
}

void RaceState::handlePhysics()
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

	player->update(forceFactor, angle);

	//update world
	world->Step((1.0f / 60.0f), 10, 10);

	//update the camera
	camera.x = 0.1*convertToPixels(player->m_body->GetPosition().x) - camera.w/2;
	camera.y = 0.1*convertToPixels(player->m_body->GetPosition().y) - camera.h/2;

	if(lockOn)
	{
		float angleDiff = cameraAngle - (M_PI - player->m_body->GetAngle());
		cameraAngle -= angleDiff/10;
	}
}

void RaceState::handleInput()
{
	fgeal::Event* ev = null;

	while(not eventQueue->isEmpty())
	{
		ev = eventQueue->waitForEvent();

		if(ev->getEventType() == fgeal::Event::Type::DISPLAY_CLOSURE)
		{
			//game.enterState(CarseGame::MENU_STATE_ID);
			game.running = false;
		}
		else if(ev->getEventType() == fgeal::Event::Type::KEY_PRESS)
		{
			switch(ev->getEventKeyCode())
			{
			case fgeal::Event::Key::ARROW_UP:
				isKeyUpPressed = true;
				car_sound_idle->stop();
				if(not car_sound_high->isPlaying())
					car_sound_high->loop();
				break;
			case fgeal::Event::Key::ARROW_DOWN:
				isKeyDownPressed = true;
				break;
			case fgeal::Event::Key::ARROW_RIGHT:
				isKeyRightPressed = true;
				break;
			case fgeal::Event::Key::ARROW_LEFT:
				isKeyLeftPressed = true;
				break;
			case fgeal::Event::Key::ESCAPE:
				break;
			case fgeal::Event::Key::ENTER:
				break;
			case fgeal::Event::Key::P:
				if(music_sample->isPlaying())
					music_sample->pause();
				else
					music_sample->resume();
				break;
			case fgeal::Event::Key::L:
				lockOn = !lockOn;
				break;
			case fgeal::Event::Key::D:
				showDebug = !showDebug;
				break;
			default:
				break;
			}
		}
		else if(ev->getEventType() == fgeal::Event::Type::KEY_RELEASE)
		{
			switch(ev->getEventKeyCode())
			{
			case fgeal::Event::Key::ARROW_UP:
				isKeyUpPressed = false;
				car_sound_high->stop();
				if(not car_sound_idle->isPlaying())
				car_sound_idle->loop();
				break;
			case fgeal::Event::Key::ARROW_DOWN:
				isKeyDownPressed = false;
				break;
			case fgeal::Event::Key::ARROW_RIGHT:
				isKeyRightPressed = false;
				break;
			case fgeal::Event::Key::ARROW_LEFT:
				isKeyLeftPressed = false;
				break;
			default:
				break;
			}
		}

		delete ev;
		ev = null;
	}
}

void RaceState::update(float delta)
{
	handleInput();
	handlePhysics();
}
