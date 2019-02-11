/*
 * main_menu_simple_list_state.cpp
 *
 *  Created on: 2 de ago de 2018
 *      Author: carlosfaruolo
 */

#include "main_menu_simple_list_state.hpp"

#include "carse_game.hpp"

using std::string;
using fgeal::Display;
using fgeal::Image;
using fgeal::Menu;
using fgeal::Font;
using fgeal::Color;
using fgeal::Keyboard;
using fgeal::Mouse;

int MainMenuSimpleListState::getId() { return CarseGame::MAIN_MENU_SIMPLE_LIST_STATE_ID; }

MainMenuSimpleListState::MainMenuSimpleListState(CarseGame* game)
: State(*game), game(*game), lastDisplaySize(),
  imgBackground(null), fntTitle(null),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null)
{}

MainMenuSimpleListState::~MainMenuSimpleListState()
{
	if(imgBackground != null) delete imgBackground;
	if(fntTitle != null) delete fntTitle;
}

void MainMenuSimpleListState::initialize()
{
	menu.setFont(new Font(game.sharedResources->font1Path), false);
	menu.setColor(Color::WHITE);
	menu.bgColor = Color::AZURE;
	menu.focusedEntryFontColor = Color::NAVY;
	menu.addEntry("Start race");
	menu.addEntry("Vehicle selection");
	menu.addEntry("Course selection");
	menu.addEntry("Options");
	menu.addEntry("Exit");

	imgBackground = new Image("assets/options-bg.jpg");

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;

	fntTitle = new Font(game.sharedResources->font2Path);
	strTitle = "Carse Project";

	strVersion = string("v")+CARSE_VERSION+" (fgeal v"+fgeal::VERSION+"/"+fgeal::ADAPTED_LIBRARY_NAME+" v"+fgeal::ADAPTED_LIBRARY_VERSION+")";
}

void MainMenuSimpleListState::onEnter()
{
	Display& display = game.getDisplay();

	// reload fonts if display size changed
	if(lastDisplaySize.x != display.getWidth() or lastDisplaySize.y != display.getHeight())
	{
		fntTitle->setFontSize(dip(32));
		menu.getFont().setFontSize(dip(18));
		lastDisplaySize.x = display.getWidth();
		lastDisplaySize.y = display.getHeight();
	}

	menu.setSelectedIndex(0);
	menu.bounds.x = 0.25f * display.getWidth();
	menu.bounds.y = 0.25f * display.getHeight();
	menu.bounds.w = 0.50f * display.getWidth();
	menu.bounds.h = 0.50f * display.getHeight();
}

void MainMenuSimpleListState::onLeave()
{}

void MainMenuSimpleListState::render()
{
	Display& display = game.getDisplay();
	display.clear();
	imgBackground->drawScaled(0, 0, scaledToSize(imgBackground, display));
	fntTitle->drawText(strTitle, 0.5*(display.getWidth() - fntTitle->getTextWidth(strTitle)), 0.05*(display.getHeight() - fntTitle->getHeight()), Color::WHITE);
	menu.draw();
	game.sharedResources->fontDev.drawText(strVersion, 4, 4, Color::CREAM);
}

void MainMenuSimpleListState::update(float delta)
{}

void MainMenuSimpleListState::onKeyPressed(Keyboard::Key key)
{
	switch(key)
	{
		case Keyboard::KEY_ESCAPE:
			game.running = false;
			break;

		case Keyboard::KEY_ENTER:
			sndCursorIn->play();
			switch(menu.getSelectedIndex())
			{
				case MENU_ITEM_RACE:     game.enterState(CarseGame::RACE_STATE_ID); break;
				case MENU_ITEM_VEHICLE:  game.enterState(game.logic.currentVehicleSelectionStateId); break;
				case MENU_ITEM_COURSE:   game.enterState(CarseGame::COURSE_SELECTION_STATE_ID); break;
				case MENU_ITEM_SETTINGS: game.enterState(CarseGame::OPTIONS_MENU_STATE_ID); break;
				case MENU_ITEM_EXIT:     game.running = false; break;
				default: break;
			}
			break;

		case Keyboard::KEY_ARROW_UP:
			menu.moveCursorUp();
			sndCursorMove->play();
			break;

		case Keyboard::KEY_ARROW_DOWN:
			menu.moveCursorDown();
			sndCursorMove->play();
			break;

		case Keyboard::KEY_2:
			game.logic.currentMainMenuStateId = CarseGame::MAIN_MENU_CLASSIC_LAYOUT_STATE_ID;
			game.enterState(game.logic.currentMainMenuStateId);
			break;

		default:break;
	}
}

void MainMenuSimpleListState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{
	if(button == fgeal::Mouse::BUTTON_LEFT)
	{
		if(menu.bounds.contains(x, y))
		{
			if(menu.getIndexAtLocation(x, y) == menu.getSelectedIndex())
			{
				sndCursorIn->play();
				this->onKeyPressed(Keyboard::KEY_ENTER);
			}
			else
			{
				sndCursorMove->play();
				menu.setSelectedIndexByLocation(x, y);
			}
		}
	}
}


void MainMenuSimpleListState::onJoystickAxisMoved(unsigned joystick, unsigned axis, float oldValue, float newValue)
{
	if(axis == 0)
	{
		if(newValue > 0.2)
			this->onKeyPressed(Keyboard::KEY_ARROW_RIGHT);
		if(newValue < -0.2)
			this->onKeyPressed(Keyboard::KEY_ARROW_LEFT);
	}
	if(axis == 1)
	{
		if(newValue > 0.2)
			this->onKeyPressed(Keyboard::KEY_ARROW_DOWN);
		if(newValue < -0.2)
			this->onKeyPressed(Keyboard::KEY_ARROW_UP);
	}
}

void MainMenuSimpleListState::onJoystickButtonPressed(unsigned joystick, unsigned button)
{
	if(button % 2 == 0)
		this->onKeyPressed(Keyboard::KEY_ENTER);
}
