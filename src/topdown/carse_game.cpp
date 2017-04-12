/*
 * carse_game.cpp
 *
 *  Created on: 12 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "carse_game.hpp"
#include "race_state.hpp"

const int  // states IDs
	TopdownCarseGame::RACE_STATE_ID = 0;

TopdownCarseGame::TopdownCarseGame()
: Game("Carse", null, 800, 600)
{
	this->maxFps = 60;
}

void TopdownCarseGame::initializeStatesList()
{
	this->addState(new TopDownRaceState(this));
}
