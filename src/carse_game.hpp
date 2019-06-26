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

class CarseGame extends public fgeal::Game
{
	public:

	/** Class to wrap together all between-states game logic. */
	class Logic
	{
		friend class CarseGame;

		std::map<std::string, EngineSoundProfile> presetEngineSoundProfiles;
		std::vector<Pseudo3DCourse::Spec> courses;
		std::vector<Pseudo3DVehicle::Spec> vehicles, trafficVehicles;
		std::map<std::string, Pseudo3DCourse::Spec::LandscapeStyle> presetLandscapeStyles;
		std::map<std::string, Pseudo3DCourse::Spec::RoadStyle> presetRoadStyles;

		// parameters for next match
		Pseudo3DRaceState::RaceSettings nextMatchRaceSettings;
		Mechanics::SimulationType nextMatchSimulationType;
		bool nextMatchJumpSimulationEnabled;
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

		// intended to run on startup, loads all traffic vehicles in the data/traffic folder
		void loadTrafficVehicles();

		// intended to run on startup, loads all course style presets in data/courses/styles/
		void loadPresetCourseStyles();

		public:
		bool raceOnlyMode, raceOnlyDebug, raceOnlyRandomCourse;
		unsigned raceOnlyCourseIndex, raceOnlyPlayerVehicleIndex;
		int raceOnlyPlayerVehicleAlternateSpriteIndex, raceOnlyRaceType;
		unsigned raceOnlyLapCount;
		float masterVolume;

		int currentMainMenuStateId, currentVehicleSelectionStateId;

		/* Creates a engine sound profile by loading and parsing the data in the given filename. */
		static EngineSoundProfile createEngineSoundProfileFromFile(const std::string& filename);

		// Returns the correct preset according to the specified preset-name from one of the built-in presets; if the given preset name is not present on the map, returns the default preset
		const EngineSoundProfile& getPresetEngineSoundProfile(const std::string& presetName) const;

		// Returns the correct preset according to the specified preset-name from one of the built-in presets; if the given preset name is not present on the map, returns the default preset
		const Pseudo3DCourse::Spec::RoadStyle& getPresetRoadStyle(const std::string& presetName) const;

		// Returns a random preset from one of the built-in presets
		const Pseudo3DCourse::Spec::RoadStyle& getRandomPresetRoadStyle() const;

		// Returns a list of available preset styles names
		std::vector<std::string> getPresetRoadStylesNames() const;

		// Returns the correct preset according to the specified preset-name from one of the built-in presets; if the given preset name is not present on the map, returns the default preset
		const Pseudo3DCourse::Spec::LandscapeStyle& getPresetLandscapeStyle(const std::string& presetName) const;

		// Returns a random preset from one of the built-in presets
		const Pseudo3DCourse::Spec::LandscapeStyle& getRandomPresetLandscapeStyle() const;

		// Returns a list of available preset styles names
		std::vector<std::string> getPresetLandscapeStylesNames() const;

		void updateCourseList();
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

		const std::vector<Pseudo3DVehicle::Spec>& getTrafficVehicleList();

		Mechanics::SimulationType getSimulationType();
		void setSimulationType(Mechanics::SimulationType type);

		bool isJumpSimulationEnabled();
		void setJumpSimulationEnabled(bool enabled=true);

		// special locations
		const static std::string
			COURSES_FOLDER,
			VEHICLES_FOLDER,
			TRAFFIC_FOLDER,
			PRESET_ENGINE_SOUND_PROFILES_FOLDER,
			PRESET_COURSE_STYLES_FOLDER;
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

// kludge needed to allow forward declaring of inner class CarseGame::Logic
struct CarseGameLogicInstance
{
	CarseGame::Logic& instance;
	CarseGameLogicInstance(CarseGame::Logic* instance) : instance(*instance) {}
	CarseGameLogicInstance(CarseGame::Logic& instance) : instance(instance) {}
};

#endif /* CARSE_GAME_HPP_ */
