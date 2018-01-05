/*
 * carse_game_logic.hpp
 *
 *  Created on: 1 de dez de 2017
 *      Author: carlosfaruolo
 */

#ifndef CARSE_GAME_LOGIC_HPP_
#define CARSE_GAME_LOGIC_HPP_
#include <ciso646>

#include "course.hpp"
#include "vehicle.hpp"
#include "automotive/engine_sound.hpp"
#include "automotive/mechanics.hpp"

#include "states/race_state.hpp"

#include "futil/language.hpp"
#include "futil/properties.hpp"

#include <map>
#include <vector>

// fwd decl
class Pseudo3DCarseGame;
class Vehicle;

/** Class to wrap together all between-states game logic. */
class CarseGameLogic
{
	friend class Pseudo3DCarseGame;
	Pseudo3DCarseGame& game;

	std::map<std::string, EngineSoundProfile> presetEngineSoundProfiles;
	std::vector<Course> courses;
	std::vector<Pseudo3DVehicle::Spec> vehicles;

	CarseGameLogic(Pseudo3DCarseGame& game);

	// intended to run on startup
	void initialize();

	// intended to run on startup, after initializing all states
	void onStatesListInitFinished();

	// intended to run on startup, loads all engine sound presets in assets/sound/engine/
	void loadPresetEngineSoundProfiles();

	// intended to run on startup, loads all courses in the data/courses folder
	void loadCourses();

	// intended to run on startup, loads all vehicles in the data/vehicles folder
	void loadVehicles();

	public:
	// gets one of the built-in engine sound presets, by name
	EngineSoundProfile& getPresetEngineSoundProfile(const std::string presetName);

	const std::vector<Course>& getCourseList();
	void setNextCourse(unsigned courseIndex);
	void setNextCourse(const Course& c);
	void setNextCourseRandom();
	void setNextCourseDebug();
	const Course& getNextCourse();
	fgeal::Image* getNextCoursePreviewImage();
	Pseudo3DRaceState::RaceSettings& getNextRaceSettings();

	const std::vector<Pseudo3DVehicle::Spec>& getVehicleList();
	void setPickedVehicle(unsigned vehicleIndex, int altSpriteIndex=-1);
	void setPickedVehicle(const Pseudo3DVehicle::Spec& v, int altSpriteIndex=-1);
	const Pseudo3DVehicle::Spec& getPickedVehicle();
	void drawPickedVehicle(float x, float y, float scale=1.0f, int angleType=0);

	Mechanics::SimulationType getSimulationType();
	void setSimulationType(Mechanics::SimulationType type);

	// spec. loading functions
	void loadVehicleSpec(Pseudo3DVehicle::Spec& spec, const futil::Properties& properties);
};

/** Wrapper to resources shared between states. */
struct CarseSharedResources
{
	fgeal::Sound sndCursorMove, sndCursorIn, sndCursorOut;
	fgeal::Font fontDev;
	std::string font1Path, font2Path, font3Path;

	CarseSharedResources();
};

#endif /* CARSE_GAME_LOGIC_HPP_ */
