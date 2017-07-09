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
using futil::Properties;
using std::string;
using std::vector;
using futil::random_between_decimal;
using futil::split;
using futil::trim;
using futil::starts_with;

//custom call to draw quad
void drawQuad(const Color& c, float x1, float y1, float w1, float x2, float y2, float w2)
{
	fgeal::Image::drawQuadrangle(c, x1-w1, y1, x2-w2, y2, x2+w2, y2, x1+w1, y1);
}

Course::Segment::Segment(Course* course) // @suppress("Class members should be properly initialized")
: course(course) {curve=x=y=z=0;}

void Course::Segment::project(int camX, int camY, int camZ, float camDepth)
{
	fgeal::Display& display = fgeal::Display::getInstance();
	scale = camDepth / (z - camZ);
	X = (1 + scale*(x + camX)) * display.getWidth()/2;
	Y = (1 - scale*(y - camY)) * display.getHeight()/2;
	W = scale * course->roadWidth * display.getWidth()/2;
}

Course::Course(float segmentLength, float roadWidth)
: lines(), roadSegmentLength(segmentLength), roadWidth(roadWidth)
{}

void Course::draw(int pos, int posX, const DrawParameters& param)
{
	const unsigned N = lines.size(), fromPos = pos/roadSegmentLength;
	float camHeight = 1500 + lines[fromPos].y;
	float x = 0, dx = 0;

	const float& cameraDepth = param.cameraDepth;
	const unsigned& drawDistance = param.drawDistance;
	float maxY = param.drawAreaHeight;

	for(unsigned n = fromPos+1; n < fromPos+drawDistance; n++)
	{
		Course::Segment& l = lines[n%N];
		l.project(posX - x, camHeight, pos - (n>N?n*roadSegmentLength:0), cameraDepth);
		x += dx;
		dx += l.curve;

		if(l.Y > maxY) continue;
		maxY = l.Y;

		Color grass  = (n/3)%2? Color(  0, 112, 0) : Color(  0, 88,  0);
		Color rumble = (n/3)%2? Color(200,200,200) : Color(152,  0,  0);
		Color road   = (n/3)%2? Color( 64, 80, 80) : Color( 40, 64, 64);

		Course::Segment& p = lines[(n-1)%N]; // previous line

		drawQuad(grass,  0,   p.Y, param.drawAreaWidth, 0, l.Y, param.drawAreaWidth);
		drawQuad(rumble, p.X, p.Y, p.W*1.2, l.X, l.Y, l.W*1.2);
		drawQuad(road,   p.X, p.Y, p.W, l.X, l.Y, l.W);
	}
}

void Course::updateReferences()
{
	for(unsigned i = 0; i < lines.size(); i++)
		lines[i].course = this;
}

//static
Course Course::createDebugCourse(float segmentLength, float roadWidth)
{
	Course course(segmentLength, roadWidth);
	for(unsigned i = 0; i < 1600; i++) // generating hardcoded course
	{
		Course::Segment line(&course);
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
		Course::Segment line(&course);
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

string charArrayToString(const char* str) { return string(str); }

//static
Course Course::createCourseFromFile(const Properties& prop)
{
	string segmentFilename = prop.getAsValueOrFail<string, charArrayToString>("segment_file", "Missing segment file for course!");
	float segmentLength = prop.getAsValueOrDefault<double, atof>("segment_length", 200);  // this may become non-customizable
	float roadWidth = prop.getAsValueOrDefault<double, atof>("road_width", 3000);
	float length = prop.getAsValueOrDefault<double, atof>("course_length", 6400);

	std::ifstream stream(segmentFilename.c_str());
	if(not stream.is_open())
		throw std::runtime_error("File could not be opened: " + segmentFilename);

	Course course(segmentLength, roadWidth);
	for(unsigned i = 0; i < length; i++)
	{
		Course::Segment line(&course);
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
