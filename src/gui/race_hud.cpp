/*
 * race_hud.cpp
 *
 *  Created on: 10 de jun de 2019
 *      Author: carlos.faruolo
 */

#include "race_hud.hpp"

using std::string;

using fgeal::Rectangle;
using fgeal::Color;
using fgeal::Point;
using fgeal::Graphics;
using fgeal::Font;

Hud::GenericDialGauge::GenericDialGauge(const Rectangle& bounds)
: bounds(bounds), angleMin(0.25*M_PI), angleMax(1.75*M_PI),
  backgroundColor(Color::WHITE),
  borderThickness(2.0f), borderColor(Color::BLACK),
  needleThickness(2.0f), needleColor(Color::RED),
  boltRadius(16.0f), boltColor(Color::BLACK),
  graduationColor(Color::BLACK), graduationFont(null), graduationValuePositionOffset(0.35f),
  graduationPrimaryLineSize(0.5f), graduationSecondaryLineSize(0.3f), graduationTertiaryLineSize(0.2f),
  graduationValueScale(1.0f), graduationLevel(1),
  fixationOffset(0), pointerOffset(0), pointerSizeScale(1.0f),
  backgroundImage(null), foregroundImage(null), pointerImage(null), imagesAreShared(false)
{}

Hud::GenericDialGauge::~GenericDialGauge()
{
	if(not imagesAreShared)
	{
		if(pointerImage != null)    delete pointerImage;
		if(backgroundImage != null) delete backgroundImage;
		if(foregroundImage != null) delete foregroundImage;
	}
}

void Hud::GenericDialGauge::compile(float min, float max, float graduationPrimarySize, float graduationSecondarySize, float graduationTertiarySize, float graduationValueOffset)
{
	const Point center = {bounds.x + 0.5f*bounds.w, bounds.y + 0.5f*bounds.h};

	graduationPrimaryCache.clear();
	graduationPrimaryNumericCache.clear();
	graduationSecondaryCache.clear();
	graduationTertiaryCache.clear();

	if(graduationLevel >= 1)  // primary graduation
	{
		bool offsetApplied = false;
		for(float g = min; g <= max; g += graduationPrimarySize)
		{
			const float gAngle = -((angleMax-angleMin)*g + angleMin*max - angleMax*min)/(max-min),
						gradeSize = 0.5f - 0.2f * graduationPrimaryLineSize;
			graduationPrimaryCache.push_back(Line(center.x + gradeSize*bounds.w*sin(gAngle), center.y + gradeSize*bounds.h*cos(gAngle),
												  center.x + 0.5f*bounds.w*sin(gAngle),      center.y + 0.5f*bounds.h*cos(gAngle)));

			// numerical graduation
			if(graduationFont != null)
			{
				const std::string str(futil::to_string(g * graduationValueScale));
				graduationPrimaryNumericCache.push_back(NumericGraduation(str, center.x + graduationValuePositionOffset*bounds.w*sin(gAngle) - 0.5*graduationFont->getTextWidth(str),
																			   center.y + graduationValuePositionOffset*bounds.h*cos(gAngle) - 0.5*graduationFont->getTextHeight()));
			}

			if(not offsetApplied)
			{
				g += graduationValueOffset;
				offsetApplied = true;
			}
		}
	}

	if(graduationLevel >= 2)  // secondary graduation
	for(float g = min; g <= max; g += graduationSecondarySize)
	{
		const float gAngle = -((angleMax-angleMin)*g + angleMin*max - angleMax*min)/(max-min),
					gradeSize = 0.5f - 0.2f * graduationSecondaryLineSize;

		const Line line(center.x + gradeSize*bounds.w*sin(gAngle), center.y + gradeSize*bounds.h*cos(gAngle),
		          center.x + 0.5f*bounds.w*sin(gAngle),      center.y + 0.5f*bounds.h*cos(gAngle));

		// check if line will not occupy the same slot of a primary graduation line
		bool isSlotFree = true;
		for(unsigned i = 0; isSlotFree and i < graduationPrimaryCache.size(); i++)
			if(line.x2 == graduationPrimaryCache[i].x2 and line.y2 == graduationPrimaryCache[i].y2)
				isSlotFree = false;

		if(isSlotFree)
			graduationSecondaryCache.push_back(line);
	}

	if(graduationLevel >= 3)  // tertiary graduation
	for(float g = min; g <= max; g += graduationTertiarySize)
	{
		const float gAngle = -((angleMax-angleMin)*g + angleMin*max - angleMax*min)/(max-min),
					gradeSize = 0.5f - 0.2f * graduationTertiaryLineSize;

		const Line line(center.x + gradeSize*bounds.w*sin(gAngle), center.y + gradeSize*bounds.h*cos(gAngle),
				  center.x + 0.5f*bounds.w*sin(gAngle),      center.y + 0.5f*bounds.h*cos(gAngle));

		// check if line will not occupy the same slot of a primary or secondary graduation line
		bool isSlotFree = true;
		for(unsigned i = 0; isSlotFree and i < graduationPrimaryCache.size(); i++)
			if(line.x2 == graduationPrimaryCache[i].x2 and line.y2 == graduationPrimaryCache[i].y2)
				isSlotFree = false;
		for(unsigned i = 0; isSlotFree and i < graduationSecondaryCache.size(); i++)
			if(line.x2 == graduationSecondaryCache[i].x2 and line.y2 == graduationSecondaryCache[i].y2)
				isSlotFree = false;

		if(isSlotFree)
			graduationTertiaryCache.push_back(line);
	}

	if(backgroundImage != null)
	{
		backgroundImageScale.x = bounds.w/backgroundImage->getWidth();
		backgroundImageScale.y = bounds.h/backgroundImage->getHeight();
	}

	if(foregroundImage != null)
	{
		foregroundImageScale.x = bounds.w/foregroundImage->getWidth();
		foregroundImageScale.y = bounds.h/foregroundImage->getHeight();
	}

	if(pointerImage != null)
		pointerImageScale.x = pointerImageScale.y = 0.5*pointerSizeScale*bounds.h/pointerImage->getHeight();
}

