/*
 * carse_game_logic.cpp
 *
 *  Created on: 1 de dez de 2017
 *      Author: carlosfaruolo
 */

#include "carse_game.hpp"

#include "pseudo3d_race_state.hpp"
#include "course_selection_state.hpp"

#include "futil/random.h"

using std::vector;
using std::map;
using std::string;

// logic constructor, booooooring!
CarseGame::Logic::Logic()
: nextMatchRaceSettings(), nextMatchSimulationType(), nextMatchJumpSimulationEnabled(),
  nextMatchCourseSpec(0, 0), nextMatchPlayerVehicleSpecAlternateSpriteIndex(-1),
  raceOnlyMode(false), raceOnlyDebug(false), raceOnlyRandomCourse(false), raceOnlyCourseIndex(0), raceOnlyPlayerVehicleIndex(0),
  raceOnlyPlayerVehicleAlternateSpriteIndex(-1), raceOnlyRaceType(-1), raceOnlyLapCount(2),
  masterVolume(1.f),
  currentMainMenuStateId(CarseGame::MAIN_MENU_CLASSIC_LAYOUT_STATE_ID),
  currentVehicleSelectionStateId(CarseGame::VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_ID)
{}

void CarseGame::Logic::initialize()
{
	this->loadPresetEngineSoundProfiles();
	this->loadPresetCourseStyles();
	this->loadCourses();
	this->loadVehicles();
	this->loadTrafficVehicles();
}

void CarseGame::Logic::onStatesListInitFinished()
{
	nextMatchRaceSettings.raceType = Pseudo3DRaceState::RACE_TYPE_LOOP_TIME_ATTACK;  // set default race type
	nextMatchRaceSettings.lapCountGoal = 2;    // set default lap count
	nextMatchRaceSettings.trafficDensity = 0;  // by default, no traffic
	nextMatchRaceSettings.isImperialUnit = false;
	nextMatchRaceSettings.hudType = Pseudo3DRaceState::HUD_TYPE_DIALGAUGE_TACHO_NUMERIC_SPEEDO;
	nextMatchRaceSettings.useCachedDialGauge = false;
	nextMatchRaceSettings.hudDialGaugePointerImageFilename.clear();
	nextMatchSimulationType = Mechanics::SIMULATION_TYPE_SLIPLESS;
	nextMatchJumpSimulationEnabled = false;

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

		if(Pseudo3DRaceState::isRaceTypeLoop(nextMatchRaceSettings.raceType))
			nextMatchRaceSettings.lapCountGoal = raceOnlyLapCount;

		nextMatchPlayerVehicleSpec = vehicles[raceOnlyPlayerVehicleIndex < vehicles.size()? raceOnlyPlayerVehicleIndex : vehicles.size()-1];
		nextMatchPlayerVehicleSpecAlternateSpriteIndex = raceOnlyPlayerVehicleAlternateSpriteIndex < (int) nextMatchPlayerVehicleSpec.alternateSprites.size()? raceOnlyPlayerVehicleAlternateSpriteIndex : nextMatchPlayerVehicleSpec.alternateSprites.size()-1;
	}
	else
	{
		this->setNextCourseRandom();  // set default course
		this->setPickedVehicle(vehicles[0]);  // set default vehicle
	}
}

const EngineSoundProfile& CarseGame::Logic::getPresetEngineSoundProfile(const std::string& presetName) const
{
	if(presetEngineSoundProfiles.find(presetName) != presetEngineSoundProfiles.end())
		return presetEngineSoundProfiles.find(presetName)->second;
	else
		return presetEngineSoundProfiles.find("default")->second;
}

const Pseudo3DCourse::Spec::LandscapeSettings& CarseGame::Logic::getPresetLandscapeStyle(const std::string& presetName) const
{
	if(presetLandscapeStyles.find(presetName) != presetLandscapeStyles.end())
		return presetLandscapeStyles.find(presetName)->second;
	else
		return presetLandscapeStyles.find("default")->second;
}

const Pseudo3DCourse::Spec::RoadColorSet& CarseGame::Logic::getPresetRoadStyle(const std::string& presetName) const
{
	if(presetRoadStyles.find(presetName) != presetRoadStyles.end())
		return presetRoadStyles.find(presetName)->second;
	else
		return presetRoadStyles.find("default")->second;
}

vector<string> CarseGame::Logic::getPresetRoadStylesNames() const
{
	vector<string> names(presetRoadStyles.size()); int i = 0;
	for(map<string, Pseudo3DCourse::Spec::RoadColorSet>::const_iterator it = presetRoadStyles.begin(); it != presetRoadStyles.end(); ++it)
		names[i++] = it->first;
	return names;
}

vector<string> CarseGame::Logic::getPresetLandscapeStylesNames() const
{
	vector<string> names(presetLandscapeStyles.size()); int i = 0;
	for(map<string, Pseudo3DCourse::Spec::LandscapeSettings>::const_iterator it = presetLandscapeStyles.begin(); it != presetLandscapeStyles.end(); ++it)
		names[i++] = it->first;
	return names;
}

const Pseudo3DCourse::Spec::LandscapeSettings& CarseGame::Logic::getRandomPresetLandscapeStyle() const
{
	map<string, Pseudo3DCourse::Spec::LandscapeSettings>::const_iterator it = presetLandscapeStyles.begin();
	std::advance(it, futil::random_between(0, presetLandscapeStyles.size()));
	return it->second;
}

const Pseudo3DCourse::Spec::RoadColorSet& CarseGame::Logic::getRandomPresetRoadStyle() const
{
	map<string, Pseudo3DCourse::Spec::RoadColorSet>::const_iterator it = presetRoadStyles.begin();
	std::advance(it, futil::random_between(0, presetRoadStyles.size()));
	return it->second;
}

void CarseGame::Logic::updateCourseList()
{
	courses.clear();
	this->loadCourses();
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
	nextMatchCourseSpec = Pseudo3DCourse::Spec::generateRandomCourseSpec(200, 3000, 6400, 1.5);
	nextMatchCourseSpec.assignStyle(this->getRandomPresetRoadStyle());
	nextMatchCourseSpec.assignStyle(this->getRandomPresetLandscapeStyle());
}

void CarseGame::Logic::setNextCourseDebug()
{
	nextMatchCourseSpec = Pseudo3DCourse::Spec::generateDebugCourseSpec(200, 3000);
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

const vector<Pseudo3DVehicle::Spec>& CarseGame::Logic::getTrafficVehicleList()
{
	return trafficVehicles;
}

Mechanics::SimulationType CarseGame::Logic::getSimulationType()
{
	return nextMatchSimulationType;
}

void CarseGame::Logic::setSimulationType(Mechanics::SimulationType type)
{
	nextMatchSimulationType = type;
}

bool CarseGame::Logic::isJumpSimulationEnabled()
{
	return nextMatchJumpSimulationEnabled;
}

void CarseGame::Logic::setJumpSimulationEnabled(bool enabled)
{
	nextMatchJumpSimulationEnabled = enabled;
}
