/*
 * box2d_util.hpp
 *
 *  Created on: 25 de out de 2016
 *      Author: Felipe
 */

#ifndef UTIL_BOX2D_UTIL_HPP_
#define UTIL_BOX2D_UTIL_HPP_

#include <Box2D/Box2D.h>

#ifndef M_PI
	#define M_PI		3.14159265358979323846
#endif

b2Vec2 b2Unit(const b2Vec2& v);

float convertToMeters(float pixels);
float convertToPixels(float meters);

struct Rect
{
	float x, y, w, h;
};

#endif /* UTIL_BOX2D_UTIL_HPP_ */
