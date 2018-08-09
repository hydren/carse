/*
 * carse_game.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "carse_game.hpp"

// states
#include "race_state.hpp"
#include "main_menu_simple_list_state.hpp"
#include "main_menu_classic_layout_state.hpp"
#include "vehicle_selection_state.hpp"
#include "course_selection_state.hpp"
#include "options_menu_state.hpp"
#include "course_editor_state.hpp"

CarseGame::CarseGame()
: Game("Carse", null, 800, 600), logic(), sharedResources(null)
{
	this->maxFps = 60;
}

CarseGame::~CarseGame()
{
	if(sharedResources != null)
		delete sharedResources;
}

void CarseGame::preInitialize()
{
	this->sharedResources = new SharedResources();
	this->logic.initialize();
}

void CarseGame::initializeStatesList()
{
	this->addState(new Pseudo3DRaceState(this));

	if(not logic.raceOnlyMode)
	{
		this->addState(new MainMenuSimpleListState(this));
		this->addState(new MainMenuClassicPanelState(this));
		this->addState(new VehicleSelectionState(this));
		this->addState(new CourseSelectionState(this));
		this->addState(new OptionsMenuState(this));
		this->addState(new CourseEditorState(this));
		this->setInitialState(MAIN_MENU_CLASSIC_LAYOUT_STATE_ID);
	}

	this->setInputManagerEnabled();
	this->logic.onStatesListInitFinished();
}
