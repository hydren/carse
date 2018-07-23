/*
 * menu_state.cpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "main_menu_state.hpp"
#include "main_menu_item.h"

#include "carse_game.hpp"

#include "futil/string_extra_operators.hpp"

#include <algorithm>
#include <cmath>

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

typedef GenericMenuStateLayout<MainMenuState> Layout;

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
	if(imgBackground != null) delete imgBackground;
	if(imgRace != null) delete imgRace;
	if(imgExit != null) delete imgExit;
	if(imgSettings != null) delete imgSettings;
}

void MainMenuState::initialize()
{
	Display& display = game.getDisplay();
	menu = new Menu();
	menu->setFont(new Font(shared.font1Path, dip(18)), false);
	menu->setColor(Color::WHITE);
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

	const static string txtVersion = string("v")+CARSE_VERSION+" (fgeal v"+fgeal::VERSION+"/"+fgeal::ADAPTED_LIBRARY_NAME+" v"+fgeal::ADAPTED_LIBRARY_VERSION+")";
	shared.fontDev.drawText(txtVersion, 4, 4, Color::CREAM);
}

void MainMenuState::update(float delta)
{
	this->layout->update(delta);
}

void MainMenuState::onKeyPressed(Keyboard::Key key)
{
	switch(key)
	{
		case Keyboard::KEY_ESCAPE:
			layout->onQuit();
			break;

		case Keyboard::KEY_ENTER:
			layout->onCursorAccept();
			break;

		case Keyboard::KEY_ARROW_UP:
			layout->onCursorUp();
			break;

		case Keyboard::KEY_ARROW_DOWN:
			layout->onCursorDown();
			break;

		case Keyboard::KEY_ARROW_LEFT:
			layout->onCursorLeft();
			break;

		case Keyboard::KEY_ARROW_RIGHT:
			layout->onCursorRight();
			break;

		case Keyboard::KEY_1:
			delete layout;
			layout = new SimpleListLayout(*this);
			break;

		case Keyboard::KEY_2:
			delete layout;
			layout = new PrototypeGridLayout(*this);
			break;

		default:break;
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
