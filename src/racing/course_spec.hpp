/*
 * course.hpp
 *
 *  Created on: 26 de fev de 2018
 *      Author: carlosfaruolo
 */

#ifndef RACING_COURSE_SPEC_HPP_
#define RACING_COURSE_SPEC_HPP_
#include <ciso646>

#include <vector>

struct CourseSpec
{
	struct Segment
	{
		float x, y, z;  // 3d center of line (delta coordinates)
		float curve;  //fixme this "curve" field completely renders the "x" field useless
		float slope;

		//todo add a slope field to control y-variation

		float clip;
		int spriteID;  // the "ID" of the sprite to show. -1 means no sprite.
		float spriteX;   // the position of this segment's sprite

		Segment() : x(0), y(0), z(0), curve(0), slope(0), clip(0), spriteID(-1), spriteX(0) {}
	};

	std::vector<Segment> lines;
	float roadSegmentLength, roadWidth;

	CourseSpec(float segmentLength, float roadWidth)
	: lines(), roadSegmentLength(segmentLength), roadWidth(roadWidth)
	{}
};

#endif /* RACING_COURSE_SPEC_HPP_ */
