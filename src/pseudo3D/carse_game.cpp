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
	this->loadPresetEngineSoundProfiles();
	this->loadVehicles();

	this->addState(new Pseudo3DRaceState(this));
	this->addState(new MainMenuState(this));
	this->addState(new ChooseVehicleState(this));

	this->setInitialState(MAIN_MENU_STATE_ID);
}

EngineSoundProfile& Pseudo3DCarseGame::getPresetEngineSoundProfile(const std::string presetName)
{
	if(presetEngineSoundProfiles.find(presetName) != presetEngineSoundProfiles.end())
		return presetEngineSoundProfiles[presetName];
	else
		return presetEngineSoundProfiles["default"];
}

std::vector<Vehicle>& Pseudo3DCarseGame::getVehicles()
{
	return vehicles;
}

void Pseudo3DCarseGame::loadPresetEngineSoundProfiles()
{
	cout << "reading preset engine sound profiles..." << endl;
	vector<string> pendingPresetFiles, presetFiles = fgeal::getFilenamesWithinDirectory("assets/sound/engine");
	for(unsigned i = 0; i < presetFiles.size(); i++)
	{
		string& filename = presetFiles[i];
		if(ends_with(filename, ".properties"))
		{
			Properties prop;
			prop.load(filename);

			const string
				filenameWithoutPath = filename.substr(filename.find_last_of("/\\")+1),
				filenameWithoutExtension = filenameWithoutPath.substr(0, filenameWithoutPath.find_last_of(".")),
				presetName = filenameWithoutExtension;

			if(EngineSoundProfile::requestsPresetProfile(prop))
			{
				pendingPresetFiles.push_back(filename);
				cout << "read profile: " << presetName << " (alias)" << endl;
			}

			else
			{
				presetEngineSoundProfiles[presetName] = EngineSoundProfile::loadFromProperties(prop);
				cout << "read profile: " << presetName << endl;
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

			if(presetEngineSoundProfiles.find(basePresetName) != presetEngineSoundProfiles.end())
			{
				presetEngineSoundProfiles[presetName] = presetEngineSoundProfiles[basePresetName];
				cout << "copied profile \"" << presetName << "\" from \"" << basePresetName << "\"" << endl;
				pendingPresetFiles.erase(pendingPresetFiles.begin() + i);
				i--;
			}
		}
		if(pendingPresetFiles.size() == previousCount)
		{
			cout << "circular dependency or unresolved reference detected when loading preset engine sound profiles. skipping resolution." << endl;
			cout << "the following preset engine sound profiles could not be loaded: " << endl;
			for(unsigned i = 0; i < pendingPresetFiles.size(); i++)
				cout << pendingPresetFiles[i] << endl;
			break;
		}
		else previousCount = pendingPresetFiles.size();
	}
}

void Pseudo3DCarseGame::loadVehicles()
{
	cout << "reading vehicles..." << endl;
	vector<string> vehicleFiles = fgeal::getFilenamesWithinDirectory("data/vehicles");
	for(unsigned i = 0; i < vehicleFiles.size(); i++)
	{
		string& filename = vehicleFiles[i];
		if(ends_with(filename, ".properties"))
		{
			Properties prop;
			prop.load(filename);
			vehicles.push_back(Vehicle(prop, *this));
			cout << "read vehicle: " << filename << endl;
		}
	}
}
