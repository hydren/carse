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

struct Pseudo3DRaceState;  //fwd_decl

struct Course
{
	struct Segment
	{
		Course* course;

		float x, y, z; // 3d center of line (delta coordinates)
		float X, Y, W; // screen coordinate
		float scale, curve;

		Segment(Course* state);

		// from "world" to screen coordinates
		void project(int camX, int camY, int camZ, float camDepth);
	};

	std::vector<Segment> lines;
	float roadSegmentLength, roadWidth;

	Course(float segmentLength, float roadWidth);

	void updateReferences();

	/** Get a copy of the debug course. */
	static Course createDebugCourse(float segmentLength, float roadWidth);

	/** Creates an random course, with given length and curveness factor. */
	static Course createRandomCourse(float segmentLength, float roadWidth, float length, float curveness);
};

#endif /* PSEUDO3D_COURSE_HPP_ */
