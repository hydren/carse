/*
 * course.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "course.hpp"

#include "futil/random.h"

#include "psimpl/psimpl.h"

#include <cstdlib>
#include <cmath>
#include <iterator>

using std::string;
using std::vector;
using fgeal::Image;
using fgeal::Graphics;
using fgeal::Color;
using fgeal::Vector2D;
using fgeal::Point;
using fgeal::Rectangle;
using futil::random_between_decimal;

Pseudo3DCourse::Pseudo3DCourse()
: spec(100, 1000), sprites(),
  drawAreaWidth(), drawAreaHeight(), drawDistance(1), cameraDepth(100),
  trafficVehicles(null)
{}

Pseudo3DCourse::Pseudo3DCourse(Spec spec)
: spec(spec), sprites(),
  drawAreaWidth(), drawAreaHeight(), drawDistance(1), cameraDepth(100),
  trafficVehicles(null)
{}

//custom call to draw quad
inline static void drawRoadQuad(const Color& c, float x1, float y1, float w1, float x2, float y2, float w2)
{
	fgeal::Graphics::drawFilledQuadrangle(x1-w1, y1, x2-w2, y2, x2+w2, y2, x1+w1, y1, c);
}

inline static void rotatePoint(Point& p, const Point& center, float angle)
{
	const float s = sin(angle),
				c = cos(angle);

	// translate point back to origin:
	p.x -= center.x;
	p.y -= center.y;

	// rotate point
	float xnew = p.x * c - p.y * s;
	float ynew = p.x * s + p.y * c;

	// translate point back:
	p.x = xnew + center.x;
	p.y = ynew + center.y;
}

namespace // static
{
	struct ScreenCoordCache
	{
		float X, Y, W, scale;
	};
}

void Pseudo3DCourse::draw(int pos, int posX)
{
	if(spec.lines.empty() or spec.roadSegmentLength == 0)
		return;

	const unsigned N = spec.lines.size(), fromPos = pos/spec.roadSegmentLength;
	const float camHeight = 1500 + spec.lines[fromPos].y;
	float x = 0, dx = 0;

	float maxY = drawAreaHeight;

	// screen coordinates cache
	vector<ScreenCoordCache> lts(N, ScreenCoordCache());

	for(unsigned n = fromPos+1; n < fromPos + drawDistance; n++)
	{
		CourseSpec::Segment& l = spec.lines[n%N];
		ScreenCoordCache& lt = lts[n%N];

		// project from "world" to "screen" coordinates
		const int camX = posX - x,
				  camY = camHeight,
				  camZ = pos - (n >= N? N*spec.roadSegmentLength : 0);
		const float scale = cameraDepth / (l.z - camZ);

		//fixme since l.x is always zero, camX is actually the one which controls the horizontal shift; it should be l.x, much like l.y controls the vertical shift
		lt.X = (1 + scale*(l.x - camX)) * drawAreaWidth/2;
		lt.Y = (1 - scale*(l.y - camY)) * drawAreaHeight/2;
		lt.W = scale * spec.roadWidth * drawAreaWidth/2;
		lt.scale = scale;

		// update curve
		x += dx;
		dx += l.curve;

		l.clip=maxY;

		if(lt.Y > maxY)
			continue;

		maxY = lt.Y;

		const Color grass  = (n/3)%2? spec.colorOffRoadPrimary : spec.colorOffRoadSecondary,
					rumble = (n/3)%2? spec.colorHumblePrimary : spec.colorHumbleSecondary,
					road   = (n/3)%2? spec.colorRoadPrimary : spec.colorRoadSecondary;

		const ScreenCoordCache& p = lts[(n-1)%N];

		drawRoadQuad(grass,  0,  p.Y, drawAreaWidth, 0, lt.Y, drawAreaWidth);
		drawRoadQuad(rumble, p.X, p.Y, p.W*1.2, lt.X, lt.Y, lt.W*1.2);
		drawRoadQuad(road,   p.X, p.Y, p.W, lt.X, lt.Y, lt.W);
	}

	for(unsigned n = fromPos + drawDistance; n >= fromPos+1; n--)
	{
		const CourseSpec::Segment& l = spec.lines[n%N];
		const ScreenCoordCache& lt = lts[n%N];

		if(l.spriteID != -1)
		{
			Image& s = *sprites[l.spriteID];
			const int w = s.getWidth(),
					  h = s.getHeight();

			const float scale = lt.W/150,
				  destW = w*scale,
				  destH = h*scale;
			float destX = lt.X + lt.scale * l.spriteX * drawAreaWidth/2;
			float destY = lt.Y + 4;

			destX += destW * l.spriteX;  // offsetX
			destY += destH * (-1);  // offsetY

			float clipH = destY+destH-l.clip;
			if(clipH < 0)
				clipH = 0;

			const float sw = w, sh = h-h*clipH/destH;

			if(not (clipH >= destH or destW > this->drawAreaWidth or destH > this->drawAreaHeight or sh <= 1))
				s.drawScaledRegion(destX, destY, scale, scale, Image::FLIP_NONE, 0, 0, sw, sh);
		}

		if(trafficVehicles != null) foreach(Pseudo3DVehicle&, trafficVehicle, vector<Pseudo3DVehicle>, *trafficVehicles)
	    {
			if((static_cast<unsigned>(trafficVehicle.position/spec.roadSegmentLength))%N == n)
			{
				const int w = trafficVehicle.spriteSpec.frameWidth * trafficVehicle.sprites.back()->scale.x,
						  h = trafficVehicle.spriteSpec.frameHeight * trafficVehicle.sprites.back()->scale.y;

				const float scale = lt.W/400,
					  destW = w*scale,
					  destH = h*scale;
				float destX = lt.X + lt.scale * trafficVehicle.horizontalPosition * drawAreaWidth/2;
				float destY = lt.Y + 4;

				destX += scale * trafficVehicle.horizontalPosition;  // offsetX

				const Point pt = { destX, destY };
				trafficVehicle.draw(pt, 0, scale);
			}
	    }
	}
}

void Pseudo3DCourse::freeAssetsData()
{
	if(not sprites.empty())
	{
		for(unsigned i = 0; i < sprites.size(); i++)
			delete sprites[i];

		sprites.clear();
	}
}

void Pseudo3DCourse::loadAssetsData()
{
	for(unsigned i = 0; i < spec.spritesFilenames.size(); i++)
		if(not spec.spritesFilenames[i].empty())
			sprites.push_back(new Image(spec.spritesFilenames[i]));
		else
			sprites.push_back(null);
}

// #################### Pseudo3D Course Spec. methods #####################################################

void Pseudo3DCourse::Spec::loadFromFile(const string& filename)
{
	this->parseProperties(filename);
	this->loadSegments(segmentFilename);
}

void Pseudo3DCourse::Spec::saveToFile(const string& filename)
{
	const string specFilename = filename + ".properties", segmentsFilename = filename + ".csv";
	this->saveProperties(specFilename, segmentsFilename);
	this->saveSegments(segmentsFilename);
}

// #################### Pseudo3D Course Map methods #####################################################

Pseudo3DCourse::Map::Map()
: spec(Pseudo3DCourse::Spec(0,0)), bounds(), offset(), scale(),
  roadColor(), segmentHighlightColor(),
  segmentHighlightSize(0), roadContrastColorEnabled(),
  geometryOtimizationEnabled()
{}

Pseudo3DCourse::Map::Map(const Spec& s)
: spec(s), bounds(), offset(), scale(),
  roadColor(), segmentHighlightColor(),
  segmentHighlightSize(0), roadContrastColorEnabled(),
  geometryOtimizationEnabled()
{}

void Pseudo3DCourse::Map::compile()
{
	Point p1 = offset;
	float angle = 0;

	// if no scale set, set one automatically to fit minimap bounds
	if(scale.isZero())
	{
		Point pmin = Point(), pmax = Point();
		for(unsigned i = 0; i < spec.lines.size(); i++)
		{
			Point p2 = p1;
			p2.x += spec.lines[i].curve;
			p2.y += sqrt(pow(spec.roadSegmentLength, 2) - pow(spec.lines[i].curve, 2));
			angle += asin(spec.lines[i].curve/spec.roadSegmentLength);
			rotatePoint(p2, p1, angle);
			if(p2.x < pmin.x)
				pmin.x = p2.x;
			else if(p2.x > pmax.x)
				pmax.x = p2.x;

			if(p2.y < pmin.y)
				pmin.y = p2.y;
			else if(p2.y > pmax.y)
				pmax.y = p2.y;

			p1 = p2;
		}

		const float deltaX = pmax.x - pmin.x, deltaY = pmax.y - pmin.y,
					newscale = (deltaX > deltaY? 0.9f*bounds.w/deltaX :
							 deltaY > deltaX? 0.9f*bounds.h/deltaY : 1.f);

		scale.x = scale.y = newscale;
		offset.x = -pmin.x + 0.5f*(bounds.w/newscale - deltaX);
		offset.y = -pmin.y + 0.5f*(bounds.h/newscale - deltaY);
	}

	vector<float> points;
	points.resize(2*(spec.lines.size()+1));

	p1 = offset; angle = 0;
	for(unsigned i = 0; i < spec.lines.size(); i++)
	{
		Point p2 = p1;
		p2.x += spec.lines[i].curve;
		p2.y += sqrt(pow(spec.roadSegmentLength, 2) - pow(spec.lines[i].curve, 2));
		angle += asin(spec.lines[i].curve/spec.roadSegmentLength);
		rotatePoint(p2, p1, angle);
		if(i == 0)
		{
			points[0] = p1.x*scale.x;
			points[1] = p1.y*scale.y;
		}
		points[2*(i+1)] =   p2.x*scale.x;
		points[2*(i+1)+1] = p2.y*scale.y;

		p1 = p2;
	}

	cache.clear();
	cacheLenght.clear();
	if(geometryOtimizationEnabled)
	{
		vector<float> simplifiedPoints;
		psimpl::simplify_douglas_peucker <2> (points.begin (), points.end (), 1.5f, std::back_inserter(simplifiedPoints));

		cache.resize(simplifiedPoints.size()/2);
		for(unsigned i = 0; i < cache.size(); i++)
		{
			cache[i].x = simplifiedPoints[2*i];
			cache[i].y = simplifiedPoints[2*i+1];
		}

		cacheLenght.resize(cache.size());
		cacheLenght[0] = 0;
		for(unsigned i = 1, j = 1; i < cache.size(); i++)
		{
			cacheLenght[i] = cacheLenght[i-1];
			for(unsigned k = j; k < spec.lines.size(); k++)
			{
				cacheLenght[i] += spec.roadSegmentLength;
				if(points[2*k] == cache[i].x and points[2*k+1] == cache[i].y)
				{
					j = k+1;
					break;
				}
			}
		}
	}
	else
	{
		cache.resize(points.size()/2);
		for(unsigned i = 0; i < cache.size(); i++)
		{
			cache[i].x = points[2*i];
			cache[i].y = points[2*i+1];
		}
	}

}

void Pseudo3DCourse::Map::drawMap(unsigned highlightedSegment)
{
	if(spec.lines.empty())
		return;
	else if(cache.empty())
		this->compile();

	const Color roadColor2(255-roadColor.r, 255-roadColor.g, 255-roadColor.b);
	for(unsigned i = 1; i < cache.size(); i++)
	{
		const Point& p1 = cache[i-1], &p2 = cache[i];
		if(p1.x > 0 and p1.x < bounds.w and p1.y > 0 and p1.y < bounds.h and p2.x > 0 and p2.x < bounds.w and p2.y > 0 and p2.y < bounds.h)
			Graphics::drawLine(bounds.x + p1.x, bounds.y + p1.y, bounds.x + p2.x, bounds.y + p2.y, (roadContrastColorEnabled and (i % 2)? roadColor2 : roadColor));
	}

	if(segmentHighlightSize != 0 and highlightedSegment < spec.lines.size())
	{
		const float overallPosition = highlightedSegment*spec.roadSegmentLength;
		for(unsigned i = 0; i < cacheLenght.size(); i++)
		{
			if(cacheLenght[i] > overallPosition)
			{
				const float segDiff = (overallPosition - cacheLenght[i-1])/(cacheLenght[i] - cacheLenght[i-1]);
				Graphics::drawFilledCircle(bounds.x + cache[i].x*segDiff + cache[i-1].x*(1-segDiff), bounds.y + cache[i].y*segDiff + cache[i-1].y*(1-segDiff), segmentHighlightSize, segmentHighlightColor);
				break;
			}
		}
	}
}

// ========================================================================================================================
// ====================== built-in generators =============================================================================

//static
Pseudo3DCourse::Spec Pseudo3DCourse::Spec::generateDebugCourseSpec(float segmentLength, float roadWidth)
{
	Pseudo3DCourse::Spec spec(segmentLength, roadWidth);
	for(unsigned i = 0; i < 1600; i++)  // generating hardcoded course
	{
		CourseSpec::Segment line;
		line.z = i*spec.roadSegmentLength;
		if(i > 300 && i < 500) line.curve = 0.3;
		if(i > 500 && i < 700) line.curve = -0.3;
		if(i > 900 && i < 1300) line.curve = -2.2;
		if(i > 750) line.y = sin(i/30.0)*1500;
		if(i % 17==0) { line.spriteX=2.0; line.spriteID=0; }
		if(i % 17==1) { line.spriteX=-3.0; line.spriteID=0; }
		spec.lines.push_back(line);
	}
	spec.spritesFilenames.push_back("assets/bush.png");  // type 0
	spec.landscapeFilename = "assets/bg.png";
	spec.colorRoadPrimary =      Color( 64, 80, 80);
	spec.colorRoadSecondary =    Color( 40, 64, 64);
	spec.colorOffRoadPrimary =   Color(  0,112,  0);
	spec.colorOffRoadSecondary = Color(  0, 88, 80);
	spec.colorHumblePrimary =    Color(200,200,200);
	spec.colorHumbleSecondary =  Color(152,  0,  0);

	spec.colorLandscape = Color(136,204,238);
	spec.colorHorizon = spec.colorOffRoadPrimary;

	spec.musicFilename = "assets/music_sample.ogg";

	return spec;
}

//static
Pseudo3DCourse::Spec Pseudo3DCourse::Spec::generateRandomCourseSpec(float segmentLength, float roadWidth, float length, float curveness)
{
	Pseudo3DCourse::Spec spec(segmentLength, roadWidth);

	float currentCurve = 0;

	const bool range1 = true,//rand()%2,
			   range2 = true,//rand;()%2,
			   range3 = true;//rand;()%2;
	const float range1size = random_between_decimal(0, 1500),
				range2size = random_between_decimal(0, 1500),
				range3size = random_between_decimal(0, 1500);

	// generating random course
	for(unsigned i = 0; i < length; i++)
	{
		CourseSpec::Segment line;
		line.z = i*spec.roadSegmentLength;

		if(currentCurve == 0)
		{
			if(rand() % 500 == 0)
				currentCurve = random_between_decimal(-5*curveness, 5*curveness);
			else if(rand() % 50 == 0)
				currentCurve = random_between_decimal(-curveness, curveness);
		}

		else if(currentCurve != 0 and rand() % 100 == 0)
			currentCurve = 0;

		line.curve = currentCurve;

		// fixme this should be parametrized, or at least random
		if(i > 750 and i < 1510 and range1) line.y = sin(i/30.0)*range1size;
		if(i > 1510 and i < 2270 and range2) line.y = sin(i/30.0)*range2size;
		if(i > 2270 and i < 3030 and range3) line.y = sin(i/30.0)*range3size;

		if(rand() % 10 == 0)
		{
			line.spriteID = 0;
			line.spriteX = (rand()%2==0? -1 : 1) * random_between_decimal(2.5, 3.0);
		}
		else if(rand() % 100 == 0)
		{
			line.spriteID = 1;
			line.spriteX = (rand()%2==0? -1 : 1) * random_between_decimal(2.0, 2.5);
		}
		else if(rand() % 1000 == 0)
		{
			line.spriteID = 2;
			line.spriteX = (rand()%2==0? -1 : 1) * random_between_decimal(2.0, 2.5);
		}

		spec.lines.push_back(line);
	}

	static struct RoadColors {
		Color primary, secondary, humblePrimary, humbleSecondary;
	}
	roadColors[] = {
		{ Color(64, 80, 80), Color(40, 64, 64), Color(200,200,200), Color(152,  0,  0) },  // racetrack road
		{ Color(12, 28, 12), Color(31, 28, 31), Color( 12, 28, 12), Color( 31, 28, 31) }   // euro country road
	};

	static struct LandscapeSettings {
		Color terrainPrimary, terrainSecondary, sky;
		string landscapeBgFilename, sprite1, sprite2, sprite3;
	}
	landscapeSettings[] = {
		{ Color(  0,112,  0), Color(  0, 88,  0), Color(136,204,238), "bg.png", "bush.png", "tree.png", "redbarn.png" },  // grasslands landscape
		{ Color( 31, 85,  0), Color( 31, 68,  0), Color(173,201,152), "bg_forest.jpg", "bush.png", "tree.png", "talltree.png" },  // forest landscape
		{ Color( 64, 80, 80), Color( 40, 64, 64), Color( 35, 31, 32), "bg_nightcity.jpg", "bush.png", "streetlight_double.png", "buildings.png" },  // night city landscape
		{ Color(220,170,139), Color(188,137,106), Color(248,156, 31), "bg_desert.jpg", "rock.png", "cactus.png", "cliff.png" },  // desert landscape
		{ Color(190,229,246), Color(159,198,213), Color(123,138,155), "bg_montains.jpg", "bush_snow.png", "talltree.png", "cliff_alpine.png" },  // snow landscape
	};

	const unsigned
		ri = futil::random_between(0, sizeof(roadColors)/sizeof(RoadColors)),
		li = futil::random_between(0, sizeof(landscapeSettings)/sizeof(LandscapeSettings));

	spec.spritesFilenames.push_back("assets/"+landscapeSettings[li].sprite1);
	spec.spritesFilenames.push_back("assets/"+landscapeSettings[li].sprite2);
	spec.spritesFilenames.push_back("assets/"+landscapeSettings[li].sprite3);
	spec.landscapeFilename = "assets/"+landscapeSettings[li].landscapeBgFilename;

	spec.colorRoadPrimary = roadColors[ri].primary;
	spec.colorRoadSecondary = roadColors[ri].secondary;
	spec.colorOffRoadPrimary = landscapeSettings[li].terrainPrimary;
	spec.colorOffRoadSecondary = landscapeSettings[li].terrainSecondary;
	spec.colorHumblePrimary = roadColors[ri].humblePrimary;
	spec.colorHumbleSecondary = roadColors[ri].humbleSecondary;

	spec.colorLandscape = landscapeSettings[li].sky;
	spec.colorHorizon = spec.colorOffRoadPrimary;

	spec.musicFilename = "assets/music_sample.ogg";

	return spec;
}
