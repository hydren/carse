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
using fgeal::Sound;
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
  menu(null), sndCursorMove(null), sndCursorAccept(null), sndCursorOut(null),
  layout(LAYOUT_PROTOTYPE_SLIDE_STAND)
{}

VehicleSelectionState::~VehicleSelectionState()
{
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(menu != null) delete menu;
	if(sndCursorMove != null) delete sndCursorMove;
	if(sndCursorAccept != null) delete sndCursorAccept;
	if(sndCursorOut != null) delete sndCursorOut;

	for(unsigned i = 0; i < vehiclePreview.size(); i++)
		delete vehiclePreview[i];
}

void VehicleSelectionState::initialize()
{
	Display& display = Display::getInstance();
	Rectangle menuBounds = {0.0625f*display.getWidth(), 0.25f*display.getHeight(), 0.4f*display.getWidth(), 0.5f*display.getHeight()};
	fontMain = new Font("assets/font2.ttf", 48 * 0.0015625*display.getHeight());
	fontInfo = new Font("assets/font.ttf", 12);

	sndCursorMove = new Sound("assets/sound/cursor_move.ogg");
	sndCursorAccept = new Sound("assets/sound/cursor_accept.ogg");
	sndCursorOut = new Sound("assets/sound/cursor_out.ogg");

	menu = new Menu(menuBounds, new Font("assets/font.ttf", 18), Color::WHITE);
	menu->fontIsOwned = true;
	menu->cursorWrapAroundEnabled = true;
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
		vehiclePreview.push_back(new Image(vehicles[i].sprite.sheetFilename));
	}

	// default vehicle
	Pseudo3DRaceState::getInstance(game)->setVehicle(vehicles[0]);
}

void VehicleSelectionState::onEnter()
{}

void VehicleSelectionState::onLeave()
{}

void VehicleSelectionState::render()
{
	Display& display = Display::getInstance();
	display.clear();
	switch(layout)
	{
		default:
		case LAYOUT_PROTOTYPE_LIST: renderMenuPrototypeList(); break;
		case LAYOUT_PROTOTYPE_SLIDE_STAND: renderMenuPrototypeSlideStand(); break;
	}
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
					sndCursorOut->play();
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::KEY_ENTER:
					sndCursorAccept->play();
					this->onMenuSelect();
					break;
				case Keyboard::KEY_ARROW_UP:
				case Keyboard::KEY_ARROW_LEFT:
					sndCursorMove->play();
					menu->cursorUp();
					break;
				case Keyboard::KEY_ARROW_DOWN:
				case Keyboard::KEY_ARROW_RIGHT:
					sndCursorMove->play();
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
	game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
}

void VehicleSelectionState::drawVehiclePreview(float x, float y, float scale, int index, int angleType)
{
	Display& display = game.getDisplay();
	if(index < 0)
		index = menu->getSelectedIndex();

	Image* sheetVehicle = vehiclePreview[index];
	Vehicle& vehicle = vehicles[index];

	const float scalex = display.getWidth() * 0.0048828125f * scale * vehicle.sprite.scale.x,
				scaley = display.getWidth() * 0.0048828125f * scale * vehicle.sprite.scale.y,
				posX = x - 0.5*vehicle.sprite.frameWidth * scalex,
				posY = y - 0.5*vehicle.sprite.frameHeight * scaley,
				offsetY = angleType == 0? 0 : vehicle.sprite.frameHeight * (vehicle.sprite.stateCount/2);

	sheetVehicle->drawScaledRegion(posX, posY, scalex, scaley, (angleType > 0 ? Image::FLIP_HORIZONTAL : Image::FLIP_NONE),
									0, offsetY, vehicle.sprite.frameWidth, vehicle.sprite.frameHeight);
}

