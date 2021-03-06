/*
 * carse_game.hpp
 *
 *  Created on: 12 de abr de 2017
 *      Author: carlosfaruolo
 */

#ifndef TOPDOWN_EXAMPLE_CARSE_GAME_HPP_
#define TOPDOWN_EXAMPLE_CARSE_GAME_HPP_
#include <ciso646>

#include "fgeal/extra/game.hpp"
#include "futil/language.hpp"

class TopdownCarseGame extends public fgeal::Game
{
	public:
	static const int RACE_STATE_ID;

	TopdownCarseGame();
	void initializeStatesList();
};

typedef TopdownCarseGame CarseGame;

#endif /* TOPDOWN_EXAMPLE_CARSE_GAME_HPP_ */
