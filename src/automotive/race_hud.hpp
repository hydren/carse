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
	/** A generic gauge. Not virtual but not functional either. */
	template <typename NumberType>
	struct GenericGauge
	{
		/** this widget's dimensions and position. */
		fgeal::Rectangle bounds;

		/** the value to measure. */
		const NumberType& value;

		/** the minimum and maximum expected values. */
		NumberType min, max;

		/** the grade's size. */
		NumberType majorGrade, minorGrade;

		GenericGauge(const fgeal::Rectangle& bounds, const NumberType& var, NumberType min, NumberType max)
		: bounds(bounds),
		  value(var), min(min), max(max), majorGrade(0.1*(max-min)), minorGrade(0.01*(max-min))
		{}
	};

	/** A generic dial-type gauge. Also not functional */
	template <typename NumberType>
	struct GenericDialGauge extends GenericGauge<NumberType>
	{
		/** The minimum and maximum angle applied on the rotating pointer. */
		float angleMin, angleMax;

		/** An optional vertical offset to the pointer's fixation position on the gauge. */
		float fixationOffset;

		/** An optional scaling factor for the grade values shown. */
		float gradeValueScale;

		GenericDialGauge(const fgeal::Rectangle& bounds, const NumberType& var, NumberType min, NumberType max)
		: GenericGauge<NumberType>(bounds, var, min, max),
		  angleMin(0.25*M_PI), angleMax(1.75*M_PI),
		  fixationOffset(0), gradeValueScale(1)
		{}

		protected:
		float getDialAngle()
		{
			return -((angleMax-angleMin)*this->value + angleMin*this->max - angleMax*this->min)/(this->max-this->min);
		}
	};

	/** A needle dial-type gauge, drawn using native primitives. */
	template <typename NumberType>
	struct NeedleDialGauge extends GenericDialGauge<NumberType>
	{
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

		/** The grade's color. */
		fgeal::Color gradeColor;

		/** The font used to display grade values. */
		fgeal::Font* gradeFont;

		NeedleDialGauge(const fgeal::Rectangle& bounds, const NumberType& var, NumberType min, NumberType max, fgeal::Font* gradeFont=null)
		: GenericDialGauge<NumberType>(bounds, var, min, max),
		  backgroundColor(fgeal::Color::WHITE),
		  borderThickness(2.0f), borderColor(fgeal::Color::BLACK),
		  needleThickness(2.0f), needleColor(fgeal::Color::RED),
		  boltRadius(16.0f), boltColor(fgeal::Color::BLACK),
		  gradeColor(fgeal::Color::BLACK), gradeFont(gradeFont)
		{}

		void draw()
		{
			// todo needle dial gauge
			const fgeal::Rectangle& bounds = this->bounds;
			const fgeal::Point center = {bounds.x + 0.5f*bounds.w, bounds.y + 0.5f*bounds.h};
			const float angle = this->getDialAngle();

			fgeal::Image::drawEllipse(borderColor,     center.x, center.y, 0.5*bounds.w, 0.5*bounds.h);
			fgeal::Image::drawEllipse(backgroundColor, center.x, center.y, 0.5*(bounds.w-borderThickness), 0.5*(bounds.h-borderThickness));
			fgeal::Image::drawLine(needleColor,        center.x, center.y, center.x + 0.4*bounds.w*sin(angle), center.y + 0.4*bounds.h*cos(angle));
			fgeal::Image::drawEllipse(boltColor,       center.x, center.y, 0.5*boltRadius*bounds.w/bounds.h, 0.5*boltRadius*bounds.h/bounds.w);

			for(NumberType g = this->min; g <= this->max; g += this->majorGrade)
			{
				float gAngle = ((this->angleMax-this->angleMin)*g + this->angleMin*this->max - this->angleMax*this->min)/(this->max-this->min);
				fgeal::Image::drawLine(gradeColor,
						center.x + 0.4*bounds.w*sin(gAngle), center.y + 0.4*bounds.h*cos(gAngle),
						center.x + 0.5*bounds.w*sin(gAngle), center.y + 0.5*bounds.h*cos(gAngle));

				if(gradeFont != null)
				{
					std::string str = std::string() + (g*this->gradeValueScale);
					gradeFont->drawText(str, center.x + 0.375*bounds.w*sin(gAngle), center.y + 0.375*bounds.h*cos(gAngle) - 0.5*gradeFont->getSize(), gradeColor);
				}
			}

			for(NumberType g = this->min; g <= this->max; g += this->minorGrade)
			{
				float gAngle = ((this->angleMax-this->angleMin)*g + this->angleMin*this->max - this->angleMax*this->min)/(this->max-this->min);
				fgeal::Image::drawLine(gradeColor,
						center.x + 0.45*bounds.w*sin(gAngle), center.y + 0.45*bounds.h*cos(gAngle),
						center.x + 0.50*bounds.w*sin(gAngle), center.y + 0.50*bounds.h*cos(gAngle));
			}
		}
	};

	/** A dial-type gauge drawn using provided custom images. */
	template <typename NumberType>
	struct CustomImageDialGauge extends GenericDialGauge<NumberType>
	{
		/** Custom gauge background and foreground images. */
		fgeal::Image* background, *foreground;

		/**	Custom pointer image. */
		fgeal::Image* pointer;

		/** An optional offset applied to the pointer in relation to its fixation point. */
		float pointerOffset;

		/** An optional scale factor applied to the pointer's size. Normally (scale=1.0), the pointer size is equal to the gauge radius.*/
		float pointerSizeScale;

		/** If true, indicates that the images used by this gauge are shared, and thus, should not be deleted when this gauge is deleted. */
		bool imagesAreShared;

		/** Creates a dial gauge with custom images. The foreground can be ommited. */
		CustomImageDialGauge(const fgeal::Rectangle& bounds, const NumberType& var, NumberType min, NumberType max, fgeal::Image* background, fgeal::Image* pointerImage, fgeal::Image* foreground=null)
		: GenericDialGauge<NumberType>(bounds, var, min, max),
		  background(background), foreground(null), pointer(pointerImage),
		  pointerOffset(0), pointerSizeScale(1.0),
		  imagesAreShared(false)
		{}

		~CustomImageDialGauge()
		{
			if(not imagesAreShared)
			{
				delete pointer;
				delete background;
				if(foreground != null)
					delete foreground;
			}
		}

		void draw()
		{
			const fgeal::Rectangle& bounds = this->bounds;
			background->drawScaled(bounds.x, bounds.y, bounds.w/background->getWidth(), bounds.h/background->getHeight());
			pointer->drawScaledRotated(bounds.x + 0.5*bounds.w, bounds.y + 0.5*bounds.h + this->fixationOffset,
					0.5*pointerSizeScale*bounds.h/pointer->getHeight(), 0.5*pointerSizeScale*bounds.h/pointer->getHeight(),
					this->getDialAngle(), 0.5*pointer->getWidth(), pointerOffset);

			if(foreground != null)
				foreground->drawScaled(bounds.x, bounds.y, bounds.w/foreground->getWidth(), bounds.h/foreground->getHeight());
		}
	};

	/** A bar-type gauge */
	template <typename NumberType>
	struct BarGauge extends GenericGauge<NumberType>
	{
		// todo bar gauge
	};

	/** A widget that displays a numeric value, possibly stylised. */
	template <typename NumberType>
	struct NumericalDisplay
	{
		// todo numerical display

		/** the value to show. */
		const NumberType& value;
	};
}

#endif /* AUTOMOTIVE_RACE_HUD_HPP_ */
