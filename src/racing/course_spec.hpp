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

/** a object that describes a course physically and logically (but not graphically) */
struct CourseSpec
{
	/** an object that describes a prop to be used on the course; note that a single prop can be used more than once. */
	struct Prop
	{
		bool blocking;  // indicates whether this prop blocks vehicles when colliding

		Prop(bool blocking=false) : blocking(blocking) {}
	};

	/** an object that describes a single road segment (its position, props, ...) */
	struct Segment
	{
		float x, y, z;  // 3d center of line (delta coordinates)
		float curve;  //fixme this "curve" field completely renders the "x" field useless
		float slope;

		//todo add a slope field to control y-variation

		int propIndex;  // the index of a registered prop. -1 means no prop at all.
		float propX;   // the position of this segment's prop

		Segment() : x(0), y(0), z(0), curve(0), slope(0), propIndex(-1), propX(0) {}
	};

	/** the list of segments of this course */
	std::vector<Segment> lines;

	/** the length and width of each road segment */
	float roadSegmentLength, roadWidth;

	/** the list of props that may be used on this course */
	std::vector<Prop> props;

	CourseSpec(float segmentLength, float roadWidth)
	: lines(), roadSegmentLength(segmentLength), roadWidth(roadWidth), props()
	{}
};

#endif /* RACING_COURSE_SPEC_HPP_ */
