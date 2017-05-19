/*
 * race_hud.hpp
 *
 *  Created on: 26 de abr de 2017
 *      Author: carlosfaruolo
 */

#ifndef AUTOMOTIVE_RACE_HUD_HPP_
#define AUTOMOTIVE_RACE_HUD_HPP_
#include <ciso646>

#include "fgeal/fgeal.hpp"

#include "futil/string/more_operators.hpp"

#include <cmath>

#ifndef M_PI
	# define M_PI		3.14159265358979323846	/* pi */
#endif

namespace Hud
{
	/** A generic dial-type gauge. Can drawn either custom images or native primitives.
	 *  To drawn using custom images, the image's fields must be non-null. Otherwise native versions will be drawn. */
	template <typename NumberType>
	struct DialGauge
	{
		/** the value to measure. */
		const NumberType& value;

		/** the minimum and maximum expected values. */
		NumberType min, max;

		/** the grade's size. */
		NumberType graduationPrimarySize, graduationSecondarySize, graduationTertiarySize;

		/** this widget's dimensions and position. */
		fgeal::Rectangle bounds;

		/** The minimum and maximum angle applied on the rotating pointer. */
		float angleMin, angleMax;

		/** The background color. */
		fgeal::Color backgroundColor;

		/** The border's thickness. If 0 (zero), no border is drawn. */
		float borderThickness;

		/** The border's color. If set as transparent, no border is drawn. */
		fgeal::Color borderColor;

		/** The thickness of the pointing needle. */
		float needleThickness;

		/** The needle's color. */
		fgeal::Color needleColor;

		/** The radius the needle's "bolt". */
		float boltRadius;

		/** The bolt's color. */
		fgeal::Color boltColor;

		/** The graduation's color. */
		fgeal::Color graduationColor;

		/** The font used to display graduation values. If null, no graduation values are shown. */
		fgeal::Font* graduationFont;

		/** An optional scaling factor for the graduation values shown. It's better used if set power of 10, like 0.1, 0.01, 0.001, etc. */
		float graduationValueScale;

		/** The level of graduation to show (currently 0 to 3). Setting as 0 makes the gauge to display no graduation at all. */
		short graduationLevel;

		/** An optional vertical offset to the pointer's fixation position on the gauge. */
		float fixationOffset;

		/** An optional offset applied to the pointer in relation to its fixation point. */
		float pointerOffset;

		/** An optional scale factor applied to the pointer's size. Normally (scale=1.0), the pointer size is equal to the gauge radius.*/
		float pointerSizeScale;

		/** Custom gauge background and foreground images. */
		fgeal::Image* backgroundImage, *foregroundImage;

		/**	Custom pointer image. */
		fgeal::Image* pointerImage;

		/** If true, indicates that the custom images used by this gauge are shared, and thus, should not be deleted when this gauge is deleted. */
		bool imagesAreShared;

		DialGauge(const NumberType& var, NumberType min, NumberType max, const fgeal::Rectangle& bounds)
		: value(var), min(min), max(max),
		  graduationPrimarySize(0.1*(max-min)), graduationSecondarySize(0.01*(max-min)), graduationTertiarySize(0.001*(max-min)),
		  bounds(bounds), angleMin(0.25*M_PI), angleMax(1.75*M_PI),
		  backgroundColor(fgeal::Color::WHITE),
		  borderThickness(2.0f), borderColor(fgeal::Color::BLACK),
		  needleThickness(2.0f), needleColor(fgeal::Color::RED),
		  boltRadius(16.0f), boltColor(fgeal::Color::BLACK),
		  graduationColor(fgeal::Color::BLACK), graduationFont(null),
		  graduationValueScale(1.0), graduationLevel(1),
		  fixationOffset(0), pointerOffset(0), pointerSizeScale(1.0),
		  backgroundImage(null), foregroundImage(null), pointerImage(null), imagesAreShared(false)
		{}

