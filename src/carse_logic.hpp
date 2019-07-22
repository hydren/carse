/*
 * carse_logic.hpp
 *
 *  Created on: 1 de jul de 2019
 *      Author: carlos.faruolo
 */

#ifndef CARSE_LOGIC_HPP_
#define CARSE_LOGIC_HPP_
#include <ciso646>

#include "pseudo3d_race_state.hpp"

#include "course.hpp"
#include "vehicle.hpp"

#include "automotive/engine_sound.hpp"
#include "automotive/mechanics.hpp"
#include "fgeal/fgeal.hpp"
#include "futil/language.hpp"

#include <map>
#include <vector>

// fwd. declared
class CarseGame;

/** Class to wrap together all between-states game logic. */
class CarseLogic
{
	friend class CarseGame;
	static CarseLogic* instance;

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

	CarseLogic();

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
	bool raceOnlyMode;

	float masterVolume;

	int currentMainMenuStateId, currentVehicleSelectionStateId;

	inline static CarseLogic& getInstance()
	{
		if(instance == null)
			instance = new CarseLogic();

		return *instance;
	}

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
};

#endif /* CARSE_LOGIC_HPP_ */
