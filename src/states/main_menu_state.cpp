/*
 * menu_state.cpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "main_menu_state.hpp"

#include "carse_game.hpp"

#include "futil/string_extra_operators.hpp"

#include <algorithm>
#include <cmath>

typedef GenericMenuStateLayout<MainMenuState> Layout;

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
: State(*game), logic(game->logic), shared(*game->sharedResources),
  menu(null), imgBackground(null), imgRace(null), imgExit(null), imgSettings(null),
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
	Display& display = game.getDisplay();
	menu = new Menu(fgeal::Rectangle(), new Font("assets/font.ttf", dip(18)), Color::WHITE);
	menu->fontIsOwned = true;
	menu->bgColor = Color::AZURE;
	menu->focusedEntryFontColor = Color::NAVY;
	menu->addEntry("Race!");
	menu->addEntry("Vehicle");
	menu->addEntry("Course");
	menu->addEntry("Options");
	menu->addEntry("Exit");

	imgBackground = new Image("assets/bg-main.jpg");
	imgRace = new Image("assets/race.png");
	imgExit = new Image("assets/exit.png");
	imgSettings = new Image("assets/settings.png");

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
	imgBackground->drawScaled(0, 0, scaledToSize(imgBackground, display));
	layout->draw();
	shared.fontDev.drawText(string("carse v")+CARSE_VERSION+" (using fgeal v"+fgeal::VERSION+" on "+fgeal::ADAPTED_LIBRARY_NAME+" v"+fgeal::ADAPTED_LIBRARY_VERSION+")", 4, 4, Color::CREAM);
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
				case Keyboard::KEY_1:
				{
					delete layout;
					layout = new PrototypeSimpleLayout(*this);
					break;
				}
				case Keyboard::KEY_2:
				{
					delete layout;
					layout = new PrototypeGridLayout(*this);
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

// -------------------------------------------------
// PrototypeSimpleLayout

MainMenuState::PrototypeSimpleLayout::PrototypeSimpleLayout(MainMenuState& state)
: Layout(state),
  fontMain("assets/font.ttf", 32 * (state.game.getDisplay().getHeight()/480.0))
{}

void MainMenuState::PrototypeSimpleLayout::draw()
{
	Display& display = state.game.getDisplay();

	state.menu->bounds.x = 0.25*display.getWidth();
	state.menu->bounds.y = 0.25*display.getHeight();
	state.menu->bounds.w = 0.5*display.getWidth();
	state.menu->bounds.h = 0.5*display.getHeight();

	state.menu->draw();
	const string title("Carse Project");
	fontMain.drawText(title, 0.5*(display.getWidth() - fontMain.getTextWidth(title)),
							  0.05*(display.getHeight() - fontMain.getHeight()), Color::WHITE);
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
  fontMain("assets/font.ttf",   18 * (state.game.getDisplay().getHeight()/480.0)),
  fontTitle("assets/font2.ttf", 72 * (state.game.getDisplay().getHeight()/480.0))
{}

void MainMenuState::PrototypeGridLayout::drawGridSlot(const fgeal::Rectangle& slot, const fgeal::Vector2D& margin, int index)
{
	const bool isSelected = (index == (int) state.menu->getSelectedIndex());
	Image::drawFilledRectangle(slot.x, slot.y, slot.w, slot.h, Color::DARK_GREY);
	Image::drawFilledRectangle(slot.x + margin.x, slot.y + margin.y, slot.w - margin.x*2, slot.h - margin.y*2, isSelected? Color::LIGHT_GREY : Color::GREY);
	const float textWidth = fontMain.getTextWidth(state.menu->at(index).label);
	fontMain.drawText(state.menu->at(index).label, slot.x + 0.5*(slot.w - textWidth), slot.y * 1.02f, isSelected? selectedSlotColor : Color::WHITE);
	if(isSelected) Image::drawRectangle(slot.x, slot.y, slot.w, slot.h, selectedSlotColor);
}

void MainMenuState::PrototypeGridLayout::draw()
{
	const float w = state.game.getDisplay().getWidth(),
				h = state.game.getDisplay().getHeight(),
				titleHeaderHeight = 0.2 * w,
				marginX = w * 0.01, marginY = h * 0.01;

	const fgeal::Vector2D margin = {marginX, marginY};

	const Rectangle slotSize = {0, 0, (w - 4*marginX)/3.0f, (h - titleHeaderHeight - 3*marginY)/2.0f};

	// draw MENU_ITEM_RACE
	const Rectangle slotMenuItemRace = {
		0.5f*(w - slotSize.w),
		titleHeaderHeight + 0.5f*(h - titleHeaderHeight - slotSize.h),
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemRace, margin, MENU_ITEM_RACE);
	state.imgRace->drawScaled(slotMenuItemRace.x*1.01, slotMenuItemRace.y*1.01,
		slotMenuItemRace.w * 0.98f / state.imgRace->getWidth(), slotMenuItemRace.h * 0.98f / state.imgRace->getHeight());

	// draw MENU_ITEM_VEHICLE
	const Rectangle slotMenuItemVehicle = {
		marginX,
		titleHeaderHeight + marginY,
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemVehicle, margin, MENU_ITEM_VEHICLE);
	state.logic.drawPickedVehicle(slotMenuItemVehicle.x + 0.5*slotMenuItemVehicle.w, slotMenuItemVehicle.y + 0.625*slotMenuItemVehicle.h, 0.75);

	// draw MENU_ITEM_COURSE
	const Rectangle slotMenuItemCourse = {
		marginX,
		slotMenuItemVehicle.y + slotMenuItemVehicle.h + marginY,
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemCourse, margin, MENU_ITEM_COURSE);
	Image* portrait = state.logic.getNextCoursePreviewImage();
	const float portraitX = slotMenuItemCourse.x + 0.125f*slotMenuItemCourse.w*(1 - 0.75f/portrait->getWidth()),
				portraitY = slotMenuItemCourse.y + 0.1750f*slotMenuItemCourse.h*(1 - 0.75f/portrait->getHeight()),
				portraitScaleX = 0.75f*slotMenuItemCourse.w/portrait->getWidth(),
				portraitScaleY = 0.75f*slotMenuItemCourse.h/portrait->getHeight();
	portrait->drawScaled(portraitX, portraitY, portraitScaleX, portraitScaleY);

	// draw MENU_ITEM_SETTINGS
	const Rectangle slotMenuItemSettings = {
		slotMenuItemRace.x + slotMenuItemRace.w + marginX,
		titleHeaderHeight + marginY,
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemSettings, margin, MENU_ITEM_SETTINGS);
	const float imgSettingsX = slotMenuItemSettings.x + 0.125f*slotMenuItemSettings.w*(1 - 0.75f/state.imgSettings->getWidth()),
				imgSettingsY = slotMenuItemSettings.y + 0.1750f*slotMenuItemSettings.h*(1 - 0.75f/state.imgSettings->getHeight()),
				imgSettingsScaleX = 0.75f*slotMenuItemSettings.w/state.imgSettings->getWidth(),
				imgSettingsScaleY = 0.75f*slotMenuItemSettings.h/state.imgSettings->getHeight();
	state.imgSettings->drawScaled(imgSettingsX, imgSettingsY, imgSettingsScaleX, imgSettingsScaleY);

	// draw MENU_ITEM_EXIT
	const Rectangle slotMenuItemExit = {
		slotMenuItemRace.x + slotMenuItemRace.w + marginX,
		slotMenuItemSettings.y + slotMenuItemSettings.h + marginY,
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemExit, margin, MENU_ITEM_EXIT);
	const float imgExitX = slotMenuItemExit.x + 0.125f*slotMenuItemExit.w*(1 - 0.75f/state.imgExit->getWidth()),
				imgExitY = slotMenuItemExit.y + 0.1750f*slotMenuItemExit.h*(1 - 0.75f/state.imgExit->getHeight()),
				imgExitScaleX = 0.75f*slotMenuItemExit.w/state.imgExit->getWidth(),
				imgExitScaleY = 0.75f*slotMenuItemExit.h/state.imgExit->getHeight();
	state.imgExit->drawScaled(imgExitX, imgExitY, imgExitScaleX, imgExitScaleY);

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
