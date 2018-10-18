/*
 * vehicle_selection_simple_list_state.cpp
 *
 *  Created on: 17 de set de 2018
 *      Author: carlosfaruolo
 */

#include "vehicle_selection_simple_list_state.hpp"

#include "carse_game.hpp"

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

static string toStrRounded(float value, unsigned placesCount=1)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(placesCount) << value;
	return ss.str();
}

int VehicleSelectionSimpleListState::getId() { return CarseGame::VEHICLE_SELECTION_SIMPLE_LIST_STATE_ID; }

VehicleSelectionSimpleListState::VehicleSelectionSimpleListState(CarseGame* game)
: State(*game), game(*game),
  fontMain(null), fontInfo(null), fontSub(null), menu(null),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  lastEnterSelectedVehicleIndex(0), lastEnterSelectedVehicleAltIndex(0)
{}

VehicleSelectionSimpleListState::~VehicleSelectionSimpleListState()
{
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(fontSub != null) delete fontSub;
	if(menu != null) delete menu;

	for(unsigned i = 0; i < previews.size(); i++)
	{
		delete previews[i].sprite;
		for(unsigned j = 0; j < previews[i].altSprites.size(); j++)
			delete previews[i].altSprites[j];
	}
}

void VehicleSelectionSimpleListState::initialize()
{
	Display& display = game.getDisplay();
	Rectangle menuBounds = {(1/64.f)*display.getWidth(), (1/5.f)*display.getHeight(), (2/5.f)*display.getWidth(), (3/4.f)*display.getHeight()};
	fontMain = new Font(game.sharedResources->font2Path, dip(28));
	fontInfo = new Font(game.sharedResources->font1Path, dip(12));
	fontSub = new Font(game.sharedResources->font3Path, dip(36));

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;

	menu = new Menu(menuBounds);
	menu->setFont(new Font(game.sharedResources->font1Path, dip(18)), false);
	menu->setColor(Color::WHITE);
	menu->cursorWrapAroundEnabled = true;
	menu->bgColor = Color::AZURE;
	menu->focusedEntryFontColor = Color::NAVY;

	const vector<Pseudo3DVehicle::Spec>& vehiclesSpecs = game.logic.getVehicleList();
	const_foreach(const Pseudo3DVehicle::Spec&, vspec, vector<Pseudo3DVehicle::Spec>, vehiclesSpecs)
	{
		menu->addEntry(vspec.name);
		previews.push_back(VehiclePreview());
		previews.back().sprite = new Image(vspec.sprite.sheetFilename);
		previews.back().altIndex = -1;

		if(not vspec.alternateSprites.empty())
			const_foreach(const Pseudo3DVehicleAnimationSpec&, alternateSprite, vector<Pseudo3DVehicleAnimationSpec>, vspec.alternateSprites)
				previews.back().altSprites.push_back(new Image(alternateSprite.sheetFilename));
	}
}

void VehicleSelectionSimpleListState::onEnter()
{
	lastEnterSelectedVehicleIndex = menu->getSelectedIndex();
	lastEnterSelectedVehicleAltIndex = previews[menu->getSelectedIndex()].altIndex;
}

void VehicleSelectionSimpleListState::onLeave()
{}

void VehicleSelectionSimpleListState::render()
{
	Display& display = game.getDisplay();
	display.clear();
	const float dw = display.getWidth(), dh = display.getHeight();
	menu->draw();
	fontSub->drawText("Choose your vehicle", 32, 25, Color::WHITE);
	drawVehiclePreview(0.7*dw, 0.35*dh);
	drawVehicleSpec((4/9.f)*dw, 0.6*dh);

	VehiclePreview& preview = previews[menu->getSelectedIndex()];
	const fgeal::Point skinArrowLeft1 =  { 0.52f*dw, 0.45f*dh }, skinArrowLeft2  = { 0.53f*dw, 0.44f*dh }, skinArrowLeft3 =  { 0.53f*dw, 0.46f*dh};
	const fgeal::Point skinArrowRight1 = { 0.88f*dw, 0.45f*dh }, skinArrowRight2 = { 0.87f*dw, 0.44f*dh }, skinArrowRight3 = { 0.87f*dw, 0.46f*dh};
	if(not preview.altSprites.empty())
	{
		fgeal::Graphics::drawFilledTriangle(skinArrowLeft1, skinArrowLeft2, skinArrowLeft3, Color::AZURE);
		fgeal::Graphics::drawFilledTriangle(skinArrowRight1, skinArrowRight2, skinArrowRight3, Color::AZURE);
		if(preview.altIndex != -1)
		{
			const string txt = "Alternate appearance" + (preview.altSprites.size() == 1? " " : " " + futil::to_string(preview.altIndex+1) + " ");
			fontInfo->drawText(txt, dw - fontInfo->getTextWidth(txt), 0.5*dh, Color::AZURE);
		}
	}
	else
	{
		fgeal::Graphics::drawFilledTriangle(skinArrowLeft1, skinArrowLeft2, skinArrowLeft3, Color::DARK_GREY);
		fgeal::Graphics::drawFilledTriangle(skinArrowRight1, skinArrowRight2, skinArrowRight3, Color::DARK_GREY);
	}
}

