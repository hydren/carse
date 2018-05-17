/*
 * carse_game.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef CARSE_GAME_HPP_
#define CARSE_GAME_HPP_
#include <ciso646>

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"

#include "carse_game_logic.hpp"

extern const std::string CARSE_VERSION;

class Pseudo3DCarseGame extends public fgeal::Game
{
	public:
	static const int
		RACE_STATE_ID,
		MAIN_MENU_STATE_ID,
		VEHICLE_SELECTION_STATE_ID,
		COURSE_SELECTION_STATE_ID,
		OPTIONS_MENU_STATE_ID;

	Pseudo3DCarseGame();
	~Pseudo3DCarseGame();
	void initializeStatesList();

	CarseSharedResources* sharedResources;
	CarseGameLogic logic;
};

typedef Pseudo3DCarseGame CarseGame;

#endif /* CARSE_GAME_HPP_ */
