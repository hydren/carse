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
#include "fgeal/extra/sprite.hpp"
#include "futil/properties.hpp"

#include <vector>

struct Course
{
	struct Segment
	{
		float x, y, z;  // 3d center of line (delta coordinates)
		float curve;  //fixme this "curve" field completely renders the "x" field useless

		//todo add a slope field to control y-variation

		float spriteX, clip;
		int spriteType;

		Segment();
	};

	std::vector<Segment> lines;
	float roadSegmentLength, roadWidth;

	std::string name, filename, author, credits, comments;

	std::vector<std::string> spritesFilenames;

	Course(float segmentLength, float roadWidth);

	struct DrawParameters
	{
		int drawAreaWidth, drawAreaHeight;
		unsigned drawDistance;
		float cameraDepth;

		std::vector<fgeal::Image*> sprites;
	};

	void draw(int positionZ, int positionX, const DrawParameters& param);

	operator std::string() const;

	/** Creates a debug course. */
	static Course createDebugCourse(float segmentLength, float roadWidth);

	/** Creates a random course, with given length and curveness factor. */
	static Course createRandomCourse(float segmentLength, float roadWidth, float length, float curveness);

	/** Creates a course from the given file. */
	static Course createCourseFromFile(const futil::Properties& properties);
};

#endif /* PSEUDO3D_COURSE_HPP_ */
