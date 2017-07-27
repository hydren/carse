/*
 * vehicle_selection_state.cpp
 *
 *  Created on: 7 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle_selection_state.hpp"

#include "race_state.hpp"
#include "futil/properties.hpp"

#include "futil/string_actions.hpp"

#include <iostream>
using std::cout; using std::endl;

#include <vector>
#include <iomanip>
#include <sstream>

using fgeal::Display;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Font;
using fgeal::Color;
using fgeal::Image;
using fgeal::Rectangle;
using fgeal::Menu;
using futil::Properties;
using futil::ends_with;
using futil::to_string;
using std::vector;
using std::string;

string toStrRounded(float value, unsigned placesCount=1)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(placesCount) << value;
	return ss.str();
}

int VehicleSelectionState::getId() { return Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID; }

VehicleSelectionState::VehicleSelectionState(Pseudo3DCarseGame* game)
: State(*game),
  fontMain(null), fontInfo(null),
  menu(null)
{}

VehicleSelectionState::~VehicleSelectionState()
{
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(menu != null) delete menu;

	for(unsigned i = 0; i < vehiclePreview.size(); i++)
		delete vehiclePreview[i];
}

void VehicleSelectionState::initialize()
{
	Display& display = Display::getInstance();
	Rectangle menuBounds = {0.0625f*display.getWidth(), 0.25f*display.getHeight(), 0.4f*display.getWidth(), 0.5f*display.getHeight()};
	fontMain = new Font("assets/font.ttf", 24);
	fontInfo = new Font("assets/font.ttf", 12);

	menu = new Menu(menuBounds, new Font("assets/font.ttf", 18), Color::WHITE);
	menu->fontIsOwned = true;
	menu->bgColor = Color::AZURE;
	menu->focusedEntryFontColor = Color::NAVY;

	cout << "reading vehicles..." << endl;
	vector<string> vehicleFiles = fgeal::filesystem::getFilenamesWithinDirectory("data/vehicles");
	for(unsigned i = 0; i < vehicleFiles.size(); i++)
	{
		const string& filename = vehicleFiles[i];
		if(fgeal::filesystem::isFilenameDirectory(filename))
		{
			vector<string> subfolderFiles = fgeal::filesystem::getFilenamesWithinDirectory(filename);
			for(unsigned j = 0; j < subfolderFiles.size(); j++)
			{
				const string& subfolderFile = subfolderFiles[j];
				if(ends_with(subfolderFile, ".properties"))
				{
					Properties prop;
					prop.load(subfolderFile);
					vehicles.push_back(Vehicle(prop, static_cast<Pseudo3DCarseGame&>(this->game)));
					cout << "read vehicle: " << subfolderFile << endl;
					break;
				}
			}
		}
		else if(ends_with(filename, ".properties"))
		{
			Properties prop;
			prop.load(filename);
			vehicles.push_back(Vehicle(prop, static_cast<Pseudo3DCarseGame&>(this->game)));
			cout << "read vehicle: " << filename << endl;
		}
	}

	for(unsigned i = 0; i < vehicles.size(); i++)
	{
		menu->addEntry(vehicles[i].name);
		vehiclePreview.push_back(new Image(vehicles[i].sheetFilename));
	}
}

void VehicleSelectionState::onEnter()
{}

void VehicleSelectionState::onLeave()
{}

void VehicleSelectionState::render()
{
	Display& display = Display::getInstance();
	display.clear();
	menu->draw();
	fontMain->drawText("Choose your vehicle", 84, 25, Color::WHITE);

	Image* sheetVehicle = vehiclePreview[menu->getSelectedIndex()];
	Vehicle& vehicle = vehicles[menu->getSelectedIndex()];

	const float scalex = display.getWidth() * 0.0048828125f * vehicle.spriteScale.x,
				scaley = display.getWidth() * 0.0048828125f * vehicle.spriteScale.y,
				posX = 0.7*display.getWidth() - 0.5*vehicle.spriteWidth * scalex,
				posY = 0.35*display.getHeight() - 0.5*vehicle.spriteHeight * scaley,
				offsetY = vehicle.spriteHeight * (vehicle.spriteStateCount/2);

	sheetVehicle->drawScaledRegion(posX, posY, scalex, scaley, Image::FLIP_NONE, 0, offsetY, vehicle.spriteWidth, vehicle.spriteHeight);

	// info sheet
	int sheetX = 0.525*display.getWidth(), sheetY = 0.525*display.getHeight();
	const string engineDesc = (vehicle.body.engine.aspiration.empty()? "" : vehicle.body.engine.aspiration + " ")
							+ (vehicle.body.engine.displacement == 0?  "" : toStrRounded(vehicle.body.engine.displacement/1000.0) + "L ")
							+ (vehicle.body.engine.valvetrain.empty()? "" : vehicle.body.engine.valvetrain + " ")
							+ (vehicle.body.engine.valveCount == 0?    "" : to_string(vehicle.body.engine.valveCount) + "-valve ")
							+ (vehicle.body.engine.configuration.empty()? "" : vehicle.body.engine.configuration);

	fontInfo->drawText("Engine: "+(engineDesc.empty()? "--" : engineDesc), sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText("Power:  " +to_string(vehicle.body.engine.maximumPower) + "hp @" + to_string((int)vehicle.body.engine.maximumPowerRpm)+"rpm", sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText("Torque: " +toStrRounded(vehicle.body.engine.torque) + "Nm @" + to_string((int)vehicle.body.engine.maximumTorqueRpm)+"rpm", sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText(to_string(vehicle.body.gearCount)+"-speed transmission", sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText("Weight: "+to_string(vehicle.body.mass) + "kg", sheetX, sheetY+=12, Color::WHITE);
}

void VehicleSelectionState::update(float delta)
{
	this->handleInput();
}

void VehicleSelectionState::handleInput()
{
	Event event;
	EventQueue& eventQueue = EventQueue::getInstance();
	while(eventQueue.hasEvents())
	{
		eventQueue.getNextEvent(&event);
		if(event.getEventType() == Event::TYPE_DISPLAY_CLOSURE)
		{
			game.running = false;
		}
		else if(event.getEventType() == Event::TYPE_KEY_PRESS)
		{
			switch(event.getEventKeyCode())
			{
				case Keyboard::KEY_ESCAPE:
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::KEY_ENTER:
					this->onMenuSelect();
					break;
				case Keyboard::KEY_ARROW_UP:
					menu->cursorUp();
					break;
				case Keyboard::KEY_ARROW_DOWN:
					menu->cursorDown();
					break;
				default:
					break;
			}
		}
	}
}

void VehicleSelectionState::onMenuSelect()
{
	Pseudo3DRaceState::getInstance(game)->setVehicle(vehicles[menu->getSelectedIndex()]);
	game.enterState(Pseudo3DCarseGame::RACE_STATE_ID);
}
