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

		/** an optional value that is offset'd from each graduation interval (except the first). */
		NumberType graduationValueOffset;

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

		DialGauge(const NumberType& var, NumberType min=NumberType(), NumberType max=NumberType(), const fgeal::Rectangle& bounds=fgeal::Rectangle())
		: value(var), min(min), max(max),
		  graduationPrimarySize(), graduationSecondarySize(), graduationTertiarySize(),
		  bounds(bounds), angleMin(0.25*M_PI), angleMax(1.75*M_PI),
		  backgroundColor(fgeal::Color::WHITE),
		  borderThickness(2.0f), borderColor(fgeal::Color::BLACK),
		  needleThickness(2.0f), needleColor(fgeal::Color::RED),
		  boltRadius(16.0f), boltColor(fgeal::Color::BLACK),
		  graduationColor(fgeal::Color::BLACK), graduationFont(null), graduationValuePositionOffset(0.35f),
		  graduationPrimaryLineSize(0.5f), graduationSecondaryLineSize(0.3f), graduationTertiaryLineSize(0.2f),
		  graduationValueScale(1.0f), graduationValueOffset(0), graduationLevel(1),
		  fixationOffset(0), pointerOffset(0), pointerSizeScale(1.0f),
		  backgroundImage(null), foregroundImage(null), pointerImage(null), imagesAreShared(false)
		{
			setRecommendedGraduationSizes();
		}

		~DialGauge()
		{
			if(not imagesAreShared)
			{
				if(pointerImage != null)    delete pointerImage;
				if(backgroundImage != null) delete backgroundImage;
				if(foregroundImage != null) delete foregroundImage;
			}
		}

		void setRecommendedGraduationSizes()
		{
			graduationPrimarySize = 0.1*(max-min);
			graduationSecondarySize = 0.01*(max-min);
			graduationTertiarySize = 0.001*(max-min);
		}

		void compile()
		{
			const fgeal::Point center = {bounds.x + 0.5f*bounds.w, bounds.y + 0.5f*bounds.h};

			graduationPrimaryCache.clear();
			graduationPrimaryNumericCache.clear();
			graduationSecondaryCache.clear();
			graduationTertiaryCache.clear();

			if(graduationLevel >= 1)  // primary graduation
			{
				bool offsetApplied = false;
				for(NumberType g = min; g <= max; g += graduationPrimarySize)
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
			for(NumberType g = min; g <= max; g += graduationSecondarySize)
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
			for(NumberType g = min; g <= max; g += graduationTertiarySize)
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

		void drawBackground()
		{
			const fgeal::Point center = {bounds.x + 0.5f*bounds.w, bounds.y + 0.5f*bounds.h};

			if(backgroundImage != null)
				backgroundImage->drawScaled(bounds.x, bounds.y, backgroundImageScale.x, backgroundImageScale.y);
			else
			{
				fgeal::Graphics::drawFilledEllipse(center.x, center.y, 0.5*bounds.w, 0.5*bounds.h, borderColor);
				fgeal::Graphics::drawFilledEllipse(center.x, center.y, 0.5*(bounds.w-borderThickness), 0.5*(bounds.h-borderThickness), backgroundColor);
			}

			if(graduationLevel >= 1)  // primary graduation
			for(unsigned i = 0; i < graduationPrimaryCache.size(); i++)
			{
				const Line& line = graduationPrimaryCache[i];
				fgeal::Graphics::drawLine(line.x1, line.y1, line.x2, line.y2, graduationColor);
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
				fgeal::Graphics::drawLine(line.x1, line.y1, line.x2, line.y2, graduationColor);
			}

			if(graduationLevel >= 3)  // tertiary graduation
			for(unsigned i = 0; i < graduationTertiaryCache.size(); i++)
			{
				const Line& line = graduationTertiaryCache[i];
				fgeal::Graphics::drawLine(line.x1, line.y1, line.x2, line.y2, graduationColor);
			}
		}

		void draw()
		{
			this->drawBackground();

			const fgeal::Point center = {bounds.x + 0.5f*bounds.w, bounds.y + 0.5f*bounds.h};
			const float angle = -((angleMax-angleMin)*value + angleMin*max - angleMax*min)/(max-min);

			if(pointerImage != null)
				pointerImage->drawScaledRotated(bounds.x + 0.5*bounds.w, bounds.y + 0.5*bounds.h + fixationOffset, pointerImageScale.x, pointerImageScale.y, angle, 0.5*pointerImage->getWidth(), pointerOffset);

			else  // draw built-in, gfx-primitives-based needle as pointer
			{
				fgeal::Graphics::drawLine(
						center.x + pointerOffset*sin(angle), center.y + pointerOffset*cos(angle) + fixationOffset,
						center.x + 0.4*(pointerSizeScale*bounds.w+pointerOffset)*sin(angle), center.y + 0.4*(pointerSizeScale*bounds.h+pointerOffset)*cos(angle), needleColor);
				fgeal::Graphics::drawFilledEllipse(center.x, center.y, 0.5*boltRadius*bounds.w/bounds.h, 0.5*boltRadius*bounds.h/bounds.w, boltColor);
			}

			if(foregroundImage != null)
				foregroundImage->drawScaled(bounds.x, bounds.y, foregroundImageScale.x, foregroundImageScale.y);
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

		BarGauge(const NumberType& var, NumberType min=NumberType(), NumberType max=NumberType(), const fgeal::Rectangle& bounds=fgeal::Rectangle())
		: value(var), min(min), max(max),
		  bounds(bounds),
		  backgroundColor(fgeal::Color::WHITE),
		  borderThickness(2.0f), borderColor(fgeal::Color::BLACK),
		  fillColor(fgeal::Color::RED)
		{}

		void draw()
		{
			const float fillRatio = (std::min(value, max)-min)/(max-min);
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
	};

	/** A widget that displays a numeric value, possibly stylised. Note: 32-char max; value will be cast to int. */
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

		/** Optional mapping of specific values to its string representations. */
		std::map<NumberType, std::string> specialCases;

		protected:

		/** A buffer to convert value to string. */
		char stringBuffer[32];

		public:

		NumericalDisplay(const NumberType& var, const fgeal::Rectangle& bounds=fgeal::Rectangle(), fgeal::Font* font=null)
		: value(var), valueScale(1.0), bounds(bounds),
		  backgroundColor(fgeal::Color::WHITE), disableBackground(false),
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
			if(not disableBackground)
			{
				fgeal::Graphics::drawFilledRectangle(bounds.x, bounds.y, bounds.w, bounds.h, borderColor);
				fgeal::Graphics::drawFilledRectangle(bounds.x + borderThickness, bounds.y + borderThickness,
												   bounds.w - 2*borderThickness, bounds.h - 2*borderThickness, backgroundColor);
			}

			if(specialCases.count(value))
				font->drawText(specialCases[value], bounds.x + borderThickness, bounds.y + borderThickness, displayColor);
			else
			{
				sprintf(stringBuffer, "%d", static_cast<int>(value*valueScale));
				font->drawText(std::string(stringBuffer), bounds.x + borderThickness, bounds.y + borderThickness, displayColor);
			}

		}
	};

	/** A widget that displays a time, possibly stylised. The variable is expected to be in millisec. If not, assign a proper value scale to match. */
	template<typename TimeType>
	struct TimerDisplay extends NumericalDisplay<TimeType>
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
			sprintf(this->stringBuffer, "%02d:%02d:%03d", timeMin, timeSec, timeMs);
			this->font->drawText(std::string(this->stringBuffer), this->bounds.x + this->borderThickness, this->bounds.y + 0.5*this->borderThickness, this->displayColor);
		}
	};
}

#endif /* AUTOMOTIVE_RACE_HUD_HPP_ */
