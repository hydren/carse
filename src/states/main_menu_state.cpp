/*
 * menu_state.cpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "main_menu_state.hpp"

#include "race_state.hpp"

#include "futil/string_extra_operators.hpp"

#include "vehicle_selection_state.hpp"
#include "course_selection_state.hpp"

#include <algorithm>

using fgeal::Display;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Font;
using fgeal::Color;
using fgeal::Sound;
using fgeal::Image;
using fgeal::Rectangle;
using fgeal::Menu;
using std::string;

int MainMenuState::getId() { return Pseudo3DCarseGame::MAIN_MENU_STATE_ID; }

MainMenuState::MainMenuState(CarseGame* game)
: State(*game),
  menu(null), fontDev(null), bg(null), imgRace(null), imgExit(null),
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
	menu->addEntry("Race!");
	menu->addEntry("Vehicle");
	menu->addEntry("Course");
	menu->addEntry("Exit");

	fontDev = new Font("assets/font.ttf", 12);
	bg = new Image("assets/bg-main.jpg");
	imgRace = new Image("assets/race.png");
	imgExit = new Image("assets/exit.png");

	layout = new PrototypeGridLayout(*this);
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
	bg->drawScaled(0, 0, display.getWidth()/(float)bg->getWidth(), display.getHeight()/(float)bg->getHeight());
	layout->draw();
	fontDev->drawText(string("Using fgeal v")+fgeal::VERSION+" on "+fgeal::ADAPTED_LIBRARY_NAME+" v"+fgeal::ADAPTED_LIBRARY_VERSION, 4, fgeal::Display::getInstance().getHeight() - fontDev->getHeight(), Color::CREAM);
}

void MainMenuState::update(float delta)
{
	this->handleInput();
	this->layout->update(delta);
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
	if(menu->getSelectedIndex() == 0)
		game.enterState(Pseudo3DCarseGame::RACE_STATE_ID);

	if(menu->getSelectedIndex() == 1)
		game.enterState(Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID);

	if(menu->getSelectedIndex() == 2)
		game.enterState(Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID);

	if(menu->getSelectedIndex() == 3)
		game.running = false;
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

void MainMenuState::PrototypeSimpleLayout::draw()
{
	const float w = state.game.getDisplay().getWidth(),
				h = state.game.getDisplay().getHeight();

	const Rectangle menuBounds = { 0.125f*w, 0.5f*h, 0.4f*w, 0.4f*h };

	state.menu->bounds = menuBounds;
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

// -------------------------------------------------
// PrototypeGridLayout

MainMenuState::PrototypeGridLayout::PrototypeGridLayout(MainMenuState& state)
: Layout(state),
  fontMain("assets/font.ttf", (1.0/27) * state.game.getDisplay().getHeight()),
  fontTitle("assets/font2.ttf", (4.0/27) * state.game.getDisplay().getHeight()),
  sndCursorMove("assets/sound/cursor_move.ogg"),
  sndCursorAccept("assets/sound/cursor_accept.ogg")
{}

void MainMenuState::PrototypeGridLayout::draw()
{
	const float w = state.game.getDisplay().getWidth(),
				h = state.game.getDisplay().getHeight(),
				marginX = w * 0.005, marginY = h * 0.005;

	Rectangle slot[4];
	slot[0].x = 0.025f*w;
	slot[0].y = 0.260f*h;
	slot[0].w = 0.450f*w;
	slot[0].h = 0.350f*h;

	slot[1].x = 0.525f*w;
	slot[1].y = 0.260f*h;
	slot[1].w = 0.450f*w;
	slot[1].h = 0.350f*h;

	slot[2].x = 0.025f*w;
	slot[2].y = 0.635f*h;
	slot[2].w = 0.450f*w;
	slot[2].h = 0.350f*h;

	slot[3].x = 0.525f*w;
	slot[3].y = 0.635f*h;
	slot[3].w = 0.450f*w;
	slot[3].h = 0.350f*h;

	for(unsigned i = 0; i < state.menu->getNumberOfEntries(); i++)
	{
		const bool isSelected = (i == state.menu->getSelectedIndex());
		Image::drawFilledRectangle(slot[i].x, slot[i].y, slot[i].w, slot[i].h, Color::DARK_GREY);
		Image::drawFilledRectangle(slot[i].x + marginX, slot[i].y + marginY, slot[i].w - marginX*2, slot[i].h - marginY*2, isSelected? Color::LIGHT_GREY : Color::GREY);
		const float textWidth = fontMain.getTextWidth(state.menu->at(i).label);
		fontMain.drawText(state.menu->at(i).label, slot[i].x + 0.5*(slot[i].w - textWidth), slot[i].y * 1.02f, isSelected? selectedSlotColor : Color::WHITE);

		switch(i)
		{
			case 0:
			{
				state.imgRace->drawScaled(slot[i].x*1.01, slot[i].y*1.01, slot[i].w * 0.98f / state.imgRace->getWidth(), slot[i].h * 0.98f / state.imgRace->getHeight());
				break;
			}
			case 1:
			{
				static_cast<VehicleSelectionState*>(state.game.getState(Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID))->drawVehiclePreview(slot[i].x*1.4, slot[i].y*1.75, 0.75);
				break;
			}
			case 2:
			{
				Image* portrait = static_cast<CourseSelectionState*>(state.game.getState(Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID))->getSelectedCoursePreview();
				portrait->drawScaled(slot[i].x*3, slot[i].y*1.1, slot[i].w * 0.75f / portrait->getWidth(), slot[i].h * 0.75f / portrait->getHeight());
				break;
			}
			case 3:
			{
				state.imgExit->drawScaled(slot[i].x*1.01, slot[i].y*1.01, slot[i].w * 0.98f / state.imgRace->getWidth(), slot[i].h * 0.98f / state.imgRace->getHeight());
				break;
			}
			default:break;
		}
	}

	const string title("Carse Project");
	fontTitle.drawText(title, 0.5*(state.game.getDisplay().getWidth() - fontTitle.getTextWidth(title)),
							  0.05*(state.game.getDisplay().getHeight() - fontTitle.getHeight()), Color::WHITE);
}

void MainMenuState::PrototypeGridLayout::update(float delta)
{
	selectedSlotColor = cos(20*fgeal::uptime()) > 0? Color::RED : Color::MAROON;
}

void MainMenuState::PrototypeGridLayout::navigate(NavigationDirection navDir)
{
	const unsigned index = state.menu->getSelectedIndex();
	switch(navDir)
	{
		case NAV_UP:
		{
			if(index > 1)
			{
				state.menu->setSelectedIndex(index-2);
				sndCursorMove.play();
			}
			break;
		}
		case NAV_DOWN:
		{
			if(index < 2)
			{
				state.menu->setSelectedIndex(index+2);
				sndCursorMove.play();
			}
			break;
		}
		case NAV_LEFT:
		{
			if(index % 2 == 1)
			{
				state.menu->setSelectedIndex(index-1);
				sndCursorMove.play();
			}
			break;
		}
		case NAV_RIGHT:
		{
			if(index % 2 == 0)
			{
				state.menu->setSelectedIndex(index+1);
				sndCursorMove.play();
			}
			break;
		}
		default:break;
	}
}

void MainMenuState::PrototypeGridLayout::onCursorAccept()
{
	sndCursorAccept.play();
	Layout::onCursorAccept();
}
