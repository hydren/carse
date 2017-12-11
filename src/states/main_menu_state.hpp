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

	GenericMenuStateLayout<MainMenuState>* layout;
	friend class GenericMenuStateLayout<MainMenuState>;

	// todo once this PrototypeSimpleLayout becames time-tested, make it all virtual (and remove Prototype prefix)
	struct PrototypeSimpleLayout extends GenericMenuStateLayout<MainMenuState>
	{
		fgeal::Font fontMain;
		PrototypeSimpleLayout(MainMenuState& state);
		void draw();
		void update(float delta);
		void navigate(NavigationDirection navDir);
		void onCursorChange();
		void onCursorAccept();
	};

	// todo once this PrototypeGridLayout becames time-tested, make it all virtual (and remove Prototype prefix)
	struct PrototypeGridLayout extends GenericMenuStateLayout<MainMenuState>
	{
		fgeal::Font fontMain, fontTitle;
		fgeal::Color selectedSlotColor;
		PrototypeGridLayout(MainMenuState& state);
		void drawGridSlot(const fgeal::Rectangle&, const fgeal::Vector2D&, int);
		void draw();
		void update(float delta);
		void navigate(NavigationDirection navDir);
		void onCursorAccept();
	};

	public:
	int getId();

	MainMenuState(Pseudo3DCarseGame* game);
	~MainMenuState();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	private:
	void handleInput();
	void menuSelectionAction();
};

#endif /* MAIN_MENU_STATE_HPP_ */
