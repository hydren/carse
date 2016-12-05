/*
 * carse_game.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef RACING_CARSE_GAME_HPP_
#define RACING_CARSE_GAME_HPP_

#include "fgeal/extra/game.hpp"
#include "futil/general/language.hpp"

using fgeal::Game;

class CarseGame extends public Game
{
	public:
	static const int RACE_STATE_ID, MENU_STATE_ID;

	CarseGame();
	void initializeStatesList();
};

#endif /* RACING_CARSE_GAME_HPP_ */
