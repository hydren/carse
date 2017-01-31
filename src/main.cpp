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

#include "carse_game.hpp"

#include "fgeal/fgeal.hpp"

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
	Display::getInstance().refresh();
	fgeal::rest(0.5);
}

void runGameTest()
{
	CarseGame game;
	game.start();
}

int main(int argc, char** argv)
{
	try
	{
		fgeal::initialize();
		Display::create(800, 600, string("carse ")+ CARSE_VERSION + " alpha");
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
