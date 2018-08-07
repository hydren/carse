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

#define getRaceStateInstance() (*static_cast<Pseudo3DRaceState*>(game.getState(Pseudo3DCarseGame::RACE_STATE_ID)))

// logic constructor, booooooring!
CarseGameLogic::CarseGameLogic(Pseudo3DCarseGame& game)
: game(game), currentMainMenuStateId(Pseudo3DCarseGame::MAIN_MENU_CLASSIC_LAYOUT_STATE_ID) {}

void CarseGameLogic::initialize()
{
	this->loadPresetEngineSoundProfiles();
	this->loadCourses();
	this->loadVehicles();
}

void CarseGameLogic::onStatesListInitFinished()
{
	this->setNextCourseRandom();  // set default course
	getRaceStateInstance().settings.raceType = Pseudo3DRaceState::RACE_TYPE_LOOP_TIME_ATTACK;  // set default race type
	getRaceStateInstance().settings.lapCountGoal = 2;    // set default lap count
	getRaceStateInstance().settings.isImperialUnit = false;
	this->setPickedVehicle(vehicles[0]);  // set default vehicle
	this->setSimulationType(Mechanics::SIMULATION_TYPE_SLIPLESS);
}

EngineSoundProfile& CarseGameLogic::getPresetEngineSoundProfile(const std::string presetName)
{
	if(presetEngineSoundProfiles.find(presetName) != presetEngineSoundProfiles.end())
		return presetEngineSoundProfiles[presetName];
	else
		return presetEngineSoundProfiles["default"];
}

const vector<Pseudo3DCourse::Spec>& CarseGameLogic::getCourseList()
{
	return courses;
}

void CarseGameLogic::setNextCourse(unsigned courseIndex)
{
	getRaceStateInstance().nextCourseSpec = courses[courseIndex];
}

void CarseGameLogic::setNextCourse(const Pseudo3DCourse::Spec& c)
{
	getRaceStateInstance().nextCourseSpec = c;
}

void CarseGameLogic::setNextCourseRandom()
{
	getRaceStateInstance().nextCourseSpec = Pseudo3DCourse::generateRandomCourseSpec(200, 3000, 6400, 1.5);
}

void CarseGameLogic::setNextCourseDebug()
{
	getRaceStateInstance().nextCourseSpec = Pseudo3DCourse::generateDebugCourseSpec(200, 3000);
	getRaceStateInstance().settings.raceType = Pseudo3DRaceState::RACE_TYPE_DEBUG;
}

const Pseudo3DCourse::Spec& CarseGameLogic::getNextCourse()
{
	return getRaceStateInstance().nextCourseSpec;
}

fgeal::Image* CarseGameLogic::getNextCoursePreviewImage()
{
	return static_cast<CourseSelectionState*>(game.getState(Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID))->getSelectedCoursePreview();
}

Pseudo3DRaceState::RaceSettings& CarseGameLogic::getNextRaceSettings()
{
	return getRaceStateInstance().settings;
}

const vector<Pseudo3DVehicle::Spec>& CarseGameLogic::getVehicleList()
{
	return vehicles;
}

const Pseudo3DVehicle::Spec& CarseGameLogic::getPickedVehicle()
{
	return getRaceStateInstance().playerVehicleSpec;
}

void CarseGameLogic::setPickedVehicle(unsigned vehicleIndex, int altSpriteIndex)
{
	getRaceStateInstance().playerVehicleSpec = vehicles[vehicleIndex];
	getRaceStateInstance().playerVehicleSpecAlternateSpriteIndex = altSpriteIndex;
}

void CarseGameLogic::setPickedVehicle(const Pseudo3DVehicle::Spec& vspec, int altSpriteIndex)
{
	getRaceStateInstance().playerVehicleSpec = vspec;
	getRaceStateInstance().playerVehicleSpecAlternateSpriteIndex = altSpriteIndex;
}

void CarseGameLogic::drawPickedVehicle(float x, float y, float scale, int angleType)
{
	static_cast<VehicleSelectionState*>(game.getState(Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID))->drawVehiclePreview(x, y, scale, -1, angleType);
}

Mechanics::SimulationType CarseGameLogic::getSimulationType()
{
	return getRaceStateInstance().simulationType;
}

void CarseGameLogic::setSimulationType(Mechanics::SimulationType type)
{
	getRaceStateInstance().simulationType = type;
}

int CarseGameLogic::getCurrentMainMenuStateId()
{
	return currentMainMenuStateId;
}

void CarseGameLogic::setCurrentMainMenuStateId(int id)
{
	currentMainMenuStateId = id;
}

// ----------------------------------------------------------------------------------------------------------


// ########################################################################################################################################################

static string getFontFilename(const string& key)
{
	futil::Properties properties;
	properties.load("assets/fonts/fonts.properties");
	return "assets/fonts/"+properties.get(key, "default.ttf");
}

CarseSharedResources::CarseSharedResources()
: sndCursorMove("assets/sound/cursor_move.ogg"),
  sndCursorIn("assets/sound/cursor_accept.ogg"),
  sndCursorOut("assets/sound/cursor_out.ogg"),
  fontDev("assets/fonts/default.ttf", 12),
  font1Path(getFontFilename("font1")),
  font2Path(getFontFilename("font2")),
  font3Path(getFontFilename("font3"))
{}
