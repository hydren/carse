/*
 * course_spec_parser.cpp
 *
 *  Created on: 26 de fev de 2018
 *      Author: carlosfaruolo
 */

#include "course.hpp"

#include "util.hpp"

#include "futil/properties.hpp"
#include "futil/string_actions.hpp"
#include "futil/string_split.hpp"
#include "futil/collection_actions.hpp"

#include <stdexcept>
#include <iostream>
#include <fstream>

using std::string;
using std::vector;
using std::cout;
using std::endl;
using fgeal::Color;
using fgeal::Display;
using fgeal::Image;
using futil::Properties;
using futil::split;
using futil::trim;
using futil::to_lower;
using futil::starts_with;

using std::to_string;

static const unsigned DEFAULT_SPRITE_COUNT = 32;

namespace  // static
{
	inline Color parseColor(const string& str)
	{
		return Color::parseCStr(str.c_str());
	}

	inline string passString(const string& str)
	{
		return str;
	}

	template<typename PresetType, typename FieldType, FieldType (*parseFunction) (const std::string&)>
	inline FieldType decideValue(Properties& prop, string key, FieldType PresetType::*field, const PresetType* preset, const PresetType& defaultPreset)
	{
		if(not prop.containsKey(key) and preset != null)
			return preset->*field;

		else if(prop.containsKey(key) and prop.get(key) != "default")
			return parseFunction(prop.get(key));

		else return defaultPreset.*field;
	}

	template<typename PresetType>
	inline Color decideColor(Properties& prop, string key, Color PresetType::*colorField, const PresetType* preset, const PresetType& defaultPreset)
	{
		return decideValue<PresetType, Color, parseColor>(prop, key, colorField, preset, defaultPreset);
	}

	template<typename PresetType>
	inline string decideString(Properties& prop, string key, string PresetType::*stringField, const PresetType* preset, const PresetType& defaultPreset)
	{
		return decideValue<PresetType, string, passString>(prop, key, stringField, preset, defaultPreset);
	}
}

