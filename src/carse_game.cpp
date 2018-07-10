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

Pseudo3DCarseGame::Pseudo3DCarseGame()
: Game("Carse", null, 800, 600), sharedResources(null), logic(*this)
{
	this->maxFps = 60;
}

Pseudo3DCarseGame::~Pseudo3DCarseGame()
{
	if(sharedResources != null)
		delete sharedResources;
}

void Pseudo3DCarseGame::preInitialize()
{
	this->sharedResources = new CarseSharedResources();
	this->logic.initialize();
}

void Pseudo3DCarseGame::initializeStatesList()
{
	this->addState(new Pseudo3DRaceState(this));
	this->addState(new MainMenuState(this));
	this->addState(new VehicleSelectionState(this));
	this->addState(new CourseSelectionState(this));
	this->addState(new OptionsMenuState(this));

	this->setInitialState(MAIN_MENU_STATE_ID);
	this->setInputManagerEnabled();

	this->logic.onStatesListInitFinished();
}
