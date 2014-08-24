/*
 * math_ex.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include "math_ex.hpp"

#include <cstdlib>
#include <sstream>

//=================== Math namespace

/** Parses the string for matching an integer. */
int Math::parseInt(const std::string& str)
{
	int i;
	std::istringstream(str.c_str()) >> i;
	return i;
}

float Math::convertToMeters(float pixels)
{
	return 0.01f * pixels;
}

float Math::convertToPixels(float meters)
{
	return 100.0f * meters;
}

int Math::aleatorioEntre(int min, int max)
{
	if(max < min) //se estiver inconsistente, ignore
		return rand();
	else if (max == min)
		return min;
	else
	{
		int x = rand();

		while ((max-min) < RAND_MAX && x >= RAND_MAX - (RAND_MAX % (max-min)))
			x = rand();
		return min + x % (max-min);

		//lento
//		while( x > (max-min) )
//			 x = rand();
//		return min + x;
	}
//	else return min + rand()%(max-min); // enviesado
}
