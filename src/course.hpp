/*
 * course.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_COURSE_HPP_
#define PSEUDO3D_COURSE_HPP_
#include <ciso646>

#include "racing/course_spec.hpp"

#include "fgeal/fgeal.hpp"

struct Pseudo3DCourse
{
	struct Spec extends CourseSpec
	{
		std::string name, filename, author, credits, comments;

		std::string landscapeFilename;
		fgeal::Color colorLandscape, colorHorizon,
					 colorRoadPrimary, colorRoadSecondary,
					 colorOffRoadPrimary, colorOffRoadSecondary,
					 colorHumblePrimary, colorHumbleSecondary;

		std::vector<std::string> spritesFilenames;

		std::string musicFilename;

		Spec(float segmentLength, float roadWidth)
		: CourseSpec(segmentLength, roadWidth) {}

		inline std::string toString() const { return not name.empty()? name : not filename.empty()? filename : "<unnamed>"; }
		inline operator std::string() const { return this->toString(); }
	};

	Spec spec;
	std::vector<fgeal::Image*> sprites;

	int drawAreaWidth, drawAreaHeight;
	unsigned drawDistance;
	float cameraDepth;

	Pseudo3DCourse();
	Pseudo3DCourse(Spec spec);

	void draw(int positionZ, int positionX);

	void drawMap(const fgeal::Rectangle& bounds, fgeal::Color color);

	void clearDynamicData();

	void setupDynamicData();

	/* Generates a debug course spec. */
	static Spec generateDebugCourseSpec(float segmentLength, float roadWidth);

	/* Generates a random course spec, with given length and curveness factor. */
	static Spec generateRandomCourseSpec(float segmentLength, float roadWidth, float length, float curveness);

	/* Reads a course spec. from the given filename. */
	static Spec parseCourseSpecFromFile(const std::string& filename);
};

#endif /* PSEUDO3D_COURSE_HPP_ */
