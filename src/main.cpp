/*
 * main.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include <cstdlib>
#include "util.hpp"
#include "game_engine.hpp"
#include "racing/race.hpp"

#define VERSION "0.2.7"

using GameEngine::Image;
using GameEngine::Display;

int main(int argc, char** argv)
{
	try
	{
		GameEngine::initialize();
		atexit(GameEngine::finalize);

		GameEngine::display = new Display(800, 600, "carse " VERSION);

		Image loading_image("carse-logo.jpg");
		loading_image.draw();
		GameEngine::display->refresh();
		GameEngine::rest(0.5);

		Race race;
		race.start();

		delete GameEngine::display;
	}
	catch(Exception& e)
	{
		cout << e.message() << endl;
	}

	return EXIT_SUCCESS;
}


