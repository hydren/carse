/*
 * carse_game.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "carse_game.hpp"

#include "race_state.hpp"

const int CarseGame::RACE_STATE_ID = 0, CarseGame::MENU_STATE_ID = 1;

CarseGame::CarseGame()
: Game("Carse", null, 800, 600)
{}

void CarseGame::initializeStatesList()
{
	this->addState(new RaceState(this));
}