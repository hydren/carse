/*
 * vector2d.hpp
 *
 *  Created on: 25/06/2014
 *      Author: felipe
 */

#ifndef VECTOR2D_HPP_
#define VECTOR2D_HPP_

struct vector2d
{
	long double x, y;

	vector2d(long double x=0, long double y=0)
	: x(x), y(y) {}

	bool operator ==(const vector2d& v) const;
	bool operator !=(const vector2d& v) const;
	bool equals(const vector2d& v) const;

	vector2d clone() const;

	long double operator ~() const;
	long double magnitude() const;

	vector2d operator !() const;
	vector2d unit() const;
	vector2d& normalize();

	vector2d operator -() const;
	vector2d opposite() const;
	vector2d& operator --();
	vector2d& reflect();
	vector2d& reflectX();
	vector2d& reflectY();

	vector2d operator +(const vector2d& v) const;
	vector2d sum(const vector2d& v) const;
	vector2d& operator +=(const vector2d& v);
	vector2d& add(const vector2d& v);

	vector2d operator -(const vector2d& v) const;
	vector2d difference(const vector2d& v) const;
	vector2d& operator -=(const vector2d& v);
	vector2d& subtract(const vector2d& v);

	vector2d operator *(const long double& factor) const;
	vector2d times(const long double& factor) const;
	vector2d& operator *=(const long double& factor);
	vector2d& scale(const long double& factor);

	vector2d operator ||(const vector2d& v) const;
	vector2d projection(const vector2d& v) const;
	vector2d rejection(const vector2d& v) const;
	vector2d operator |(const vector2d& v) const;
	vector2d reflection(const vector2d& v) const;

	long double operator /(const vector2d& v) const;
	long double distance(const vector2d& v) const;

	long double operator ^(const vector2d& v) const;
	long double innerProduct(const vector2d& v) const;

	vector2d operator <(const long double& radians) const;
	vector2d rotation(const long double& radians) const;
	vector2d& rotate(const long double& radians);
	vector2d& operator <<(const long double& radians);
	vector2d perpendicular() const;

	const vector2d* const getCanonicalBase() const;
	long double* getCoordinates() const;


	/** Represents the null/zero vector. It has coordinates (0, 0). */
	const static vector2d NULL_VECTOR;

	/** A vector codirectional with the X axis, with length 1. */
	const static vector2d X_VERSOR;

	/** A vector codirectional with the Y axis, with length 1. */
	const static vector2d Y_VERSOR;
};

#endif /* VECTOR2D_HPP_ */
