/*
 * course.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "course.hpp"

#include "fgeal/fgeal.hpp"

Course::Segment::Segment(Course& course) // @suppress("Class members should be properly initialized")
: course(course) {curve=x=y=z=0;}

Course::Segment& Course::Segment::operator= (const Segment& s)
{
	if(this != &s)
	{
		delete this;
		new (this) Segment(course);
		x = s.x; y = s.y; z = s.z;
		X = s.X; Y = s.Y; W = s.W;
		scale = s.scale;
	}
	return *this;
}

void Course::Segment::project(int camX, int camY, int camZ)
{
	fgeal::Display& display = fgeal::Display::getInstance();
	scale = course.cameraDepth / (z - camZ);
	X = (1 + scale*(x + camX)) * display.getWidth()/2;
	Y = (1 - scale*(y - camY)) * display.getHeight()/2;
	W = scale * course.roadWidth * display.getWidth()/2;
}

Course::Course(RaceState& state) // xxx @suppress("Class members should be properly initialized")
: state(state)
{}
