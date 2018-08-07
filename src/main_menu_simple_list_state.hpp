/*
 * main_menu_simple_list_state.hpp
 *
 *  Created on: 2 de ago de 2018
 *      Author: carlosfaruolo
 */

#ifndef MAIN_MENU_SIMPLE_LIST_STATE_HPP_
#define MAIN_MENU_SIMPLE_LIST_STATE_HPP_
#include <ciso646>

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"

#include "util.hpp"

class CarseGame;
class CarseSharedResources;

class MainMenuSimpleListState extends public fgeal::Game::State
{
	CarseGame& game;
	CarseSharedResources& shared;

	// version string
	std::string strVersion;

	// the menu
	fgeal::Menu* menu;
	std::string strTitle;

	// the background image
	fgeal::Image* imgBackground;

	// title font
	fgeal::Font* fntTitle;

	// these guys helps giving semantics to menu indexes.
	enum MenuItem
	{
		MENU_ITEM_RACE = 0,
		MENU_ITEM_VEHICLE = 1,
		MENU_ITEM_COURSE = 2,
		MENU_ITEM_SETTINGS = 3,
		MENU_ITEM_EXIT = 4
	};

	public:
	MainMenuSimpleListState(CarseGame* game);
	~MainMenuSimpleListState();

	virtual int getId();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void onKeyPressed(fgeal::Keyboard::Key k);

	virtual void render();
	virtual void update(float delta);
};

#endif /* MAIN_MENU_SIMPLE_LIST_STATE_HPP_ */
