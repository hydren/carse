/*
 * course.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "course.hpp"

#include "futil/random.h"
#include "futil/string_actions.hpp"
#include "futil/string_split.hpp"
#include "futil/collection_actions.hpp"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

#include <cstdlib>
#include <ctime>
#include <cmath>

using fgeal::Color;
using fgeal::Display;
using fgeal::Image;
using fgeal::Sprite;
using futil::Properties;
using std::string;
using std::vector;
using futil::random_between_decimal;
using futil::split;
using futil::trim;
using futil::starts_with;

//custom call to draw quad
void drawRoadQuad(const Color& c, float x1, float y1, float w1, float x2, float y2, float w2)
{
	Image::drawFilledQuadrangle(x1-w1, y1, x2-w2, y2, x2+w2, y2, x1+w1, y1, c);
}

// needed to ensure consistency
Course::Segment::Segment() : x(0), y(0), z(0), curve(0), slope(0),
		clip(0), spriteID(-1), spriteX(0)  {}

Course::Course(float segmentLength, float roadWidth)
: lines(), roadSegmentLength(segmentLength), roadWidth(roadWidth), spritesFilenames()
{}

struct ScreenCoordCache
{
	float X, Y, W, scale;
};

void Course::draw(int pos, int posX, const DrawParameters& param)
{
	const unsigned N = lines.size(), fromPos = pos/roadSegmentLength;
	const float camHeight = 1500 + lines[fromPos].y;
	float x = 0, dx = 0;

	float maxY = param.drawAreaHeight;

	// screen coordinates cache
	vector<ScreenCoordCache> lts(N, ScreenCoordCache());

	for(unsigned n = fromPos+1; n < fromPos + param.drawDistance; n++)
	{
		Segment& l = lines[n%N];
		ScreenCoordCache& lt = lts[n%N];

		// project from "world" to "screen" coordinates
		const int camX = posX - x,
				  camY = camHeight,
				  camZ = pos - (n >= N? N*roadSegmentLength : 0);
		const float scale = param.cameraDepth / (l.z - camZ);

		//fixme since l.x is always zero, camX is actually the one which controls the horizontal shift; it should be l.x, much like l.y controls the vertical shift
		lt.X = (1 + scale*(l.x - camX)) * param.drawAreaWidth/2;
		lt.Y = (1 - scale*(l.y - camY)) * param.drawAreaHeight/2;
		lt.W = scale * roadWidth * param.drawAreaWidth/2;
		lt.scale = scale;

		// update curve
		x += dx;
		dx += l.curve;

		l.clip=maxY;
		if(lt.Y > maxY) continue;
		maxY = lt.Y;

		Color grass  = (n/3)%2? Color(  0, 112, 0) : Color(  0, 88,  0);
		Color rumble = (n/3)%2? Color(200,200,200) : Color(152,  0,  0);
		Color road   = (n/3)%2? Color( 64, 80, 80) : Color( 40, 64, 64);

		ScreenCoordCache& p = lts[(n-1)%N];

		drawRoadQuad(grass,  0,  p.Y, param.drawAreaWidth, 0, lt.Y, param.drawAreaWidth);
		drawRoadQuad(rumble, p.X, p.Y, p.W*1.2, lt.X, lt.Y, lt.W*1.2);
		drawRoadQuad(road,   p.X, p.Y, p.W, lt.X, lt.Y, lt.W);
	}

	for(unsigned n = fromPos + param.drawDistance; n >= fromPos+1; n--)
	{
		Segment& l = lines[n%N];
		if(l.spriteID == -1)
			continue;

	    Image& s = *param.sprites[l.spriteID];
	    int w = s.getWidth();
	    int h = s.getHeight();

	    ScreenCoordCache& lt = lts[n%N];

	    float destX = lt.X + lt.scale * l.spriteX * param.drawAreaWidth/2;
	    float destY = lt.Y + 4;
	    float destW  = w * lt.W / 150;
	    float destH  = h * lt.W / 150;

	    destX += destW * l.spriteX; //offsetX
	    destY += destH * (-1);    //offsetY

	    float clipH = destY+destH-l.clip;
	    if (clipH<0) clipH=0;

	    if (clipH>=destH) continue;
	    s.drawScaledRegion(destX, destY, destW/w, destH/h, Image::FLIP_NONE, 0, 0, w, h-h*clipH/destH);
	}
}

Course::operator std::string() const
{
	return not name.empty()? name : not filename.empty()? filename : "<unnamed>";
}

//static
Course Course::createDebugCourse(float segmentLength, float roadWidth)
{
	Course course(segmentLength, roadWidth);
	for(unsigned i = 0; i < 1600; i++) // generating hardcoded course
	{
		Segment line;
		line.z = i*course.roadSegmentLength;
		if(i > 300 && i < 500) line.curve = 0.3;
		if(i > 500 && i < 700) line.curve = -0.3;
		if(i > 900 && i < 1300) line.curve = -2.2;
		if(i > 750) line.y = sin(i/30.0)*1500;
		if(i % 17==0) { line.spriteX=2.0; line.spriteID=0; }
		if(i % 17==1) { line.spriteX=-3.0; line.spriteID=0; }
		course.lines.push_back(line);
	}
	course.spritesFilenames.push_back("assets/bush.png");  // type 0

	return course;
}

//static
Course Course::createRandomCourse(float segmentLength, float roadWidth, float length, float curveness)
{
	srand(time(null));

	Course course(segmentLength, roadWidth);
	float currentCurve = 0;

	const bool range1 = true,//rand()%2,
			   range2 = true,//rand;()%2,
			   range3 = true;//rand;()%2;
	const float range1size = random_between_decimal(0, 1500),
				range2size = random_between_decimal(0, 1500),
				range3size = random_between_decimal(0, 1500);

	// generating random course
	for(unsigned i = 0; i < length; i++)
	{
		Segment line;
		line.z = i*course.roadSegmentLength;

		if(currentCurve == 0)
		{
			if(rand() % 500 == 0)
				currentCurve = random_between_decimal(-5*curveness, 5*curveness);
			else if(rand() % 50 == 0)
				currentCurve = random_between_decimal(-curveness, curveness);
		}

		else if(currentCurve != 0 and rand() % 100 == 0)
			currentCurve = 0;

		line.curve = currentCurve;

		// fixme this should be parametrized, or at least random
		if(i > 750 and i < 1510 and range1) line.y = sin(i/30.0)*range1size;
		if(i > 1510 and i < 2270 and range2) line.y = sin(i/30.0)*range2size;
		if(i > 2270 and i < 3030 and range3) line.y = sin(i/30.0)*range3size;

		if(rand() % 10 == 0)
		{
			line.spriteID = 0;
			line.spriteX = (rand()%2==0? -1 : 1) * random_between_decimal(2.5, 3.0);
		}
		else if(rand() % 100 == 0)
		{
			line.spriteID = 1;
			line.spriteX = (rand()%2==0? -1 : 1) * random_between_decimal(2.0, 2.5);
		}

		course.lines.push_back(line);
	}

	course.spritesFilenames.push_back("assets/bush.png");
	course.spritesFilenames.push_back("assets/tree.png");

	return course;
}

//static
Course Course::createCourseFromFile(const Properties& prop)
{
	float segmentLength = prop.getParsedCStrAllowDefault<double, atof>("segment_length", 200);  // this may become non-customizable
	float roadWidth = prop.getParsedCStrAllowDefault<double, atof>("road_width", 3000);

	Course course(segmentLength, roadWidth);
	course.name = prop.get("name");
	course.author = prop.get("author");
	course.credits = prop.get("credits");
	course.comments = prop.get("comments");

	unsigned spriteIdCount = prop.getParsedCStrAllowDefault<int, atoi>("sprite_max_id", 32);
	for(unsigned id = 0; id < spriteIdCount; id++)
		course.spritesFilenames.push_back(prop.get("sprite" + futil::to_string(id)));

	string segmentFilename = prop.getIfContains("segment_file", "Missing segment file for course!");
	std::ifstream stream(segmentFilename.c_str());
	if(not stream.is_open())
		throw std::runtime_error("File could not be opened: " + segmentFilename);

	float length = prop.getParsedCStrAllowDefault<double, atof>("course_length", 6400);
	for(unsigned i = 0; i < length; i++)
	{
		Segment line;
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

		if(tokens.size() > 1)
			line.y = atof(tokens[1].c_str());

		if(tokens.size() > 2)
			line.curve = atof(tokens[2].c_str());

		if(tokens.size() >= 3)
			line.slope = atof(tokens[3].c_str());

		if(tokens.size() >= 5)
		{
			line.spriteID = atoi(tokens[4].c_str());
			line.spriteX = atof(tokens[5].c_str());

			if(line.spriteID != -1 and
			  (line.spriteID + 1 > (int) course.spritesFilenames.size() or course.spritesFilenames[line.spriteID].empty()))
				throw std::runtime_error("Error: course indicates usage of an unspecified sprite ID. \n " + segmentFilename);
		}
		else if(tokens.size() == 4)
			std::cout << "warning: line " << i << " had an unexpected number of parameters (" << tokens.size() << ") - some of them we'll be ignored." << std::endl;

		course.lines.push_back(line);
	}

	stream.close();
	return course;
}
