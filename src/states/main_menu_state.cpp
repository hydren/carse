/*
 * menu_state.cpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "main_menu_state.hpp"

#include "race_state.hpp"

#include "futil/string/more_operators.hpp"

using fgeal::Display;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Font;
using fgeal::Color;
using fgeal::Rectangle;
using fgeal::Menu;
using std::string;

int MainMenuState::getId() { return Pseudo3DCarseGame::MAIN_MENU_STATE_ID; }

MainMenuState::MainMenuState(CarseGame* game)
: State(*game),
  fontMain(null), fontDev(null),
  menu(null)
{}

MainMenuState::~MainMenuState()
{
	if(fontMain != null) delete fontMain;
	if(fontDev != null) delete fontDev;
	if(menu != null) delete menu;
}

void MainMenuState::initialize()
{
	Display& display = Display::getInstance();
	Rectangle menuBounds = {0.125f*display.getWidth(), 0.5f*display.getHeight(), 0.4f*display.getWidth(), 0.4f*display.getHeight()};
	fontMain = new Font("assets/font.ttf", 32);
	fontDev = new Font("assets/font.ttf", 12);
	menu = new Menu(menuBounds, new Font("assets/font.ttf", 18), Color::WHITE);
	menu->fontIsOwned = true;
	menu->bgColor = Color::AZURE;
	menu->focusedEntryFontColor = Color::NAVY;
	menu->addEntry("Start debug course");
	menu->addEntry("Start random course");
	menu->addEntry("Start a loaded course");
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
	fontDev->drawText(string("Using fgeal v")+fgeal::VERSION+" on "+fgeal::ADAPTED_LIBRARY_NAME+" v"+fgeal::ADAPTED_LIBRARY_VERSION, 4, fgeal::Display::getInstance().getHeight() - fontDev->getFontHeight(), Color::CREAM);
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
		raceState->setCourse(isDebug? Course::createDebugCourse(200, 3000) : Course::createRandomCourse(200, 3000, 6400, 2.0));
		game.enterState(Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID);
	}

	else if(menu->getSelectedIndex() == 2)
	{
		game.enterState(Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID);
	}

	else if(menu->getSelectedIndex() == 3)
	{
		game.running = false;
	}
}
