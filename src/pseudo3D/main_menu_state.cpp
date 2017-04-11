/*
 * menu_state.cpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "main_menu_state.hpp"

#include "race_state.hpp"

using fgeal::Display;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Font;
using fgeal::Color;

int MainMenuState::getId() { return CarseGame::MAIN_MENU_STATE_ID; }

MainMenuState::MainMenuState(CarseGame* game)
: State(*game),
  fontMain(null),
  menu(null)
{}

MainMenuState::~MainMenuState()
{
	if(fontMain != null) delete fontMain;
	if(menu != null) delete menu;
}

void MainMenuState::initialize()
{
	Display& display = Display::getInstance();
	Rectangle menuBounds = {0.125f*display.getWidth(), 0.5f*display.getHeight(), 0.4f*display.getWidth(), 0.4f*display.getHeight()};
	fontMain = new Font("assets/font.ttf", 32);
	menu = new Menu(menuBounds, new Font("assets/font.ttf", 18), Color::WHITE);
	menu->manageFontDeletion = true;
	menu->bgColor = Color::AZURE;
	menu->selectedColor = Color::NAVY;
	menu->addEntry("Start debug course");
	menu->addEntry("Start random course");
	menu->addEntry("Exit");
}

void MainMenuState::onEnter()
{
	menu->setSelectedIndex(0);
}

void MainMenuState::onLeave()
{
	// todo
}

void MainMenuState::render()
{
	Display& display = Display::getInstance();
	display.clear();
	menu->draw();
	fontMain->drawText("Carse Project", 84, 25, Color::WHITE);
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
					this->onMenuSelect();
					break;
				case Keyboard::Key::ARROW_UP:
					menu->cursorUp();
					break;
				case Keyboard::Key::ARROW_DOWN:
					menu->cursorDown();
					break;
				default:
					break;
			}
		}
	}
}

void MainMenuState::onMenuSelect()
{
	if(menu->getSelectedIndex() == 0 or menu->getSelectedIndex() == 1)
	{
		const bool isDebug = (menu->getSelectedIndex() == 0);
		Pseudo3DRaceState* raceState = static_cast<Pseudo3DRaceState*>(game.getState(CarseGame::RACE_STATE_ID));
		raceState->setCourse(isDebug? Course::createDebugCourse(200, 2000) : Course::createRandomCourse(200, 2000, 6400, 2.0));
		game.enterState(CarseGame::CHOOSE_VEHICLE_STATE_ID);
	}

	else if(menu->getSelectedIndex() == 2)
	{
		game.running = false;
	}
}