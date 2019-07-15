/*
 * course.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_COURSE_HPP_
#define PSEUDO3D_COURSE_HPP_
#include <ciso646>

#include "vehicle.hpp"

#include "racing/course_spec.hpp"

#include "fgeal/fgeal.hpp"

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
		void loadFromFile(const std::string& filename);

		/* Saves this course spec. to the given filename. */
		void saveToFile(const std::string& filename);

		/* Creates a course spec. by loading and parsing the data in the given filename. */
		inline static Spec createFromFile(const std::string& filename) { Spec spec(0, 0); spec.loadFromFile(filename); return spec; }

		/* Generates a debug course spec. */
		static Spec createDebug();

		/* Generates a random course spec using the given parameters. Note: course length will be equal to (segment lentgh x segment count) */
		static Spec createRandom(float segmentLength, float roadWidth, unsigned segmentCount, float curveness);

		struct RoadStyle {
			fgeal::Color roadPrimary, roadSecondary, humblePrimary, humbleSecondary;
			std::string name;
			void loadFromFile(const std::string& filename, const std::string& name=std::string());
			static const RoadStyle DEFAULT;
		};

		struct LandscapeStyle {
			fgeal::Color terrarinPrimary, terrainSecondary, landscape;
			std::string landscapeBackgroundFilename, sprite1, sprite2, sprite3, name;  // todo create blocking flag
			void loadFromFile(const std::string& filename, const std::string& name=std::string());
			static const LandscapeStyle DEFAULT;
		};

		inline void assignStyle(const RoadStyle& style)
		{
			presetRoadStyleName = style.name;
			colorRoadPrimary = style.roadPrimary;
			colorRoadSecondary = style.roadSecondary;
			colorHumblePrimary = style.humblePrimary;
			colorHumbleSecondary = style.humbleSecondary;
		}

		inline void assignStyle(const LandscapeStyle& style)
		{
			presetLandscapeStyleName = style.name;
			colorOffRoadPrimary = style.terrarinPrimary;
			colorOffRoadSecondary = style.terrainSecondary;
			colorLandscape = style.landscape;
			colorHorizon = style.terrarinPrimary;
			landscapeFilename = style.landscapeBackgroundFilename;
			if(spritesFilenames.size() < 3) spritesFilenames.resize(3);
			spritesFilenames[0] = style.sprite1;
			spritesFilenames[1] = style.sprite2;
			spritesFilenames[2] = style.sprite3;
			if(props.size() < 3) props.resize(3);  // todo use blocking flag from preset (when ready)
			props[0].blocking = false;
			props[1].blocking = props[2].blocking = true;
		}

		private:
		void parseProperties(const std::string& filename);
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

	/** A list of vehicles (pointers) to be displayed */
	std::vector<const Pseudo3DVehicle*> vehicles;

	/** Scale factor of the position of vehicles. */
	float lengthScale;

	Pseudo3DCourse();
	~Pseudo3DCourse();

	void loadSpec(const Spec&);

	void draw(int positionZ, int positionX);
};

#endif /* PSEUDO3D_COURSE_HPP_ */
