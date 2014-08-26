/*
 * main.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include <cstdlib>
#include "util.hpp"
#include "game_engine.hpp"
#include "race.hpp"

#define VERSION 0.2.3

using GameEngine::Image;
using GameEngine::Display;

int main(int argc, char** argv)
{
	try
	{
		GameEngine::initialize();
		GameEngine::display = new Display(640, 480, "carse");

		Image loading_image("carse-logo.jpg");
		loading_image.draw();
		GameEngine::display->refresh();
		GameEngine::rest(0.5);

		Race race;
		race.start();

		delete GameEngine::display;
		GameEngine::finalize();
	}
	catch(Exception& e)
	{
		cout << e.message() << endl;
	}

	return EXIT_SUCCESS;
}


