/*
 * box2d_util.cpp
 *
 *  Created on: 25 de out de 2016
 *      Author: Felipe
 */

#include "box2d_util.hpp"

b2Vec2 b2Unit(const b2Vec2& v)
{
	b2Vec2 u (v.x, v.y);
	u.Normalize();
	return u;
}
