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
#include <algorithm>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using futil::ends_with;
using futil::Properties;
using fgeal::Color;

// to reduce typing is good
#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

const string
	CarseGame::Logic::COURSES_FOLDER = "data/courses",
	CarseGame::Logic::VEHICLES_FOLDER = "data/vehicles",
	CarseGame::Logic::TRAFFIC_FOLDER = "data/traffic",
	CarseGame::Logic::PRESET_ENGINE_SOUND_PROFILES_FOLDER = "assets/sound/engine",
	CarseGame::Logic::PRESET_COURSE_STYLES_FOLDER = "data/courses/styles";

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

			if(prop.containsKey("sound") and not prop.get("sound").empty())
			{
				if(prop.get("sound") != "custom")
				{
					pendingPresetFiles.push_back(filename);
					cout << "read engine sound profile: " << presetName << " (alias)" << endl;
				}
				else
				{
					presetEngineSoundProfiles[presetName] = createEngineSoundProfileFromFile(filename);
					cout << "read engine sound profile: " << presetName << endl;
				}
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
			try { courses.push_back(Pseudo3DCourse::Spec::createFromFile(courseFiles[i], CarseGameLogicInstance(this))); }
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
				try { vehicles.push_back(Pseudo3DVehicle::Spec()); vehicles.back().loadFromFile(filename, CarseGameLogicInstance(this)); }
				catch(const std::exception& e) { cout << "error while reading vehicle specification: " << e.what() << endl; continue; }
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
				try { trafficVehicles.push_back(Pseudo3DVehicle::Spec()); trafficVehicles.back().loadFromFile(filename, CarseGameLogicInstance(this)); }
				catch(const std::exception& e) { cout << "error while reading traffic specification: " << e.what() << endl; continue; }
				cout << "read traffic specification: " << filename << endl;
			}
		}
	}
}

void CarseGame::Logic::loadPresetCourseStyles()
{
	cout << "reading preset course styles..." << endl;
	presetLandscapeStyles["default"] = Pseudo3DCourse::Spec::LandscapeStyle::DEFAULT;
	presetRoadStyles["default"] = Pseudo3DCourse::Spec::RoadStyle::DEFAULT;
	vector<string> pendingRoadStylePresetFiles, pendingLandscapePresetFiles, presetFiles = fgeal::filesystem::getFilenamesWithinDirectory(CarseGame::Logic::PRESET_COURSE_STYLES_FOLDER);
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

			if(prop.containsKey("preset_road_style") and not prop.get("preset_road_style").empty())
			{
				if(prop.get("preset_road_style") != "custom")
				{
					pendingRoadStylePresetFiles.push_back(filename);
					cout << "read course road style: " << presetName << " (alias)" << endl;
				}
				else
				{
					presetRoadStyles[presetName].loadFromFile(filename, presetName);
					cout << "read course road style: " << presetName << endl;
				}
			}

			if(prop.containsKey("preset_landscape_style") and not prop.get("preset_landscape_style").empty())
			{
				if(prop.get("preset_landscape_style") != "custom")
				{
					pendingLandscapePresetFiles.push_back(filename);
					cout << "read course landscape style: " << presetName << " (alias)" << endl;
				}
				else
				{
					presetLandscapeStyles[presetName].loadFromFile(filename, presetName);
					cout << "read course landscape style: " << presetName << endl;
				}
			}
		}
	}

	unsigned previousCount = pendingRoadStylePresetFiles.size();
	while(not pendingRoadStylePresetFiles.empty())
	{
		for(unsigned i = 0; i < pendingRoadStylePresetFiles.size(); i++)
		{
			string filename = pendingRoadStylePresetFiles[i];
			Properties prop;
			prop.load(filename);

			const string
				basePresetName = prop.get("preset_road_style"),
				filenameWithoutPath = filename.substr(filename.find_last_of("/\\")+1),
				filenameWithoutExtension = filenameWithoutPath.substr(0, filenameWithoutPath.find_last_of(".")),
				presetName = filenameWithoutExtension;

			if(presetRoadStyles.find(basePresetName) != presetRoadStyles.end())
			{
				presetRoadStyles[presetName] = presetRoadStyles[basePresetName];
				cout << "copied profile \"" << presetName << "\" from \"" << basePresetName << "\"" << endl;
				pendingRoadStylePresetFiles.erase(pendingRoadStylePresetFiles.begin() + i);
				i--;
			}
		}
		if(pendingRoadStylePresetFiles.size() == previousCount)
		{
			cout << "circular dependency or unresolved reference detected when loading preset road style. skipping resolution." << endl;
			cout << "the following preset road style could not be loaded: " << endl;
			for(unsigned i = 0; i < pendingRoadStylePresetFiles.size(); i++)
				cout << pendingRoadStylePresetFiles[i] << endl;
			break;
		}
		else previousCount = pendingRoadStylePresetFiles.size();
	}
	previousCount = pendingLandscapePresetFiles.size();
	while(not pendingLandscapePresetFiles.empty())
	{
		for(unsigned i = 0; i < pendingLandscapePresetFiles.size(); i++)
		{
			string filename = pendingLandscapePresetFiles[i];
			Properties prop;
			prop.load(filename);

			const string
				basePresetName = prop.get("preset_landscape_style"),
				filenameWithoutPath = filename.substr(filename.find_last_of("/\\")+1),
				filenameWithoutExtension = filenameWithoutPath.substr(0, filenameWithoutPath.find_last_of(".")),
				presetName = filenameWithoutExtension;

			if(presetLandscapeStyles.find(basePresetName) != presetLandscapeStyles.end())
			{
				presetLandscapeStyles[presetName] = presetLandscapeStyles[basePresetName];
				cout << "copied profile \"" << presetName << "\" from \"" << basePresetName << "\"" << endl;
				pendingLandscapePresetFiles.erase(pendingLandscapePresetFiles.begin() + i);
				i--;
			}
		}
		if(pendingLandscapePresetFiles.size() == previousCount)
		{
			cout << "circular dependency or unresolved reference detected when loading preset landscape style. skipping resolution." << endl;
			cout << "the following preset landscape style could not be loaded: " << endl;
			for(unsigned i = 0; i < pendingLandscapePresetFiles.size(); i++)
				cout << pendingLandscapePresetFiles[i] << endl;
			break;
		}
		else previousCount = pendingLandscapePresetFiles.size();
	}
}

