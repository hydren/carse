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
  menu(null), fontDev(null),
  layout(null)
{}

MainMenuState::~MainMenuState()
{
	if(menu != null) delete menu;
	if(fontDev != null) delete fontDev;
	if(layout != null) delete layout;
}

void MainMenuState::initialize()
{
	menu = new Menu(fgeal::Rectangle(), new Font("assets/font.ttf", 18), Color::WHITE);
	menu->fontIsOwned = true;
	menu->bgColor = Color::AZURE;
	menu->focusedEntryFontColor = Color::NAVY;
	menu->addEntry("Start debug course");
	menu->addEntry("Start random course");
	menu->addEntry("Start a loaded course");
	menu->addEntry("Exit");

	fontDev = new Font("assets/font.ttf", 12);
	layout = new PrototypeSimpleLayout(*this);
}

void MainMenuState::onEnter()
{
	layout->pack(game.getDisplay());
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
				{
					layout->onQuit();
					break;
				}
				case Keyboard::KEY_ENTER:
				{
					layout->onCursorAccept();
					break;
				}
				case Keyboard::KEY_ARROW_UP:
				{
					layout->navigate(Layout::NAV_UP);
					break;
				}
				case Keyboard::KEY_ARROW_DOWN:
				{
					layout->navigate(Layout::NAV_DOWN);
					break;
				}
				case Keyboard::KEY_ARROW_LEFT:
				{
					layout->navigate(Layout::NAV_LEFT);
					break;
				}
				case Keyboard::KEY_ARROW_RIGHT:
				{
					layout->navigate(Layout::NAV_RIGHT);
					break;
				}
				default:break;
			}
		}
	}
}

void MainMenuState::menuSelectionAction()
{
	if(menu->getSelectedIndex() == 0 or menu->getSelectedIndex() == 1)
	{
		const bool isDebug = (menu->getSelectedIndex() == 0);
		Pseudo3DRaceState::getInstance(game)->setCourse(isDebug? Course::createDebugCourse(200, 3000) : Course::createRandomCourse(200, 3000, 6400, 1.5));
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

// ============================================================================================
// Layout

MainMenuState::Layout::Layout(MainMenuState& state)
: state(state)
{}

MainMenuState::Layout::~Layout()
{}

void MainMenuState::Layout::onCursorAccept()
{
	state.menuSelectionAction();
}

void MainMenuState::Layout::onQuit()
{
	state.game.running = false;
}

// -------------------------------------------------
// PrototypeSimpleLayout

MainMenuState::PrototypeSimpleLayout::PrototypeSimpleLayout(MainMenuState& state)
: Layout(state),
  fontMain("assets/font.ttf", 32),
  sndCursorMove("assets/sound/cursor_move.ogg"),
  sndCursorAccept("assets/sound/cursor_accept.ogg")
{}

void MainMenuState::PrototypeSimpleLayout::pack(Display& display)
{
	Rectangle menuBounds = {
		0.125f*display.getWidth(), 0.5f*display.getHeight(),
		0.4f*display.getWidth(), 0.4f*display.getHeight()
	};

	state.menu->bounds = menuBounds;
}

void MainMenuState::PrototypeSimpleLayout::draw()
{
	state.menu->draw();
	fontMain.drawText("Carse Project", 84, 25, Color::WHITE);
}

void MainMenuState::PrototypeSimpleLayout::update(float delta)
{}

void MainMenuState::PrototypeSimpleLayout::navigate(NavigationDirection navDir)
{
	switch(navDir)
	{
		case NAV_UP:
		{
			state.menu->cursorUp();
			this->onCursorChange();
			break;
		}
		case NAV_DOWN:
		{
			state.menu->cursorDown();
			this->onCursorChange();
			break;
		}
		case NAV_LEFT:
		case NAV_RIGHT:
		default:break;
	}
}

void MainMenuState::PrototypeSimpleLayout::onCursorChange()
{
	sndCursorMove.play();
}

void MainMenuState::PrototypeSimpleLayout::onCursorAccept()
{
	sndCursorAccept.play();
	Layout::onCursorAccept();
}
