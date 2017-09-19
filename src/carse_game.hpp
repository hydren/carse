/*
 * carse_game.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef CARSE_GAME_HPP_
#define CARSE_GAME_HPP_
#include <ciso646>

#include "futil/language.hpp"
#include "futil/properties.hpp"
#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"

#include "course.hpp"
#include "vehicle.hpp"
#include "automotive/engine_sound.hpp"

#include <map>
#include <vector>

struct Vehicle;  // foward declaration

class Pseudo3DCarseGame extends public fgeal::Game
{
	public:
	static const int
		RACE_STATE_ID,
		MAIN_MENU_STATE_ID,
		VEHICLE_SELECTION_STATE_ID,
		COURSE_SELECTION_STATE_ID,
		OPTIONS_MENU_STATE_ID;

	Pseudo3DCarseGame();
	void initializeStatesList();

	/** Wrapper to resources shared between states. */
	struct SharedResources
	{
		fgeal::Sound sndCursorMove, sndCursorIn, sndCursorOut;
		fgeal::Font fontDev;

		SharedResources();
	}  *sharedResources;

	/** Class to wrap together all between-states game logic. */
	class Logic
	{
		friend class Pseudo3DCarseGame;
		Pseudo3DCarseGame& game;

		std::map<std::string, EngineSoundProfile> presetEngineSoundProfiles;
		std::vector<Course> courses;
		std::vector<Vehicle> vehicles;

		Logic(Pseudo3DCarseGame& game);

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

		const std::vector<Vehicle>& getVehicleList();
		void setPickedVehicle(unsigned vehicleIndex, int skin=-1);
		void setPickedVehicle(const Vehicle& v, int skin=-1);

		bool isImperialUnitEnabled();
		void setImperialUnitEnabled(bool choice=true);
	} logic;
};

typedef Pseudo3DCarseGame CarseGame;

#endif /* CARSE_GAME_HPP_ */
