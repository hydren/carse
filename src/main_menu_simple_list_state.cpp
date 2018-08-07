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

int MainMenuSimpleListState::getId() { return CarseGame::MAIN_MENU_SIMPLE_LIST_STATE_ID; }

MainMenuSimpleListState::MainMenuSimpleListState(CarseGame* game)
: State(*game), game(*game), shared(*game->sharedResources), menu(null),
  imgBackground(null), fntTitle(null)
{}

MainMenuSimpleListState::~MainMenuSimpleListState()
{
	if(menu != null) delete menu;
	if(imgBackground != null) delete imgBackground;
	if(fntTitle != null) delete fntTitle;
}

void MainMenuSimpleListState::initialize()
{
	Display& display = game.getDisplay();
	menu = new Menu();
	menu->setFont(new Font(shared.font1Path, dip(18)), false);
	menu->setColor(Color::WHITE);
	menu->bgColor = Color::AZURE;
	menu->focusedEntryFontColor = Color::NAVY;
	menu->addEntry("Start race");
	menu->addEntry("Vehicle selection");
	menu->addEntry("Course selection");
	menu->addEntry("Options");
	menu->addEntry("Exit");

	imgBackground = new Image("assets/options-bg.jpg");

	fntTitle = new Font(shared.font2Path, dip(32));
	strTitle = "Carse Project";

	strVersion = string("v")+CARSE_VERSION+" (fgeal v"+fgeal::VERSION+"/"+fgeal::ADAPTED_LIBRARY_NAME+" v"+fgeal::ADAPTED_LIBRARY_VERSION+")";
}

void MainMenuSimpleListState::onEnter()
{
	Display& display = game.getDisplay();
	menu->setSelectedIndex(0);
	menu->bounds.x = 0.25f * display.getWidth();
	menu->bounds.y = 0.25f * display.getHeight();
	menu->bounds.w = 0.50f * display.getWidth();
	menu->bounds.h = 0.50f * display.getHeight();
}

void MainMenuSimpleListState::onLeave()
{}

void MainMenuSimpleListState::render()
{
	Display& display = game.getDisplay();
	display.clear();
	imgBackground->drawScaled(0, 0, scaledToSize(imgBackground, display));
	fntTitle->drawText(strTitle, 0.5*(display.getWidth() - fntTitle->getTextWidth(strTitle)), 0.05*(display.getHeight() - fntTitle->getHeight()), Color::WHITE);
	menu->draw();
	shared.fontDev.drawText(strVersion, 4, 4, Color::CREAM);
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
			shared.sndCursorIn.stop();
			shared.sndCursorIn.play();
			switch(menu->getSelectedIndex())
			{
				case MENU_ITEM_RACE:     game.enterState(CarseGame::RACE_STATE_ID); break;
				case MENU_ITEM_VEHICLE:  game.enterState(CarseGame::VEHICLE_SELECTION_STATE_ID); break;
				case MENU_ITEM_COURSE:   game.enterState(CarseGame::COURSE_SELECTION_STATE_ID); break;
				case MENU_ITEM_SETTINGS: game.enterState(CarseGame::OPTIONS_MENU_STATE_ID); break;
				case MENU_ITEM_EXIT:     game.running = false; break;
				default: break;
			}
			break;

		case Keyboard::KEY_ARROW_UP:
			menu->moveCursorUp();
			shared.sndCursorMove.stop();
			shared.sndCursorMove.play();
			break;

		case Keyboard::KEY_ARROW_DOWN:
			menu->moveCursorDown();
			shared.sndCursorMove.stop();
			shared.sndCursorMove.play();
			break;

		case Keyboard::KEY_2:
			game.logic.setCurrentMainMenuStateId(CarseGame::MAIN_MENU_CLASSIC_LAYOUT_STATE_ID);
			game.enterState(CarseGame::MAIN_MENU_CLASSIC_LAYOUT_STATE_ID);
			break;

		default:break;
	}
}
