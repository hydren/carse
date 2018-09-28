/*
 * carse_game.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef CARSE_GAME_HPP_
#define CARSE_GAME_HPP_
#include <ciso646>

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"

#include "futil/language.hpp"
#include "futil/properties.hpp"

#include "automotive/engine_sound.hpp"
#include "automotive/mechanics.hpp"

#include "course.hpp"
#include "vehicle.hpp"

#include "pseudo3d_race_state.hpp"

#include <map>
#include <vector>

extern const std::string CARSE_VERSION;

// fwd decl
class Vehicle;

class CarseGame extends public fgeal::Game
{
	public:

	/** Class to wrap together all between-states game logic. */
	class Logic
	{
		friend class CarseGame;

		std::map<std::string, EngineSoundProfile> presetEngineSoundProfiles;
		std::vector<Pseudo3DCourse::Spec> courses;
		std::vector<Pseudo3DVehicle::Spec> vehicles;

		// parameters for next match
		Pseudo3DRaceState::RaceSettings nextMatchRaceSettings;
		Mechanics::SimulationType nextMatchSimulationType;
		Pseudo3DCourse::Spec nextMatchCourseSpec;
		Pseudo3DVehicle::Spec nextMatchPlayerVehicleSpec;
		int nextMatchPlayerVehicleSpecAlternateSpriteIndex;

		Logic();

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
		bool raceOnlyMode, raceOnlyDebug, raceOnlyRandomCourse;
		unsigned raceOnlyCourseIndex, raceOnlyPlayerVehicleIndex;
		int raceOnlyPlayerVehicleAlternateSpriteIndex, raceOnlyRaceType;
		unsigned raceOnlyLapCount;

		int currentMainMenuStateId, currentVehicleSelectionStateId;

		// gets one of the built-in engine sound presets, by name
		EngineSoundProfile& getPresetEngineSoundProfile(const std::string presetName);

		const std::vector<Pseudo3DCourse::Spec>& getCourseList();
		void setNextCourse(unsigned courseIndex);
		void setNextCourse(const Pseudo3DCourse::Spec& c);
		void setNextCourseRandom();
		void setNextCourseDebug();
		const Pseudo3DCourse::Spec& getNextCourse();
		Pseudo3DRaceState::RaceSettings& getNextRaceSettings();

		const std::vector<Pseudo3DVehicle::Spec>& getVehicleList();
		void setPickedVehicle(unsigned vehicleIndex, int altSpriteIndex=-1);
		void setPickedVehicle(const Pseudo3DVehicle::Spec& v, int altSpriteIndex=-1);
		const Pseudo3DVehicle::Spec& getPickedVehicle();
		const int getPickedVehicleAlternateSpriteIndex();

		Mechanics::SimulationType getSimulationType();
		void setSimulationType(Mechanics::SimulationType type);

		// spec. loading functions
		void loadVehicleSpec(Pseudo3DVehicle::Spec& spec, const futil::Properties& properties);

		// special locations
		const static std::string
			VEHICLES_FOLDER,
			COURSES_FOLDER,
			PRESET_ENGINE_SOUND_PROFILES_FOLDER;
	} logic;

	/** Wrapper to resources shared between states. */
	struct SharedResources
	{
		fgeal::Sound sndCursorMove, sndCursorIn, sndCursorOut;
		fgeal::Font fontDev;
		std::string font1Path, font2Path, font3Path;

		SharedResources();
	} *sharedResources;

	enum StateID
	{
		RACE_STATE_ID,
		MAIN_MENU_SIMPLE_LIST_STATE_ID,
		MAIN_MENU_CLASSIC_LAYOUT_STATE_ID,
		OPTIONS_MENU_STATE_ID,
		VEHICLE_SELECTION_SIMPLE_LIST_STATE_ID,
		VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_ID,
		COURSE_SELECTION_STATE_ID,
		COURSE_EDITOR_STATE_ID,
	};

	CarseGame();
	~CarseGame();
	virtual void initialize();
};

#endif /* CARSE_GAME_HPP_ */
