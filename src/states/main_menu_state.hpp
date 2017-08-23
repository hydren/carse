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

	struct Layout
	{
		MainMenuState& state;
		Layout(MainMenuState& state);
		virtual ~Layout();

		// draws the layout
		virtual void draw() abstract;

		// performs any logic-related updates, if needed
		virtual void update(float delta) abstract;

		// updates all stuff's sizes to fit the display properly, if needed
		virtual void pack(fgeal::Display& display) abstract;

		enum NavigationDirection { NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT };

		// action when user navigates
		virtual void navigate(NavigationDirection navDir) abstract;

		// action when user accept or selects and confirm a item of the menu
		virtual void onCursorAccept();

		// stuff to be done when exiting the menu
		virtual void onQuit();
	};

	Layout* layout;

	struct PrototypeSimpleLayout extends Layout
	{
		fgeal::Font fontMain;
		fgeal::Sound sndCursorMove, sndCursorAccept;
		PrototypeSimpleLayout(MainMenuState& state);
		virtual void draw();
		virtual void update(float delta);
		virtual void pack(fgeal::Display&);
		virtual void navigate(NavigationDirection navDir);
		virtual void onCursorChange();
		virtual void onCursorAccept();
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
