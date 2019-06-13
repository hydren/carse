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

#include "futil/string_extra_operators.hpp"
#include "futil/string_actions.hpp"
#include "futil/map_actions.hpp"

#include <vector>
#include <map>
#include <cstdio>
#include <cmath>

#ifndef M_PI
	# define M_PI		3.14159265358979323846	/* pi */
#endif

namespace Hud
{
	/** A generic dial-type gauge. Can drawn either custom images or native primitives.
	 *  To drawn using custom images, the image's fields must be non-null. Otherwise native versions will be drawn. */
	struct GenericDialGauge
	{
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

		/** The thickness of the pointing needle (built-in pointer). */
		float needleThickness;

		/** The needle's color (built-in pointer). */
		fgeal::Color needleColor;

		/** The radius the needle's "bolt" (built-in pointer). */
		float boltRadius;

		/** The bolt's color (built-in pointer). */
		fgeal::Color boltColor;

		/** The graduation's color. */
		fgeal::Color graduationColor;

		/** The font used to display graduation values. If null, no graduation values are shown. */
		fgeal::Font* graduationFont;

		/** an optional, relative offset, from the center, to the position of graduation values, in the range ]0, 1] */
		float graduationValuePositionOffset;

		/** the (non-zero) size of the line of each graduation level, in the range ]0, 1] */
		float graduationPrimaryLineSize, graduationSecondaryLineSize, graduationTertiaryLineSize;

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

		struct Line { float x1, y1, x2, y2; Line(float x1, float y1, float x2, float y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}  };

		struct NumericGraduation { std::string str; float x, y; NumericGraduation(std::string str, float x, float y) : str(str), x(x), y(y) {} };

		private:

		/* A vector of cached coordinates to be drawn. It's necessary to call compile() to update this cache.  */
		std::vector<Line> graduationPrimaryCache, graduationSecondaryCache, graduationTertiaryCache;

		/* A vector of cached numbers and its coordinates to be drawn. It's necessary to call compile() to update this cache. */
		std::vector<NumericGraduation> graduationPrimaryNumericCache;

		/* The scales of the background, foreground and pointer images */
		fgeal::Vector2D backgroundImageScale, foregroundImageScale, pointerImageScale;

		public:

		GenericDialGauge(const fgeal::Rectangle& bounds=fgeal::Rectangle());
		~GenericDialGauge();

		void compile(float min, float max, float graduationPrimarySize, float graduationSecondarySize, float graduationTertiarySize, float graduationValueOffset);
		void draw(float angle);
		void drawBackground();
	};

	/** A value-typed version of the GenericDialGauge. It holds value parameters and a reference to the main value to measure.
	 *  Note that the type must be able to be cast to float, support arithmetic operations and support comparations. */
	template <typename NumberType>
	struct DialGauge extends public GenericDialGauge
	{
		/** the value to measure. */
		const NumberType& value;

		/** the minimum and maximum expected values. */
		NumberType min, max;

		/** the grade's size. */
		NumberType graduationPrimarySize, graduationSecondarySize, graduationTertiarySize;

		/** an optional value that is offset'd from each graduation interval (except the first). */
		NumberType graduationValueOffset;

		public:

		DialGauge(const NumberType& var, NumberType min=NumberType(), NumberType max=NumberType(), const fgeal::Rectangle& bounds=fgeal::Rectangle())
		: GenericDialGauge(bounds),
		  value(var), min(min), max(max),
		  graduationPrimarySize(), graduationSecondarySize(), graduationTertiarySize(), graduationValueOffset(0)
		{
			setRecommendedGraduationSizes();
		}

		void setRecommendedGraduationSizes()
		{
			graduationPrimarySize = 0.1*(max-min);
			graduationSecondarySize = 0.01*(max-min);
			graduationTertiarySize = 0.001*(max-min);
		}

		void compile()
		{
			GenericDialGauge::compile(min, max, graduationPrimarySize, graduationSecondarySize, graduationTertiarySize, graduationValueOffset);
		}

		void draw()
		{
			GenericDialGauge::draw(-((angleMax-angleMin)*value + angleMin*max - angleMax*min)/(max-min));
		}
	};

	// todo improve BarGauge with more options
	/** A generic bar-type gauge */
	struct GenericBarGauge
	{
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

		GenericBarGauge(const fgeal::Rectangle& bounds=fgeal::Rectangle());
		void draw(float fillRatio);
	};

