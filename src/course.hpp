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

#include "vehicle.hpp"

#include <vector>

struct Pseudo3DCourse
{
	struct Spec extends CourseSpec
	{
		std::string filename, segmentFilename;

		std::string name, author, credits, comments;
		std::string previewFilename;

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

		/* Loads data from the given filename, parse its course spec data and store in this object. */
		void loadFromFile(const std::string& filename);

		/* Saves this course spec. to the given filename. */
		void saveToFile(const std::string& filename);

		/* Creates a course spec. by loading and parsing the data in the given filename. */
		inline static Spec createFromFile(const std::string& filename) { Spec spec(0, 0); spec.loadFromFile(filename); return spec; }

		/* Generates a debug course spec. */
		static Spec generateDebugCourseSpec(float segmentLength, float roadWidth);

		/* Generates a random course spec, with given length and curveness factor. */
		static Spec generateRandomCourseSpec(float segmentLength, float roadWidth, float length, float curveness);

		private:
		void parseProperties(const std::string& filename);
		void loadSegments(const std::string& filename);
		void saveProperties(const std::string& specFile, const std::string& segmentsFile);
		void saveSegments(const std::string& filename);
	};

	struct Map
	{
		Spec spec;
		fgeal::Rectangle bounds;
		fgeal::Vector2D offset, scale;
		fgeal::Color roadColor, segmentHighlightColor;
		float segmentHighlightSize;
		bool roadContrastColorEnabled, geometryOtimizationEnabled;

		Map();
		Map(const Spec& spec);

		void compile();

		void drawMap(unsigned highlightedSegment=0);

		private:
		std::vector<fgeal::Point> cache;
		std::vector<float> cacheLenght;
	};

	Spec spec;
	std::vector<fgeal::Image*> sprites;

	int drawAreaWidth, drawAreaHeight;
	unsigned drawDistance;
	float cameraDepth;

	std::vector<Pseudo3DVehicle>* trafficVehicles;

	Pseudo3DCourse();

	void loadSpec(Spec);

	void draw(int positionZ, int positionX);
};

#endif /* PSEUDO3D_COURSE_HPP_ */