void Pseudo3DCourse::Spec::parseProperties(const string& filename)
{
	const string baseDir = filename.substr(0, filename.find_last_of("/\\")+1);

	*this = Spec(0, 0);  // resets all data in this object

	Properties prop;
	prop.load(filename);

	roadSegmentLength = prop.getParsedCStrAllowDefault<double, atof>("segment_length", 200);  // this may become non-customizable
	roadWidth = prop.getParsedCStrAllowDefault<double, atof>("road_width", 3000);

	name = prop.get("name");
	author = prop.get("author");
	credits = prop.get("credits");
	comments = prop.get("comments");
	this->filename = filename;

	musicFilename = trim(prop.get("music"));
	if(musicFilename.empty()) musicFilename = "assets/music_sample.ogg";  // todo remove this line

	// check if preset specified
	const RoadColorSet *presetRoadStyle = null, &defaultRoadStyle = presetRoadColors[0];
	presetRoadStyleName = to_lower(trim(prop.get("preset_road_style")));
	if(not presetRoadStyleName.empty())
	{
		for(unsigned i = 0; i < presetRoadColorsSize and presetRoadStyle == null; i++)
			if(presetRoadStyleName == presetRoadColors[i].name)
				presetRoadStyle = &presetRoadColors[i];
	}
	const LandscapeSettings *presetScenery = null, &defaultScenery = presetLandscapeSettings[0];
	presetSceneryName = to_lower(trim(prop.get("preset_landscape_style")));
	if(not presetSceneryName.empty())
	{
		for(unsigned i = 0; i < presetLandscapeSettingsSize and presetScenery == null; i++)
		if(presetSceneryName == presetLandscapeSettings[i].name)
		{
			const LandscapeSettings& landscape = presetLandscapeSettings[i];

			props.push_back(Prop());
			spritesFilenames.push_back("assets/"+landscape.sprite1);
			props.push_back(Prop(true));
			spritesFilenames.push_back("assets/"+landscape.sprite2);
			props.push_back(Prop(true));
			spritesFilenames.push_back("assets/"+landscape.sprite3);

			presetScenery = &presetLandscapeSettings[i];
		}
	}

	// decide some values between preset and custom defined
	colorRoadPrimary = decideColor(prop, "road_color_primary", &RoadColorSet::primary, presetRoadStyle, defaultRoadStyle);
	colorRoadSecondary = decideColor(prop, "road_color_secondary", &RoadColorSet::secondary, presetRoadStyle, defaultRoadStyle);
	colorOffRoadPrimary = decideColor(prop, "offroad_color_primary", &LandscapeSettings::terrainPrimary, presetScenery, defaultScenery);
	colorOffRoadSecondary = decideColor(prop, "offroad_color_secondary", &LandscapeSettings::terrainSecondary, presetScenery, defaultScenery);
	colorHumblePrimary = decideColor(prop, "humble_color_primary", &RoadColorSet::humblePrimary, presetRoadStyle, defaultRoadStyle);
	colorHumbleSecondary = decideColor(prop, "humble_color_secondary", &RoadColorSet::humbleSecondary, presetRoadStyle, defaultRoadStyle);
	colorLandscape = decideColor(prop, "landscape_color", &LandscapeSettings::sky, presetScenery, defaultScenery);
	colorHorizon = decideColor(prop, "horizon_color", &LandscapeSettings::terrainPrimary, presetScenery, defaultScenery);

	landscapeFilename = getContextualizedFilename(decideString(prop, "landscape_image", &LandscapeSettings::landscapeBgFilename, presetScenery, defaultScenery), baseDir, "assets/");
	if(landscapeFilename.empty())
	{
		cout << "warning: image file specified in \"landscape_image\" entry could not be found" << ", specified by \"" << filename << "\". using default instead..." << endl;
		landscapeFilename = defaultScenery.landscapeBgFilename;
	}

	const int propMaxIndex = prop.getParsedCStrAllowDefault<int, atoi>("prop_max_id", DEFAULT_SPRITE_COUNT);
	for(int id = 0; id <= propMaxIndex; id++)
	{
		string specifiedSpriteFilename = prop.get("prop" + to_string(id) + "_sprite"), spriteFilename;
		if(not specifiedSpriteFilename.empty())
		{
			spriteFilename = getContextualizedFilename(specifiedSpriteFilename, baseDir, "assets/");
			if(spriteFilename.empty())
				cout << "warning: could not load sprite for prop ID #" << id << ": missing file \"" << specifiedSpriteFilename << "\". Prop sprite will be left unspecified!" << endl;
		}

		if(id + 1 > (int) spritesFilenames.size())
			spritesFilenames.push_back(spriteFilename);
		else if(not spriteFilename.empty())
			spritesFilenames[id] = spriteFilename;

		const string blockingFlagStr = to_lower(trim(prop.get("prop" + to_string(id) + "_blocking")));

		if(id + 1 > (int) props.size())
			props.push_back(Prop());

		if(not blockingFlagStr.empty())
			props[id].blocking = (blockingFlagStr == "true");
	}


	float length = prop.getParsedCStrAllowDefault<double, atof>("course_length", 6400);
	lines.resize(length);

	const string specifiedSegmentFilename = prop.getIfContains("segment_file", "Missing segment file for course!");
	segmentFilename = getContextualizedFilename(specifiedSegmentFilename, baseDir);

	std::ifstream stream(segmentFilename.c_str());
	if(not stream.is_open())
		throw std::runtime_error("Course description file could not be opened: \"" + specifiedSegmentFilename + "\", specified by \"" + filename + "\"");
}

