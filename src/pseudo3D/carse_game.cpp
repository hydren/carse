/*
 * carse_game.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "carse_game.hpp"

#include "futil/string/actions.hpp"
#include "futil/string/split.hpp"

// XXX debug code
#include <iostream>
using std::cout;
using std::endl;
// XXX debug code

#include <vector>

// states
#include "race_state.hpp"
#include "main_menu_state.hpp"
#include "choose_vehicle_state.hpp"

using util::Properties;
using std::vector;
using std::string;
using std::map;

const int  // states IDs
	Pseudo3DCarseGame::RACE_STATE_ID = 0,
	Pseudo3DCarseGame::MAIN_MENU_STATE_ID = 1,
	Pseudo3DCarseGame::CHOOSE_VEHICLE_STATE_ID = 2;

Pseudo3DCarseGame::Pseudo3DCarseGame()
: Game("Carse", null, 800, 600)
{
	this->maxFps = 60;
}

void Pseudo3DCarseGame::initializeStatesList()
{
	this->loadBuiltinEngineSoundPresets();

	this->addState(new Pseudo3DRaceState(this));
	this->addState(new MainMenuState(this));
	this->addState(new ChooseVehicleState(this));

	this->setInitialState(MAIN_MENU_STATE_ID);
}

void Pseudo3DCarseGame::loadBuiltinEngineSoundPresets()
{
	Properties prop;

	vector<string> presetFiles = fgeal::getFilenamesWithinDirectory("assets/sound/engine");
	for(unsigned i = 0; i < presetFiles.size(); i++)
	{
		string filename = presetFiles[i];
		if(ends_with(filename, ".properties"))
		{
			util::Properties prop;
			prop.load(filename);

			string presetName = filename.substr(0, filename.find_last_of("."));

			if(EngineSoundProfile::requestsPresetProfile(prop))
			{
				// todo
			}
			else
			{
				string presetName = filename.substr(0, filename.find_last_of("."));
				builtinEngineSoundPresets[presetName] = EngineSoundProfile::loadFromProperties(prop);
			}

			cout << "read preset sound profile: " << presetName << endl;
		}
	}
}
