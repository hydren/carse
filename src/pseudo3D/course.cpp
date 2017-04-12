/*
 * course.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "course.hpp"

#include "fgeal/fgeal.hpp"

#include "futil/math/more_random.h"

#include <cstdlib>
#include <ctime>
#include <cmath>

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
				currentCurve = random_decimal_between(-5*curveness, 5*curveness);
			else if(rand() % 50 == 0)
				currentCurve = random_decimal_between(-curveness, curveness);
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