void Pseudo3DCourse::Spec::loadSegments(const string& segmentFilename)
{
	std::ifstream stream(segmentFilename.c_str());
	if(not stream.is_open())
		throw std::runtime_error("Course description file could not be opened: \"" + segmentFilename + "\", specified by \"" + filename + "\"");

	for(unsigned i = 0; i < lines.size(); i++)
	{
		CourseSpec::Segment& line = lines[i];
		line.z = i*roadSegmentLength;

		string str;
		do{
			if(stream.good())
			{
				str = trim(str);
				getline(stream, str);
			}
			else
			{
				str.clear();  // if no more input, signal no data by clearing str
				break;
			}
		}
		while(str.empty() or starts_with(str, "#") or starts_with(str, "!")); // ignore empty lines or commented out ones

		vector<string> tokens = split(str, ',');

		line.x = atof(tokens[0].c_str());

		if(tokens.size() >= 2)
			line.y = atof(tokens[1].c_str());

		if(tokens.size() >= 3)
			line.curve = atof(tokens[2].c_str());

		if(tokens.size() >= 4)
			line.slope = atof(tokens[3].c_str());

		if(tokens.size() >= 6)
		{
			line.propIndex = atoi(tokens[4].c_str());
			line.propX = atof(tokens[5].c_str());

			if(line.propIndex != -1 and
			  (line.propIndex + 1 > (int) spritesFilenames.size() or spritesFilenames[line.propIndex].empty()))
				throw std::logic_error("Course indicates usage of an unspecified prop ID (#" + to_string(line.propIndex) + "), specified by \"" + segmentFilename+"\"");
		}

		if(tokens.size() == 5 or tokens.size() > 6)
			std::cout << "warning: line " << i << " had an unexpected number of parameters (" << tokens.size() << ") - some of them we'll be ignored (specified by \"" << segmentFilename << "\")" << std::endl;
	}

	stream.close();
}

void Pseudo3DCourse::Spec::storeProperties(const string& filename, const string& segmentsFilename)
{
	Properties prop;
	prop.put("name", name);
	prop.put("author", author);
	prop.put("credits", credits);
	prop.put("comments", comments);

	prop.put("segment_file", segmentsFilename);
	prop.put("segment_length", to_string(roadSegmentLength));
	prop.put("road_width", to_string(roadWidth));
	prop.put("course_length", to_string(lines.size()));

	if(not presetSceneryName.empty())
		prop.put("preset_landscape_style", presetSceneryName);

	if(not presetRoadStyleName.empty())
		prop.put("preset_road_style", presetRoadStyleName);

	if(not landscapeFilename.empty())
		prop.put("landscape_image", landscapeFilename);
	prop.put("landscape_color", colorLandscape.toRgbString());
	prop.put("horizon_color", colorHorizon.toRgbString());
	prop.put("road_color_primary", colorRoadPrimary.toRgbString());
	prop.put("road_color_secondary", colorRoadSecondary.toRgbString());
	prop.put("offroad_color_primary", colorOffRoadPrimary.toRgbString());
	prop.put("offroad_color_secondary", colorOffRoadSecondary.toRgbString());
	prop.put("humble_color_primary", colorHumblePrimary.toRgbString());
	prop.put("humble_color_secondary", colorHumbleSecondary.toRgbString());

	if(not musicFilename.empty())
		prop.put("music", musicFilename);

	if(spritesFilenames.size() > DEFAULT_SPRITE_COUNT)
		prop.put("prop_max_id", to_string(spritesFilenames.size()-1));

	for(unsigned i = 0; i < spritesFilenames.size(); i++)
	{
		prop.put("prop"+to_string(i)+"_sprite", spritesFilenames[i]);
		prop.put("prop"+to_string(i)+"_blocking", to_string(props[i].blocking));
	}

	prop.store(filename);
}

void Pseudo3DCourse::Spec::saveSegments(const string& filename)
{
	std::ofstream stream(filename.c_str());

	if(not stream.is_open())
		throw std::runtime_error("Course description file could not be saved: \"" + filename + "\"");

	stream << "#segment file created by carse editor\n";

	for(unsigned i = 0; i < lines.size() and stream.good(); i++)
	{
		const Segment& line = lines[i];
		stream << line.x << ',' << line.y << ',' << line.curve << ',' << line.slope;
		if(line.propIndex != -1)
			stream << ',' << line.propIndex << ',' << line.propX;
		stream << endl;
	}

	stream.close();
}
