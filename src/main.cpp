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

#include <tclap/CmdLine.h>

//#define TOPDOWN_EXAMPLE_MODE

#ifdef TOPDOWN_EXAMPLE_MODE
	#include "topdown_example/carse_game.hpp"
#else
	#include "carse_game.hpp"
#endif

#include "fgeal/fgeal.hpp"
#include "futil/string_actions.hpp"
#include "futil/string_split.hpp"

#define CARSE_VERSION_MACRO "0.5-dev"

using std::cout;
using std::endl;
using std::string;

using TCLAP::CmdLine;
using TCLAP::SwitchArg;
using TCLAP::ValueArg;

using fgeal::Image;
using fgeal::Display;
using fgeal::Color;

using futil::starts_with;
using futil::split;

const string CARSE_VERSION = CARSE_VERSION_MACRO,
             programPresentation = "carse - pseudo 3d racing engine";

void runSplash()
{
	Display& display = Display::getInstance();
	Image logoImage("assets/carse_logo.png");
	fgeal::Graphics::drawFilledRectangle(0, 0, display.getWidth(), display.getHeight(), Color::WHITE);
	logoImage.draw(0.5*display.getWidth() - 0.5*logoImage.getWidth(), 0.5*display.getHeight() - 0.5*logoImage.getHeight());
	display.refresh();
	fgeal::rest(0.5);
}

int main(int argc, char** argv)
{
	//configure arguments parser
	CmdLine cmd(programPresentation, ' ', CARSE_VERSION, true);

	//trick to modify the output from --version
	struct MyOutput : public TCLAP::StdOutput { virtual void version(TCLAP::CmdLineInterface& c) {
	cout << "\n" << programPresentation << " - version " << CARSE_VERSION << "\n" << endl;
	}} my_output; cmd.setOutput(&my_output);

	SwitchArg argFullscreen("f", "fullscreen", "Tells carse to go start in fullscreen mode.", false);
	cmd.add(argFullscreen);

	SwitchArg argCentered("c", "centered", "Tells carse to attempt to center the window. Does nothing in fullscreen mode", false);
	cmd.add(argCentered);

	ValueArg<string> argResolution("r", "resolution", "If in windowed mode, tells carse to attempt to create a window of the given size."
			" If in fullscreen mode, tells carse to set the given resolution", false, string(), "<WIDTHxHEIGHT>");
	cmd.add(argResolution);

	cmd.parse(argc, argv);

	int screenWidth = 800, screenHeight = 600;
	if(argResolution.isSet())
	{
		std::vector<string> tokens = split(argResolution.getValue(), 'x');
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
	}

	//greeter :)
	cout << programPresentation << " - version " << CARSE_VERSION << endl;

	try
	{
		fgeal::initialize();

		Display::Options options;
		options.title = "carse";
		options.iconFilename = "assets/carse-icon.png";
		options.fullscreen = argFullscreen.getValue();
		options.width = screenWidth;
		options.height = screenHeight;
		if(argCentered.getValue())
			options.positioning = Display::Options::POSITION_CENTERED;

		Display::create(options);

		runSplash();

		CarseGame().start();

		fgeal::finalize();
	}
	catch(const fgeal::AdapterException& e)
	{
		cout << e.what() << endl;
	}

	return EXIT_SUCCESS;
}
