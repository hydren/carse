/*
 * menu_state.hpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_MENU_STATE_HPP_
#define PSEUDO3D_MENU_STATE_HPP_
#include <ciso646>

#include "carse_game.hpp"

#include "futil/general/language.hpp"
#include "fgeal/fgeal.hpp"

class MenuState extends public fgeal::Game::State
{
	public:
	int getId();

	MenuState(CarseGame* game);
	~MenuState();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	private:
	void handleInput();
};

#endif /* PSEUDO3D_MENU_STATE_HPP_ */
