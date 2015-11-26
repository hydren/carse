/*
 * b2_ex.hpp
 *
 *  Created on: 28/08/2014
 *      Author: felipe
 */

#ifndef B2MATH_EX_HPP_
#define B2MATH_EX_HPP_

#include <Box2D/Box2D.h>

b2Vec2 b2Unit(const b2Vec2& v)
{
	b2Vec2 u (v.x, v.y);
	u.Normalize();
	return u;
}


#endif /* B2MATH_EX_HPP_ */
