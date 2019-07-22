/*
 * carse_game_logic.cpp
 *
 *  Created on: 1 de dez de 2017
 *      Author: carlosfaruolo
 */

#include "carse_logic.hpp"

#include "carse_game.hpp"

#include "race_only_args.hpp"

#include "util.hpp"

#include "futil/random.h"
#include "futil/string_actions.hpp"
#include "futil/properties.hpp"

#include <iostream>
#include <algorithm>

// to reduce typing is good
#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;
using futil::ends_with;
using futil::Properties;
using fgeal::Color;

const string
	CarseLogic::COURSES_FOLDER = "data/courses",
	CarseLogic::VEHICLES_FOLDER = "data/vehicles",
	CarseLogic::TRAFFIC_FOLDER = "data/traffic",
	CarseLogic::PRESET_ENGINE_SOUND_PROFILES_FOLDER = "assets/sound/engine",
	CarseLogic::PRESET_COURSE_STYLES_FOLDER = "data/courses/styles";

CarseLogic* CarseLogic::instance = null;

// logic constructor, booooooring!
CarseLogic::CarseLogic()
: nextMatchRaceSettings(), nextMatchSimulationType(), nextMatchJumpSimulationEnabled(),
  nextMatchCourseSpec(0, 0), nextMatchPlayerVehicleSpecAlternateSpriteIndex(-1),
  raceOnlyMode(), masterVolume(0.9f),
  currentMainMenuStateId(CarseGame::MAIN_MENU_CLASSIC_LAYOUT_STATE_ID),
  currentVehicleSelectionStateId(CarseGame::VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_ID)
{}

void CarseLogic::initialize()
{
	this->loadPresetEngineSoundProfiles();
	this->loadPresetCourseStyles();
	this->loadCourses();
	this->loadVehicles();
	this->loadTrafficVehicles();
}

void CarseLogic::onStatesListInitFinished()
{
	const Pseudo3DRaceState::RaceType defaultRaceType = Pseudo3DRaceState::RACE_TYPE_LOOP_TIME_TRIAL;
	nextMatchRaceSettings.raceType = defaultRaceType;  // set default race type
	nextMatchRaceSettings.lapCountGoal = 2;    // set default lap count
	nextMatchRaceSettings.trafficDensity = 0;  // by default, no traffic
	nextMatchRaceSettings.isImperialUnit = false;
	nextMatchRaceSettings.hudType = Pseudo3DRaceState::HUD_TYPE_DIALGAUGE_TACHO_NUMERIC_SPEEDO;
	nextMatchRaceSettings.useCachedDialGauge = false;
	nextMatchRaceSettings.hudDialGaugePointerImageFilename.clear();
	nextMatchSimulationType = Mechanics::SIMULATION_TYPE_SLIPLESS;
	nextMatchJumpSimulationEnabled = false;

	if(raceOnlyMode)
	{
		if(RaceOnlyArgs::debugMode.isSet())
		{
			this->nextMatchRaceSettings.raceType = Pseudo3DRaceState::RACE_TYPE_DEBUG;
			this->setNextCourseDebug();
		}
		else
		{
			if(RaceOnlyArgs::randomCourse.isSet())
				this->setNextCourseRandom();
			else
			{
				if(RaceOnlyArgs::courseIndex.getValue() < courses.size())
					nextMatchCourseSpec = courses[RaceOnlyArgs::courseIndex.getValue()];
				else
				{
					nextMatchCourseSpec = courses.back();
					cout << "warning: specified course index is out of bounds! using another valid index instead..." << endl;
				}
			}

			if(RaceOnlyArgs::raceType.getValue() > 0 and RaceOnlyArgs::raceType.getValue() < Pseudo3DRaceState::RACE_TYPE_COUNT)
				nextMatchRaceSettings.raceType = static_cast<Pseudo3DRaceState::RaceType>(RaceOnlyArgs::raceType.getValue());
			else
			{
				nextMatchRaceSettings.raceType = defaultRaceType;
				if(RaceOnlyArgs::raceType.getValue() != 0)
					cout << "warning: specified race type index is out of bounds! using default race type instead..." << endl;
			}

			if(Pseudo3DRaceState::isRaceTypeLoop(nextMatchRaceSettings.raceType))
				nextMatchRaceSettings.lapCountGoal = RaceOnlyArgs::lapCount.getValue();
		}

		if(RaceOnlyArgs::vehicleIndex.getValue() < vehicles.size())
			nextMatchPlayerVehicleSpec = vehicles[RaceOnlyArgs::vehicleIndex.getValue()];
		else
		{
			nextMatchPlayerVehicleSpec = vehicles.back();
			cout << "warning: specified player vehicle index is out of bounds! using another valid index instead..." << endl;
		}

		if(RaceOnlyArgs::vehicleAlternateSpriteIndex.getValue() < (int) nextMatchPlayerVehicleSpec.alternateSprites.size())
			nextMatchPlayerVehicleSpecAlternateSpriteIndex = RaceOnlyArgs::vehicleAlternateSpriteIndex.getValue();
		else
		{
			nextMatchPlayerVehicleSpecAlternateSpriteIndex = nextMatchPlayerVehicleSpec.alternateSprites.size()-1;
			cout << "warning: specified player vehicle alternate sprite index is out of bounds! using another valid index instead..." << endl;
		}
	}
	else
	{
		this->setNextCourseRandom();  // set default course
		this->setPickedVehicle(vehicles[0]);  // set default vehicle
	}
}

