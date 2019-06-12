/*
 * vehicle_selection_simple_list_state.cpp
 *
 *  Created on: 17 de set de 2018
 *      Author: carlosfaruolo
 */

#include "vehicle_selection_simple_list_state.hpp"

#include "carse_game.hpp"

#include "util.hpp"

#include <vector>
#include <iomanip>
#include <sstream>
#include <cmath>

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
using fgeal::Button;
using fgeal::Mouse;
using fgeal::Graphics;
using fgeal::Point;
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
: State(*game), game(*game), lastDisplaySize(),
  fontMain(null), fontInfo(null), fontSub(null),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  lastEnterSelectedVehicleIndex(0), lastEnterSelectedVehicleAltIndex(0),
  menuUpButton(), menuDownButton(), selectButton(), backButton(), appearanceLeftButton(), appearanceRightButton(),
  previewSprite()
{}

VehicleSelectionSimpleListState::~VehicleSelectionSimpleListState()
{
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(fontSub != null) delete fontSub;

	if(previewSprite != null)
		delete previewSprite;
}

void VehicleSelectionSimpleListState::initialize()
{
	Display& display = game.getDisplay();

	fontMain = new Font(game.sharedResources->font2Path);
	fontInfo = new Font(game.sharedResources->font1Path);
	fontSub = new Font(game.sharedResources->font3Path);

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;

	menu.setFont(new Font(game.sharedResources->font1Path), false);
	menu.setColor(Color::WHITE);
	menu.cursorWrapAroundEnabled = true;
	menu.bgColor = Color::AZURE;
	menu.focusedEntryFontColor = Color::NAVY;

	const vector<Pseudo3DVehicle::Spec>& vehiclesSpecs = game.logic.getVehicleList();
	const_foreach(const Pseudo3DVehicle::Spec&, vspec, vector<Pseudo3DVehicle::Spec>, vehiclesSpecs)
		menu.addEntry(vspec.name);

	previewAltIndex.clear();
	previewAltIndex.resize(vehiclesSpecs.size(), -1);

	menuUpButton.shape = Button::SHAPE_ROUNDED_RECTANGULAR;
	menuUpButton.bgColor = Color::AZURE;
	menuUpButton.highlightColor = menu.bgColor;
	menuUpButton.highlightSpacing = 0.003*display.getHeight();

	menuDownButton = menuUpButton;

	appearanceLeftButton = menuUpButton;
	appearanceLeftButton.highlightSpacing = 0.007*display.getHeight();
	appearanceRightButton = appearanceLeftButton;

	backButton = appearanceLeftButton;
	backButton.bgColor = menu.bgColor;
	backButton.font = &menu.getFont();
	backButton.textColor = Color::WHITE;
	backButton.label = " Back ";

	selectButton = backButton;
	selectButton.label = " Select ";
}

