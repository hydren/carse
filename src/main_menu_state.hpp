/*
 * menu_state.hpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef MAIN_MENU_STATE_HPP_
#define MAIN_MENU_STATE_HPP_
#include <ciso646>

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"

#include "util.hpp"

class CarseGameLogic;
class CarseSharedResources;
class Pseudo3DCarseGame;

class MainMenuState extends public fgeal::Game::State
{
	CarseGameLogic& logic;
	CarseSharedResources& shared;

	// the menu
	fgeal::Menu* menu;

	// the background image
	fgeal::Image* imgBackground;

	// ilustrations
	fgeal::Image* imgRace, *imgExit, *imgSettings;

	// these guys helps giving semantics to menu indexes.
	enum MenuItem
	{
		MENU_ITEM_RACE = 0,
		MENU_ITEM_VEHICLE = 1,
		MENU_ITEM_COURSE = 2,
		MENU_ITEM_SETTINGS = 3,
		MENU_ITEM_EXIT = 4
	};

	GenericMenuStateLayout<MainMenuState>* layout;
	friend class GenericMenuStateLayout<MainMenuState>;

	struct SimpleListLayout extends GenericMenuStateLayout<MainMenuState>
	{
		fgeal::Font fontMain;
		SimpleListLayout(MainMenuState& state);
		virtual void draw();
		virtual void update(float delta);
		virtual void onCursorUp();
		virtual void onCursorDown();
		virtual void onCursorAccept();
	};

	// todo once this PrototypeGridLayout becames time-tested, make it all virtual (and remove Prototype prefix)
	struct PrototypeGridLayout extends GenericMenuStateLayout<MainMenuState>
	{
		fgeal::Font fontMain, fontTitle;
		fgeal::Color selectedSlotColor;
		PrototypeGridLayout(MainMenuState& state);
		void drawGridSlot(const fgeal::Rectangle&, const fgeal::Vector2D&, int);
		virtual void draw();
		virtual void update(float delta);
		virtual void onCursorUp();
		virtual void onCursorDown();
		virtual void onCursorLeft();
		virtual void onCursorRight();
		virtual void onCursorAccept();
	};

	public:
	virtual int getId();

	MainMenuState(Pseudo3DCarseGame* game);
	~MainMenuState();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void onKeyPressed(fgeal::Keyboard::Key k);

	virtual void render();
	virtual void update(float delta);

	private:
	void menuSelectionAction();
};

#endif /* MAIN_MENU_STATE_HPP_ */