void VehicleSelectionState::renderMenuPrototypeList()
{
	Display& display = Display::getInstance();
	menu->draw();
	fontMain->drawText("Choose your vehicle", 84, 25, Color::WHITE);
	drawVehiclePreview(0.7*display.getWidth(), 0.35*display.getHeight());

	Vehicle& vehicle = vehicles[menu->getSelectedIndex()];

	// info sheet
	int sheetX = 0.525*display.getWidth(), sheetY = 0.525*display.getHeight();
	const string engineDesc = (vehicle.engine.aspiration.empty()? "" : vehicle.engine.aspiration + " ")
							+ (vehicle.engine.displacement == 0?  "" : vehicle.engine.displacement >= 950? toStrRounded(vehicle.engine.displacement/1000.0) + "L " : to_string(vehicle.engine.displacement)+"cc ")
							+ (vehicle.engine.valvetrain.empty()? "" : vehicle.engine.valvetrain + " ")
							+ (vehicle.engine.valveCount == 0?    "" : to_string(vehicle.engine.valveCount) + "-valve ")
							+ (vehicle.engine.configuration.empty()? "" : vehicle.engine.configuration);

	fontInfo->drawText("Engine: "+(engineDesc.empty()? "--" : engineDesc), sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText("Power:  " +to_string(vehicle.engine.maximumPower) + "hp @" + to_string((int)vehicle.engine.maximumPowerRpm)+"rpm", sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText("Torque: " +toStrRounded(vehicle.engine.maximumTorque) + "Nm @" + to_string((int)vehicle.engine.maximumTorqueRpm)+"rpm", sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText(to_string(vehicle.engine.gearCount)+"-speed transmission", sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText("Weight: "+to_string(vehicle.mass) + "kg", sheetX, sheetY+=12, Color::WHITE);
}

void VehicleSelectionState::renderMenuPrototypeSlideStand()
{
	Display& display = Display::getInstance();

	// draw previous vehicle
	if(vehiclePreview.size() > 2 or (vehiclePreview.size() == 2 and menu->getSelectedIndex() == 1))
	{
		const unsigned index = menu->getSelectedIndex() == 0? menu->getNumberOfEntries()-1 : menu->getSelectedIndex()-1;
		drawVehiclePreview(0.25*display.getWidth(), 0.30*display.getHeight(), 0.75, index, -1);
	}

	// draw next vehicle
	if(vehiclePreview.size() > 2 or (vehiclePreview.size() == 2 and menu->getSelectedIndex() == 0))
	{
		const unsigned index = menu->getSelectedIndex() == menu->getNumberOfEntries()-1? 0 : menu->getSelectedIndex()+1;
		drawVehiclePreview(0.75*display.getWidth(), 0.30*display.getHeight(), 0.75, index, +1);
	}

	// darkening other vehicles
	Image::drawFilledRectangle(0, 0, display.getWidth(), display.getHeight(),Color(0, 0, 0, 128));

	// draw current vehicle
	drawVehiclePreview(0.5*display.getWidth(), 0.35*display.getHeight());

	// draw current vehicle info
	const string lblChooseVehicle = "Choose your vehicle";
	fontMain->drawText(lblChooseVehicle, 0.95*display.getWidth() - fontMain->getTextWidth(lblChooseVehicle), 25, Color::WHITE);

	// info sheet
	Vehicle& vehicle = vehicles[menu->getSelectedIndex()];
	const int infoX = 0.333*display.getWidth(); int infoY = 0.525*display.getHeight();
	const string engineDesc = (vehicle.engine.aspiration.empty()? "" : vehicle.engine.aspiration + " ")
							+ (vehicle.engine.displacement == 0?  "" : vehicle.engine.displacement >= 950? toStrRounded(vehicle.engine.displacement/1000.0) + "L " : to_string(vehicle.engine.displacement)+"cc ")
							+ (vehicle.engine.valvetrain.empty()? "" : vehicle.engine.valvetrain + " ")
							+ (vehicle.engine.valveCount == 0?    "" : to_string(vehicle.engine.valveCount) + "-valve ")
							+ (vehicle.engine.configuration.empty()? "" : vehicle.engine.configuration);

	fontMain->drawText((vehicle.name.empty()? "--" : vehicle.name), infoX, infoY, Color::WHITE);
	fontInfo->drawText("Engine: "+(engineDesc.empty()? "--" : engineDesc), infoX, infoY+=48, Color::WHITE);
	fontInfo->drawText("Power:  " +to_string(vehicle.engine.maximumPower) + "hp @" + to_string((int)vehicle.engine.maximumPowerRpm)+"rpm", infoX, infoY+=12, Color::WHITE);
	fontInfo->drawText("Torque: " +toStrRounded(vehicle.engine.maximumTorque) + "Nm @" + to_string((int)vehicle.engine.maximumTorqueRpm)+"rpm", infoX, infoY+=12, Color::WHITE);
	fontInfo->drawText(to_string(vehicle.engine.gearCount)+"-speed transmission", infoX, infoY+=12, Color::WHITE);
	fontInfo->drawText("Weight: "+to_string(vehicle.mass) + "kg", infoX, infoY+=12, Color::WHITE);
}