void Hud::GenericDialGauge::drawBackground()
{
	const Point center = {bounds.x + 0.5f*bounds.w, bounds.y + 0.5f*bounds.h};

	if(backgroundImage != null)
		backgroundImage->drawScaled(bounds.x, bounds.y, backgroundImageScale.x, backgroundImageScale.y);
	else
	{
		Graphics::drawFilledEllipse(center.x, center.y, 0.5*bounds.w, 0.5*bounds.h, borderColor);
		Graphics::drawFilledEllipse(center.x, center.y, 0.5*(bounds.w-borderThickness), 0.5*(bounds.h-borderThickness), backgroundColor);
	}

	if(graduationLevel >= 1)  // primary graduation
	for(unsigned i = 0; i < graduationPrimaryCache.size(); i++)
	{
		const Line& line = graduationPrimaryCache[i];
		Graphics::drawLine(line.x1, line.y1, line.x2, line.y2, graduationColor);
		// numerical graduation
		if(graduationFont != null and i < graduationPrimaryNumericCache.size())
		{
			const NumericGraduation& grad = graduationPrimaryNumericCache[i];
			graduationFont->drawText(grad.str, grad.x, grad.y, graduationColor);
		}
	}

	if(graduationLevel >= 2)  // secondary graduation
	for(unsigned i = 0; i < graduationSecondaryCache.size(); i++)
	{
		const Line& line = graduationSecondaryCache[i];
		Graphics::drawLine(line.x1, line.y1, line.x2, line.y2, graduationColor);
	}

	if(graduationLevel >= 3)  // tertiary graduation
	for(unsigned i = 0; i < graduationTertiaryCache.size(); i++)
	{
		const Line& line = graduationTertiaryCache[i];
		Graphics::drawLine(line.x1, line.y1, line.x2, line.y2, graduationColor);
	}
}