		~DialGauge()
		{
			if(not imagesAreShared)
			{
				if(pointerImage != null)    delete pointerImage;
				if(backgroundImage != null) delete backgroundImage;
				if(foregroundImage != null) delete foregroundImage;
			}
		}

		void draw()
		{
			const fgeal::Point center = {bounds.x + 0.5f*bounds.w, bounds.y + 0.5f*bounds.h};
			const float angle = -((angleMax-angleMin)*value + angleMin*max - angleMax*min)/(max-min);

			if(backgroundImage != null)
				backgroundImage->drawScaled(bounds.x, bounds.y, bounds.w/backgroundImage->getWidth(), bounds.h/backgroundImage->getHeight());
			else
			{
				fgeal::Image::drawEllipse(borderColor,     center.x, center.y, 0.5*bounds.w, 0.5*bounds.h);
				fgeal::Image::drawEllipse(backgroundColor, center.x, center.y, 0.5*(bounds.w-borderThickness), 0.5*(bounds.h-borderThickness));
			}

			if(pointerImage != null)
			{
				pointerImage->drawScaledRotated(bounds.x + 0.5*bounds.w, bounds.y + 0.5*bounds.h + fixationOffset,
						0.5*pointerSizeScale*bounds.h/pointerImage->getHeight(), 0.5*pointerSizeScale*bounds.h/pointerImage->getHeight(),
						angle, 0.5*pointerImage->getWidth(), pointerOffset);
			}
			else
			{
				fgeal::Image::drawLine(needleColor,
						center.x + pointerOffset*sin(angle), center.y + pointerOffset*cos(angle) + fixationOffset,
						center.x + 0.4*(pointerSizeScale*bounds.w+pointerOffset)*sin(angle), center.y + 0.4*(pointerSizeScale*bounds.h+pointerOffset)*cos(angle));
				fgeal::Image::drawEllipse(boltColor, center.x, center.y, 0.5*boltRadius*bounds.w/bounds.h, 0.5*boltRadius*bounds.h/bounds.w);
			}

			if(graduationLevel >= 1)  // primary graduation
			for(NumberType g = min; graduationLevel > 0 and g <= max; g += graduationPrimarySize)
			{
				const float gAngle = -((angleMax-angleMin)*g + angleMin*max - angleMax*min)/(max-min);
				fgeal::Image::drawLine(graduationColor,
						center.x + 0.4*bounds.w*sin(gAngle), center.y + 0.4*bounds.h*cos(gAngle),
						center.x + 0.5*bounds.w*sin(gAngle), center.y + 0.5*bounds.h*cos(gAngle));

				// numerical graduation
				if(graduationFont != null)
				{
					std::string str = std::string() + (g * graduationValueScale);
					graduationFont->drawText(str, center.x + 0.35*bounds.w*sin(gAngle) - 0.5*graduationFont->getTextWidth(str), center.y + 0.35*bounds.h*cos(gAngle) - 0.5*graduationFont->getFontHeight(), graduationColor);
				}
			}

			if(graduationLevel >= 2)  // secondary graduation
			for(NumberType g = min; g <= max; g += graduationSecondarySize)
			{
				const float gAngle = -((angleMax-angleMin)*g + angleMin*max - angleMax*min)/(max-min);
				fgeal::Image::drawLine(graduationColor,
						center.x + 0.44*bounds.w*sin(gAngle), center.y + 0.44*bounds.h*cos(gAngle),
						center.x + 0.50*bounds.w*sin(gAngle), center.y + 0.50*bounds.h*cos(gAngle));
			}

			if(graduationLevel >= 3)  // tertiary graduation
			for(NumberType g = min; g <= max; g += graduationTertiarySize)
			{
				const float gAngle = -((angleMax-angleMin)*g + angleMin*max - angleMax*min)/(max-min);
				fgeal::Image::drawLine(graduationColor,
						center.x + 0.46*bounds.w*sin(gAngle), center.y + 0.46*bounds.h*cos(gAngle),
						center.x + 0.50*bounds.w*sin(gAngle), center.y + 0.50*bounds.h*cos(gAngle));
			}

			if(foregroundImage != null)
				foregroundImage->drawScaled(bounds.x, bounds.y, bounds.w/foregroundImage->getWidth(), bounds.h/foregroundImage->getHeight());
		}
	};

