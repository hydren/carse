/*
 * math_ex.hpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#ifndef MATH_EX_HPP_
#define MATH_EX_HPP_

#include <string>

namespace Math
{
	const double PI = 3.14159265358979323846;

	inline int abs(int i)
	{
		return i < 0 ? -i : i;
	}

	/** Parses the string for matching an integer. */
	int parseInt(const std::string& str);

	float convertToMeters(float pixels);

	float convertToPixels(float meters);

	/**
	 * Retorna um numero aleatorio entre 'min' (inclusive) e 'max' (exclusive). Se max < min, retorna um numero aleatorio entre 0 e MAX_INT.
	 * */
	int aleatorioEntre(int min, int max);

	template<typename T>
	T max(T a, T b)
	{
		return (a > b)? a : b;
	}

	template<typename T>
	T min(T a, T b)
	{
		return (a < b)? a : b;
	}
}



#endif /* MATH_EX_HPP_ */
