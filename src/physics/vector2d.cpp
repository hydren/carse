/*
 * vector2d.cpp
 *
 *  Created on: 28/06/2014
 *      Author: felipe
 */

#include "vector2d.hpp"

#include <cmath>

/** Represents the null/zero vector. It has coordinates (0, 0). */
const vector2d vector2d::NULL_VECTOR (0, 0);

/** A vector codirectional with the X axis, with length 1. */
const vector2d vector2d::X_VERSOR (1, 0);

/** A vector codirectional with the Y axis, with length 1. */
const vector2d vector2d::Y_VERSOR (0, 1);

bool vector2d::operator ==(const vector2d& v) const
{
	return v.x == x && v.y == y;
}

bool vector2d::operator !=(const vector2d& v) const
{
	return not (*this == v);
}

/** Returns true if the given vector coordinates equals the ones from this */
bool vector2d::equals(const vector2d& vector) const
{
	return *this == vector;
}
/**
 * Creates and returns a copy of this vector.
 * @return a copy of this vector.
 */
vector2d vector2d::clone() const
{
	return vector2d(x, y);
}

long double vector2d::operator ~() const
{
	return sqrt( x*x + y*y );
}

/**
@return the length/magnitude of this vector. */
long double vector2d::magnitude() const
{
	return ~(*this);
}

vector2d vector2d::operator !() const
{
	return *this==NULL_VECTOR? NULL_VECTOR.clone() : vector2d(x/magnitude(), y/magnitude());
}

/** Creates a vector with length 1 and same direction as this vector. In other words, a new vector that is a normalized version of this vector. Note that the original vector remains unchanged.
@return a new vector2d instance with length 1 and same direction as this vector. */
vector2d vector2d::unit() const
{
	return !(*this);
}

/** Divides this vector's coordinates by its length/magnitude, normalizing it.
<br> The returned object is the vector instance itself after normalization.
@return this vector normalized. */
vector2d& vector2d::normalize()
{
	if(*this!=NULL_VECTOR)
	{
		long double length = magnitude();
		x /= length;
		y /= length;
	}
	return *this;
}

vector2d vector2d::operator -() const
{
	return vector2d(-x, -y);
}

/** Creates the opposite of this vector. In other words, returns a vector with same coordinates as this, but with changed signal. Note that the original vector remains unchanged.
@return a new vector2d instance with opposite direction.*/
vector2d vector2d::opposite() const
{
	return -(*this);
}

vector2d& vector2d::operator --()
{
	x = -x;
	y = -y;
	return *this;
}

/** Changes the signal of this vector coordinates, effectively reflecting it.
<br> The returned object is the vector instance itself after reflection.
@return this vector after reflection. */
vector2d& vector2d::reflect()
{
	return --*this;
}

vector2d& vector2d::reflectX()
{
	x = -x;
	return *this;
}

vector2d& vector2d::reflectY()
{
	y = -y;
	return *this;
}

vector2d vector2d::operator +(const vector2d& v) const
{
	return vector2d(x + v.x, y + v.y);
}

/** Creates a vector that represents the sum of this vector and the given vector. Note that the original vector remains unchanged.
@param v the summand vector instance
@return a new vector2d instance that is the sum of this vector and the given vector, when applicable.*/
vector2d vector2d::sum(const vector2d& v) const
{
	return *this + v;
}

