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

	vector<string> pendingPresetFiles, presetFiles = fgeal::getFilenamesWithinDirectory("assets/sound/engine");
	for(unsigned i = 0; i < presetFiles.size(); i++)
	{
		string filename = presetFiles[i];
		if(ends_with(filename, ".properties"))
		{
			util::Properties prop;
			prop.load(filename);

			const string
				filenameWithoutPath = filename.substr(filename.find_last_of("/\\")+1),
				filenameWithoutExtension = filenameWithoutPath.substr(0, filenameWithoutPath.find_last_of(".")),
				presetName = filenameWithoutExtension;

			if(EngineSoundProfile::requestsPresetProfile(prop))
			{
				pendingPresetFiles.push_back(filename);
				cout << "read preset sound profile: " << presetName << " (alias)" << endl;
			}

			else
			{
				builtinEngineSoundPresets[presetName] = EngineSoundProfile::loadFromProperties(prop);
				cout << "read preset sound profile: " << presetName << endl;
			}
		}
	}

	unsigned previousCount = pendingPresetFiles.size();
	while(not pendingPresetFiles.empty())
	{
		for(unsigned i = 0; i < pendingPresetFiles.size(); i++)
		{
			string filename = pendingPresetFiles[i];
			util::Properties prop;
			prop.load(filename);

			const string
				basePresetName = EngineSoundProfile::getSoundDefinitionFromProperties(prop),
				filenameWithoutPath = filename.substr(filename.find_last_of("/\\")+1),
				filenameWithoutExtension = filenameWithoutPath.substr(0, filenameWithoutPath.find_last_of(".")),
				presetName = filenameWithoutExtension;

			if(builtinEngineSoundPresets.find(basePresetName) != builtinEngineSoundPresets.end())
			{
				builtinEngineSoundPresets[presetName] = builtinEngineSoundPresets[basePresetName];
				cout << "copied preset sound profile \"" << presetName << "\" from \"" << basePresetName << "\"" << endl;
				pendingPresetFiles.erase(pendingPresetFiles.begin() + i);
				i--;
			}
		}
		if(pendingPresetFiles.size() == previousCount)
		{
			cout << "circular dependency or unresolved reference detected when loading preset sound profiles. skipping resolution." << endl;
			cout << "the following preset sound profiles could not be loaded: " << endl;
			for(unsigned i = 0; i < pendingPresetFiles.size(); i++)
				cout << pendingPresetFiles[i] << endl;
			break;
		}
		else previousCount = pendingPresetFiles.size();
	}
}
