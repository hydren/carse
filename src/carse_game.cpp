/*
 * carse_game.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "carse_game.hpp"

#include "pseudo3D/race_state.hpp"
#include "topdown/race_state.hpp"

const int CarseGame::RACE_STATE_ID = 0, CarseGame::MENU_STATE_ID = 1;

CarseGame::CarseGame()
: Game("Carse", null, 800, 600)
{
	this->maxFps = 60;
}

void CarseGame::initializeStatesList()
{
	#if defined(PSEUDO_3D_MODE)
		this->addState(new Pseudo3DRaceState(this));
	#elif defined(TOPDOWN_MODE)
		this->addState(new TopDownRaceState(this));
	#endif
}
