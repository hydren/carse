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

		GenericGauge(const fgeal::Rectangle& bounds, const NumberType& var, NumberType min, NumberType max)
		: bounds(bounds),
		  value(var), min(min), max(max)
		{}
	};

	/** A generic dial-type gauge. Also not functional */
	template <typename NumberType>
	struct GenericDialGauge extends GenericGauge<NumberType>
	{
		/** The minimum and maximum angle applied on the rotating pointer. */
		float angleMin, angleMax;

		/** Specifies the offset between the pointer's fixation point and its endpoint. */
		float fixationOffset;

		GenericDialGauge(const fgeal::Rectangle& bounds, const NumberType& var, NumberType min, NumberType max)
		: GenericGauge<NumberType>(bounds, var, min, max),
		  angleMin(0.5*M_PI), angleMax(2.25*M_PI),
		  fixationOffset(0)
		{}

		protected:
		float getDialAngle()
		{
			return ((angleMax-angleMin)*this->value + angleMin*this->max - angleMax*this->min)/(this->max-this->min);
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

		NeedleDialGauge(const fgeal::Rectangle& bounds, const NumberType& var, NumberType min, NumberType max)
		: GenericDialGauge<NumberType>(bounds, var, min, max),
		  backgroundColor(fgeal::Color::WHITE),
		  borderThickness(2.0f), borderColor(fgeal::Color::BLACK),
		  needleThickness(2.0f), needleColor(fgeal::Color::RED),
		  boltRadius(16.0f), boltColor(fgeal::Color::BLACK)
		{}

		void draw()
		{
			// todo needle dial gauge
			const fgeal::Rectangle& bounds = this->bounds;
			const fgeal::Point center = {bounds.x + 0.5*bounds.w, bounds.y + 0.5*bounds.h};
			const float angle = this->getDialAngle();

			fgeal::Image::drawEllipse(borderColor,     center.x, center.y, 0.5*bounds.w, 0.5*bounds.h);
			fgeal::Image::drawEllipse(backgroundColor, center.x, center.y, 0.5*(bounds.w-borderThickness), 0.5*(bounds.h-borderThickness));
			fgeal::Image::drawLine(needleColor,        center.x, center.y, center.x + 0.4*bounds.w*cos(angle), center.y + 0.4*bounds.h*sin(angle));
			fgeal::Image::drawEllipse(boltColor,       center.x, center.y, 0.5*boltRadius*bounds.w/bounds.h, 0.5*boltRadius*bounds.h/bounds.w);
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

		/** If true, indicates that the images used by this gauge are shared, and thus, should not be deleted when this gauge is deleted. */
		bool imagesAreShared;

		/** Creates a dial gauge with custom images. The foreground can be ommited. */
		CustomImageDialGauge(fgeal::Image* background, fgeal::Image* pointerImage, fgeal::Image* foreground=null)
		: background(background),  pointer(pointerImage), foreground(null), imagesAreShared(false)
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
					0.5*bounds.h/pointer->getHeight(), 0.5*bounds.h/pointer->getHeight(),
					this->getDialAngle(), 0.5*pointer->getWidth(), 0);

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
	struct NumericIndicator
	{
		// todo numeric indicator

		/** the value to show. */
		const NumberType& value;
	};
}

#endif /* AUTOMOTIVE_RACE_HUD_HPP_ */