const EngineSoundProfile& CarseLogic::getPresetEngineSoundProfile(const std::string& presetName) const
{
	if(presetEngineSoundProfiles.find(presetName) != presetEngineSoundProfiles.end())
		return presetEngineSoundProfiles.find(presetName)->second;
	else
		return presetEngineSoundProfiles.find("default")->second;
}

const Pseudo3DCourse::Spec::LandscapeStyle& CarseLogic::getPresetLandscapeStyle(const std::string& presetName) const
{
	if(presetLandscapeStyles.find(presetName) != presetLandscapeStyles.end())
		return presetLandscapeStyles.find(presetName)->second;
	else
		return presetLandscapeStyles.find("default")->second;
}

const Pseudo3DCourse::Spec::RoadStyle& CarseLogic::getPresetRoadStyle(const std::string& presetName) const
{
	if(presetRoadStyles.find(presetName) != presetRoadStyles.end())
		return presetRoadStyles.find(presetName)->second;
	else
		return presetRoadStyles.find("default")->second;
}

vector<string> CarseLogic::getPresetRoadStylesNames() const
{
	vector<string> names(presetRoadStyles.size()); int i = 0;
	for(map<string, Pseudo3DCourse::Spec::RoadStyle>::const_iterator it = presetRoadStyles.begin(); it != presetRoadStyles.end(); ++it)
		names[i++] = it->first;
	return names;
}

vector<string> CarseLogic::getPresetLandscapeStylesNames() const
{
	vector<string> names(presetLandscapeStyles.size()); int i = 0;
	for(map<string, Pseudo3DCourse::Spec::LandscapeStyle>::const_iterator it = presetLandscapeStyles.begin(); it != presetLandscapeStyles.end(); ++it)
		names[i++] = it->first;
	return names;
}

const Pseudo3DCourse::Spec::LandscapeStyle& CarseLogic::getRandomPresetLandscapeStyle() const
{
	map<string, Pseudo3DCourse::Spec::LandscapeStyle>::const_iterator it = presetLandscapeStyles.begin();
	std::advance(it, futil::random_between(0, presetLandscapeStyles.size()));
	return it->second;
}

const Pseudo3DCourse::Spec::RoadStyle& CarseLogic::getRandomPresetRoadStyle() const
{
	map<string, Pseudo3DCourse::Spec::RoadStyle>::const_iterator it = presetRoadStyles.begin();
	std::advance(it, futil::random_between(0, presetRoadStyles.size()));
	return it->second;
}

void CarseLogic::updateCourseList()
{
	courses.clear();
	this->loadCourses();
}

const vector<Pseudo3DCourse::Spec>& CarseLogic::getCourseList()
{
	return courses;
}

void CarseLogic::setNextCourse(unsigned courseIndex)
{
	nextMatchCourseSpec = courses[courseIndex];
}

void CarseLogic::setNextCourse(const Pseudo3DCourse::Spec& c)
{
	nextMatchCourseSpec = c;
}

void CarseLogic::setNextCourseRandom()
{
	nextMatchCourseSpec = Pseudo3DCourse::Spec::createRandom(200, 3000, 6400, 1.5);
	nextMatchCourseSpec.assignStyle(this->getRandomPresetRoadStyle());
	nextMatchCourseSpec.assignStyle(this->getRandomPresetLandscapeStyle());
}

void CarseLogic::setNextCourseDebug()
{
	nextMatchCourseSpec = Pseudo3DCourse::Spec::createDebug();
	nextMatchRaceSettings.raceType = Pseudo3DRaceState::RACE_TYPE_DEBUG;
}

