/*
 * main.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include <cstdlib>
#include "util.hpp"
#include "game_engine.hpp"

#define VERSION 0.2

using GameEngine::Image;
using GameEngine::Display;

int main(int argc, char** argv)
{
	try
	{
		GameEngine::initialize();
		GameEngine::display = new Display(640, 480, "Project SpeedME - CARSE Engine");

		Image loading_image("carse-logo.jpg");
		loading_image.draw();
		GameEngine::display->refresh();

		Image car("car-delorean-dmc12.png");
		for(int i = 20; i < 200; i++)
		{
			GameEngine::display->clear();

			car.draw_rotated(i, i, 23, 48, (3.0/4.0)*Math::PI, 0, 0);
			GameEngine::rest(0.1);

			GameEngine::display->refresh();
		}

		GameEngine::rest(2);

		delete GameEngine::display;
		GameEngine::finalize();
	}
	catch(Exception& e)
	{
		cout << e.message() << endl;
	}

	return EXIT_SUCCESS;
}


