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

class Pseudo3DCarseGame extends public fgeal::Game
{
	public:
	static const int RACE_STATE_ID, MAIN_MENU_STATE_ID, CHOOSE_VEHICLE_STATE_ID;

	Pseudo3DCarseGame();
	void initializeStatesList();
};

typedef Pseudo3DCarseGame CarseGame;

#endif /* CARSE_GAME_HPP_ */