const Pseudo3DCourse::Spec& CarseLogic::getNextCourse()
{
	return nextMatchCourseSpec;
}

Pseudo3DRaceState::RaceSettings& CarseLogic::getNextRaceSettings()
{
	return nextMatchRaceSettings;
}

const vector<Pseudo3DVehicle::Spec>& CarseLogic::getVehicleList()
{
	return vehicles;
}

const Pseudo3DVehicle::Spec& CarseLogic::getPickedVehicle()
{
	return nextMatchPlayerVehicleSpec;
}

const int CarseLogic::getPickedVehicleAlternateSpriteIndex()
{
	return nextMatchPlayerVehicleSpecAlternateSpriteIndex;
}

void CarseLogic::setPickedVehicle(unsigned vehicleIndex, int altSpriteIndex)
{
	nextMatchPlayerVehicleSpec = vehicles[vehicleIndex];
	nextMatchPlayerVehicleSpecAlternateSpriteIndex = altSpriteIndex;
}

void CarseLogic::setPickedVehicle(const Pseudo3DVehicle::Spec& vspec, int altSpriteIndex)
{
	nextMatchPlayerVehicleSpec = vspec;
	nextMatchPlayerVehicleSpecAlternateSpriteIndex = altSpriteIndex;
}

const vector<Pseudo3DVehicle::Spec>& CarseLogic::getTrafficVehicleList()
{
	return trafficVehicles;
}

Mechanics::SimulationType CarseLogic::getSimulationType()
{
	return nextMatchSimulationType;
}

void CarseLogic::setSimulationType(Mechanics::SimulationType type)
{
	nextMatchSimulationType = type;
}

bool CarseLogic::isJumpSimulationEnabled()
{
	return nextMatchJumpSimulationEnabled;
}

void CarseLogic::setJumpSimulationEnabled(bool enabled)
{
	nextMatchJumpSimulationEnabled = enabled;
}

//===============================================================================================================================================
void CarseLogic::loadPresetEngineSoundProfiles()
{
	cout << "reading preset engine sound profiles..." << endl;
	vector<string> pendingPresetFiles, presetFiles = fgeal::filesystem::getFilenamesWithinDirectory(CarseLogic::PRESET_ENGINE_SOUND_PROFILES_FOLDER);
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

void CarseLogic::loadCourses()
{
	cout << "reading courses..." << endl;

	vector<string> courseFiles = fgeal::filesystem::getFilenamesWithinDirectory(CarseLogic::COURSES_FOLDER);
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

void CarseLogic::loadVehicles()
{
	cout << "reading vehicles specs..." << endl;

	// create a list of files inside the vehicles folder and inside its subfolders (but not recursively)
	vector<string> possibleVehiclePropertiesFilenames;
	const vector<string> vehiclesFolderFilenames = fgeal::filesystem::getFilenamesWithinDirectory(CarseLogic::VEHICLES_FOLDER);
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
				cout << "read vehicle specification: " << filename << endl;
			}
		}
	}
}

void CarseLogic::loadTrafficVehicles()
{
	cout << "reading traffic vehicles specs..." << endl;

	// create a list of files inside the traffic folder and inside its subfolders (but not recursively)
	vector<string> possibleTrafficPropertiesFilenames;
	const vector<string> trafficFolderFilenames = fgeal::filesystem::getFilenamesWithinDirectory(CarseLogic::TRAFFIC_FOLDER);
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
				trafficVehicles.back().soundProfile = EngineSoundProfile();  // force no sound for traffic FIXME remove this line and deal with engine sound sharing properly between traffic vehicles
			}
		}
	}
}

void CarseLogic::loadPresetCourseStyles()
{
	cout << "reading preset course styles..." << endl;
	presetLandscapeStyles["default"] = Pseudo3DCourse::Spec::LandscapeStyle::DEFAULT;
	presetRoadStyles["default"] = Pseudo3DCourse::Spec::RoadStyle::DEFAULT;
	vector<string> pendingRoadStylePresetFiles, pendingLandscapePresetFiles, presetFiles = fgeal::filesystem::getFilenamesWithinDirectory(CarseLogic::PRESET_COURSE_STYLES_FOLDER);
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
EngineSoundProfile CarseLogic::createEngineSoundProfileFromFile(const string& filename)
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
					const string sndFilename = getContextualizedFilename(prop.get(subBaseKey), baseDir, CarseLogic::VEHICLES_FOLDER+"/", CarseLogic::PRESET_ENGINE_SOUND_PROFILES_FOLDER+"/");
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

