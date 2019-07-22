/*
 * main.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>

#include <cstdlib>
#include <ctime>

//#define TOPDOWN_EXAMPLE_MODE

#ifdef TOPDOWN_EXAMPLE_MODE
	#include "topdown_example/carse_game.hpp"
#else
	#include "carse_game.hpp"
#endif

#include "race_only_args.hpp"
#include <tclap/CmdLine.h>

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

namespace RaceOnlyArgs
{
	ValueArg<unsigned> raceType("T", "race-type", "When used together with the --race parameter, specifies the race type, represented by its index", false, 0, "integer"),
					   lapCount("L", "lap-count", "When used together with the --race-type parameter, specifies the number of laps of the race (loop race types only).", false, 2, "unsigned integer"),
					   courseIndex("C", "course", "When used in conjunction with the --race parameter, specifies the race course, represented by its index", false, 0, "unsigned integer");
	SwitchArg randomCourse("X", "random-course", "When used in conjunction with the --race parameter, generates and sets a random race course", false),
			  debugMode("D", "debug-mode", "When used in conjunction with the --race parameter, sets a predefined debug race course, in debug mode.", false);

	ValueArg<unsigned> vehicleIndex("V", "vehicle", "When used in conjunction with the --race parameter, specifies the player vehicle, represented by its index", false, 0, "unsigned integer");
	ValueArg<int> vehicleAlternateSpriteIndex("S", "vehicle-alternate-sprite", "When used in conjunction with the --vehicle parameter, specifies the alternate player vehicle sprite, represented by its index", false, -1, "integer");
	ValueArg<unsigned> simulationType("P", "simulation-type", "When used in conjunction with the --race parameter, specifies simulation type, represented by its index", false, 0, "unsigned integer"),
					   hudType("H", "hud", "When used in conjunction with the --race parameter, specifies HUD type, represented by its index", false, 0, "unsigned index");
	SwitchArg imperialUnit("U", "imperial-units", "When used in conjunction with the --race parameter, uses imperial units instead of metric", false);
}

int main(int argc, char** argv)
{
	// configure custom arguments parser
	struct CustomCmdLine extends public CmdLine
	{
		CustomCmdLine() : CmdLine(programPresentation, ' ', CARSE_VERSION, true)
		{
			//trick to modify the output from --version
			struct CustomOutput extends public TCLAP::StdOutput
			{
				virtual void version(TCLAP::CmdLineInterface& c)
				{
					cout << "\n" << programPresentation << " - version " << CARSE_VERSION << "\n" << endl;
				}
			};
			this->setOutput(new CustomOutput());
		}
		//trick to modify the output from --help to print args in reverse order of inclusion
		void reverseArgList() { std::reverse(_argList.begin(), ------_argList.end()); }
	} cmd;

	SwitchArg argFullscreen("f", "fullscreen", "Start in fullscreen mode.", false);
	cmd.add(argFullscreen);

	SwitchArg argCentered("c", "centered", "Attempt to center the window. Does nothing in fullscreen mode", false);
	cmd.add(argCentered);

	ValueArg<string> argResolution("r", "resolution", "If in windowed mode, attempt to create a window of the given size. If in fullscreen mode, attempt to start in the given resolution", false, string(), "<WIDTHxHEIGHT>");
	cmd.add(argResolution);

	ValueArg<float> argMasterVolume("v", "master-volume", "Specifies the master volume, in the range [0-1] (0 being no sound, 1.0 being maximum volume)", false, 0.9f, "decimal");
	cmd.add(argMasterVolume);

	SwitchArg argRace("R", "race", "Skip menus and go straight to race with current vehicle and course.", false);
	cmd.add(argRace);
	cmd.add(RaceOnlyArgs::raceType);
	cmd.add(RaceOnlyArgs::lapCount);
	cmd.add(RaceOnlyArgs::courseIndex);
	cmd.add(RaceOnlyArgs::randomCourse);
	cmd.add(RaceOnlyArgs::debugMode);
	cmd.add(RaceOnlyArgs::vehicleIndex);
	cmd.add(RaceOnlyArgs::vehicleAlternateSpriteIndex);
	cmd.add(RaceOnlyArgs::simulationType);
	cmd.add(RaceOnlyArgs::hudType);
	cmd.add(RaceOnlyArgs::imperialUnit);

	cmd.reverseArgList();
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
		game.logic.raceOnlyMode = argRace.isSet();
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
