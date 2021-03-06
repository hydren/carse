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
  drawAreaWidth(), drawAreaHeight(), drawDistance(1),
  cameraDepth(100), cameraHeight(1500),
  lengthScale(1)
{}

inline static float fractional_part(float value)
{
	return value - (int) value;
}

Pseudo3DCourse::~Pseudo3DCourse()
{
	if(not sprites.empty())
	{
		for(unsigned i = 0; i < sprites.size(); i++)
			delete sprites[i];
	}
}

void Pseudo3DCourse::loadSpec(const Spec& s)
{
	// free assets' data
	if(not sprites.empty())
	{
		for(unsigned i = 0; i < sprites.size(); i++)
			delete sprites[i];

		sprites.clear();
	}

	// set new spec and reset some values
	spec = s;

	// resize cache when needed
	if(coordCache.size() != spec.lines.size())
		coordCache.resize(spec.lines.size(), ScreenCoordCache());

	// load assets' data
	for(unsigned i = 0; i < spec.spritesFilenames.size(); i++)
		if(not spec.spritesFilenames[i].empty())
			sprites.push_back(new Image(spec.spritesFilenames[i]));
		else
			sprites.push_back(null);
}

//custom call to draw quad
inline static void drawRoadQuad(float x1, float y1, float w1, float x2, float y2, float w2, const Color& c=Color::WHITE)
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

