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
	struct DialGauge<NumberType>
	{
		// the value to measure, the minimum and maximum expected values.
		const NumberType& value, min, max;

		// the minimum and maximum angle applied on the rotating pointer.
		float angleMin, angleMax;

		// specifies whether the pointer (the "needle") is drawn using geometric primitives (native) or is a custom image.
		enum PointerType { NATIVE, CUSTOM_IMAGE } pointerType;

		// specifies the pointer shape (if native). there is a separate shape for the tip.
		enum NativePointerShape { NONE, BAR, TRIANGLE, ELLIPSE } nativePointerShape, nativePointerTipShape;

		// custom pointer image, if custom at all. otherwise should be null.
		fgeal::Image* customPointerImage;

		// custom gauge backgroud image. otherwise should be null.
		fgeal::Image* customBackground;

		DialGauge(const NumberType& var, NumberType min, NumberType max)
		: value(var), min(min), max(max),
		  angleMin(0.25*M_PI), angleMax(1.75*M_PI),
		  pointerType(NATIVE), nativePointerShape(BAR), nativePointerTipShape(BAR),
		  customPointerImage(null), customBackground(null)
		{}

		void draw()
		{
		}
	};

	struct BarGauge
	{

	};

	struct NumericGauge
	{

	};
}

#endif /* AUTOMOTIVE_RACE_HUD_HPP_ */
