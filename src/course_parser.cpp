/*
 * course_parser.cpp
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
using futil::starts_with;

using std::to_string;

static const unsigned DEFAULT_SPRITE_COUNT = 32;

namespace  // static
{
	inline Color parseColor(const char* cstr)
	{
		return Color::parseCStr(cstr);
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

	landscapeFilename = futil::trim(prop.get("landscape_image"));

	if(not landscapeFilename.empty() and landscapeFilename != "default")
		landscapeFilename = getContextualizedFilename(landscapeFilename, baseDir, "assets/");

	if(landscapeFilename.empty() or landscapeFilename == "default")
	{
		if(landscapeFilename.empty())
			cout << "warning: image file specified in \"landscape_image\" entry could not be found"
			<< ", specified by \"" << filename << "\". using default instead..." << endl;
		landscapeFilename = "assets/bg.png";
	}

	musicFilename = futil::trim(prop.get("music"));
	if(musicFilename.empty()) musicFilename = "assets/music_sample.ogg";  // todo remove this line

	colorRoadPrimary = prop.getParsedCStrAllowDefault<Color, parseColor>("road_color_primary", Color( 64, 80, 80));
	colorRoadSecondary = prop.getParsedCStrAllowDefault<Color, parseColor>("road_color_secondary", Color( 40, 64, 64));
	colorOffRoadPrimary = prop.getParsedCStrAllowDefault<Color, parseColor>("offroad_color_primary", Color(  0,112,  0));
	colorOffRoadSecondary = prop.getParsedCStrAllowDefault<Color, parseColor>("offroad_color_secondary", Color(  0, 88, 80));
	colorHumblePrimary = prop.getParsedCStrAllowDefault<Color, parseColor>("humble_color_primary", Color(200,200,200));
	colorHumbleSecondary = prop.getParsedCStrAllowDefault<Color, parseColor>("humble_color_secondary", Color(152,  0,  0));
	colorLandscape = prop.getParsedCStrAllowDefault<Color, parseColor>("landscape_color", Color(136,204,238));
	colorHorizon = prop.getParsedCStrAllowDefault<Color, parseColor>("horizon_color", colorOffRoadPrimary);

	int spriteMaxId = prop.getParsedCStrAllowDefault<int, atoi>("sprite_max_id", DEFAULT_SPRITE_COUNT);
	for(int id = 0; id <= spriteMaxId; id++)
	{
		const string specifiedSpriteFilename = prop.get("sprite" + to_string(id));
		if(not specifiedSpriteFilename.empty())
		{
			const string spriteFilename = getContextualizedFilename(specifiedSpriteFilename, baseDir, "assets/");
			if(spriteFilename.empty())
				cout << "warning: could not load sprite for ID #" << id << ": missing file \"" << specifiedSpriteFilename << "\". ID will be treated as unspecified!" << endl;
			spritesFilenames.push_back(spriteFilename);
		}
		else spritesFilenames.push_back(string());
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
			line.spriteID = atoi(tokens[4].c_str());
			line.spriteX = atof(tokens[5].c_str());

			if(line.spriteID != -1 and
			  (line.spriteID + 1 > (int) spritesFilenames.size() or spritesFilenames[line.spriteID].empty()))
				throw std::logic_error("Course indicates usage of an unspecified sprite ID (#" + to_string(line.spriteID) + "), specified by \"" + segmentFilename+"\"");
		}

		if(tokens.size() == 5 or tokens.size() > 6)
			std::cout << "warning: line " << i << " had an unexpected number of parameters (" << tokens.size() << ") - some of them we'll be ignored (specified by \"" << segmentFilename << "\")" << std::endl;
	}

	stream.close();
}

void Pseudo3DCourse::Spec::saveProperties(const string& filename, const string& segmentsFilename)
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
		prop.put("sprite_max_id", to_string(spritesFilenames.size()-1));

	for(unsigned i = 0; i < spritesFilenames.size(); i++)
		prop.put("sprite"+to_string(i), spritesFilenames[i]);

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
		if(line.spriteID != -1)
			stream << ',' << line.spriteID << ',' << line.spriteX;
		stream << endl;
	}

	stream.close();
}
