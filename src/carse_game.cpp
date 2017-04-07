/*
 * carse_game.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "carse_game.hpp"

const int  // states IDs
	CarseGame::RACE_STATE_ID = 0,
	CarseGame::MAIN_MENU_STATE_ID = 1,
	CarseGame::CHOOSE_VEHICLE_STATE_ID = 2;

CarseGame::CarseGame()
: Game("Carse", null, 800, 600)
{
	this->maxFps = 60;
}

#if defined(PSEUDO_3D_MODE)

#include "pseudo3D/race_state.hpp"
#include "pseudo3D/main_menu_state.hpp"
#include "pseudo3D/choose_vehicle_state.hpp"

void CarseGame::initializeStatesList()
{
	this->addState(new Pseudo3DRaceState(this));
	this->addState(new MainMenuState(this));
	this->addState(new ChooseVehicleState(this));

	this->setInitialState(MAIN_MENU_STATE_ID);
}

#elif defined(TOPDOWN_MODE)

#include "topdown/race_state.hpp"

void CarseGame::initializeStatesList()
{
	this->addState(new TopDownRaceState(this));
}

#endif