// ========================================================================================================================
EngineSoundProfile CarseGame::Logic::createEngineSoundProfileFromFile(const string& filename)
{
	EngineSoundProfile profile;
	Properties prop;
	prop.load(filename);

	const string baseDir = filename.substr(0, filename.find_last_of("/\\")+1);
	const short maxRpm = prop.getParsedCStrAllowDefault<int, atoi>("engine_maximum_rpm", 7000);
	profile.allowRpmPitching = true;

	const string baseKey = "sound";
	if(prop.containsKey(baseKey))
	{
		if(prop.get(baseKey) == "none")
		{
			profile.ranges.clear();
		}
		else if(prop.get(baseKey) == "custom")
		{
			string key = baseKey + "_rpm_pitching";
			if(isValueSpecified(prop, key))
			{
				string value = futil::trim(prop.get(key));
				if(value == "false" or value == "no")
					profile.allowRpmPitching = false;
			}

			key = baseKey + "_count";
			unsigned soundCount = prop.getParsedCStrAllowDefault<int, atoi>(key, 16);

			for(unsigned i = 0; i < soundCount; i++)
			{
				const string subBaseKey = baseKey + futil::to_string(i);
				if(prop.containsKey(subBaseKey))
				{
					const string sndFilename = getContextualizedFilename(prop.get(subBaseKey), baseDir, CarseGame::Logic::VEHICLES_FOLDER+"/", CarseGame::Logic::PRESET_ENGINE_SOUND_PROFILES_FOLDER+"/");
					if(sndFilename.empty())
						cout << "warning: sound file \"" << prop.get(subBaseKey) << "\" could not be found!"  // todo use default sound?
						<< " (specified by \"" << filename << "\")" << endl;

					// now try to read _rpm property
					key = subBaseKey + "_rpm";
					short rpm = -1;
					if(prop.containsKey(key))
						rpm = atoi(prop.get(key).c_str());

					// if rpm < 0, either rpm wasn't specified, or was intentionally left -1 (or other negative number)
					if(rpm < 0)
					{
						if(i == 0) rpm = 0;
						else       rpm = (maxRpm - profile.ranges.rbegin()->startRpm)/2;
					}

					key = subBaseKey + "_depicted_rpm";
					short depictedRpm = -1;
					if(prop.containsKey(key))
						depictedRpm = atoi(prop.get(key).c_str());

					key = subBaseKey + "_pitch_factor";
					if(prop.containsKey(key))
					{
						const double pitchFactor = atof(prop.get(key).c_str());
						if(pitchFactor > 0)
							depictedRpm = rpm*pitchFactor;
					}

					if(depictedRpm < 0)
						depictedRpm = rpm;

					if(depictedRpm == 0)
						depictedRpm = 1;  // to avoid division by zero

					// save filename and settings for given rpm
					const EngineSoundProfile::RangeProfile range = {rpm, depictedRpm, sndFilename};
					profile.ranges.push_back(range);
				}
				else cout << "warning: missing expected entry \"" << subBaseKey << "\" (specified by \"" << filename << "\")" << endl;
			}

			struct RangeProfileCompare { static bool function(const EngineSoundProfile::RangeProfile& p1, const EngineSoundProfile::RangeProfile& p2) { return p1.startRpm < p2.startRpm; } };
			std::stable_sort(profile.ranges.begin(), profile.ranges.end(), RangeProfileCompare::function);
		}
		else
			throw std::logic_error("properties specify a preset profile instead of a custom one");
	}

	return profile;
}
