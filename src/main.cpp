/*
 * main.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include <iostream>
#include <string>
#include <stdexcept>

#include <cstdlib>

#include "racing/race.hpp"

#include "fgeal.hpp"

#define CARSE_VERSION "0.3.0"

using std::cout;
using std::endl;
using std::string;
using std::exception;

using fgeal::Image;
using fgeal::Display;

int main(int argc, char** argv)
{
	try
	{
		fgeal::initialize();
		atexit(fgeal::finalize);

		fgeal::display = new Display(800, 600, string("carse ")+ CARSE_VERSION);

		Image loading_image("carse-logo.jpg");
		loading_image.draw();
		fgeal::display->refresh();
		fgeal::rest(0.5);

		Race race;
		race.start();

		delete fgeal::display;
	}
	catch(exception& e)
	{
		cout << e.what() << endl;
	}

	return EXIT_SUCCESS;
}


