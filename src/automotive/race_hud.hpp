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
		: GenericGauge(bounds, var, min, max),
		  angleMin(0.25*M_PI), angleMax(1.75*M_PI),
		  fixationOffset(0)
		{}

		protected:
		float getPointerAngle()
		{
			return ((angleMax-angleMin)*value + angleMin*max - angleMax*min)/(max-min);
		}
	};

	/** A needle dial-type gauge, drawn using native primitives. */
	template <typename NumberType>
	struct NeedleDialGauge extends GenericDialGauge<NumberType>
	{
		/** The thickness of the pointer. */
		float thickness;

		void draw()
		{
			// todo needle dial gauge
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
			background->drawScaled(bounds.x, bounds.y, bounds.w/background->getWidth(), bounds.h/background->getHeight());
			pointer->drawScaledRotated(bounds.x + 0.5*bounds.w, bounds.y + 0.5*bounds.h + fixationOffset,
					0.5*bounds.h/pointer->getHeight(), 0.5*bounds.h/pointer->getHeight(),
					this->getPointerAngle(), 0.5*pointer->getWidth(), 0);

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
