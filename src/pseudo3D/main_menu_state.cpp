/*
 * menu_state.cpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#include <pseudo3D/main_menu_state.hpp>

using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Font;
using fgeal::Color;


int MainMenuState::getId() { return CarseGame::MAIN_MENU_STATE_ID; }

MainMenuState::MainMenuState(CarseGame* game)
: State(*game),
  fontMain(null)
{}

MainMenuState::~MainMenuState()
{
	if(fontMain != null) delete fontMain;
}

void MainMenuState::initialize()
{
	fontMain = new Font("font.ttf", 32);
}

void MainMenuState::onEnter()
{
	// todo
}

void MainMenuState::onLeave()
{
	// todo
}

void MainMenuState::render()
{
	fgeal::Display& display = fgeal::Display::getInstance();
	display.clear();
	fontMain->drawText("Press enter to start!", 84, 25, Color::AZURE);
}

void MainMenuState::update(float delta)
{
	this->handleInput();
}

void MainMenuState::handleInput()
{
	Event event;
	EventQueue& eventQueue = EventQueue::getInstance();

	while(eventQueue.hasEvents())
	{
		eventQueue.getNextEvent(&event);
		if(event.getEventType() == Event::Type::DISPLAY_CLOSURE)
		{
			game.running = false;
		}
		else if(event.getEventType() == Event::Type::KEY_PRESS)
		{
			switch(event.getEventKeyCode())
			{
				case Keyboard::Key::ESCAPE:
					game.running = false;
					break;
				case Keyboard::Key::ENTER:
					game.enterState(CarseGame::RACE_STATE_ID);
					break;
				default:
					break;
			}
		}
	}
}