void VehicleSelectionSimpleListState::update(float delta)
{}

void VehicleSelectionSimpleListState::onKeyPressed(Keyboard::Key key)
{
	switch(key)
	{
		case Keyboard::KEY_ESCAPE:
			sndCursorOut->play();
			menu->setSelectedIndex(lastEnterSelectedVehicleIndex);
			previews[menu->getSelectedIndex()].altIndex = lastEnterSelectedVehicleAltIndex;
			game.enterState(game.logic.currentMainMenuStateId);
			break;
		case Keyboard::KEY_ENTER:
			sndCursorIn->play();
			this->menuSelectionAction();
			break;

		case Keyboard::KEY_ARROW_UP:
			menu->moveCursorUp();
			game.sharedResources->sndCursorMove.play();
			break;

		case Keyboard::KEY_ARROW_DOWN:
			menu->moveCursorDown();
			game.sharedResources->sndCursorMove.play();
			break;

		case Keyboard::KEY_ARROW_LEFT:
			changeSprite(false);
			break;

		case Keyboard::KEY_ARROW_RIGHT:
			changeSprite();
			break;

		case Keyboard::KEY_2:
			game.logic.currentVehicleSelectionStateId = CarseGame::VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_ID;
			game.enterState(game.logic.currentVehicleSelectionStateId);
			break;

		default:
			break;
	}
}

void VehicleSelectionSimpleListState::menuSelectionAction()
{
	game.logic.setPickedVehicle(menu->getSelectedIndex(), previews[menu->getSelectedIndex()].altIndex);
	game.enterState(game.logic.currentMainMenuStateId);
}

void VehicleSelectionSimpleListState::drawVehiclePreview(float x, float y, float scale, int index, int angleType)
{
	Display& display = game.getDisplay();
	if(index < 0)
		index = menu->getSelectedIndex();

	VehiclePreview& preview = previews[index];
	const bool isNotAlternateSprite = (preview.altIndex == -1 or preview.altSprites.empty());

	const Pseudo3DVehicle::Spec& vspec = game.logic.getVehicleList()[index];
	const Pseudo3DVehicleAnimationSpec& spriteSpec = (isNotAlternateSprite? vspec.sprite : vspec.alternateSprites[preview.altIndex]);
	Image& sprite = *(isNotAlternateSprite? preview.sprite : preview.altSprites[preview.altIndex]);

	const Image::FlipMode flipMode = (angleType > 0 ? Image::FLIP_HORIZONTAL : Image::FLIP_NONE);
	const float scalex = display.getWidth() * 0.0048828125f * scale * spriteSpec.scale.x,
				scaley = display.getWidth() * 0.0048828125f * scale * spriteSpec.scale.y,
				posX = x - 0.5*spriteSpec.frameWidth * scalex,
				posY = y - 0.5*spriteSpec.frameHeight * scaley,
				offsetY = (angleType == 0? 0 : spriteSpec.frameHeight * (spriteSpec.stateCount/2));

	sprite.drawScaledRegion(posX, posY, scalex, scaley, flipMode, 0, offsetY, spriteSpec.frameWidth, spriteSpec.frameHeight);
}

void VehicleSelectionSimpleListState::drawVehicleSpec(float infoX, float infoY, float index)
{
	// info sheet
	const Pseudo3DVehicle::Spec& vehicle = game.logic.getVehicleList()[index != -1? index : menu->getSelectedIndex()];

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

void VehicleSelectionSimpleListState::changeSprite(bool forward)
{
	VehiclePreview& preview = previews[menu->getSelectedIndex()];
	if(not preview.altSprites.empty())
	{
		sndCursorMove->play();

		if(forward)
		{
			if(preview.altIndex == (int) preview.altSprites.size() - 1)
				preview.altIndex = -1;
			else
				preview.altIndex++;
		}
		else
		{
			if(preview.altIndex == -1)
				preview.altIndex = preview.altSprites.size() - 1;
			else
				preview.altIndex--;
		}
	}
}
