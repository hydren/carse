/*
 * course_spec_parser.cpp
 *
 *  Created on: 26 de fev de 2018
 *      Author: carlosfaruolo
 */

#include "course.hpp"

#include "util.hpp"

#include "carse_game.hpp"

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
using std::to_string;
using fgeal::Color;
using fgeal::Display;
using fgeal::Image;
using futil::Properties;
using futil::split;
using futil::trim;
using futil::to_lower;
using futil::starts_with;


static const unsigned DEFAULT_SPRITE_COUNT = 32;

namespace  // static
{
	inline Color parseColor(const string& str)
	{
		return Color::parseCStr(str.c_str());
	}
}

// -------------------------------------------------------------------------------------------------------------------------------------
void Pseudo3DCourse::Spec::RoadColorSet::loadFromFile(const string& filename, const string& presetName)
{
	Properties prop;
	prop.load(filename);
	name = presetName;
	primary = prop.getParsedAllowDefault<Color, parseColor>("road_color_primary", DEFAULT.primary);
	secondary = prop.getParsedAllowDefault<Color, parseColor>("road_color_secondary", DEFAULT.secondary);
	humblePrimary = prop.getParsedAllowDefault<Color, parseColor>("humble_color_primary", DEFAULT.humblePrimary);
	humbleSecondary = prop.getParsedAllowDefault<Color, parseColor>("humble_color_secondary", DEFAULT.humbleSecondary);
}

void Pseudo3DCourse::Spec::LandscapeSettings::loadFromFile(const string& filename, const string& presetName)
{
	const string baseDir = filename.substr(0, filename.find_last_of("/\\")+1);
	Properties prop;
	prop.load(filename);

	name = presetName;
	terrainPrimary = prop.getParsedAllowDefault<Color, parseColor>("offroad_color_primary", DEFAULT.terrainPrimary);
	terrainSecondary = prop.getParsedAllowDefault<Color, parseColor>("offroad_color_secondary", DEFAULT.terrainSecondary);
	sky = prop.getParsedAllowDefault<Color, parseColor>("landscape_color", DEFAULT.sky);

	landscapeBgFilename = getContextualizedFilename(prop.get("landscape_image"), baseDir, "assets/");
	if(landscapeBgFilename.empty())
	{
		cout << "warning: image file specified in \"landscape_image\" entry could not be found" << ", specified by \"" << filename << "\". using default instead..." << endl;
		landscapeBgFilename = DEFAULT.landscapeBgFilename;
	}

	// TODO rewrite this code once presets have support for any number of props
	sprite1 = getContextualizedFilename(prop.get("prop0_sprite"), baseDir, "assets/");
	sprite2 = getContextualizedFilename(prop.get("prop1_sprite"), baseDir, "assets/");
	sprite3 = getContextualizedFilename(prop.get("prop2_sprite"), baseDir, "assets/");

	if(not presetName.empty())  // only care about sprite field correctness if loading an actual preset
	{
		if(sprite1.empty())
		{
			cout << "warning: image file specified in \"propX_sprite\" entry could not be found" << ", specified by \"" << filename << "\". using default instead..." << endl;
			sprite1 = DEFAULT.sprite1;
		}
		if(sprite2.empty())
		{
			cout << "warning: image file specified in \"propX_sprite\" entry could not be found" << ", specified by \"" << filename << "\". using default instead..." << endl;
			sprite2 = DEFAULT.sprite2;
		}
		if(sprite3.empty())
		{
			cout << "warning: image file specified in \"propX_sprite\" entry could not be found" << ", specified by \"" << filename << "\". using default instead..." << endl;
			sprite3 = DEFAULT.sprite3;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------------------------
void Pseudo3DCourse::Spec::parseProperties(const string& filename, const CarseGameLogicInstance& logic)
{
	const string baseDir = filename.substr(0, filename.find_last_of("/\\")+1);
	string key;

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

	musicFilename = prop.get("music");
	if(musicFilename.empty()) musicFilename = "assets/music_sample.ogg";  // todo remove this line

	key = "preset_road_style";
	if(prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "custom")
	{
		// attempts to set a preset style and, if not found or unspecified, set the default
		assignStyle(logic.instance.getPresetRoadStyle(prop.get(key)));
		presetRoadStyleName = prop.get(key);
	}
	else  // reads style from course spec
	{
		RoadColorSet customRoadStyle;
		customRoadStyle.loadFromFile(filename);
		assignStyle(customRoadStyle);
	}

	key = "preset_landscape_style";
	if(prop.containsKey(key) and not prop.get(key).empty() and prop.get(key) != "custom")
	{
		// attempts to set a preset style and, if not found or unspecified, set the default
		assignStyle(logic.instance.getPresetLandscapeStyle(prop.get(key)));
		presetLandscapeStyleName = prop.get(key);
	}
	else  // reads style from course spec
	{
		LandscapeSettings customLandscapeStyle;
		customLandscapeStyle.loadFromFile(filename);
		assignStyle(customLandscapeStyle);

		// clear these so we can build them using custom specs (landscape preset loader doesn't support prop count other than 3)
		props.clear();
		spritesFilenames.clear();

		// read custom prop definitions
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
	}

	colorHorizon = prop.getParsedAllowDefault<Color, parseColor>("horizon_color", colorOffRoadPrimary);

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

	if(not author.empty())
		prop.put("author", author);

	if(not credits.empty())
		prop.put("credits", credits);

	if(not comments.empty())
		prop.put("comments", comments);

	prop.put("segment_file", segmentsFilename);
	prop.put("segment_length", to_string(roadSegmentLength));
	prop.put("road_width", to_string(roadWidth));
	prop.put("course_length", to_string(lines.size()));

	if(not musicFilename.empty())
		prop.put("music", musicFilename);

	if(not presetRoadStyleName.empty())
		prop.put("preset_road_style", presetRoadStyleName);
	else
	{
		prop.put("road_color_primary", colorRoadPrimary.toRgbString());
		prop.put("road_color_secondary", colorRoadSecondary.toRgbString());
		prop.put("humble_color_primary", colorHumblePrimary.toRgbString());
		prop.put("humble_color_secondary", colorHumbleSecondary.toRgbString());
	}

	if(not presetLandscapeStyleName.empty())
		prop.put("preset_landscape_style", presetLandscapeStyleName);
	else
	{
		prop.put("offroad_color_primary", colorOffRoadPrimary.toRgbString());
		prop.put("offroad_color_secondary", colorOffRoadSecondary.toRgbString());
		prop.put("landscape_color", colorLandscape.toRgbString());
		prop.put("horizon_color", colorHorizon.toRgbString());

		if(not landscapeFilename.empty())
			prop.put("landscape_image", landscapeFilename);

		for(unsigned i = 0; i < spritesFilenames.size(); i++)
		{
			prop.put("prop"+to_string(i)+"_sprite", spritesFilenames[i]);
			prop.put("prop"+to_string(i)+"_blocking", to_string(props[i].blocking));
		}

		if(spritesFilenames.size() > DEFAULT_SPRITE_COUNT)
			prop.put("prop_max_id", to_string(spritesFilenames.size()-1));
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
