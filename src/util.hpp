/*
 * util.hpp
 *
 *  Created on: Apr 10, 2013
 *      Author: felipe
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_

//utilities, multi-purpose things

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <list>

using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;
using std::map;
using std::list;
using std::pair;

#include "util/exception.hpp"
#include "util/math_ex.hpp"
#include "util/rect.hpp"
#include "util/string_ex.hpp"

// * * * * * Useful macros

/** Automatic bridge macro, to encapsulate the implementation and speed up compilation time. It also cleans the global namespace.
 * This technique is called PImpl idiom, Compiler firewall idiom, handle classes, Cheshire Cat, etc... */
#define encapsulated struct Implementation; Implementation *implementation
#define encapsulation(PARENT_CLASS) struct PARENT_CLASS::Implementation

// Just being hipster
#define abstract =0
#define extends :
#define sets :

// ...questionable choice
#define null NULL
//#define null nullptr

typedef ifstream FileInputStream;
typedef ofstream FileOutputStream;

namespace Util
{
	template <typename Type>
	vector< vector <Type> > transpose(const vector< vector<Type> >& matrix)
	{
		//if empty, return a new empty
		if(matrix.size() == 0)
			return vector< vector<int> >();

		//safety check
		for(unsigned i = 0, size=matrix[0].size(); i < matrix.size(); i++)
			if(matrix[i].size() != size)
				throw Exception("Matrix with differing row sizes! " + matrix[i].size());

		vector< vector<Type> > matrix_t(matrix[0].size(), vector<Type>(matrix.size()));

		for(unsigned i = 0; i < matrix.size(); i++)
			for(unsigned j = 0; j < matrix[i].size(); j++)
				matrix_t[j][i] = matrix[i][j];

		return matrix_t;
	}
}

#endif /* UTIL_HPP_ */
