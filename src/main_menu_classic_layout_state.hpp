/*
 * main_menu_classic_layout_state.hpp
 *
 *  Created on: 2 de ago de 2018
 *      Author: carlosfaruolo
 */

#ifndef MAIN_MENU_CLASSIC_LAYOUT_STATE_HPP_
#define MAIN_MENU_CLASSIC_LAYOUT_STATE_HPP_
#include <ciso646>

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"

#include "futil/language.hpp"

#include "util.hpp"

class CarseGame;
class CarseSharedResources;

class MainMenuClassicPanelState extends public fgeal::Game::State
{
	CarseGame& game;
	CarseSharedResources& shared;

	std::string strVersion, strTitle;

	// the background image
	fgeal::Image* imgBackground;

	// ilustrations
	fgeal::Image* imgRace, *imgExit, *imgSettings, *imgCourse, *imgVehicle;

	// fonts
	fgeal::Font* fntTitle, *fntMain;

	// a list of strings with the items to be shown
	std::vector<std::string> vecStrItems;

	// the index of the currently focused item
	unsigned selectedItemIndex;

	fgeal::Color selectedSlotColor;

	// the geometry of the menu slots
	fgeal::Rectangle slotMenuItemRace, slotMenuItemVehicle, slotMenuItemCourse, slotMenuItemSettings, slotMenuItemExit;

	// used for vehicle preview
	fgeal::Point ptVehiclePreview;
	fgeal::Rectangle rtSrcVehiclePreview;
	fgeal::Vector2D scaleVehiclePreview;

	// these guys helps giving semantics to menu indexes.
	enum MenuItem
	{
		MENU_ITEM_RACE = 0,
		MENU_ITEM_VEHICLE = 1,
		MENU_ITEM_COURSE = 2,
		MENU_ITEM_SETTINGS = 3,
		MENU_ITEM_EXIT = 4
	};

	void drawGridSlot(const fgeal::Rectangle&, const fgeal::Vector2D&, int);
	void onMenuAccept();

	public:
	MainMenuClassicPanelState(CarseGame* game);
	~MainMenuClassicPanelState();

	virtual int getId();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void onKeyPressed(fgeal::Keyboard::Key k);
	virtual void onMouseButtonPressed(fgeal::Mouse::Button button, int x, int y);
	virtual void onMouseMoved(int oldx, int oldy, int newx, int newy);

	virtual void render();
	virtual void update(float delta);
};

#endif /* MAIN_MENU_CLASSIC_LAYOUT_STATE_HPP_ */
