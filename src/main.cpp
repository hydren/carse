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

//#define TOPDOWN_EXAMPLE_MODE

#ifdef TOPDOWN_EXAMPLE_MODE
	#include "topdown_example/carse_game.hpp"
#else
	#include "carse_game.hpp"
#endif

#include "fgeal/fgeal.hpp"
#include "futil/string_actions.hpp"
#include "futil/string_split.hpp"

#define CARSE_VERSION_MACRO "0.4.9-dev"

using std::cout;
using std::endl;
using std::string;

using fgeal::Image;
using fgeal::Display;
using fgeal::Color;

using futil::starts_with;
using futil::split;

const string CARSE_VERSION = CARSE_VERSION_MACRO;

void runSplash()
{
	Display& display = Display::getInstance();
	Image logoImage("assets/carse_logo.png");
	Image::drawFilledRectangle(0, 0, display.getWidth(), display.getHeight(), Color::WHITE);
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
	int screenWidth = 800, screenHeight = 600;
	bool fullscreen = false, centered=false;
	for(int i = 0; i < argc; i++)
	{
		if(starts_with(string(argv[i]), "-r"))
		{
			if(i+1 < argc)
			{
				string txtResolution(argv[i+1]);
				std::vector<string> tokens = split(txtResolution, 'x');
				if(tokens.size() == 2)
				{
					int customWidth =  atoi(tokens[0].c_str());
					int customHeight = atoi(tokens[1].c_str());
					if(customWidth != 0 and customHeight != 0)
					{
						screenWidth = customWidth;
						screenHeight = customHeight;
					}

				}
				else cout << "Failed to parse custom resolution" << endl;
			}
			else cout << "Missing argument to -r parameter" << endl;
		}

		if(string(argv[i]) == "-f" or string(argv[i]) == "--fullscreen")
			fullscreen = true;

		if(string(argv[i]) == "-c" or string(argv[i]) == "--centered")
			centered = true;
	}

	try
	{
		fgeal::initialize();
		Display::Options options;
		options.title = "carse";
		options.iconFilename = "assets/carse-icon.png";
		options.fullscreen = fullscreen;
		options.width = screenWidth;
		options.height = screenHeight;
		if(centered) options.positioning = Display::Options::POSITION_CENTERED;
		Display::create(options);
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