	// todo improve BarGauge with more options
	/** A bar-type gauge */
	template <typename NumberType>
	struct BarGauge
	{
		/** the value to measure. */
		const NumberType& value;

		/** the minimum and maximum expected values. */
		NumberType min, max;

		/** this widget's dimensions and position. */
		fgeal::Rectangle bounds;

		/** The background color. */
		fgeal::Color backgroundColor;

		/** The border's thickness. If 0 (zero), no border is drawn. */
		float borderThickness;

		/** The border's color. If set as transparent, no border is drawn. */
		fgeal::Color borderColor;

		/** The needle's color. */
		fgeal::Color fillColor;

		BarGauge(const NumberType& var, NumberType min, NumberType max, const fgeal::Rectangle& bounds)
		: value(var), min(min), max(max),
		  bounds(bounds),
		  backgroundColor(fgeal::Color::WHITE),
		  borderThickness(2.0f), borderColor(fgeal::Color::BLACK),
		  fillColor(fgeal::Color::RED)
		{}

		void draw()
		{
			const float fillRatio = (value-min)/(max-min);
			fgeal::Image::drawRectangle(borderColor, bounds.x, bounds.y, bounds.w, bounds.h);
			fgeal::Image::drawRectangle(backgroundColor,
					bounds.x + 0.5*borderThickness, bounds.y + 0.5*borderThickness,
					bounds.w - borderThickness, bounds.h - borderThickness);
			fgeal::Image::drawRectangle(fillColor,
					bounds.x + 0.5*borderThickness, bounds.y + 0.5*borderThickness,
					fillRatio*(bounds.w - borderThickness), bounds.h - borderThickness);
		}
	};

	/** A widget that displays a numeric value, possibly stylised. */
	template <typename NumberType>
	struct NumericalDisplay
	{
		/** the value to show. */
		const NumberType& value;

		/** An optional scaling factor for the value shown. */
		float valueScale;

		/** this widget's dimensions and position. */
		fgeal::Rectangle bounds;

		/** The background color. */
		fgeal::Color backgroundColor;

		/** The border's thickness. If 0 (zero), no border is drawn. */
		float borderThickness;

		/** The border's color. If set as transparent, no border is drawn. */
		fgeal::Color borderColor;

		/** The display's color. */
		fgeal::Color displayColor;

		/** The font used to display the value. */
		fgeal::Font* font;

		/** If true, indicates that the font is shared, and thus, should not be deleted when this display is deleted. */
		bool fontIsShared;

		NumericalDisplay(const NumberType& var, const fgeal::Rectangle& bounds, fgeal::Font* font)
		: value(var), valueScale(1.0), bounds(bounds),
		  backgroundColor(fgeal::Color::WHITE),
		  borderThickness(2.0f), borderColor(fgeal::Color::BLACK), displayColor(fgeal::Color::GREEN),
		  font(font), fontIsShared(false)
		{}

		~NumericalDisplay()
		{
			if(font != null and not fontIsShared)
				delete font;
		}

		void draw()
		{
			fgeal::Image::drawRectangle(borderColor, bounds.x, bounds.y, bounds.w, bounds.h);
			fgeal::Image::drawRectangle(backgroundColor,
						bounds.x + 0.5*borderThickness, bounds.y + 0.5*borderThickness,
						bounds.w - borderThickness, bounds.h - borderThickness);

			const std::string str = std::string() + static_cast<int>(value*valueScale);
			font->drawText(str, bounds.x + borderThickness, bounds.y + 0.5*borderThickness, displayColor);
		}
	};
}

#endif /* AUTOMOTIVE_RACE_HUD_HPP_ */
