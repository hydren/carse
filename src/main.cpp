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
		GameEngine::display = new Display(640, 480, "carse");

		Image loading_image("carse-logo.jpg");
		loading_image.draw();
		GameEngine::display->refresh();
		GameEngine::rest(0.5);

		Image car("car-delorean-dmc12.png");

		GameEngine::EventQueue eventQueue;
		GameEngine::Event* ev;

		int posx=20, posy=20;
		bool running = true;
		do
		{
			while(!eventQueue.isEmpty())
			{
				ev = eventQueue.waitForEvent();

				if(ev->getEventType() == GameEngine::Event::Type::DISPLAY_CLOSURE)
				{
					running=false;
				}

			}

			GameEngine::display->clear();

			car.draw_rotated(posx, posy, 23, 48, (5.0/4.0)*Math::PI, 0, 0);
			GameEngine::rest(0.1);
			GameEngine::display->refresh();

		}while(running);

		delete GameEngine::display;
		GameEngine::finalize();
	}
	catch(Exception& e)
	{
		cout << e.message() << endl;
	}

	return EXIT_SUCCESS;
}