void VehicleSelectionSimpleListState::onEnter()
{
	Display& display = game.getDisplay();
	const unsigned dw = display.getWidth(), dh = display.getHeight();

	// reload fonts if display size changed
	if(lastDisplaySize.x != dw or lastDisplaySize.y != dh)
	{
		const FontSizer fs(display.getHeight());
		fontMain->setSize(fs(28));
		fontSub->setSize(fs(36));
		fontInfo->setSize(fs(12));
		menu.getFont().setSize(fs(18));
		lastDisplaySize.x = dw;
		lastDisplaySize.y = dh;
	}

	lastEnterSelectedVehicleIndex = menu.getSelectedIndex();
	lastEnterSelectedVehicleAltIndex = previewAltIndex[lastEnterSelectedVehicleIndex];

	menu.bounds.x = (1/64.f)*dw;
	menu.bounds.y = (1/5.f)*dh;
	menu.bounds.w = (2/5.f)*dw;
	menu.bounds.h = (3/4.f)*dh;

	menuUpButton.bounds.x = menu.bounds.x + menu.bounds.w + 0.005*dw;
	menuUpButton.bounds.y = menu.bounds.y;
	menuUpButton.bounds.w = 0.025*dw;
	menuUpButton.bounds.h = 0.025*dh;

	menuUpButtonArrow1.x = menuUpButton.bounds.x + 0.0050*dw;
	menuUpButtonArrow1.y = menuUpButton.bounds.y + 0.0200*dh;
	menuUpButtonArrow2.x = menuUpButton.bounds.x + 0.0125*dw;
	menuUpButtonArrow2.y = menuUpButton.bounds.y + 0.0050*dh;
	menuUpButtonArrow3.x = menuUpButton.bounds.x + 0.0200*dw;
	menuUpButtonArrow3.y = menuUpButton.bounds.y + 0.0200*dh;

	menuDownButton.bounds = menuUpButton.bounds;
	menuDownButton.bounds.y = menu.bounds.y + menu.bounds.h - menuDownButton.bounds.h;

	menuDownButtonArrow1.x = menuDownButton.bounds.x + 0.0050*dw;
	menuDownButtonArrow1.y = menuDownButton.bounds.y + 0.0050*dh;
	menuDownButtonArrow2.x = menuDownButton.bounds.x + 0.0200*dw;
	menuDownButtonArrow2.y = menuDownButton.bounds.y + 0.0050*dh;
	menuDownButtonArrow3.x = menuDownButton.bounds.x + 0.0125*dw;
	menuDownButtonArrow3.y = menuDownButton.bounds.y + 0.0200*dh;

	appearanceLeftButton.bounds.x = 0.51*dw;
	appearanceLeftButton.bounds.y = 0.435*dh;
	appearanceLeftButton.bounds.w = 0.03*dw;
	appearanceLeftButton.bounds.h = 0.03*dh;

	appearanceRightButton.bounds.x = 0.86*dw;
	appearanceRightButton.bounds.y = 0.435*dh;
	appearanceRightButton.bounds.w = 0.03*dw;
	appearanceRightButton.bounds.h = 0.03*dh;

	backButton.bounds.x = 0.875*dw;
	backButton.bounds.y = 0.025*dh;
	backButton.bounds.w = menu.getFont().getTextWidth(" Back ");
	backButton.bounds.h = menu.getFont().getTextHeight();

	selectButton.bounds.w = menu.getFont().getTextWidth(" Select ");
	selectButton.bounds.x = 0.70*dw - selectButton.bounds.w/2;
	selectButton.bounds.y = 0.90*dh;
	selectButton.bounds.h = menu.getFont().getTextHeight();
}

void VehicleSelectionSimpleListState::onLeave()
{}

void VehicleSelectionSimpleListState::update(float delta)
{
	const vector<Pseudo3DVehicle::Spec>& vehiclesSpecs = game.logic.getVehicleList();
	if(previewSprite == null
		or (previewAltIndex[menu.getSelectedIndex()] == -1 and previewSpriteFilename != vehiclesSpecs[menu.getSelectedIndex()].sprite.sheetFilename)
		or (previewAltIndex[menu.getSelectedIndex()] >=  0 and previewSpriteFilename != vehiclesSpecs[menu.getSelectedIndex()].alternateSprites[previewAltIndex[menu.getSelectedIndex()]].sheetFilename))
	{
		if(previewSprite != null)
			delete previewSprite;

		const Pseudo3DVehicle::Spec& spec = vehiclesSpecs[menu.getSelectedIndex()];

		if(previewAltIndex[menu.getSelectedIndex()] == -1)
			previewSpriteFilename = spec.sprite.sheetFilename;
		else
			previewSpriteFilename = spec.alternateSprites[previewAltIndex[menu.getSelectedIndex()]].sheetFilename;

		previewSprite = new Image(previewSpriteFilename);
	}
}

