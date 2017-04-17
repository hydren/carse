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
#include "util/properties.hpp"

#include <map>

class Pseudo3DCarseGame extends public fgeal::Game
{
	public:
	static const int RACE_STATE_ID, MAIN_MENU_STATE_ID, CHOOSE_VEHICLE_STATE_ID;

	Pseudo3DCarseGame();
	void initializeStatesList();

	// gets one of the built-in engine sound presets, by name
	EngineSoundProfile& getEngineSoundPreset(const std::string presetName);

	private:
	std::map<std::string, EngineSoundProfile> builtinEngineSoundPresets;

	// intended to run on startup, loads all engine sound presets in assets/sound/engine/
	void loadBuiltinEngineSoundPresets();
};

typedef Pseudo3DCarseGame CarseGame;

#endif /* CARSE_GAME_HPP_ */
