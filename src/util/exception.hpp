/*
 * exception.hpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <string>

/** Class created to behave like the Java's Exception
 * Contains a string messsage with the error message
 */
class Exception
{
    private:

    std::string msg;

    public:

    Exception(std::string str)
    : msg(str)
    {}

    Exception(const char* format, ...);

    inline std::string message() const { return msg; }
};



#endif /* EXCEPTION_HPP_ */