	/** A value-typed version of the GenericBarGauge class. It holds value parameters and a reference to the main value to measure.
	 *  Note that the type must be able to be cast to float, support arithmetic operations and support comparations */
	template <typename NumberType>
	struct BarGauge extends public GenericBarGauge
	{
		/** the value to measure. */
		const NumberType& value;

		/** the minimum and maximum expected values. */
		NumberType min, max;

		BarGauge(const NumberType& var, NumberType min=NumberType(), NumberType max=NumberType(), const fgeal::Rectangle& bounds=fgeal::Rectangle())
		: GenericBarGauge(bounds),
		  value(var), min(min), max(max)
		{}

		void draw()
		{
			GenericBarGauge::draw((std::min(value, max)-min)/(max-min));
		}
	};

	/** A widget that displays a numeric/text value, possibly stylised. */
	struct GenericNumericalDisplay
	{
		/** An optional scaling factor for the value shown. */
		float valueScale;

		/** this widget's dimensions and position. */
		fgeal::Rectangle bounds;

		/** the minimum spacing between the border and the text. */
		fgeal::Vector2D padding;

		/** The background color. */
		fgeal::Color backgroundColor;

		/** If true, no background is drawn at all (border is also not drawn). */
		bool disableBackground;

		/** The border's thickness. If 0 (zero), no border is drawn. */
		float borderThickness;

		/** The border's color. */
		fgeal::Color borderColor;

		/** The display's color. */
		fgeal::Color displayColor;

		/** The font used to display the value. */
		fgeal::Font* font;

		/** If true, indicates that the font is shared, and thus, should not be deleted when this display is deleted. */
		bool fontIsShared;

		GenericNumericalDisplay(const fgeal::Rectangle& bounds=fgeal::Rectangle(), fgeal::Font* font=null);
		~GenericNumericalDisplay();

		void draw(const std::string& valueStr);
		void pack(unsigned char digitCount=1);
	};

	/** A value-typed version of the GenericNumericalDisplay class. Note that the type must be able to be cast to int.
	 *  Also, this class has a 32-char max. */
	template <typename NumberType>
	struct NumericalDisplay extends public GenericNumericalDisplay
	{
		/** the value to show. */
		const NumberType& value;

		/** Optional mapping of specific values to its string representations. */
		std::map<NumberType, std::string> specialCases;

		NumericalDisplay(const NumberType& var, const fgeal::Rectangle& bounds=fgeal::Rectangle(), fgeal::Font* font=null)
		: GenericNumericalDisplay(bounds, font),
		  value(var)
		{}

		void draw()
		{
			if(specialCases.count(value))
				GenericNumericalDisplay::draw(specialCases[value]);
			else
			{
				static char buffer[32];
				sprintf(buffer, "%d", static_cast<int>(value*valueScale));
				GenericNumericalDisplay::draw(buffer);
			}
		}

		inline void packToValue(NumberType maxValue)
		{
			pack(maxValue == 0? 1 : log10(maxValue*valueScale) + 1);
		}
	};

	/** A widget that displays a time, possibly stylised. The variable is expected to be in millisec. If not, assign a proper value scale to match. */
	template<typename TimeType>
	struct TimerDisplay extends public NumericalDisplay<TimeType>
	{
		TimerDisplay(const TimeType& var, const fgeal::Rectangle& bounds=fgeal::Rectangle(), fgeal::Font* font=null)
		: NumericalDisplay<TimeType>(var, bounds, font),
		  showMillisec(false)
		{}

		bool showMillisec;

		void draw()
		{
			if(not this->disableBackground)
			{
				fgeal::Graphics::drawFilledRectangle(this->bounds.x, this->bounds.y, this->bounds.w, this->bounds.h, this->borderColor);
				fgeal::Graphics::drawFilledRectangle(this->bounds.x + 0.5*this->borderThickness, this->bounds.y + 0.5*this->borderThickness,
						this->bounds.w - this->borderThickness, this->bounds.h - this->borderThickness, this->backgroundColor);
			}

			int timeMs = this->value*this->valueScale;
			int timeSec = timeMs/1000; timeMs -= timeSec*1000;
			int timeMin = timeSec/60; timeSec -= timeMin*60;
			static char buffer[32];
			sprintf(buffer, "%02d:%02d:%03d", timeMin, timeSec, timeMs);
			this->font->drawText(buffer, this->bounds.x + this->borderThickness, this->bounds.y + 0.5*this->borderThickness, this->displayColor);
		}
	};
}

#endif /* AUTOMOTIVE_RACE_HUD_HPP_ */
