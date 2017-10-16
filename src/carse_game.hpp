/*
 * carse_game.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef CARSE_GAME_HPP_
#define CARSE_GAME_HPP_
#include <ciso646>

#include "futil/language.hpp"
#include "futil/properties.hpp"
#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"

#include "course.hpp"
#include "vehicle.hpp"
#include "automotive/engine_sound.hpp"
#include "automotive/mechanics.hpp"

#include <map>
#include <vector>

struct Vehicle;  // foward declaration

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
	void initializeStatesList();

	#include "carse_game_logic.hxx"
	SharedResources* sharedResources;  // @suppress("Type cannot be resolved")
	Logic logic;					   // @suppress("Type cannot be resolved")
};

typedef Pseudo3DCarseGame CarseGame;

#endif /* CARSE_GAME_HPP_ */
