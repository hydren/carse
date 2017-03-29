/*
 * carse_game.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef CARSE_GAME_HPP_
#define CARSE_GAME_HPP_
#include <ciso646>

#include "fgeal/extra/game.hpp"
#include "futil/general/language.hpp"

#define PSEUDO_3D_MODE

class CarseGame extends public fgeal::Game
{
	public:
	static const int RACE_STATE_ID, MENU_STATE_ID;

	CarseGame();
	void initializeStatesList();
};

#endif /* CARSE_GAME_HPP_ */
