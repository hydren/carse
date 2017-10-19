/*
 * carse_game_logic.cxx
 *
 *  Created on: 16 de out de 2017
 *      Author: carlosfaruolo
 */

#include "carse_game.hpp"

// XXX debug code
#include <iostream>
using std::cout;
using std::endl;
// XXX debug code

#include "states/race_state.hpp"

#include "futil/string_actions.hpp"
#include "futil/string_split.hpp"

#include <vector>

using futil::Properties;
using futil::ends_with;
using std::vector;
using std::string;
using std::map;

// to reduce typing is good
#define getRaceState() static_cast<Pseudo3DRaceState*>(game.getState(Pseudo3DCarseGame::RACE_STATE_ID))

Pseudo3DCarseGame::Logic::Logic(Pseudo3DCarseGame& game) : game(game) {}

void Pseudo3DCarseGame::Logic::initialize()
{
	this->loadPresetEngineSoundProfiles();
	this->loadCourses();
	this->loadVehicles();
}

void Pseudo3DCarseGame::Logic::onStatesListInitFinished()
{
	this->setNextCourseRandom();  // set default course
	this->setPickedVehicle(vehicles[0]);  // set default vehicle
	this->setImperialUnitEnabled(false);
	this->setSimulationType(Mechanics::SIMULATION_TYPE_SLIPLESS);
}

void Pseudo3DCarseGame::Logic::loadPresetEngineSoundProfiles()
{
	cout << "reading preset engine sound profiles..." << endl;
	vector<string> pendingPresetFiles, presetFiles = fgeal::filesystem::getFilenamesWithinDirectory("assets/sound/engine");
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
			Properties prop;
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

EngineSoundProfile& Pseudo3DCarseGame::Logic::getPresetEngineSoundProfile(const std::string presetName)
{
	if(presetEngineSoundProfiles.find(presetName) != presetEngineSoundProfiles.end())
		return presetEngineSoundProfiles[presetName];
	else
		return presetEngineSoundProfiles["default"];
}

void Pseudo3DCarseGame::Logic::loadCourses()
{
	cout << "reading courses..." << endl;

	vector<string> courseFiles = fgeal::filesystem::getFilenamesWithinDirectory("data/courses");
	for(unsigned i = 0; i < courseFiles.size(); i++)
	{
		if(ends_with(courseFiles[i], ".properties"))
		{
			Properties prop;
			prop.load(courseFiles[i]);
			courses.push_back(Course::createCourseFromFile(prop));
			courses.back().filename = courseFiles[i];
			cout << "read course: " << courseFiles[i] << endl;
		}
	}
}

const vector<Course>& Pseudo3DCarseGame::Logic::getCourseList()
{
	return courses;
}

void Pseudo3DCarseGame::Logic::setNextCourse(unsigned courseIndex)
{
	getRaceState()->course = courses[courseIndex];
}

void Pseudo3DCarseGame::Logic::setNextCourse(const Course& c)
{
	getRaceState()->course = c;
}

void Pseudo3DCarseGame::Logic::setNextCourseRandom()
{
	getRaceState()->course = Course::createRandomCourse(200, 3000, 6400, 1.5);
}

void Pseudo3DCarseGame::Logic::setNextCourseDebug()
{
	getRaceState()->course = Course::createDebugCourse(200, 3000);
}

void Pseudo3DCarseGame::Logic::loadVehicles()
{
	cout << "reading vehicles..." << endl;
	vector<string> vehicleFiles = fgeal::filesystem::getFilenamesWithinDirectory("data/vehicles");
	for(unsigned i = 0; i < vehicleFiles.size(); i++)
	{
		const string& filename = vehicleFiles[i];
		if(fgeal::filesystem::isFilenameDirectory(filename))
		{
			vector<string> subfolderFiles = fgeal::filesystem::getFilenamesWithinDirectory(filename);
			for(unsigned j = 0; j < subfolderFiles.size(); j++)
			{
				const string& subfolderFile = subfolderFiles[j];
				if(ends_with(subfolderFile, ".properties"))
				{
					Properties prop;
					prop.load(subfolderFile);
					vehicles.push_back(Vehicle(prop, game));
					cout << "read vehicle: " << subfolderFile << endl;
					break;
				}
			}
		}
		else if(ends_with(filename, ".properties"))
		{
			Properties prop;
			prop.load(filename);
			vehicles.push_back(Vehicle(prop, game));
			cout << "read vehicle: " << filename << endl;
		}
	}
}

const std::vector<Vehicle>& Pseudo3DCarseGame::Logic::getVehicleList()
{
	return vehicles;
}

void Pseudo3DCarseGame::Logic::setPickedVehicle(unsigned vehicleIndex, int skin)
{
	getRaceState()->vehicle = vehicles[vehicleIndex];
	getRaceState()->vehicle.activeSkin = skin;
}

void Pseudo3DCarseGame::Logic::setPickedVehicle(const Vehicle& v, int skin)
{
	getRaceState()->vehicle = v;
	getRaceState()->vehicle.activeSkin = skin;
}

bool Pseudo3DCarseGame::Logic::isImperialUnitEnabled()
{
	return getRaceState()->isImperialUnit;
}

void Pseudo3DCarseGame::Logic::setImperialUnitEnabled(bool choice)
{
	getRaceState()->isImperialUnit = choice;
}

Mechanics::SimulationType Pseudo3DCarseGame::Logic::getSimulationType()
{
	return getRaceState()->simulationType;
}

void Pseudo3DCarseGame::Logic::setSimulationType(Mechanics::SimulationType type)
{
	getRaceState()->simulationType = type;
}

// ----------------------------------------------------------------------------------------------------------

Pseudo3DCarseGame::SharedResources::SharedResources()
: sndCursorMove("assets/sound/cursor_move.ogg"),
  sndCursorIn("assets/sound/cursor_accept.ogg"),
  sndCursorOut("assets/sound/cursor_out.ogg"),
  fontDev("assets/font.ttf", 12)
{}