void Pseudo3DCourse::draw(int pos, int posX)
{
	if(spec.lines.empty() or spec.roadSegmentLength == 0)
		return;

	if(pos < 0)
		pos = 0;

	const unsigned N = spec.lines.size(), fromPos = pos/spec.roadSegmentLength;
	const float camHeight = cameraHeight + spec.lines[fromPos].y;
	float x = 0, dx = 0, maxY = drawAreaHeight;

	for(unsigned n = fromPos+1; n < fromPos + drawDistance; n++)
	{
		const CourseSpec::Segment& segment = spec.lines[n%N];

		// project from "world" to "screen" coordinates
		const int camX = posX - x,
				  camY = camHeight,
				  camZ = pos - (n >= N? N*spec.roadSegmentLength : 0);
		const float scale = cameraDepth / (segment.z - camZ);

		//fixme since segment.x is always zero, camX is actually the one which controls the horizontal shift; it should be segment.x, much like segment.y controls the vertical shift
		ScreenCoordCache& sc = coordCache[n%N];
		sc.X = (1 + scale*(segment.x - camX)) * drawAreaWidth/2;
		sc.Y = (1 - scale*(segment.y - camY)) * drawAreaHeight/2;
		sc.W = scale * spec.roadWidth * drawAreaWidth/2;
		sc.scale = scale;

		// update curve
		x += dx;
		dx += segment.curve;

		sc.clip=maxY;

		if(sc.Y > maxY)
			continue;

		maxY = sc.Y;

		const bool oddn = (n/3)%2;
		const ScreenCoordCache& psc = coordCache[(n-1)%N];  // previous "screen" coordinate

		drawRoadQuad(0,     psc.Y, drawAreaWidth, 0,    sc.Y, drawAreaWidth, oddn? spec.colorOffRoadPrimary : spec.colorOffRoadSecondary);
		drawRoadQuad(psc.X, psc.Y, psc.W*1.2,     sc.X, sc.Y, sc.W*1.2,      oddn? spec.colorHumblePrimary  : spec.colorHumbleSecondary);
		drawRoadQuad(psc.X, psc.Y, psc.W,         sc.X, sc.Y, sc.W,          oddn? spec.colorRoadPrimary    : spec.colorRoadSecondary);
	}

	for(unsigned n = fromPos + drawDistance; n >= fromPos+1; n--)
	{
		const CourseSpec::Segment& segment = spec.lines[n%N];
		const ScreenCoordCache& sc = coordCache[n%N];  // get cached "screen" coordinate

		if(segment.propIndex != -1)
		{
			Image& propImage = *sprites[segment.propIndex];
			const int w = propImage.getWidth(),
					  h = propImage.getHeight();

			const float scale = sc.W/150,
				  destW = w*scale,
				  destH = h*scale;
			float destX = sc.X + sc.scale * segment.propX * drawAreaWidth/2;
			float destY = sc.Y + 4;

			destX += destW * segment.propX;  // offsetX
			destY += destH * (-1);  // offsetY

			float clipH = destY+destH-sc.clip;
			if(clipH < 0)
				clipH = 0;

			const float sw = w, sh = h-h*clipH/destH;

			if(not (clipH >= destH or destW > this->drawAreaWidth or destH > this->drawAreaHeight or sh <= 1))
				propImage.drawScaledRegion(destX, destY, scale, scale, Image::FLIP_NONE, 0, 0, sw, sh);
		}

		for(unsigned i = 0; i < vehicles.size() and vehicles[i] != null; i++)
		{
			const Pseudo3DVehicle& vehicle = *vehicles[i];
			const float vehiclePosition = vehicle.position * lengthScale / spec.roadSegmentLength;
			if(((unsigned) vehiclePosition) % N == n)
			{
				const ScreenCoordCache& psc = coordCache[(n-1)%N];  // get previous cached "screen" coordinate
				const float segProp = fractional_part(vehiclePosition), prevSegProp = 1 - segProp;
				const ScreenCoordCache isc = {  // interpolated screen coordinates
						sc.X * segProp + psc.X * prevSegProp,
						sc.Y * segProp + psc.Y * prevSegProp,
						sc.W * segProp + psc.W * prevSegProp,
						sc.scale * segProp + psc.scale * prevSegProp
				};

				const int w = vehicle.spriteSpec.frameWidth,
						  h = vehicle.spriteSpec.frameHeight;

				const float scale = isc.W * 1.2f,
					  destW = w*scale*vehicle.sprites.back()->scale.x,
					  destH = h*scale*vehicle.sprites.back()->scale.y;
				float destX = isc.X + isc.scale * vehicle.horizontalPosition * drawAreaWidth/2;
				float destY = isc.Y + 4;

				destX += 0.135f * scale * vehicle.horizontalPosition;  // offsetX

				float clipH = destY - sc.clip;
				if(clipH < 0)
					clipH = 0;

				const float sh = h-h*clipH/destH;

				if(not (clipH >= destH or destW > this->drawAreaWidth or destH > this->drawAreaHeight or sh <= 1))
					vehicle.draw(destX, destY, 0, scale, sh);
			}
		}
	}
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
	this->storeProperties(specFilename, segmentsFilename);
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

const Pseudo3DCourse::Spec::RoadStyle Pseudo3DCourse::Spec::RoadStyle::DEFAULT = {
		Color(64, 80, 80), Color(40, 64, 64), Color(200, 200, 200), Color(152, 0, 0), "default" };

const Pseudo3DCourse::Spec::LandscapeStyle Pseudo3DCourse::Spec::LandscapeStyle::DEFAULT = {
		Color(0, 112, 0), Color(0, 88, 0), Color(136, 204, 238), "assets/bg.png", "assets/bush.png", "assets/tree.png", "assets/redbarn.png", "default" };

// ====================== built-in generators =============================================================================

//static
Pseudo3DCourse::Spec Pseudo3DCourse::Spec::createDebug()
{
	Pseudo3DCourse::Spec spec(200, 3000);
	for(unsigned i = 0; i < 1600; i++)  // generating hardcoded course
	{
		CourseSpec::Segment line;
		line.z = i*spec.roadSegmentLength;
		if(i > 300 && i < 500) line.curve = 0.3;
		if(i > 500 && i < 700) line.curve = -0.3;
		if(i > 900 && i < 1300) line.curve = -2.2;
		if(i > 750) line.y = sin(i/30.0)*1500;
		if(i % 17==0) { line.propX=2.0; line.propIndex=0; }
		if(i % 17==1) { line.propX=-3.0; line.propIndex=0; }
		spec.lines.push_back(line);
	}
	spec.props.push_back(Prop());  // type 0
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
Pseudo3DCourse::Spec Pseudo3DCourse::Spec::createRandom(float segmentLength, float roadWidth, unsigned segmentCount, float curveness)
{
	Spec spec(segmentLength, roadWidth);
	spec.lines.resize(segmentCount);

	// generating random course
	float currentCurve = 0, currentSlopeScale = 0;
	unsigned currentSlopeStart = 0, currentSlopeCycle = 1;
	for(unsigned i = 0; i < segmentCount; i++)
	{
		Segment& line = spec.lines[i];
		line.z = i*segmentLength;

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

		if(currentSlopeScale == 0)
		{
			if(rand() % 500 == 0)
			{
				currentSlopeScale = random_between_decimal(0.1 * segmentLength, 1500);
				currentSlopeCycle = 20 * currentSlopeScale / segmentLength;
				currentSlopeStart = i;
			}
		}
		else if(currentSlopeScale != 0 and (i - currentSlopeStart) % currentSlopeCycle == 0 and rand()%2 == 0)
			currentSlopeScale = 0;

		line.y = currentSlopeScale != 0? currentSlopeScale * sin(M_PI * (i - currentSlopeStart)/(float) currentSlopeCycle) : 0;

		if(rand() % 10 == 0)
		{
			line.propIndex = 0;
			line.propX = (rand()%2==0? -1 : 1) * random_between_decimal(2.5, 3.0);
		}
		else if(rand() % 100 == 0)
		{
			line.propIndex = 1;
			line.propX = (rand()%2==0? -1 : 1) * random_between_decimal(2.0, 2.5);
		}
		else if(rand() % 1000 == 0)
		{
			line.propIndex = 2;
			line.propX = (rand()%2==0? -1 : 1) * random_between_decimal(2.0, 2.5);
		}
	}

	spec.spritesFilenames.push_back(string());
	spec.props.push_back(Prop());
	spec.spritesFilenames.push_back(string());
	spec.props.push_back(Prop(true));
	spec.spritesFilenames.push_back(string());
	spec.props.push_back(Prop(true));
	spec.landscapeFilename = "";

	spec.colorRoadPrimary = Color(32, 32, 32);
	spec.colorRoadSecondary = Color(64, 64, 64);
	spec.colorOffRoadPrimary = Color(96, 96, 96);
	spec.colorOffRoadSecondary = Color(128, 128, 128);
	spec.colorHumblePrimary = Color(255, 255, 255);
	spec.colorHumbleSecondary = Color(240, 240, 240);

	spec.colorLandscape = Color(0, 255, 255);
	spec.colorHorizon = spec.colorOffRoadPrimary;

	spec.musicFilename = "";

	return spec;
}
