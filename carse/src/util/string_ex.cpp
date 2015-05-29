/*
 * string_ex.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include "string_ex.hpp"

#include <sstream>

// ====================  adding almost java-like habilities to String

/** Returns a copy of the string, with leading and trailing whitespace omitted. */
string String::trim(const string& str)
{
	string str2 = str;
	size_t found;
	found = str2.find_last_not_of(" ");
	if (found != string::npos)
	  	str2.erase(found+1);
	else
	   	str2.clear();

    return str2;
}

/** Returns true if the given string str ends with the given string ending */
bool String::endsWith (string const& str, string const& ending)
{
    if (str.length() >= ending.length()) {
        return (0 == str.compare (str.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

//just to reduce redundant text
#define CONCATENATION { std::stringstream ss; ss << a << b; return ss.str(); }

string operator + (string a, string b) CONCATENATION;
string operator + (string a, int b) CONCATENATION;
string operator + (string a, long b) CONCATENATION
string operator + (string a, short b) CONCATENATION;
string operator + (string a, unsigned b) CONCATENATION;
string operator + (string a, float b) CONCATENATION;
string operator + (string a, double b) CONCATENATION;
string operator + (string a, char b) CONCATENATION;
string operator + (string a, char* b) CONCATENATION;

//backwards versions

string operator + (int a, string b) CONCATENATION;
string operator + (long a, string b) CONCATENATION;
string operator + (short a, string b) CONCATENATION;
string operator + (unsigned a, string b) CONCATENATION;
string operator + (float a, string b) CONCATENATION;
string operator + (double a, string b) CONCATENATION;
string operator + (char a, string b) CONCATENATION;
string operator + (char* a, string b) CONCATENATION;
