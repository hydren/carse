/*
 * course.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_COURSE_HPP_
#define PSEUDO3D_COURSE_HPP_
#include <ciso646>

#include "fgeal/fgeal.hpp"
#include "futil/properties.hpp"

#include <vector>

struct Course
{
	struct Segment
	{
		float x, y, z; // 3d center of line (delta coordinates)
		float curve;

		Segment();
	};

	std::vector<Segment> lines;
	float roadSegmentLength, roadWidth;

	Course(float segmentLength, float roadWidth);

	struct DrawParameters
	{
		int drawAreaWidth, drawAreaHeight;
		unsigned drawDistance;
		float cameraDepth;
	};

	void draw(int positionZ, int positionX, const DrawParameters& param);

	/** Creates a debug course. */
	static Course createDebugCourse(float segmentLength, float roadWidth);

	/** Creates a random course, with given length and curveness factor. */
	static Course createRandomCourse(float segmentLength, float roadWidth, float length, float curveness);

	/** Creates a course from the given file. */
	static Course createCourseFromFile(const futil::Properties& properties);
};

#endif /* PSEUDO3D_COURSE_HPP_ */
