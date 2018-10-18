/*
 * options_menu_state.hpp
 *
 *  Created on: 5 de set de 2017
 *      Author: carlosfaruolo
 */

#ifndef OPTIONS_MENU_STATE_HPP_
#define OPTIONS_MENU_STATE_HPP_
#include <ciso646>

#include "carse_game.hpp"

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"

class CarseGame;

class OptionsMenuState extends public fgeal::Game::State
{
	CarseGame& game;

	fgeal::Menu* menu, *menuResolution;
	fgeal::Font* fontTitle, *font;
	fgeal::Image* background;

	fgeal::Sound* sndCursorMove, *sndCursorIn, *sndCursorOut;

	bool isResolutionMenuActive;

	// these guys helps giving semantics to menu indexes.
	enum MenuItem
	{
		MENU_ITEM_RESOLUTION = 0,
		MENU_ITEM_FULLSCREEN = 1,
		MENU_ITEM_UNIT = 2,
		MENU_ITEM_SIMULATION_TYPE = 3,
		MENU_ITEM_TACHOMETER_TYPE = 4,
		MENU_ITEM_CACHE_TACHOMETER = 5
	};

	public:
	virtual int getId();

	OptionsMenuState(CarseGame* game);
	~OptionsMenuState();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void render();
	virtual void update(float delta);

	virtual void onKeyPressed(fgeal::Keyboard::Key);
	virtual void onMouseButtonPressed(fgeal::Mouse::Button button, int x, int y);
	virtual void onMouseMoved(int oldx, int oldy, int x, int y);

	private:
	void onMenuSelect();
	void updateOnResolutionMenu(fgeal::Keyboard::Key);
	void updateLabels();
	void setResolution();

	void updateFonts();
};

#endif /* OPTIONS_MENU_STATE_HPP_ */
