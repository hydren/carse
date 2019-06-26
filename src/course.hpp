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

		std::string presetLandscapeStyleName, presetRoadStyleName;

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
		void loadFromFile(const std::string& filename, const CarseGameLogicInstance&);

		/* Saves this course spec. to the given filename. */
		void saveToFile(const std::string& filename);

		/* Creates a course spec. by loading and parsing the data in the given filename. */
		inline static Spec createFromFile(const std::string& filename, const CarseGameLogicInstance& logic) { Spec spec(0, 0); spec.loadFromFile(filename, logic); return spec; }

		/* Generates a debug course spec. */
		static Spec generateDebugCourseSpec(float segmentLength, float roadWidth);

		/* Generates a random course spec, with given length and curveness factor. */
		static Spec generateRandomCourseSpec(float segmentLength, float roadWidth, float length, float curveness);

		struct RoadColorSet {
			fgeal::Color primary, secondary, humblePrimary, humbleSecondary;
			std::string name;
			void loadFromFile(const std::string& filename, const std::string& name=std::string());
			static const RoadColorSet DEFAULT;
		};

		struct LandscapeSettings {
			fgeal::Color terrainPrimary, terrainSecondary, sky;
			std::string landscapeBgFilename, sprite1, sprite2, sprite3, name;  // todo create blocking flag
			void loadFromFile(const std::string& filename, const std::string& name=std::string());
			static const LandscapeSettings DEFAULT;
		};

		inline void assignStyle(const RoadColorSet& style)
		{
			presetRoadStyleName = style.name;
			colorRoadPrimary = style.primary;
			colorRoadSecondary = style.secondary;
			colorHumblePrimary = style.humblePrimary;
			colorHumbleSecondary = style.humbleSecondary;
		}

		inline void assignStyle(const LandscapeSettings& style)
		{
			presetLandscapeStyleName = style.name;
			colorOffRoadPrimary = style.terrainPrimary;
			colorOffRoadSecondary = style.terrainSecondary;
			colorLandscape = style.sky;
			colorHorizon = style.terrainPrimary;
			landscapeFilename = style.landscapeBgFilename;
			if(spritesFilenames.size() < 3) spritesFilenames.resize(3);
			spritesFilenames[0] = style.sprite1;
			spritesFilenames[1] = style.sprite2;
			spritesFilenames[2] = style.sprite3;
			if(props.size() < 3) props.resize(3);  // todo use blocking flag from preset (when ready)
			props[0].blocking = false;
			props[1].blocking = props[2].blocking = true;
		}

		private:
		void parseProperties(const std::string& filename, const CarseGameLogicInstance&);
		void loadSegments(const std::string& filename);
		void storeProperties(const std::string& specFile, const std::string& segmentsFile);
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

	std::vector<const Pseudo3DVehicle*> vehicles;
	float lengthScale;

	Pseudo3DCourse();
	~Pseudo3DCourse();

	void loadSpec(const Spec&);

	void draw(int positionZ, int positionX);
};

#endif /* PSEUDO3D_COURSE_HPP_ */
