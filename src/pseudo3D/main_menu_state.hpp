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
#include "gui/menu.hpp"

#include "futil/general/language.hpp"
#include "fgeal/fgeal.hpp"

class MainMenuState extends public fgeal::Game::State
{
	fgeal::Font* fontMain;
	Menu* menu;

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
	void onMenuSelect();
};

#endif /* PSEUDO3D_MAIN_MENU_STATE_HPP_ */
