/*
 * carse_game_properties_io.cpp
 *
 *  Created on: 9 de jan de 2018
 *      Author: carlosfaruolo
 */

#include "carse_game.hpp"

#include "util.hpp"

#include "futil/string_actions.hpp"
#include "futil/properties.hpp"

#include <iostream>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using futil::ends_with;
using futil::Properties;

// to reduce typing is good
#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

// returns true if the given properties requests a preset sound profile instead of specifying a custom one.
static bool isEngineSoundProfileRequestingPreset(const Properties& prop)
{
	return prop.containsKey("sound") and prop.get("sound") != "custom" and prop.get("sound") != "no";
}

// ========================================================================================================================

const string
	CarseGame::Logic::COURSES_FOLDER = "data/courses",
	CarseGame::Logic::VEHICLES_FOLDER = "data/vehicles",
	CarseGame::Logic::TRAFFIC_FOLDER = "data/traffic",
	CarseGame::Logic::PRESET_ENGINE_SOUND_PROFILES_FOLDER = "assets/sound/engine";

void CarseGame::Logic::loadPresetEngineSoundProfiles()
{
	cout << "reading preset engine sound profiles..." << endl;
	vector<string> pendingPresetFiles, presetFiles = fgeal::filesystem::getFilenamesWithinDirectory(CarseGame::Logic::PRESET_ENGINE_SOUND_PROFILES_FOLDER);
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

			if(isEngineSoundProfileRequestingPreset(prop))
			{
				pendingPresetFiles.push_back(filename);
				cout << "read engine sound profile: " << presetName << " (alias)" << endl;
			}

			else
			{
				presetEngineSoundProfiles[presetName] = Pseudo3DVehicle::Spec::createEngineSoundProfileFromFile(filename);
				cout << "read engine sound profile: " << presetName << endl;
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
				basePresetName = prop.get("sound"),
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

void CarseGame::Logic::loadCourses()
{
	cout << "reading courses..." << endl;

	vector<string> courseFiles = fgeal::filesystem::getFilenamesWithinDirectory(CarseGame::Logic::COURSES_FOLDER);
	for(unsigned i = 0; i < courseFiles.size(); i++)
	{
		if(ends_with(courseFiles[i], ".properties"))
		{
			try { courses.push_back(Pseudo3DCourse::Spec::createFromFile(courseFiles[i])); }
			catch(const std::exception& e) { cout << "error while reading course specification: " << e.what() << endl; continue; }
			cout << "read course specification: " << courseFiles[i] << endl;
		}
	}
}

void CarseGame::Logic::loadVehicles()
{
	cout << "reading vehicles specs..." << endl;

	// create a list of files inside the vehicles folder and inside its subfolders (but not recursively)
	vector<string> possibleVehiclePropertiesFilenames;
	const vector<string> vehiclesFolderFilenames = fgeal::filesystem::getFilenamesWithinDirectory(CarseGame::Logic::VEHICLES_FOLDER);
	for(unsigned i = 0; i < vehiclesFolderFilenames.size(); i++)
	{
		const string& filename = vehiclesFolderFilenames[i];
		if(fgeal::filesystem::isFilenameDirectory(filename))
		{
			const vector<string> subfolderFilenames = fgeal::filesystem::getFilenamesWithinDirectory(filename);
			for(unsigned j = 0; j < subfolderFilenames.size(); j++)
				possibleVehiclePropertiesFilenames.push_back(subfolderFilenames[j]);
		}
		else possibleVehiclePropertiesFilenames.push_back(filename);
	}

	// check the list of possible "vehicle properties" filenames
	for(unsigned i = 0; i < possibleVehiclePropertiesFilenames.size(); i++)
	{
		const string& filename = possibleVehiclePropertiesFilenames[i];
		if(fgeal::filesystem::isFilenameArchive(filename) and ends_with(filename, ".properties"))
		{
			Properties prop;
			prop.load(filename);
			if(prop.containsKey("definition") and prop.get("definition") == "vehicle")
			{
				try { vehicles.push_back(Pseudo3DVehicle::Spec::createFromFile(filename)); }
				catch(const std::exception& e) { cout << "error while reading vehicle specification: " << e.what() << endl; continue; }

				// vehicle specifies a preset profile instead of a custom one, so read it
				if(isEngineSoundProfileRequestingPreset(prop))
				{
					const string profileName = prop.get("sound");
					if(presetEngineSoundProfiles.find(profileName) != presetEngineSoundProfiles.end())
						vehicles.back().soundProfile = presetEngineSoundProfiles[profileName];
				}

				cout << "read vehicle specification: " << filename << endl;
			}
		}
	}
}

void CarseGame::Logic::loadTrafficVehicles()
{
	cout << "reading traffic vehicles specs..." << endl;

	// create a list of files inside the traffic folder and inside its subfolders (but not recursively)
	vector<string> possibleTrafficPropertiesFilenames;
	const vector<string> trafficFolderFilenames = fgeal::filesystem::getFilenamesWithinDirectory(CarseGame::Logic::TRAFFIC_FOLDER);
	for(unsigned i = 0; i < trafficFolderFilenames.size(); i++)
	{
		const string& filename = trafficFolderFilenames[i];
		if(fgeal::filesystem::isFilenameDirectory(filename))
		{
			const vector<string> subfolderFilenames = fgeal::filesystem::getFilenamesWithinDirectory(filename);
			for(unsigned j = 0; j < subfolderFilenames.size(); j++)
				possibleTrafficPropertiesFilenames.push_back(subfolderFilenames[j]);
		}
		else possibleTrafficPropertiesFilenames.push_back(filename);
	}

	// check the list of possible "traffic vehicles properties" filenames
	for(unsigned i = 0; i < possibleTrafficPropertiesFilenames.size(); i++)
	{
		const string& filename = possibleTrafficPropertiesFilenames[i];
		if(fgeal::filesystem::isFilenameArchive(filename) and ends_with(filename, ".properties"))
		{
			Properties prop;
			prop.load(filename);
			if(prop.containsKey("definition") and prop.get("definition") == "vehicle")
			{
				try { trafficVehicles.push_back(Pseudo3DVehicle::Spec::createFromFile(filename)); }
				catch(const std::exception& e) { cout << "error while reading traffic specification: " << e.what() << endl; continue; }
				cout << "read traffic specification: " << filename << endl;
			}
		}
	}
}
