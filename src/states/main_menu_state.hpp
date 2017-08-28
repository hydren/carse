/*
 * menu_state.hpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_MAIN_MENU_STATE_HPP_
#define PSEUDO3D_MAIN_MENU_STATE_HPP_
#include <ciso646>

#include "carse_game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"
#include "fgeal/fgeal.hpp"

class MainMenuState extends public fgeal::Game::State
{
	// the menu
	fgeal::Menu* menu;

	// a font for debugging
	fgeal::Font* fontDev;

	// the background image
	fgeal::Image* bg;

	// ilustrations
	fgeal::Image* imgRace, *imgExit;

	struct Layout
	{
		MainMenuState& state;
		Layout(MainMenuState& state);
		virtual ~Layout();

		// draws the layout
		virtual void draw() abstract;

		// performs any logic-related updates, if needed
		virtual void update(float delta) abstract;

		enum NavigationDirection { NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT };

		// action when user navigates
		virtual void navigate(NavigationDirection navDir) abstract;

		// action when user accept or selects and confirm a item of the menu
		virtual void onCursorAccept();

		// stuff to be done when exiting the menu
		virtual void onQuit();
	};

	Layout* layout;

	// todo once this PrototypeSimpleLayout becames time-tested, make it all virtual (and remove Prototype prefix)
	struct PrototypeSimpleLayout extends Layout
	{
		fgeal::Font fontMain;
		fgeal::Sound sndCursorMove, sndCursorAccept;
		PrototypeSimpleLayout(MainMenuState& state);
		void draw();
		void update(float delta);
		void navigate(NavigationDirection navDir);
		void onCursorChange();
		void onCursorAccept();
	};

	// todo once this PrototypeGridLayout becames time-tested, make it all virtual (and remove Prototype prefix)
	struct PrototypeGridLayout extends Layout
	{
		fgeal::Font fontMain, fontTitle;
		fgeal::Sound sndCursorMove, sndCursorAccept;
		fgeal::Color selectedSlotColor;
		PrototypeGridLayout(MainMenuState& state);
		void draw();
		void update(float delta);
		void navigate(NavigationDirection navDir);
		void onCursorAccept();
	};

	public:
	int getId();

	MainMenuState(CarseGame* game);
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

#endif /* PSEUDO3D_MAIN_MENU_STATE_HPP_ */
