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
	template <typename NumberType>
	struct GenericGauge
	{
		// this widget's dimensions and position.
		fgeal::Rectangle bounds;

		// the value to measure, the minimum and maximum expected values.
		const NumberType& value;

		// the minimum and maximum expected values.
		NumberType min, max;

		GenericGauge(const fgeal::Rectangle& bounds, const NumberType& var, NumberType min, NumberType max)
		: bounds(bounds),
		  value(var), min(min), max(max)
		{}
	};

	template <typename NumberType>
	struct DialGauge extends GenericGauge<NumberType>
	{
		// the minimum and maximum angle applied on the rotating pointer.
		float angleMin, angleMax;

		// specifies the offset between the pointer's fixation point and its endpoint.
		float fixationOffset;

		DialGauge(const fgeal::Rectangle& bounds, const NumberType& var, NumberType min, NumberType max)
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

	template <typename NumberType>
	struct NeedleDialGauge extends DialGauge<NumberType>
	{
		void draw()
		{
			// todo needle dial gauge
		}
	};

	template <typename NumberType>
	struct CustomImageDialGauge extends DialGauge<NumberType>
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

	template <typename NumberType>
	struct BarGauge extends GenericGauge<NumberType>
	{
		// todo bar gauge
	};

	template <typename NumberType>
	struct NumericGauge extends GenericGauge<NumberType>
	{
		// todo numeric gauge
	};
}

#endif /* AUTOMOTIVE_RACE_HUD_HPP_ */
