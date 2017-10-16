/*
 * carse_game.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "carse_game.hpp"

// states
#include "states/race_state.hpp"
#include "states/main_menu_state.hpp"
#include "states/vehicle_selection_state.hpp"
#include "states/course_selection_state.hpp"
#include "states/options_menu_state.hpp"

const int  // states IDs
	Pseudo3DCarseGame::RACE_STATE_ID = 0,
	Pseudo3DCarseGame::MAIN_MENU_STATE_ID = 1,
	Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID = 2,
	Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID = 3,
	Pseudo3DCarseGame::OPTIONS_MENU_STATE_ID = 4;

Pseudo3DCarseGame::Pseudo3DCarseGame()
: Game("Carse", null, 800, 600), sharedResources(null), logic(*this)
{
	this->maxFps = 60;
}

void Pseudo3DCarseGame::initializeStatesList()
{
	this->sharedResources = new SharedResources();
	this->logic.initialize();  // @suppress("Method cannot be resolved")

	this->addState(new Pseudo3DRaceState(this));
	this->addState(new MainMenuState(this));
	this->addState(new VehicleSelectionState(this));
	this->addState(new CourseSelectionState(this));
	this->addState(new OptionsMenuState(this));

	this->setInitialState(MAIN_MENU_STATE_ID);

	this->logic.onStatesListInitFinished();  // @suppress("Method cannot be resolved")
}
