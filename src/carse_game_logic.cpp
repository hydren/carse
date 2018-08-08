/*
 * carse_game_logic.cpp
 *
 *  Created on: 1 de dez de 2017
 *      Author: carlosfaruolo
 */

#include "carse_game.hpp"

#include "race_state.hpp"
#include "vehicle_selection_state.hpp"
#include "course_selection_state.hpp"

using std::vector;
using std::string;

// logic constructor, booooooring!
CarseGame::Logic::Logic()
: currentMainMenuStateId(CarseGame::MAIN_MENU_CLASSIC_LAYOUT_STATE_ID),
  nextMatchRaceSettings(), nextMatchSimulationType(), nextMatchCourseSpec(0, 0), nextMatchPlayerVehicleSpecAlternateSpriteIndex(-1),
  raceOnlyMode(false), raceOnlyDebug(false), raceOnlyRandomCourse(false), raceOnlyCourseIndex(0), raceOnlyPlayerVehicleIndex(0),
  raceOnlyPlayerVehicleAlternateSpriteIndex(-1), raceOnlyRaceType(-1)
{}

void CarseGame::Logic::initialize()
{
	this->loadPresetEngineSoundProfiles();
	this->loadCourses();
	this->loadVehicles();
}

void CarseGame::Logic::onStatesListInitFinished()
{
	nextMatchRaceSettings.raceType = Pseudo3DRaceState::RACE_TYPE_LOOP_TIME_ATTACK;  // set default race type
	nextMatchRaceSettings.lapCountGoal = 2;    // set default lap count
	nextMatchRaceSettings.isImperialUnit = false;
	nextMatchSimulationType = Mechanics::SIMULATION_TYPE_SLIPLESS;

	if(raceOnlyMode)
	{
		if(raceOnlyDebug)
			this->setNextCourseDebug();
		else if(raceOnlyRandomCourse)
			this->setNextCourseRandom();
		else
			nextMatchCourseSpec = courses[raceOnlyCourseIndex < courses.size()? raceOnlyCourseIndex : courses.size()-1];

		if(raceOnlyRaceType < 0)
			nextMatchRaceSettings.raceType = Pseudo3DRaceState::RACE_TYPE_LOOP_TIME_ATTACK;
		else if(raceOnlyRaceType < Pseudo3DRaceState::RACE_TYPE_COUNT)
			nextMatchRaceSettings.raceType = (Pseudo3DRaceState::RaceType) raceOnlyRaceType;
		else
			nextMatchRaceSettings.raceType = (Pseudo3DRaceState::RaceType) (Pseudo3DRaceState::RACE_TYPE_COUNT - 1);

		nextMatchPlayerVehicleSpec = vehicles[raceOnlyPlayerVehicleIndex < vehicles.size()? raceOnlyPlayerVehicleIndex : vehicles.size()-1];
		nextMatchPlayerVehicleSpecAlternateSpriteIndex = raceOnlyPlayerVehicleAlternateSpriteIndex < (int) nextMatchPlayerVehicleSpec.alternateSprites.size()? raceOnlyPlayerVehicleAlternateSpriteIndex : nextMatchPlayerVehicleSpec.alternateSprites.size()-1;
	}
	else
	{
		this->setNextCourseRandom();  // set default course
		this->setPickedVehicle(vehicles[0]);  // set default vehicle
	}
}

EngineSoundProfile& CarseGame::Logic::getPresetEngineSoundProfile(const std::string presetName)
{
	if(presetEngineSoundProfiles.find(presetName) != presetEngineSoundProfiles.end())
		return presetEngineSoundProfiles[presetName];
	else
		return presetEngineSoundProfiles["default"];
}

const vector<Pseudo3DCourse::Spec>& CarseGame::Logic::getCourseList()
{
	return courses;
}

void CarseGame::Logic::setNextCourse(unsigned courseIndex)
{
	nextMatchCourseSpec = courses[courseIndex];
}

void CarseGame::Logic::setNextCourse(const Pseudo3DCourse::Spec& c)
{
	nextMatchCourseSpec = c;
}

void CarseGame::Logic::setNextCourseRandom()
{
	nextMatchCourseSpec = Pseudo3DCourse::generateRandomCourseSpec(200, 3000, 6400, 1.5);
}

void CarseGame::Logic::setNextCourseDebug()
{
	nextMatchCourseSpec = Pseudo3DCourse::generateDebugCourseSpec(200, 3000);
	nextMatchRaceSettings.raceType = Pseudo3DRaceState::RACE_TYPE_DEBUG;
}

const Pseudo3DCourse::Spec& CarseGame::Logic::getNextCourse()
{
	return nextMatchCourseSpec;
}

Pseudo3DRaceState::RaceSettings& CarseGame::Logic::getNextRaceSettings()
{
	return nextMatchRaceSettings;
}

const vector<Pseudo3DVehicle::Spec>& CarseGame::Logic::getVehicleList()
{
	return vehicles;
}

const Pseudo3DVehicle::Spec& CarseGame::Logic::getPickedVehicle()
{
	return nextMatchPlayerVehicleSpec;
}

const int CarseGame::Logic::getPickedVehicleAlternateSpriteIndex()
{
	return nextMatchPlayerVehicleSpecAlternateSpriteIndex;
}

void CarseGame::Logic::setPickedVehicle(unsigned vehicleIndex, int altSpriteIndex)
{
	nextMatchPlayerVehicleSpec = vehicles[vehicleIndex];
	nextMatchPlayerVehicleSpecAlternateSpriteIndex = altSpriteIndex;
}

void CarseGame::Logic::setPickedVehicle(const Pseudo3DVehicle::Spec& vspec, int altSpriteIndex)
{
	nextMatchPlayerVehicleSpec = vspec;
	nextMatchPlayerVehicleSpecAlternateSpriteIndex = altSpriteIndex;
}

//void CarseGame::Logic::drawPickedVehicle(float x, float y, float scale, int angleType)
//{
//	static_cast<VehicleSelectionState*>(game.getState(CarseGame::VEHICLE_SELECTION_STATE_ID))->drawVehiclePreview(x, y, scale, -1, angleType);
//}

Mechanics::SimulationType CarseGame::Logic::getSimulationType()
{
	return nextMatchSimulationType;
}

void CarseGame::Logic::setSimulationType(Mechanics::SimulationType type)
{
	nextMatchSimulationType = type;
}

int CarseGame::Logic::getCurrentMainMenuStateId()
{
	return currentMainMenuStateId;
}

void CarseGame::Logic::setCurrentMainMenuStateId(int id)
{
	currentMainMenuStateId = id;
}

// ########################################################################################################################################################

static string getFontFilename(const string& key)
{
	futil::Properties properties;
	properties.load("assets/fonts/fonts.properties");
	return "assets/fonts/"+properties.get(key, "default.ttf");
}

CarseGame::SharedResources::SharedResources()
: sndCursorMove("assets/sound/cursor_move.ogg"),
  sndCursorIn("assets/sound/cursor_accept.ogg"),
  sndCursorOut("assets/sound/cursor_out.ogg"),
  fontDev("assets/fonts/default.ttf", 12),
  font1Path(getFontFilename("font1")),
  font2Path(getFontFilename("font2")),
  font3Path(getFontFilename("font3"))
{}