void VehicleSelectionSimpleListState::render()
{
	Display& display = game.getDisplay();
	display.clear();
	const float dw = display.getWidth(), dh = display.getHeight();
	const Point mousePos = Mouse::getPosition();
	const bool blinkCycle = (cos(20*fgeal::uptime()) > 0);
	fontSub->drawText("Choose your vehicle", 32, 25, Color::WHITE);
	menu.draw();

	menuUpButton.highlighted = blinkCycle and menuUpButton.bounds.contains(mousePos);
	menuUpButton.draw();
	fgeal::Graphics::drawFilledTriangle(menuUpButtonArrow1, menuUpButtonArrow2, menuUpButtonArrow3, menu.focusedEntryFontColor);

	menuDownButton.highlighted = blinkCycle and menuDownButton.bounds.contains(mousePos);
	menuDownButton.draw();
	fgeal::Graphics::drawFilledTriangle(menuDownButtonArrow1, menuDownButtonArrow2, menuDownButtonArrow3, menu.focusedEntryFontColor);

	drawVehiclePreview(0.7*dw, 0.35*dh);
	drawVehicleSpec((4/9.f)*dw, 0.6*dh);

	const fgeal::Point skinArrowLeft1 =  { 0.52f*dw, 0.45f*dh }, skinArrowLeft2  = { 0.53f*dw, 0.44f*dh }, skinArrowLeft3 =  { 0.53f*dw, 0.46f*dh};
	const fgeal::Point skinArrowRight1 = { 0.88f*dw, 0.45f*dh }, skinArrowRight2 = { 0.87f*dw, 0.44f*dh }, skinArrowRight3 = { 0.87f*dw, 0.46f*dh};

	if(not game.logic.getVehicleList()[menu.getSelectedIndex()].alternateSprites.empty())
	{
		appearanceLeftButton.bgColor = appearanceRightButton.bgColor = menu.bgColor;

		appearanceLeftButton.highlighted = blinkCycle and appearanceLeftButton.bounds.contains(mousePos);
		appearanceLeftButton.draw();
		fgeal::Graphics::drawFilledTriangle(skinArrowLeft1, skinArrowLeft2, skinArrowLeft3, menu.focusedEntryFontColor);

		appearanceRightButton.highlighted = blinkCycle and appearanceRightButton.bounds.contains(mousePos);
		appearanceRightButton.draw();
		fgeal::Graphics::drawFilledTriangle(skinArrowRight1, skinArrowRight2, skinArrowRight3, menu.focusedEntryFontColor);

		if(previewAltIndex[menu.getSelectedIndex()] != -1)
		{
			const string txt = "Alternate appearance" + (previewAltIndex[menu.getSelectedIndex()] == 0? "" : " " + futil::to_string(previewAltIndex[menu.getSelectedIndex()]+1) + " ");
			fontInfo->drawText(txt, dw - fontInfo->getTextWidth(txt), 0.5*dh, Color::AZURE);
		}
	}
	else
	{
		appearanceLeftButton.highlighted = appearanceRightButton.highlighted = false;
		appearanceLeftButton.bgColor = appearanceRightButton.bgColor = Color::GREY;

		appearanceLeftButton.draw();
		fgeal::Graphics::drawFilledTriangle(skinArrowLeft1, skinArrowLeft2, skinArrowLeft3, Color::DARK_GREY);

		appearanceRightButton.draw();
		fgeal::Graphics::drawFilledTriangle(skinArrowRight1, skinArrowRight2, skinArrowRight3, Color::DARK_GREY);
	}

	backButton.highlighted = blinkCycle and backButton.bounds.contains(mousePos);
	backButton.draw();

	selectButton.highlighted = blinkCycle and selectButton.bounds.contains(mousePos);
	selectButton.draw();
}

