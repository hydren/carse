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
: State(*game), shared(*game->sharedResources),
  menu(null), bg(null), imgRace(null), imgExit(null), imgSettigns(null),
  layout(null)
{}

MainMenuState::~MainMenuState()
{
	if(menu != null) delete menu;
	if(layout != null) delete layout;
}

// xxx THESE ENUM MUST MATCH MENU ENTRIES' ORDER.
// these guys helps giving semantics to menu indexes.
enum MenuItem
{
	MENU_ITEM_RACE = 0,
	MENU_ITEM_VEHICLE = 1,
	MENU_ITEM_COURSE = 2,
	MENU_ITEM_SETTINGS = 3,
	MENU_ITEM_EXIT = 4
};

void MainMenuState::initialize()
{
	menu = new Menu(fgeal::Rectangle(), new Font("assets/font.ttf", 18), Color::WHITE);
	menu->fontIsOwned = true;
	menu->bgColor = Color::AZURE;
	menu->focusedEntryFontColor = Color::NAVY;
	menu->addEntry("Race!");
	menu->addEntry("Vehicle");
	menu->addEntry("Course");
	menu->addEntry("Settings");
	menu->addEntry("Exit");

	bg = new Image("assets/bg-main.jpg");
	imgRace = new Image("assets/race.png");
	imgExit = new Image("assets/exit.png");
	imgSettigns = new Image("assets/settings.png");

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
	Display& display = game.getDisplay();
	display.clear();
	bg->drawScaled(0, 0, display.getWidth()/(float)bg->getWidth(), display.getHeight()/(float)bg->getHeight());
	layout->draw();
	shared.fontDev.drawText(string("Using fgeal v")+fgeal::VERSION+" on "+fgeal::ADAPTED_LIBRARY_NAME+" v"+fgeal::ADAPTED_LIBRARY_VERSION, 4, display.getHeight() - shared.fontDev.getHeight(), Color::CREAM);
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
	switch(menu->getSelectedIndex())
	{
		case MENU_ITEM_RACE: game.enterState(Pseudo3DCarseGame::RACE_STATE_ID); break;
		case MENU_ITEM_VEHICLE: game.enterState(Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID); break;
		case MENU_ITEM_COURSE: game.enterState(Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID); break;
		case MENU_ITEM_SETTINGS: game.enterState(Pseudo3DCarseGame::OPTIONS_MENU_STATE_ID); break;
		case MENU_ITEM_EXIT: game.running = false; break;
		default: break;
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
  fontMain("assets/font.ttf", 32)
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
	state.shared.sndCursorMove.stop();
	state.shared.sndCursorMove.play();
}

void MainMenuState::PrototypeSimpleLayout::onCursorAccept()
{
	state.shared.sndCursorIn.stop();
	state.shared.sndCursorIn.play();
	Layout::onCursorAccept();
}

// -------------------------------------------------
// PrototypeGridLayout

MainMenuState::PrototypeGridLayout::PrototypeGridLayout(MainMenuState& state)
: Layout(state),
  fontMain("assets/font.ttf", (1.0/27) * state.game.getDisplay().getHeight()),
  fontTitle("assets/font2.ttf", (4.0/27) * state.game.getDisplay().getHeight())
{}

void MainMenuState::PrototypeGridLayout::draw()
{
	const float w = state.game.getDisplay().getWidth(),
				h = state.game.getDisplay().getHeight(),
				titleHeaderHeight = 0.2 * w,
				marginX = w * 0.01, marginY = h * 0.01;

	Rectangle slots[5];
	{
		Rectangle& slot = slots[MENU_ITEM_RACE];
		slot.w = (w - 4*marginX)/3;
		slot.h = (h - titleHeaderHeight - 3*marginY)/2;
		slot.x = 0.5*(w - slot.w);
		slot.y = titleHeaderHeight + 0.5*(h - titleHeaderHeight - slot.h);
	}
	{
		Rectangle& slot = slots[MENU_ITEM_VEHICLE];
		slot.x = marginX;
		slot.y = titleHeaderHeight + marginY;
		slot.w = slots->w;
		slot.h = slots->h;
	}
	{
		Rectangle& slot = slots[MENU_ITEM_COURSE];
		slot.x = marginX;
		slot.y = slots[MENU_ITEM_VEHICLE].y + slots[MENU_ITEM_VEHICLE].h + marginY;
		slot.w = slots->w;
		slot.h = slots->h;
	}
	{
		Rectangle& slot = slots[MENU_ITEM_SETTINGS];
		slot.x = slots[MENU_ITEM_RACE].x + slots[MENU_ITEM_RACE].w + marginX;
		slot.y = titleHeaderHeight + marginY;
		slot.w = slots->w;
		slot.h = slots->h;
	}
	{
		Rectangle& slot = slots[MENU_ITEM_EXIT];
		slot.x = slots[MENU_ITEM_RACE].x + slots[MENU_ITEM_RACE].w + marginX;
		slot.y = slots[MENU_ITEM_SETTINGS].y + slots[MENU_ITEM_SETTINGS].h + marginY;
		slot.w = slots->w;
		slot.h = slots->h;
	}

	for(unsigned i = 0; i < state.menu->getNumberOfEntries(); i++)
	{
		const bool isSelected = (i == state.menu->getSelectedIndex());
		Image::drawFilledRectangle(slots[i].x, slots[i].y, slots[i].w, slots[i].h, Color::DARK_GREY);
		Image::drawFilledRectangle(slots[i].x + marginX, slots[i].y + marginY, slots[i].w - marginX*2, slots[i].h - marginY*2, isSelected? Color::LIGHT_GREY : Color::GREY);
		const float textWidth = fontMain.getTextWidth(state.menu->at(i).label);
		fontMain.drawText(state.menu->at(i).label, slots[i].x + 0.5*(slots[i].w - textWidth), slots[i].y * 1.02f, isSelected? selectedSlotColor : Color::WHITE);

		switch(i)
		{
			case MENU_ITEM_RACE:
			{
				state.imgRace->drawScaled(slots[i].x*1.01, slots[i].y*1.01, slots[i].w * 0.98f / state.imgRace->getWidth(), slots[i].h * 0.98f / state.imgRace->getHeight());
				break;
			}
			case MENU_ITEM_VEHICLE:
			{
				static_cast<VehicleSelectionState*>(state.game.getState(Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID))->drawVehiclePreview(slots[i].x + 0.5*slots[i].w, slots[i].y + 0.625*slots[i].h, 0.75);
				break;
			}
			case MENU_ITEM_COURSE:
			{
				Image* portrait = static_cast<CourseSelectionState*>(state.game.getState(Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID))->getSelectedCoursePreview();
				const float portraitX = slots[i].x + 0.125f*slots[i].w*(1 - 0.75f/portrait->getWidth()),
							portraitY = slots[i].y + 0.1750f*slots[i].h*(1 - 0.75f/portrait->getHeight()),
							portraitScaleX = 0.75f*slots[i].w/portrait->getWidth(),
							portraitScaleY = 0.75f*slots[i].h/portrait->getHeight();
				portrait->drawScaled(portraitX, portraitY, portraitScaleX, portraitScaleY);
				break;
			}
			case MENU_ITEM_SETTINGS:
			{
				const float imgX = slots[i].x + 0.125f*slots[i].w*(1 - 0.75f/state.imgSettigns->getWidth()),
							imgY = slots[i].y + 0.1750f*slots[i].h*(1 - 0.75f/state.imgSettigns->getHeight()),
							imgScaleX = 0.75f*slots[i].w/state.imgSettigns->getWidth(),
							imgScaleY = 0.75f*slots[i].h/state.imgSettigns->getHeight();
				state.imgSettigns->drawScaled(imgX, imgY, imgScaleX, imgScaleY);
				break;
			}
			case MENU_ITEM_EXIT:
			{
				const float imgX = slots[i].x + 0.125f*slots[i].w*(1 - 0.75f/state.imgExit->getWidth()),
							imgY = slots[i].y + 0.1750f*slots[i].h*(1 - 0.75f/state.imgExit->getHeight()),
							imgScaleX = 0.75f*slots[i].w/state.imgExit->getWidth(),
							imgScaleY = 0.75f*slots[i].h/state.imgExit->getHeight();
				state.imgExit->drawScaled(imgX, imgY, imgScaleX, imgScaleY);
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
			if(index == 2 or index == 4)
			{
				state.menu->setSelectedIndex(index-1);
				state.shared.sndCursorMove.stop();
				state.shared.sndCursorMove.play();
			}
			break;
		}
		case NAV_DOWN:
		{
			if(index == 1 or index == 3)
			{
				state.menu->setSelectedIndex(index+1);
				state.shared.sndCursorMove.stop();
				state.shared.sndCursorMove.play();
			}
			break;
		}
		case NAV_LEFT:
		{
			if(index == 0 or index == 3 or index == 4)
			{
				state.menu->setSelectedIndex(index==0? 1 : 0);
				state.shared.sndCursorMove.stop();
				state.shared.sndCursorMove.play();
			}
			break;
		}
		case NAV_RIGHT:
		{
			if(index == 0 or index == 1 or index == 2)
			{
				state.menu->setSelectedIndex(index==0? 3 : 0);
				state.shared.sndCursorMove.stop();
				state.shared.sndCursorMove.play();
			}
			break;
		}
		default:break;
	}
}

void MainMenuState::PrototypeGridLayout::onCursorAccept()
{
	state.shared.sndCursorIn.play();
	Layout::onCursorAccept();
}
