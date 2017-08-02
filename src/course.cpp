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

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

#include <cstdlib>
#include <ctime>
#include <cmath>

using fgeal::Color;
using fgeal::Display;
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
	fgeal::Image::drawQuadrangle(c, x1-w1, y1, x2-w2, y2, x2+w2, y2, x1+w1, y1);
}

// needed to ensure consistency
Course::Segment::Segment() : x(0), y(0), z(0), curve(0) {}

Course::Course(float segmentLength, float roadWidth)
: lines(), roadSegmentLength(segmentLength), roadWidth(roadWidth)
{}

void Course::draw(int pos, int posX, const DrawParameters& param)
{
	Display& display = Display::getInstance();
	const unsigned N = lines.size(), fromPos = pos/roadSegmentLength;
	float camHeight = 1500 + lines[fromPos].y;
	float x = 0, dx = 0;

	const float& cameraDepth = param.cameraDepth;
	const unsigned& drawDistance = param.drawDistance;
	float maxY = param.drawAreaHeight;

	// screen coordinates
	float lX = 0, lY = 0, lW = 0;  // current segment
	float pX = 0, pY = 0, pW = 0;  // previous segment

	for(unsigned n = fromPos+1; n < fromPos+drawDistance; n++)
	{
		Segment& l = lines[n%N];

		// shift current values to previous before fetching new coordinates
		pX = lX; pY = lY; pW = lW;

		// project from "world" to "screen" coordinates
		const int camX = posX - x,
				  camY = camHeight,
				  camZ = pos - (n>=N?n*roadSegmentLength:0);
		const float camDepth = cameraDepth,
					scale = camDepth / (l.z - camZ);

		//fixme since l.x is always zero, camX is actually the one which controls the horizontal shift; it should be l.x, much like l.y controls the vertical shift
		lX = (1 + scale*(l.x - camX)) * display.getWidth()/2;
		lY = (1 - scale*(l.y - camY)) * display.getHeight()/2;
		lW = scale * roadWidth * display.getWidth()/2;

		// update curve
		x += dx;
		dx += l.curve;

		if(lY > maxY) continue;
		maxY = lY;

		Color grass  = (n/3)%2? Color(  0, 112, 0) : Color(  0, 88,  0);
		Color rumble = (n/3)%2? Color(200,200,200) : Color(152,  0,  0);
		Color road   = (n/3)%2? Color( 64, 80, 80) : Color( 40, 64, 64);

		drawRoadQuad(grass,  0,   pY, param.drawAreaWidth, 0, lY, param.drawAreaWidth);
		drawRoadQuad(rumble, pX, pY, pW*1.2, lX, lY, lW*1.2);
		drawRoadQuad(road,   pX, pY, pW, lX, lY, lW);
	}
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
		course.lines.push_back(line);
	}
	return course;
}

//static
Course Course::createRandomCourse(float segmentLength, float roadWidth, float length, float curveness)
{
	srand(time(null));

	Course course(segmentLength, roadWidth);
	float currentCurve = 0;

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
		if(i > 750 and i < 1350) line.y = sin(i/30.0)*1500;
		course.lines.push_back(line);
	}

	return course;
}

//static
Course Course::createCourseFromFile(const Properties& prop)
{
	string segmentFilename = prop.getIfContains("segment_file", "Missing segment file for course!");
	float segmentLength = prop.getParsedCStrAllowDefault<double, atof>("segment_length", 200);  // this may become non-customizable
	float roadWidth = prop.getParsedCStrAllowDefault<double, atof>("road_width", 3000);
	float length = prop.getParsedCStrAllowDefault<double, atof>("course_length", 6400);

	std::ifstream stream(segmentFilename.c_str());
	if(not stream.is_open())
		throw std::runtime_error("File could not be opened: " + segmentFilename);

	Course course(segmentLength, roadWidth);
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
		if(tokens.size() < 2)
		{
			std::cout << "ill formed segment file line " << i << std::endl;
			line.curve = line.y = 0;
		}
		else
		{
			line.curve = atof(tokens[0].c_str());
			line.y = atof(tokens[1].c_str());
		}

		course.lines.push_back(line);
	}

	stream.close();

	return course;
}
