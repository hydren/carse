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

#include "automotive/engine_sound.hpp"
#include "vehicle.hpp"
#include "futil/properties.hpp"

#include <map>
#include <vector>

struct Vehicle;  // foward declaration

class Pseudo3DCarseGame extends public fgeal::Game
{
	public:
	static const int RACE_STATE_ID, MAIN_MENU_STATE_ID, VEHICLE_SELECTION_STATE_ID, COURSE_SELECTION_STATE_ID, OPTIONS_MENU_STATE_ID;

	Pseudo3DCarseGame();
	void initializeStatesList();

	// gets one of the built-in engine sound presets, by name
	EngineSoundProfile& getPresetEngineSoundProfile(const std::string presetName);

	private:
	std::map<std::string, EngineSoundProfile> presetEngineSoundProfiles;

	// intended to run on startup, loads all engine sound presets in assets/sound/engine/
	void loadPresetEngineSoundProfiles();
};

typedef Pseudo3DCarseGame CarseGame;

#endif /* CARSE_GAME_HPP_ */
