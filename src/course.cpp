/*
 * course.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "course.hpp"

#include "futil/random.h"

#include <cstdlib>
#include <ctime>
#include <cmath>

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
  drawAreaWidth(), drawAreaHeight(), drawDistance(1), cameraDepth(100)
{}

Pseudo3DCourse::Pseudo3DCourse(Spec spec)
: spec(spec), sprites(),
  drawAreaWidth(), drawAreaHeight(), drawDistance(1), cameraDepth(100)
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
		if(l.spriteID == -1)
			continue;

	    Image& s = *sprites[l.spriteID];
	    const int w = s.getWidth(), h = s.getHeight();
	    const ScreenCoordCache& lt = lts[n%N];

	    float destX = lt.X + lt.scale * l.spriteX * drawAreaWidth/2;
	    float destY = lt.Y + 4;
	    float destW  = w * lt.W / 150;
	    float destH  = h * lt.W / 150;

	    destX += destW * l.spriteX; //offsetX
	    destY += destH * (-1);    //offsetY

	    float clipH = destY+destH-l.clip;
	    if(clipH < 0)
	    	clipH = 0;

	    const float sw = w, sh = h-h*clipH/destH;

	    if(clipH >= destH or destW > this->drawAreaWidth or destH > this->drawAreaHeight or sh <= 1)
	    	continue;

	    s.drawScaledRegion(destX, destY, destW/w, destH/h, Image::FLIP_NONE, 0, 0, sw, sh);
	}
}

void Pseudo3DCourse::drawMap(const Color& color, const Point& offset, const Vector2D& scale, const Rectangle& bounds)
{
	const Color color2(255-color.r, 255-color.g, 255-color.b);
	Point p1 = offset;
	float angle = 0;
	for(unsigned i = 0; i < spec.lines.size(); i++)
	{
		Point p2 = p1;
		p2.x += spec.lines[i].curve;
		p2.y += sqrt(pow(spec.roadSegmentLength, 2) - pow(spec.lines[i].curve, 2));
		angle += asin(spec.lines[i].curve/spec.roadSegmentLength);
		rotatePoint(p2, p1, angle);

		Graphics::drawLine(p1.x * scale.x, p1.y * scale.y, p2.x * scale.x, p2.y * scale.y, (i % 2? color : color2));

		p1 = p2;
	}
}

/*

void Pseudo3DCourse::drawMap(const fgeal::Rectangle& bounds, fgeal::Color color)
{
	if(bounds.w == 0 or bounds.h == 0)
		return;

	float xmin = 0, xmax = 0, xval = 0;
	for(unsigned i = 0; i < spec.lines.size()-1; i++)
	{
		xval += spec.lines[i].curve;
		if(xval < xmin)
			xmin = xval;
		if(xval > xmax)
			xmax = xval;
	}

	const float courseWidth = xmax-xmin, courseHeight = spec.roadSegmentLength*spec.lines.size();

	const Vector2D scale = { 0.95f*bounds.w/(courseWidth!=0? courseWidth : bounds.w), 0.95f*bounds.h/courseHeight };
	Point pt = { 0.025f*bounds.w-xmin*scale.x, 0.975f*bounds.h };
	for(unsigned i = 0; i < spec.lines.size(); i++)
	{
		Point pt2 = pt;
		pt2.x += spec.lines[i].curve*scale.x;
		pt2.y -= spec.roadSegmentLength*scale.y;
		Graphics::drawLine(bounds.x + pt.x, bounds.y + pt.y, bounds.x + pt2.x, bounds.y + pt2.y, color);
		pt = pt2;
	}
}

 */

void Pseudo3DCourse::clearDynamicData()
{
	if(not sprites.empty())
	{
		for(unsigned i = 0; i < sprites.size(); i++)
			delete sprites[i];

		sprites.clear();
	}
}

void Pseudo3DCourse::setupDynamicData()
{
	for(unsigned i = 0; i < spec.spritesFilenames.size(); i++)
		if(not spec.spritesFilenames[i].empty())
			sprites.push_back(new Image(spec.spritesFilenames[i]));
		else
			sprites.push_back(null);
}

// ========================================================================================================================
// ====================== built-in generators =============================================================================

//static
Pseudo3DCourse::Spec Pseudo3DCourse::generateDebugCourseSpec(float segmentLength, float roadWidth)
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
Pseudo3DCourse::Spec Pseudo3DCourse::generateRandomCourseSpec(float segmentLength, float roadWidth, float length, float curveness)
{
	Pseudo3DCourse::Spec spec(segmentLength, roadWidth);
	srand(time(null));

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

	spec.spritesFilenames.push_back("assets/bush.png");
	spec.spritesFilenames.push_back("assets/tree.png");
	spec.spritesFilenames.push_back("assets/redbarn.png");
	spec.landscapeFilename = "assets/bg.png";

	spec.colorRoadPrimary =      Color( 64, 80, 80);
	spec.colorRoadSecondary =    Color( 40, 64, 64);
	spec.colorOffRoadPrimary =   Color(  0,112,  0);
	spec.colorOffRoadSecondary = Color(  0, 88,  0);
	spec.colorHumblePrimary =    Color(200,200,200);
	spec.colorHumbleSecondary =  Color(152,  0,  0);

	spec.colorLandscape = Color(136,204,238);
	spec.colorHorizon = spec.colorOffRoadPrimary;

	spec.musicFilename = "assets/music_sample.ogg";

	return spec;
}