void Hud::GenericDialGauge::draw(float angle)
{
	this->drawBackground();

	const Point center = {bounds.x + 0.5f*bounds.w, bounds.y + 0.5f*bounds.h};

	if(pointerImage != null)
		pointerImage->drawScaledRotated(bounds.x + 0.5*bounds.w, bounds.y + 0.5*bounds.h + fixationOffset, pointerImageScale.x, pointerImageScale.y, angle, 0.5*pointerImage->getWidth(), pointerOffset);

	else  // draw built-in, gfx-primitives-based needle as pointer
	{
		Graphics::drawLine(
				center.x + pointerOffset*sin(angle), center.y + pointerOffset*cos(angle) + fixationOffset,
				center.x + 0.4*(pointerSizeScale*bounds.w+pointerOffset)*sin(angle), center.y + 0.4*(pointerSizeScale*bounds.h+pointerOffset)*cos(angle), needleColor);
		Graphics::drawFilledEllipse(center.x, center.y, 0.5*boltRadius*bounds.w/bounds.h, 0.5*boltRadius*bounds.h/bounds.w, boltColor);
	}

	if(foregroundImage != null)
		foregroundImage->drawScaled(bounds.x, bounds.y, foregroundImageScale.x, foregroundImageScale.y);
}

Hud::GenericBarGauge::GenericBarGauge(const Rectangle& bounds)
: bounds(bounds),
  backgroundColor(fgeal::Color::WHITE),
  borderThickness(2.0f), borderColor(fgeal::Color::BLACK),
  fillColor(fgeal::Color::RED)
{}

void Hud::GenericBarGauge::draw(float fillRatio)
{
	if(backgroundColor.a != 255)
	{
		fgeal::Graphics::drawFilledRectangle(bounds.x, bounds.y, bounds.w, borderThickness, borderColor);
		fgeal::Graphics::drawFilledRectangle(bounds.x, bounds.y + bounds.h - borderThickness, bounds.w, borderThickness, borderColor);
		fgeal::Graphics::drawFilledRectangle(bounds.x, bounds.y, borderThickness, bounds.h, borderColor);
		fgeal::Graphics::drawFilledRectangle(bounds.x + bounds.w - borderThickness, bounds.y, borderThickness, bounds.h, borderColor);
	}
	else
		fgeal::Graphics::drawFilledRectangle(bounds.x, bounds.y, bounds.w, bounds.h, borderColor);

	fgeal::Graphics::drawFilledRectangle(bounds.x + borderThickness + 1, bounds.y + borderThickness + 1, bounds.w - 2*borderThickness - 2, bounds.h - 2*borderThickness - 2, backgroundColor);
	fgeal::Graphics::drawFilledRectangle(bounds.x + borderThickness + 1, bounds.y + borderThickness + 1, fillRatio*(bounds.w - 2*borderThickness - 2), bounds.h - 2*borderThickness - 2, fillColor);
}

Hud::GenericNumericalDisplay::GenericNumericalDisplay(const Rectangle& bounds, Font* font)
: valueScale(1.0), bounds(bounds), padding({1, 1}),
  backgroundColor(fgeal::Color::WHITE), disableBackground(false),
  borderThickness(2.0f), borderColor(fgeal::Color::BLACK), displayColor(fgeal::Color::GREEN),
  font(font), fontIsShared(false)
{}

Hud::GenericNumericalDisplay::~GenericNumericalDisplay()
{
	if(font != null and not fontIsShared)
		delete font;
}

void Hud::GenericNumericalDisplay::draw(const string& valueStr)
{
	if(not disableBackground)
	{
		fgeal::Graphics::drawFilledRectangle(bounds.x, bounds.y, bounds.w, bounds.h, borderColor);
		fgeal::Graphics::drawFilledRectangle(bounds.x + borderThickness, bounds.y + borderThickness,
										   bounds.w - 2*borderThickness, bounds.h - 2*borderThickness, backgroundColor);
	}
	font->drawText(valueStr, bounds.x + borderThickness + padding.x, bounds.y + borderThickness + padding.y, displayColor);
}

void Hud::GenericNumericalDisplay::pack(unsigned char digitCount)
{
	if(font != null)
	{
		const std::string sampleValue(digitCount, '9');
		bounds.w = font->getTextWidth(sampleValue) + 2*(borderThickness + padding.x);
		bounds.h = font->getTextHeight() + 2*(borderThickness + padding.y);
	}
}
