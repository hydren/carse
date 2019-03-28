/*
 * carse_game.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "carse_game.hpp"

// states
#include "pseudo3d_race_state.hpp"
#include "main_menu_simple_list_state.hpp"
#include "main_menu_retro_layout_state.hpp"
#include "vehicle_selection_simple_list_state.hpp"
#include "vehicle_selection_showroom_layout_state.hpp"
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

void CarseGame::initialize()
{
	this->sharedResources = new SharedResources();
	this->sharedResources->sndCursorIn.setVolume(logic.masterVolume);
	this->sharedResources->sndCursorOut.setVolume(logic.masterVolume);
	this->sharedResources->sndCursorMove.setVolume(logic.masterVolume);

	this->logic.initialize();

	this->addState(new Pseudo3DRaceState(this));

	if(not logic.raceOnlyMode)
	{
		this->addState(new MainMenuSimpleListState(this));
		this->addState(new MainMenuRetroLayoutState(this));
		this->addState(new VehicleSelectionSimpleListState(this));
		this->addState(new VehicleSelectionShowroomLayoutState(this));
		this->addState(new CourseSelectionState(this));
		this->addState(new OptionsMenuState(this));
		this->addState(new CourseEditorState(this));
		this->setInitialState(MAIN_MENU_CLASSIC_LAYOUT_STATE_ID);
	}

	this->setInputManagerEnabled();
	this->logic.onStatesListInitFinished();

	Game::initialize();
}
