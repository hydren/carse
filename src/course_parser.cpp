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

namespace // static
{
	inline Color parseColor(const char* cstr)
	{
		return Color::parseCStr(cstr);
	}

	inline string to_string(Color c)
	{
		return ::to_string(c.r)+","+::to_string(c.g)+","+::to_string(c.b);
	}
}

//static
Pseudo3DCourse::Spec Pseudo3DCourse::parseCourseSpecFromFile(const string& filename)
{
	Properties prop;
	prop.load(filename);
	prop.put("filename", filename);  // done so we can later get properties filename
	prop.put("base_dir", filename.substr(0, filename.find_last_of("/\\")+1));  // done so we can later get properties base dir

	float segmentLength = prop.getParsedCStrAllowDefault<double, atof>("segment_length", 200);  // this may become non-customizable
	float roadWidth = prop.getParsedCStrAllowDefault<double, atof>("road_width", 3000);

	Pseudo3DCourse::Spec course(segmentLength, roadWidth);
	course.name = prop.get("name");
	course.author = prop.get("author");
	course.credits = prop.get("credits");
	course.comments = prop.get("comments");
	course.filename = prop.get("filename");  // property provided by course loader

	course.landscapeFilename = futil::trim(prop.get("landscape_image"));

	if(not course.landscapeFilename.empty() and course.landscapeFilename != "default")
		course.landscapeFilename = getContextualizedFilename(course.landscapeFilename, prop.get("base_dir"), "assets/");

	if(course.landscapeFilename.empty() or course.landscapeFilename == "default")
	{
		if(course.landscapeFilename.empty())
			cout << "warning: image file specified in \"landscape_image\" entry could not be found"
			<< ", specified by \"" << course.filename << "\". using default instead..." << endl;
		course.landscapeFilename = "assets/bg.png";
	}

	course.musicFilename = futil::trim(prop.get("music"));
	if(course.musicFilename.empty()) course.musicFilename = "assets/music_sample.ogg";  // todo remove this line

	course.colorRoadPrimary = prop.getParsedCStrAllowDefault<Color, parseColor>("road_color_primary", Color( 64, 80, 80));
	course.colorRoadSecondary = prop.getParsedCStrAllowDefault<Color, parseColor>("road_color_secondary", Color( 40, 64, 64));
	course.colorOffRoadPrimary = prop.getParsedCStrAllowDefault<Color, parseColor>("offroad_color_primary", Color(  0,112,  0));
	course.colorOffRoadSecondary = prop.getParsedCStrAllowDefault<Color, parseColor>("offroad_color_secondary", Color(  0, 88, 80));
	course.colorHumblePrimary = prop.getParsedCStrAllowDefault<Color, parseColor>("humble_color_primary", Color(200,200,200));
	course.colorHumbleSecondary = prop.getParsedCStrAllowDefault<Color, parseColor>("humble_color_secondary", Color(152,  0,  0));
	course.colorLandscape = prop.getParsedCStrAllowDefault<Color, parseColor>("landscape_color", Color(136,204,238));
	course.colorHorizon = prop.getParsedCStrAllowDefault<Color, parseColor>("horizon_color", course.colorOffRoadPrimary);

	unsigned spriteIdCount = prop.getParsedCStrAllowDefault<int, atoi>("sprite_max_id", DEFAULT_SPRITE_COUNT);
	for(unsigned id = 0; id < spriteIdCount; id++)
	{
		const string specifiedSpriteFilename = prop.get("sprite" + to_string(id));
		if(not specifiedSpriteFilename.empty())
		{
			const string spriteFilename = getContextualizedFilename(specifiedSpriteFilename, prop.get("base_dir"), "assets/");
			if(spriteFilename.empty())
				cout << "warning: could not load sprite for ID #" << id << ": missing file \"" << specifiedSpriteFilename << "\". ID will be treated as unspecified!" << endl;
			course.spritesFilenames.push_back(spriteFilename);
		}
		else course.spritesFilenames.push_back(string());
	}

	const string specifiedSegmentFilename = prop.getIfContains("segment_file", "Missing segment file for course!");
	const string segmentFilename = getContextualizedFilename(specifiedSegmentFilename, prop.get("base_dir"));

	std::ifstream stream(segmentFilename.c_str());
	if(not stream.is_open())
		throw std::runtime_error("Course description file could not be opened: \"" + specifiedSegmentFilename + "\", specified by \"" + course.filename + "\"");

	float length = prop.getParsedCStrAllowDefault<double, atof>("course_length", 6400);
	for(unsigned i = 0; i < length; i++)
	{
		CourseSpec::Segment line;
		line.z = i*course.roadSegmentLength;

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
			  (line.spriteID + 1 > (int) course.spritesFilenames.size() or course.spritesFilenames[line.spriteID].empty()))
				throw std::logic_error("Course indicates usage of an unspecified sprite ID (#" + to_string(line.spriteID) + "), specified by \"" + course.filename);
		}

		if(tokens.size() == 5 or tokens.size() > 6)
			std::cout << "warning: line " << i << " had an unexpected number of parameters (" << tokens.size() << ") - some of them we'll be ignored." << std::endl;

		course.lines.push_back(line);
	}

	stream.close();
	return course;
}

void Pseudo3DCourse::Spec::saveToFile(const string& filename)
{
	const string specFilename = filename + ".properties", segmentsFilename = filename + ".csv";
	this->saveProperties(specFilename, segmentsFilename);
	this->saveSegments(segmentsFilename);
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
	prop.put("landscape_color", to_string(colorLandscape));
	prop.put("horizon_color", to_string(colorHorizon));
	prop.put("road_color_primary", to_string(colorRoadPrimary));
	prop.put("road_color_secondary", to_string(colorRoadSecondary));
	prop.put("offroad_color_primary", to_string(colorOffRoadPrimary));
	prop.put("offroad_color_secondary", to_string(colorOffRoadSecondary));
	prop.put("humble_color_primary", to_string(colorHumblePrimary));
	prop.put("humble_color_secondary", to_string(colorHumbleSecondary));

	if(not musicFilename.empty())
		prop.put("music", musicFilename);

	if(spritesFilenames.size() < DEFAULT_SPRITE_COUNT)
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
