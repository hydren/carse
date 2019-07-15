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
#include <ctime>

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

const string CARSE_VERSION = "0.5.1-dev",
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

	ValueArg<float> argMasterVolume("v", "master-volume", "Sets the master volume, in the range [0-1] (0 being no sound, 1.0 being maximum volume)", false, -1, "decimal");
	cmd.add(argMasterVolume);

	SwitchArg argRace("R", "race", "Tells carse to go directly start a race with current vehicle and course.", false);
	cmd.add(argRace);

	ValueArg<int> argRaceType("T", "race-type", "When used in conjunction with the --race parameter, tells carse which race type to "
			"run, represented by its index", false, -1, "integer");
	cmd.add(argRaceType);

	ValueArg<unsigned> argLapCount("L", "lap-count", "When used in conjunction with the --race-type parameter with value loop types, "
			"tells carse the number of laps of the race", false, 0, "unsigned integer");
	cmd.add(argLapCount);

	ValueArg<unsigned> argCourse("C", "course", "When used in conjunction with the --race parameter, tells carse to use the given course,"
			" represented by its index", false, 0, "unsigned integer");
	cmd.add(argCourse);

	SwitchArg argRandomCourse("X", "random-course", "When used in conjunction with the --race parameter, tells carse to use a generated"
			" random course", false);
	cmd.add(argRandomCourse);

	SwitchArg argDebugCourse("D", "debug-course", "When used in conjunction with the --race parameter, tells carse to use a predefined"
			" debug course, in debug mode.", false);
	cmd.add(argDebugCourse);

	ValueArg<unsigned> argVehicle("V", "vehicle", "When used in conjunction with the --race parameter, tells carse to use the given vehicle,"
			" represented by its index", false, 0, "unsigned integer");
	cmd.add(argVehicle);

	ValueArg<int> argVehicleAltSprite("S", "vehicle-alternate-sprite", "When used in conjunction with the --vehicle parameter, tells carse to "
			"use the given vehicle alternate sprite, represented by its index", false, -1, "integer");
	cmd.add(argVehicleAltSprite);

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
	}
	catch (const fgeal::AdapterException& e)
	{
		cout << "failed to initialize: " << e.what() << endl;
	}

	Display::Options options;
	options.title = "carse";
	options.iconFilename = "assets/carse-icon.png";
	options.fullscreen = argFullscreen.getValue();
	options.width = screenWidth;
	options.height = screenHeight;
	if(argCentered.getValue())
		options.positioning = Display::Options::POSITION_CENTERED;

	try
	{
		Display::create(options);
	}
	catch (const fgeal::AdapterException& e)
	{
		cout << "failed to open display: " << e.what() << endl;
	}

	try
	{
		runSplash();
		srand(time(null));
		CarseGame game;
		if(argRace.isSet())
		{
			game.logic.raceOnlyMode = true;
			if(argRaceType.isSet())
				game.logic.raceOnlyRaceType = argRaceType.getValue();
			if(argLapCount.isSet())
				game.logic.raceOnlyLapCount = argLapCount.getValue();
			if(argDebugCourse.isSet())
				game.logic.raceOnlyDebug = true;
			else if(argRandomCourse.isSet())
				game.logic.raceOnlyRandomCourse = true;
			else if(argCourse.isSet())
				game.logic.raceOnlyCourseIndex = argCourse.getValue();
			if(argVehicle.isSet())
			{
				game.logic.raceOnlyPlayerVehicleIndex = argVehicle.getValue();
				if(argVehicleAltSprite.isSet())
					game.logic.raceOnlyPlayerVehicleAlternateSpriteIndex = argVehicleAltSprite.getValue();
			}
		}
		if(argMasterVolume.isSet())
		{
			if(argMasterVolume.getValue() < 0.f or argMasterVolume.getValue() > 1.f)
				cout << "volume argument out of the range [1, 0]. ignoring..." << endl;
			else
				game.logic.masterVolume = argMasterVolume.getValue();
		}

		game.start();
	}
	catch(const fgeal::AdapterException& e)
	{
		cout << e.what() << endl;
	}

	try
	{
		fgeal::finalize();
	}
	catch (const fgeal::AdapterException& e)
	{
		cout << "failed to deinitialize: " << e.what() << endl;
	}

	return EXIT_SUCCESS;
}
