/*
 * vehicle_selection_state.cpp
 *
 *  Created on: 7 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle_selection_state.hpp"

#include "carse_game.hpp"

#include "race_state.hpp"

#include "futil/properties.hpp"
#include "futil/string_actions.hpp"

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
: State(*game), shared(*game->sharedResources), gameLogic(game->logic),
  fontMain(null), fontInfo(null), menu(null),
  lastEnterSelectedVehicleIndex(0), lastEnterSelectedVehicleAltIndex(0),
  layout(LAYOUT_PROTOTYPE_SLIDE_STAND)
{}

VehicleSelectionState::~VehicleSelectionState()
{
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(menu != null) delete menu;

	for(unsigned i = 0; i < previews.size(); i++)
	{
		delete previews[i].sprite;
		for(unsigned j = 0; j < previews[i].altSprites.size(); j++)
			delete previews[i].altSprites[j];
	}
}

void VehicleSelectionState::initialize()
{
	Display& display = game.getDisplay();
	Rectangle menuBounds = {0.0625f*display.getWidth(), 0.25f*display.getHeight(), 0.4f*display.getWidth(), 0.5f*display.getHeight()};
	fontMain = new Font("assets/font2.ttf", 48 * 0.0015625*display.getHeight());
	fontInfo = new Font("assets/font.ttf", 12);

	menu = new Menu(menuBounds, new Font("assets/font.ttf", 18), Color::WHITE);
	menu->fontIsOwned = true;
	menu->cursorWrapAroundEnabled = true;
	menu->bgColor = Color::AZURE;
	menu->focusedEntryFontColor = Color::NAVY;

	const vector<Pseudo3DVehicle::Spec>& vehiclesSpecs = gameLogic.getVehicleList();
	const_foreach(const Pseudo3DVehicle::Spec&, vspec, vector<Pseudo3DVehicle::Spec>, vehiclesSpecs)
	{
		menu->addEntry(vspec.name);
		previews.push_back(VehiclePreview());
		previews.back().sprite = new Image(vspec.sprite.sheetFilename);
		previews.back().altIndex = -1;

		if(not vspec.alternateSprites.empty())
			const_foreach(const Pseudo3DVehicleAnimationProfile&, alternateSprite, vector<Pseudo3DVehicleAnimationProfile>, vspec.alternateSprites)
				previews.back().altSprites.push_back(new Image(alternateSprite.sheetFilename));
	}
}

void VehicleSelectionState::onEnter()
{
	lastEnterSelectedVehicleIndex = menu->getSelectedIndex();
	lastEnterSelectedVehicleAltIndex = previews[menu->getSelectedIndex()].altIndex;
}

void VehicleSelectionState::onLeave()
{}

void VehicleSelectionState::render()
{
	Display& display = game.getDisplay();
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
					shared.sndCursorOut.stop();
					shared.sndCursorOut.play();
					menu->setSelectedIndex(lastEnterSelectedVehicleIndex);
					previews[menu->getSelectedIndex()].altIndex = lastEnterSelectedVehicleAltIndex;
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::KEY_ENTER:
					shared.sndCursorIn.stop();
					shared.sndCursorIn.play();
					this->onMenuSelect();
					break;
				case Keyboard::KEY_ARROW_UP:
				case Keyboard::KEY_ARROW_LEFT:
					shared.sndCursorMove.stop();
					shared.sndCursorMove.play();
					menu->cursorUp();
					break;
				case Keyboard::KEY_ARROW_DOWN:
				case Keyboard::KEY_ARROW_RIGHT:
					shared.sndCursorMove.stop();
					shared.sndCursorMove.play();
					menu->cursorDown();
					break;
				case Keyboard::KEY_PAGE_UP:
				{
					VehiclePreview& preview = previews[menu->getSelectedIndex()];
					if(not preview.altSprites.empty())
					{
						shared.sndCursorMove.stop();
						shared.sndCursorMove.play();
						if(preview.altIndex == -1)
							preview.altIndex = preview.altSprites.size() - 1;
						else
							preview.altIndex--;
					}
					break;
				}
				case Keyboard::KEY_PAGE_DOWN:
				{
					VehiclePreview& preview = previews[menu->getSelectedIndex()];
					if(not preview.altSprites.empty())
					{
						shared.sndCursorMove.stop();
						shared.sndCursorMove.play();
						if(preview.altIndex == (int) preview.altSprites.size() - 1)
							preview.altIndex = -1;
						else
							preview.altIndex++;
					}
					break;
				}
				default:
					break;
			}
		}
	}
}

void VehicleSelectionState::onMenuSelect()
{
	gameLogic.setPickedVehicle(menu->getSelectedIndex(), previews[menu->getSelectedIndex()].altIndex);
	game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
}

void VehicleSelectionState::drawVehiclePreview(float x, float y, float scale, int index, int angleType)
{
	Display& display = game.getDisplay();
	if(index < 0)
		index = menu->getSelectedIndex();

	VehiclePreview& preview = previews[index];
	const bool isNotAlternateSprite = (preview.altIndex == -1 or preview.altSprites.empty());

	const Pseudo3DVehicle::Spec& vspec = gameLogic.getVehicleList()[index];
	const Pseudo3DVehicleAnimationProfile& spriteSpec = (isNotAlternateSprite? vspec.sprite : vspec.alternateSprites[preview.altIndex]);
	Image& sprite = *(isNotAlternateSprite? preview.sprite : preview.altSprites[preview.altIndex]);

	const Image::FlipMode flipMode = (angleType > 0 ? Image::FLIP_HORIZONTAL : Image::FLIP_NONE);
	const float scalex = display.getWidth() * 0.0048828125f * scale * spriteSpec.scale.x,
				scaley = display.getWidth() * 0.0048828125f * scale * spriteSpec.scale.y,
				posX = x - 0.5*spriteSpec.frameWidth * scalex,
				posY = y - 0.5*spriteSpec.frameHeight * scaley,
				offsetY = (angleType == 0? 0 : spriteSpec.frameHeight * (spriteSpec.stateCount/2));

	sprite.drawScaledRegion(posX, posY, scalex, scaley, flipMode, 0, offsetY, spriteSpec.frameWidth, spriteSpec.frameHeight);
}

void VehicleSelectionState::renderMenuPrototypeList()
{
	Display& display = game.getDisplay();
	menu->draw();
	fontMain->drawText("Choose your vehicle", 84, 25, Color::WHITE);
	drawVehiclePreview(0.7*display.getWidth(), 0.35*display.getHeight());

	const Pseudo3DVehicle::Spec& vspec = gameLogic.getVehicleList()[menu->getSelectedIndex()];

	// info sheet
	int sheetX = 0.525*display.getWidth(), sheetY = 0.525*display.getHeight();
	const string engineDesc = (vspec.engineAspiration.empty()? "" : vspec.engineAspiration + " ")
							+ (vspec.engineDisplacement == 0?  "" : vspec.engineDisplacement >= 950? toStrRounded(vspec.engineDisplacement/1000.0) + "L " : to_string(vspec.engineDisplacement)+"cc ")
							+ (vspec.engineValvetrain.empty()? "" : vspec.engineValvetrain + " ")
							+ (vspec.engineValveCount == 0?    "" : to_string(vspec.engineValveCount) + "-valve ")
							+ (vspec.engineConfiguration.empty()? "" : vspec.engineConfiguration);

	fontInfo->drawText("Engine: "+(engineDesc.empty()? "--" : engineDesc), sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText("Power:  " +to_string(vspec.engineMaximumPower) + "hp @" + to_string((int)vspec.engineMaximumPowerRpm)+"rpm", sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText("Torque: " +toStrRounded(vspec.engineMaximumTorque) + "Nm @" + to_string((int)vspec.engineMaximumTorqueRpm)+"rpm", sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText(to_string(vspec.engineGearCount)+"-speed transmission", sheetX, sheetY+=12, Color::WHITE);
	fontInfo->drawText("Weight: "+to_string(vspec.mass) + "kg", sheetX, sheetY+=12, Color::WHITE);
}

void VehicleSelectionState::renderMenuPrototypeSlideStand()
{
	Display& display = game.getDisplay();
	const vector<Pseudo3DVehicle::Spec>& vehicles = gameLogic.getVehicleList();

	// draw previous vehicle
	if(vehicles.size() > 2 or (vehicles.size() == 2 and menu->getSelectedIndex() == 1))
	{
		const unsigned index = menu->getSelectedIndex() == 0? menu->getNumberOfEntries()-1 : menu->getSelectedIndex()-1;
		drawVehiclePreview(0.25*display.getWidth(), 0.30*display.getHeight(), 0.75, index, -1);
	}

	// draw next vehicle
	if(vehicles.size() > 2 or (vehicles.size() == 2 and menu->getSelectedIndex() == 0))
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
	Image::drawFilledRectangle(0.9*display.getWidth() - fontMain->getTextWidth(lblChooseVehicle), 0, display.getWidth(), 25+fontMain->getHeight(), Color::DARK_GREEN);
	fontMain->drawText(lblChooseVehicle, 0.95*display.getWidth() - fontMain->getTextWidth(lblChooseVehicle), 25, Color::WHITE);

	// info sheet
	const Pseudo3DVehicle::Spec& vehicle = vehicles[menu->getSelectedIndex()];
	const int infoX = 0.25*display.getWidth(); int infoY = 0.66*display.getHeight();

	const string name = vehicle.name.empty()? "--" : vehicle.name;
	const unsigned nameWidth = fontMain->getTextWidth(name);
	const int nameOffset = nameWidth > display.getWidth()? 0.6*sin(fgeal::uptime())*(nameWidth - display.getWidth()) : 0;

	Image::drawFilledRectangle(0, infoY - 1.1*fontMain->getHeight(), display.getWidth(), fontMain->getHeight(), Color::AZURE);
	fontMain->drawText(name, 0.5*(display.getWidth()-fontMain->getTextWidth(name)) + nameOffset, infoY - 1.1*fontMain->getHeight(), Color::WHITE);

	Image::drawFilledRectangle(0, infoY - 0.1*fontMain->getHeight(), display.getWidth(), 0.25*display.getHeight(), Color::NAVY);
	const string txtVehicleType = string("Type: ") + (vehicle.type == Mechanics::TYPE_CAR? "Car" : vehicle.type == Mechanics::TYPE_BIKE? "Bike" : "Other");
	fontInfo->drawText(txtVehicleType, infoX, infoY, Color::WHITE);

	const string txtEngineDesc = (vehicle.engineAspiration.empty()? "" : vehicle.engineAspiration + " ")
	                           + (vehicle.engineDisplacement == 0?  "" : vehicle.engineDisplacement >= 950? toStrRounded(vehicle.engineDisplacement/1000.0) + "L " : to_string(vehicle.engineDisplacement)+"cc ")
	                           + (vehicle.engineValvetrain.empty()? "" : vehicle.engineValvetrain + " ")
	                           + (vehicle.engineValveCount == 0?    "" : to_string(vehicle.engineValveCount) + "-valve ")
	                           + (vehicle.engineConfiguration.empty()? "" : vehicle.engineConfiguration);
	fontInfo->drawText("Engine: "+(txtEngineDesc.empty()? "--" : txtEngineDesc), infoX, infoY+=fontInfo->getHeight(), Color::WHITE);

	const string txtPowerInfo = "Power:  " +to_string(vehicle.engineMaximumPower) + "hp @" + to_string((int)vehicle.engineMaximumPowerRpm)+"rpm";
	fontInfo->drawText(txtPowerInfo, infoX, infoY+=fontInfo->getHeight(), Color::WHITE);

	const string txtTorqueInfo = "Torque: " +toStrRounded(vehicle.engineMaximumTorque) + "Nm @" + to_string((int)vehicle.engineMaximumTorqueRpm)+"rpm";
	fontInfo->drawText(txtTorqueInfo, infoX, infoY+=fontInfo->getHeight(), Color::WHITE);

	const string txtTransmissionInfo = to_string(vehicle.engineGearCount)+"-speed transmission";
	fontInfo->drawText(txtTransmissionInfo, infoX, infoY+=fontInfo->getHeight(), Color::WHITE);

	const string txtWeightInfo = "Weight: "+to_string(vehicle.mass) + "kg";
	fontInfo->drawText(txtWeightInfo, infoX, infoY+=fontInfo->getHeight(), Color::WHITE);

	const string txtDrivetrainInfo = "Drivetrain: "+(vehicle.drivenWheelsType == Mechanics::DRIVEN_WHEELS_ALL? "AWD"
	                                            : (  vehicle.engineLocation   == Mechanics::ENGINE_LOCATION_ON_FRONT? "F"
	                                            :    vehicle.engineLocation   == Mechanics::ENGINE_LOCATION_ON_REAR? "R" : "M")
	                                        + string(vehicle.drivenWheelsType == Mechanics::DRIVEN_WHEELS_ON_REAR? "R" : "F"));
	fontInfo->drawText(txtDrivetrainInfo, infoX, infoY+=fontInfo->getHeight(), Color::WHITE);
}