vector2d& vector2d::operator +=(const vector2d& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

/** Adds to this vector the given vector. In other words, it performs an addition to this vector coordinates.
 <br> The returned object is the vector instance itself after summation.
@param v the summand vector instance
@return this vector after summation with the given vector. */
vector2d& vector2d::add(const vector2d& v)
{
	return *this += v;
}

vector2d vector2d::operator -(const vector2d& v) const
{
	return vector2d(x - v.x, y - v.y);
}

/** Creates a vector that represents the difference/displacement of this vector and the given vector, in this order. It's useful to remember that vector subtraction is not commutative: a-b != b-a.
<br> Note that the original vector remains unchanged.
@param v the subtrahend vector instance
@return a new vector2d instance that is the difference between this vector and the given vector*/
vector2d vector2d::difference(const vector2d& v) const
{
	return sum(((vector2d) v).opposite());
}

vector2d& vector2d::operator -=(const vector2d& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

/** Subtracts from this vector the given vector. In other words, it performs an subtraction to this vector coordinates.
It's useful to remember that vector subtraction is not commutative: a-b != b-a.
<br> The returned object is the the vector instance itself after subtraction.
@param v the subtrahend vector instance
@return this vector after subtraction*/
vector2d& vector2d::subtract(const vector2d& v)
{

	return *this -= v;
}

vector2d vector2d::operator *(const long double& factor) const
{
	return vector2d(x * factor, y * factor);
}

/** Creates a vector that represents the scalar multiplication of this vector by the given factor. Note that the original vector remains unchanged.
@param factor the factor from which multiply by
@return a new vector2d instance that is this vector multiplied by the given factor */
vector2d vector2d::times(const long double& factor) const
{
	return *this * factor;
}

vector2d& vector2d::operator *=(const long double& factor)
{
	x *= factor;
	y *= factor;
	return *this;
}

/** Multiply this vectors coordinates by the given factor. The returned object is the vector instance itself after multiplication.
@param factor the factor from which multiply by
@return this vector after multiplication*/
vector2d& vector2d::scale(const long double& factor)
{
	return *this *= factor;
}

vector2d vector2d::operator ||(const vector2d& v) const
{
	return (*this) * (((*this)^v)/(v^v));
}

/** Creates a vector that represents the projection of this vector on the given vector v.
 @param vector to project on
 @return a new vector that represents the projection of this vector on the given vector v
 */
vector2d vector2d::projection(const vector2d& v) const
{
	return (*this) || v;
}

/** Creates a vector that represents the rejection of this vector on the given vector v. The rejection is defined as rej(u, v) = u - proj(u, v)
 @param vector to project on
 @return a new vector that represents the rejection of this vector on the given vector v
 */
vector2d vector2d::rejection(const vector2d& v) const
{
	return (*this) - ((*this) || v);
}

vector2d vector2d::operator |(const vector2d& v) const
{
	return (*this) - (this->rejection(v)*2);
}

/** Creates a vector that represents the reflection of this vector in the axis represented by the given vector v. */
vector2d vector2d::reflection(const vector2d& v) const
{
	return (*this)|v;
}

long double vector2d::operator /(const vector2d& v) const
{
	return ~(*this-v);
}

/** Compute the distance between this vector and the given vector. In other words, returns the length/magnitude of the displacement between this and the given vector.
@param v a vector from which to compute the distance.
@return the distance between this vector and the given vector*/
long double vector2d::distance(const vector2d& vector) const
{
	return difference(vector).magnitude();
}

long double vector2d::operator ^(const vector2d& v) const
{
	return x*v.x + y*v.y;
}

/** Compute the inner/dot product between this and the given vector.
@param v a vector from which to compute the inner product
@return the inner/dot product of this and the given vector*/
long double vector2d::innerProduct(const vector2d& v) const
{
	return *this ^ v;
}

vector2d vector2d::perpendicular() const
{
	return rotation(M_PI);
}

vector2d vector2d::operator <(const long double& radians) const
{
	return vector2d(x*cos(radians) - y*sin(radians), x*sin(radians) - y*cos(radians));
}

vector2d vector2d::rotation(const long double& radians) const
{
	return (*this)<radians;
}

vector2d& vector2d::operator <<(const long double& radians)
{
	long double
	nx = x*cos(radians) - y*sin(radians),
	ny = x*sin(radians) - y*cos(radians);
	x = nx; y = ny;
	return *this;
}

vector2d& vector2d::rotate(const long double& radians)
{
	return (*this) << radians;
}

/**
 * Creates an array of unit vectors corresponding to the canonical base from which this vector2d is based.
 @return An array of versors in the order from which this vector2d coordinates are represented.
 */
const vector2d* const vector2d::getCanonicalBase() const
{
	vector2d* v = new vector2d[2];
	v[0] = X_VERSOR;
	v[1] = Y_VERSOR;
	return v;
}

/** Creates an array with this vector2ds coordinates.
@return An array with this vector2ds coordinates in correct order.
*/
long double* vector2d::getCoordinates() const
{
	long double* v = new long double[2];
	v[0] = x;
	v[1] = y;
	return v;
}

