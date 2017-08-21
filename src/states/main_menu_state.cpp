/*
 * menu_state.cpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "main_menu_state.hpp"

#include "race_state.hpp"

#include "futil/string_extra_operators.hpp"

using fgeal::Display;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Font;
using fgeal::Color;
using fgeal::Sound;
using fgeal::Rectangle;
using fgeal::Menu;
using std::string;

int MainMenuState::getId() { return Pseudo3DCarseGame::MAIN_MENU_STATE_ID; }

MainMenuState::MainMenuState(CarseGame* game)
: State(*game),
  layout(null), fontDev(null)
{}

MainMenuState::~MainMenuState()
{
	if(layout != null) delete layout;
	if(fontDev != null) delete fontDev;
}

void MainMenuState::initialize()
{
	layout = new PrototypeSimpleLayout();
	fontDev = new Font("assets/font.ttf", 12);
}

void MainMenuState::onEnter()
{
	layout->updateBounds(game.getDisplay());
	layout->menu.setSelectedIndex(0);
}

void MainMenuState::onLeave()
{
	// todo
}

void MainMenuState::render()
{
	Display& display = Display::getInstance();
	display.clear();
	layout->draw();
	fontDev->drawText(string("Using fgeal v")+fgeal::VERSION+" on "+fgeal::ADAPTED_LIBRARY_NAME+" v"+fgeal::ADAPTED_LIBRARY_VERSION, 4, fgeal::Display::getInstance().getHeight() - fontDev->getHeight(), Color::CREAM);
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
		if(event.getEventType() == Event::TYPE_DISPLAY_CLOSURE)
		{
			game.running = false;
		}
		else if(event.getEventType() == Event::TYPE_KEY_PRESS)
		{
			switch(event.getEventKeyCode())
			{
				case Keyboard::KEY_ESCAPE:
					game.running = false;
					break;
				case Keyboard::KEY_ENTER:
					this->onMenuSelect();
					layout->onCursorAccept();
					break;
				case Keyboard::KEY_ARROW_UP:
					layout->menu.cursorUp();
					layout->onCursorChange();
					break;
				case Keyboard::KEY_ARROW_DOWN:
					layout->menu.cursorDown();
					layout->onCursorChange();
					break;
				default:
					break;
			}
		}
	}
}

void MainMenuState::onMenuSelect()
{
	if(layout->menu.getSelectedIndex() == 0 or layout->menu.getSelectedIndex() == 1)
	{
		const bool isDebug = (layout->menu.getSelectedIndex() == 0);
		Pseudo3DRaceState::getInstance(game)->setCourse(isDebug? Course::createDebugCourse(200, 3000) : Course::createRandomCourse(200, 3000, 6400, 1.5));
		game.enterState(Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID);
	}

	else if(layout->menu.getSelectedIndex() == 2)
	{
		game.enterState(Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID);
	}

	else if(layout->menu.getSelectedIndex() == 3)
	{
		game.running = false;
	}
}

MainMenuState::Layout::Layout()
: menu(fgeal::Rectangle(), new Font("assets/font.ttf", 18), Color::WHITE)
{
	menu.fontIsOwned = true;
	menu.bgColor = Color::AZURE;
	menu.focusedEntryFontColor = Color::NAVY;
	menu.addEntry("Start debug course");
	menu.addEntry("Start random course");
	menu.addEntry("Start a loaded course");
	menu.addEntry("Exit");
}

MainMenuState::Layout::~Layout()
{}

MainMenuState::PrototypeSimpleLayout::PrototypeSimpleLayout()
: fontMain("assets/font.ttf", 32),
  sndCursorMove("assets/sound/cursor_move.ogg"), sndCursorAccept("assets/sound/cursor_accept.ogg")
{}

void MainMenuState::PrototypeSimpleLayout::updateBounds(Display& display)
{
	Rectangle menuBounds = {
		0.125f*display.getWidth(), 0.5f*display.getHeight(),
		0.4f*display.getWidth(), 0.4f*display.getHeight()
	};

	menu.bounds = menuBounds;
}

void MainMenuState::PrototypeSimpleLayout::draw()
{
	menu.draw();
	fontMain.drawText("Carse Project", 84, 25, Color::WHITE);
}

void MainMenuState::PrototypeSimpleLayout::onCursorChange()
{
	sndCursorAccept.play();
}

void MainMenuState::PrototypeSimpleLayout::onCursorAccept()
{
	sndCursorAccept.play();
}
