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

#define CARSE_VERSION "0.4.0-dev"

using std::cout;
using std::endl;
using std::string;

using fgeal::Image;
using fgeal::Display;
using fgeal::Color;


void runSplash()
{
	Display& display = Display::getInstance();
	Image logoImage("carse_logo.png");
	Image::drawRectangle(Color::WHITE, 0, 0, display.getWidth(), display.getHeight());
	logoImage.draw(0.5*display.getWidth() - 0.5*logoImage.getWidth(), 0.5*display.getHeight() - 0.5*logoImage.getHeight());
	display.refresh();
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
		Display::create(1024, 768, string("carse ")+ CARSE_VERSION + " alpha");
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
