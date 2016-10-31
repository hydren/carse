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

using fgeal::Image;
using fgeal::Display;


void runSplash()
{
	Image loading_image("carse-logo.jpg");
	loading_image.draw();
	fgeal::display->refresh();
	fgeal::rest(0.5);
}

void runGameTest()
{
	Race race;
	race.run();
}

int main(int argc, char** argv)
{
	try
	{
		fgeal::initialize();
		new Display(800, 600, string("carse ")+ CARSE_VERSION);
		runSplash();
		runGameTest();
		fgeal::finalize();
	}
	catch(const fgeal::AdapterException& e)
	{
		cout << e.what() << endl;
	}

	return EXIT_SUCCESS;
}


