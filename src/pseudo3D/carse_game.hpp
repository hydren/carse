/*
 * carse_game.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef CARSE_GAME_HPP_
#define CARSE_GAME_HPP_
#include <ciso646>

#include "fgeal/extra/game.hpp"
#include "futil/general/language.hpp"

#include "automotive/engine_sound.hpp"
#include "vehicle.hpp"
#include "util/properties.hpp"

#include <map>
#include <vector>

struct Vehicle;  // foward declaration

class Pseudo3DCarseGame extends public fgeal::Game
{
	public:
	static const int RACE_STATE_ID, MAIN_MENU_STATE_ID, CHOOSE_VEHICLE_STATE_ID;

	Pseudo3DCarseGame();
	void initializeStatesList();

	// gets one of the built-in engine sound presets, by name
	EngineSoundProfile& getPresetEngineSoundProfile(const std::string presetName);

	// gets the vehicle list loaded at startup
	std::vector<Vehicle>& getVehicles();

	private:
	std::map<std::string, EngineSoundProfile> presetEngineSoundProfiles;
	std::vector<Vehicle> vehicles;

	// intended to run on startup, loads all engine sound presets in assets/sound/engine/
	void loadPresetEngineSoundProfiles();

	// intended to run on startup, loads all vehicle in data/vehicles/
	void loadVehicles();
};

typedef Pseudo3DCarseGame CarseGame;

#endif /* CARSE_GAME_HPP_ */
