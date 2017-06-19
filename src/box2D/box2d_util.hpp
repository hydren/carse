/*
 * box2d_util.hpp
 *
 *  Created on: 25 de out de 2016
 *      Author: Felipe
 */

#ifndef TOPDOWN_BOX2D_UTIL_HPP_
#define TOPDOWN_BOX2D_UTIL_HPP_

#include <Box2D/Box2D.h>

namespace util
{
	float toMeters(float pixels);
	float toPixels(float meters);
	b2Vec2 b2Unit(const b2Vec2& v);
}

#endif /* TOPDOWN_BOX2D_UTIL_HPP_ */