void VehicleSelectionSimpleListState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{
	if(button == fgeal::Mouse::BUTTON_LEFT)
	{
		if(menu.bounds.contains(x, y))
		{
			if(menu.getIndexAtLocation(x, y) != menu.getSelectedIndex())
			{
				sndCursorMove->play();
				menu.setSelectedIndexByLocation(x, y);
			}
		}
		else if(menuUpButton.bounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ARROW_UP);

		else if(menuDownButton.bounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ARROW_DOWN);

		else if(appearanceLeftButton.bounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ARROW_LEFT);

		else if(appearanceRightButton.bounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ARROW_RIGHT);

		else if(selectButton.bounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ENTER);

		else if(backButton.bounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ESCAPE);
	}
}

void VehicleSelectionSimpleListState::onKeyPressed(Keyboard::Key key)
{
	switch(key)
	{
		case Keyboard::KEY_ESCAPE:
			sndCursorOut->play();
			menu.setSelectedIndex(lastEnterSelectedVehicleIndex);
			previewAltIndex[menu.getSelectedIndex()] = lastEnterSelectedVehicleAltIndex;
			game.enterState(game.logic.currentMainMenuStateId);
			break;
		case Keyboard::KEY_ENTER:
			sndCursorIn->play();
			this->menuSelectionAction();
			break;

		case Keyboard::KEY_ARROW_UP:
			menu.moveCursorUp();
			game.sharedResources->sndCursorMove.play();
			break;

		case Keyboard::KEY_ARROW_DOWN:
			menu.moveCursorDown();
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

void VehicleSelectionSimpleListState::onJoystickAxisMoved(unsigned joystick, unsigned axis, float oldValue, float newValue)
{
	if(axis == 0)
	{
		if(newValue > 0.2)
			this->onKeyPressed(Keyboard::KEY_ARROW_RIGHT);
		if(newValue < -0.2)
			this->onKeyPressed(Keyboard::KEY_ARROW_LEFT);
	}
	if(axis == 1)
	{
		if(newValue > 0.2)
			this->onKeyPressed(Keyboard::KEY_ARROW_DOWN);
		if(newValue < -0.2)
			this->onKeyPressed(Keyboard::KEY_ARROW_UP);
	}
}

void VehicleSelectionSimpleListState::onJoystickButtonPressed(unsigned joystick, unsigned button)
{
	if(button % 2 == 0)
		this->onKeyPressed(Keyboard::KEY_ENTER);
	if(button % 2 == 1)
		this->onKeyPressed(Keyboard::KEY_ESCAPE);
}

void VehicleSelectionSimpleListState::menuSelectionAction()
{
	game.logic.setPickedVehicle(menu.getSelectedIndex(), previewAltIndex[menu.getSelectedIndex()]);
	game.enterState(game.logic.currentMainMenuStateId);
}

void VehicleSelectionSimpleListState::drawVehiclePreview(float x, float y, float scale, int index, int angleType)
{
	Display& display = game.getDisplay();
	if(index < 0)
		index = menu.getSelectedIndex();

	const bool isNotAlternateSprite = (previewAltIndex[menu.getSelectedIndex()] == -1);

	const Pseudo3DVehicle::Spec& vspec = game.logic.getVehicleList()[index];
	const Pseudo3DVehicleAnimationSpec& spriteSpec = (isNotAlternateSprite? vspec.sprite : vspec.alternateSprites[previewAltIndex[menu.getSelectedIndex()]]);

	const Image::FlipMode flipMode = (angleType > 0 ? Image::FLIP_HORIZONTAL : Image::FLIP_NONE);
	const float scalex = display.getWidth() * 0.0048828125f * scale * spriteSpec.scale.x,
				scaley = display.getWidth() * 0.0048828125f * scale * spriteSpec.scale.y,
				posX = x - 0.5*spriteSpec.frameWidth * scalex,
				posY = y - 0.5*spriteSpec.frameHeight * scaley,
				offsetY = (angleType == 0? 0 : spriteSpec.frameHeight * (spriteSpec.stateCount/2));

	previewSprite->drawScaledRegion(posX, posY, scalex, scaley, flipMode, 0, offsetY, spriteSpec.frameWidth, spriteSpec.frameHeight);
}

void VehicleSelectionSimpleListState::drawVehicleSpec(float infoX, float infoY, float index)
{
	// info sheet
	const Pseudo3DVehicle::Spec& vehicle = game.logic.getVehicleList()[index != -1? index : menu.getSelectedIndex()];

	const string txtVehicleType = string("Type: ") + (vehicle.type == Mechanics::TYPE_CAR? "Car" : vehicle.type == Mechanics::TYPE_BIKE? "Bike" : "Other");
	fontInfo->drawText(txtVehicleType, infoX, infoY, Color::WHITE);

	const string txtEngineDesc = (vehicle.engineAspiration.empty()? "" : vehicle.engineAspiration + " ")
	                           + (vehicle.engineDisplacement == 0?  "" : vehicle.engineDisplacement >= 950? toStrRounded(vehicle.engineDisplacement/1000.0) + "L " : to_string(vehicle.engineDisplacement)+"cc ")
	                           + (vehicle.engineValvetrain.empty()? "" : vehicle.engineValvetrain + " ")
	                           + (vehicle.engineValveCount == 0?    "" : to_string(vehicle.engineValveCount) + "-valve ")
	                           + (vehicle.engineConfiguration.empty()? "" : vehicle.engineConfiguration);
	fontInfo->drawText("Engine: "+(txtEngineDesc.empty()? "--" : txtEngineDesc), infoX, infoY+=fontInfo->getTextHeight(), Color::WHITE);

	const string txtPowerInfo = "Power:  " +to_string(vehicle.engineMaximumPower) + "hp @" + to_string((int)vehicle.engineMaximumPowerRpm)+"rpm";
	fontInfo->drawText(txtPowerInfo, infoX, infoY+=fontInfo->getTextHeight(), Color::WHITE);

	const string txtTorqueInfo = "Torque: " +toStrRounded(vehicle.engineMaximumTorque) + "Nm @" + to_string((int)vehicle.engineMaximumTorqueRpm)+"rpm";
	fontInfo->drawText(txtTorqueInfo, infoX, infoY+=fontInfo->getTextHeight(), Color::WHITE);

	const string txtTransmissionInfo = to_string(vehicle.engineGearCount)+"-speed transmission";
	fontInfo->drawText(txtTransmissionInfo, infoX, infoY+=fontInfo->getTextHeight(), Color::WHITE);

	const string txtWeightInfo = "Weight: "+to_string(vehicle.mass) + "kg";
	fontInfo->drawText(txtWeightInfo, infoX, infoY+=fontInfo->getTextHeight(), Color::WHITE);

	const string txtDrivetrainInfo = "Drivetrain: "+(vehicle.drivenWheelsType == Mechanics::DRIVEN_WHEELS_ALL? "AWD"
	                                            : (  vehicle.engineLocation   == Mechanics::ENGINE_LOCATION_ON_FRONT? "F"
	                                            :    vehicle.engineLocation   == Mechanics::ENGINE_LOCATION_ON_REAR? "R" : "M")
	                                        + string(vehicle.drivenWheelsType == Mechanics::DRIVEN_WHEELS_ON_REAR? "R" : "F"));
	fontInfo->drawText(txtDrivetrainInfo, infoX, infoY+=fontInfo->getTextHeight(), Color::WHITE);
}

void VehicleSelectionSimpleListState::changeSprite(bool forward)
{
	const unsigned selected = menu.getSelectedIndex(),
	               alternateSpritesCount = game.logic.getVehicleList()[selected].alternateSprites.size();
	if(alternateSpritesCount > 0)
	{
		sndCursorMove->play();

		if(forward)
		{
			if(previewAltIndex[selected] == (int) alternateSpritesCount - 1)
				previewAltIndex[selected] = -1;
			else
				previewAltIndex[selected]++;
		}
		else
		{
			if(previewAltIndex[selected] == -1)
				previewAltIndex[selected] = alternateSpritesCount - 1;
			else
				previewAltIndex[selected]--;
		}
	}
}
