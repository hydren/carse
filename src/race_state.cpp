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
using fgeal::Font;
using fgeal::Color;
using fgeal::Keyboard;
using fgeal::Event;
using fgeal::EventQueue;

RaceState::RaceState(CarseGame* game)
: State(*game),
  font(null), font2(null)
{}

RaceState::~RaceState()
{}

void RaceState::initialize()
{
	font = new Font("font.ttf");
	font2 = new Font("font.ttf");
}

void RaceState::onEnter()
{
	cout << "race start!" << endl;
}

void RaceState::onLeave()
{
	cout << "race end!" << endl;
}

void drawQuad(const Color& c, float x1, float y1, float w1, float x2, float y2, float w2)
{
	Image::drawQuadrangle(c, x1-w1, y1, x2-w2, y2, x2+w2, y2, x1+w1, y1);
}

void RaceState::render()
{
	Display::getInstance().clear();
	drawQuad(Color::GREEN, 500, 500, 200, 500, 300, 100);

	// DEBUG
	{
		font2->drawText("FPS:", 25, 25, fgeal::Color::WHITE);
		sprintf(buffer, "%d", game.getFpsCount());
		font->drawText(std::string(buffer), 55, 25, fgeal::Color::WHITE);
	}

	fgeal::rest(0.01);
}

void RaceState::update(float delta)
{
	handleInput();
	handlePhysics(delta);
}

void RaceState::handleInput()
{
	Event event;
	EventQueue& eventQueue = EventQueue::getInstance();
	while(not eventQueue.isEmpty())
	{
		eventQueue.waitNextEvent(&event);
		if(event.getEventType() == Event::Type::DISPLAY_CLOSURE)
		{
			//game.enterState(CarseGame::MENU_STATE_ID);
			game.running = false;
		}
		else if(event.getEventType() == Event::Type::KEY_PRESS)
		{
			switch(event.getEventKeyCode())
			{
				case Keyboard::Key::ESCAPE:
					game.running = false;
					break;
				default:
					break;
			}
		}
	}
}

void RaceState::handlePhysics(float delta)
{
}
