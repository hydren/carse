/*
 * course.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_COURSE_HPP_
#define PSEUDO3D_COURSE_HPP_
#include <ciso646>

#include <vector>

struct RaceState;  //fwd_decl

struct Course
{
	struct Segment
	{
		Course& course;

		float x, y, z; // 3d center of line (delta coordinates)
		float X, Y, W; // screen coordinate
		float scale, curve;

		Segment(Course& state);

		Segment& operator= (const Segment& s);

		// from "world" to screen coordinates
		void project(int camX, int camY, int camZ);
	};

	RaceState& state;

	float roadSegmentLength, roadWidth;
	float cameraDepth;

	std::vector<Segment> lines;

	Course(RaceState& state);
};

#endif /* PSEUDO3D_COURSE_HPP_ */
