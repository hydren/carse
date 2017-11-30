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

#include <cstdlib>
#include <cmath>

using futil::Properties;
using futil::ends_with;
using std::vector;
using std::string;
using std::map;

// to reduce typing is good
#define getRaceState() static_cast<Pseudo3DRaceState*>(game.getState(Pseudo3DCarseGame::RACE_STATE_ID))
#define isValueSpecified(prop, key) (prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "default")

// default float constants
static const float
	DEFAULT_VEHICLE_MASS = 1250,  // kg
	DEFAULT_TIRE_DIAMETER = 678;  // mm

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

void Pseudo3DCarseGame::Logic::loadVehicleSpec(Pseudo3DVehicle::Spec& spec, const futil::Properties& prop)
{
	// aux. var
	string key;

	// logic data

	key = "vehicle_type";
	if(prop.containsKey(key))
	{
		string t = futil::to_lower(prop.get(key));
		if(t == "car" or t == "default") spec.type = Mechanics::TYPE_CAR;
		else if(t == "bike") spec.type = Mechanics::TYPE_BIKE;
		else spec.type = Mechanics::TYPE_OTHER;
	}
	else spec.type = Mechanics::TYPE_CAR;

	spec.body = Mechanics(Engine(prop), spec.type);

	// info data

	key = "vehicle_name";
	spec.name = prop.containsKey(key)? prop.get(key) : "unnamed";

	key = "authors";
	spec.authors = prop.containsKey(key)? prop.get(key) : "unknown";

	key = "credits";
	spec.credits = prop.containsKey(key)? prop.get(key) : "";

	key = "comments";
	spec.comments = prop.containsKey(key)? prop.get(key) : "";

	// todo read more data from properties

	// physics data

	key = "vehicle_mass";
	spec.mass = isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_VEHICLE_MASS;

	key = "tire_diameter";
	spec.tireRadius = (isValueSpecified(prop, key)? atof(prop.get(key).c_str()) : DEFAULT_TIRE_DIAMETER) * 0.0005;

	key = "engine_location";
	if(isValueSpecified(prop, key))
	{
		const string value = prop.get(key);
		if(value == "mid" or value == "middle") spec.engineLocation = Mechanics::ENGINE_LOCATION_ON_MIDDLE;
		else if(value == "rear") spec.engineLocation = Mechanics::ENGINE_LOCATION_ON_REAR;
		else spec.engineLocation = Mechanics::ENGINE_LOCATION_ON_FRONT;
	}
	else
		spec.engineLocation = Mechanics::ENGINE_LOCATION_ON_FRONT;

	key = "driven_wheels";
	if(isValueSpecified(prop, key))
	{
		const string value = prop.get(key);
		if(value == "all") spec.drivenWheelsType = Mechanics::DRIVEN_WHEELS_ALL;
		else if(value == "front") spec.drivenWheelsType = Mechanics::DRIVEN_WHEELS_ON_FRONT;
		else spec.drivenWheelsType = Mechanics::DRIVEN_WHEELS_ON_REAR;
	}
	else
		spec.drivenWheelsType = Mechanics::DRIVEN_WHEELS_ON_REAR;

	body.setSuggestedWeightDistribuition();

	// sound data

	if(EngineSoundProfile::requestsPresetProfile(prop))
		spec.sound = getPresetEngineSoundProfile(EngineSoundProfile::getSoundDefinitionFromProperties(prop));
	else
		spec.sound = EngineSoundProfile::loadFromProperties(prop);

	// sprite data

	spec.sprite = Pseudo3DVehicleAnimationProfile(prop);

	// ########################################################################################################################################################
	// These properties need to be loaded after sprite data to make sure that some fields are ready ('depictedVehicleWidth', 'sprite_sheet_file', etc)

	if(spec.sprite.sheetFilename == "DEFAULT")
	{
		// uncomment when there is a default sprite for bikes
//		switch(type)
//		{
//			case TYPE_BIKE:  sprite.sheetFilename = "assets/bike-sheet-default.png"; break;
//			default:
//			case TYPE_OTHER:
//			case TYPE_CAR:   sprite.sheetFilename = "assets/car-sheet-default.png"; break;
//		}

		spec.sprite.sheetFilename = "assets/car-sheet-default.png";
	}

	// attempt to estimate center's of gravity height
	key = "vehicle_height";
	if(isValueSpecified(prop, key))
		spec.centerOfGravityHeight = 0.5f*atof(prop.get(key).c_str());  // aprox. half the height
	else
		spec.centerOfGravityHeight = 0.3506f * spec.sprite.depictedVehicleWidth * spec.sprite.scale.x * 895.0/24.0;  // proportion aprox. of a fairlady z32


	// attempt to estimate wheelbase
	{
		key = "vehicle_wheelbase";
		if(isValueSpecified(prop, key))
			spec.wheelbase = atof(prop.get(key).c_str());
		else
			spec.wheelbase = -1;

		key = "vehicle_length";
		if(spec.wheelbase == -1 and isValueSpecified(prop, key))
			spec.wheelbase = atof(prop.get(key).c_str());

		key = "vehicle_width";
		if(spec.wheelbase == -1 and isValueSpecified(prop, key))
			spec.wheelbase = 2.5251f * atof(prop.get(key).c_str());  // proportion aprox. of a fairlady z32

		key = "vehicle_height";
		if(spec.wheelbase == -1 and isValueSpecified(prop, key))
			spec.wheelbase = 3.6016f * atof(prop.get(key).c_str());  // proportion aprox. of a fairlady z32

		if(spec.wheelbase == -1)
		{
			spec.wheelbase = 2.5251f * spec.sprite.depictedVehicleWidth * spec.sprite.scale.x * 895.0/24.0;  // proportion aprox. of a fairlady z32
		}
	}
}

// ----------------------------------------------------------------------------------------------------------

Pseudo3DCarseGame::SharedResources::SharedResources()
: sndCursorMove("assets/sound/cursor_move.ogg"),
  sndCursorIn("assets/sound/cursor_accept.ogg"),
  sndCursorOut("assets/sound/cursor_out.ogg"),
  fontDev("assets/font.ttf", 12)
{}
