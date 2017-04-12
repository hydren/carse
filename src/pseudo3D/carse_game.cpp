/*
 * carse_game.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "carse_game.hpp"

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

map<short, string> Pseudo3DCarseGame::loadEngineSoundPreset(const Properties& prop)
{
	map<short, string> soundsFilenames;

	string key = "sound";
	if(prop.containsKey(key) and prop.get(key) != "default")
	{
		string soundOption = prop.get(key);
		soundsFilenames.clear();
		if(soundOption == "no")
			soundsFilenames.clear();

		// todo create engine sound classes: crossplane_v8, inline_6, flat_4, etc

		else if(soundOption == "custom")
		{
			key = "sound_redline_last";
			if(prop.containsKey(key) and prop.get(key) == "true")
				isLastSoundRedline = true;

			int i = 0;
			key = "sound0";
			while(prop.containsKey(key))
			{
				string filename = prop.get(key);

				// now try to read _rpm property
				key += "_rpm";
				short rpm = -1;
				if(prop.containsKey(key))
					rpm = atoi(prop.get(key).c_str());

				// if rpm < 0, either rpm wasn't specified, or was intentionally left -1 (or other negative number)
				if(rpm < 0)
				{
					if(i == 0) rpm = 0;
					else       rpm = (engine.maxRpm - soundsFilenames.rbegin()->first)/2;
				}

				// save filename for given rpm
				soundsFilenames[rpm] = filename;
				i += 1;
				key = string("sound") + i;
			}
		}
	}
}

void Pseudo3DCarseGame::loadBuiltinEngineSoundPresets()
{
	Properties prop;

	vector<string> vehicleFiles = fgeal::getFilenamesWithinDirectory("assets/sound/engine");
	for(unsigned i = 0; i < vehicleFiles.size(); i++)
	{
		string filename = vehicleFiles[i];
		if(ends_with(filename, ".properties"))
		{
			util::Properties prop;
			prop.load(filename);



			vehicles.push_back(Vehicle(prop));

			Vehicle& v = vehicles.back();
			cout << "read vehicle " << v.name << endl;
		}
	}
}
