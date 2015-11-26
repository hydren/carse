/*
 * exception.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include "exception.hpp"

#include <cstdio>
#include <cstdarg>

//================== Exception

Exception::Exception(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsprintf(buffer, format, args);
    msg = std::string(buffer);
    va_end(args);
}
