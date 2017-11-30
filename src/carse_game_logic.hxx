/*
 * carse_game_logic.hxx
 *
 *  Created on: 16 de out de 2017
 *      Author: carlosfaruolo
 */

/** Class to wrap together all between-states game logic. */
class Logic
{
	friend class Pseudo3DCarseGame;
	Pseudo3DCarseGame& game;

	std::map<std::string, EngineSoundProfile> presetEngineSoundProfiles;
	std::vector<Course> courses;
	std::vector<Pseudo3DVehicle::Spec> vehicles;

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

	const std::vector<Pseudo3DVehicle::Spec>& getVehicleList();
	void setPickedVehicle(unsigned vehicleIndex, int skin=-1);
	void setPickedVehicle(const Pseudo3DVehicle::Spec& v, int skin=-1);

	bool isImperialUnitEnabled();
	void setImperialUnitEnabled(bool choice=true);

	Mechanics::SimulationType getSimulationType();
	void setSimulationType(Mechanics::SimulationType type);

	// spec. loading functions
	void loadVehicleSpec(Pseudo3DVehicle::Spec& spec, const futil::Properties& properties);
};

// ----------------------------------------------------------------------------------------------------------

/** Wrapper to resources shared between states. */
struct SharedResources
{
	fgeal::Sound sndCursorMove, sndCursorIn, sndCursorOut;
	fgeal::Font fontDev;

	SharedResources();
};
