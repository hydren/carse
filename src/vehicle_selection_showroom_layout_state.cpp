/*
 * vehicle_selection_showroom_layout_state.cpp
 *
 *  Created on: 17 de set de 2018
 *      Author: carlosfaruolo
 */

#include "vehicle_selection_showroom_layout_state.hpp"

#include "carse_game.hpp"

#include <vector>
#include <iomanip>
#include <sstream>

using fgeal::Display;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Mouse;
using fgeal::Font;
using fgeal::Sound;
using fgeal::Color;
using fgeal::Image;
using fgeal::Rectangle;
using fgeal::Point;
using fgeal::Graphics;
using fgeal::Menu;
using fgeal::Button;
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

int VehicleSelectionShowroomLayoutState::getId() { return CarseGame::VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_ID; }

VehicleSelectionShowroomLayoutState::VehicleSelectionShowroomLayoutState(CarseGame* game)
: State(*game), game(*game), lastDisplaySize(),
  fontTitle(null), fontSubtitle(null), fontInfo(null), fontGui(null),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  selectButton(), backButton(),
  lastEnterSelectedVehicleIndex(0), lastEnterSelectedVehicleAltIndex(0),
  previewCurrentSprite(null), previewPreviousSprite(null), previewNextSprite(null),
  imgBackground(null), imgArrow1(null), imgArrow2(null),
  isSelectionTransitioning(false), previousIndex(-1), selectionTransitionProgress(0)
{}

VehicleSelectionShowroomLayoutState::~VehicleSelectionShowroomLayoutState()
{
	if(fontTitle != null) delete fontTitle;
	if(fontSubtitle != null) delete fontSubtitle;
	if(fontInfo != null) delete fontInfo;
	if(fontGui != null) delete fontGui;

	if(previewCurrentSprite != null)
		delete previewCurrentSprite;

	if(previewPreviousSprite != null)
		delete previewPreviousSprite;

	if(previewNextSprite != null)
		delete previewNextSprite;

	if(imgBackground != null)
		delete imgBackground;

	if(imgArrow1 != null)
		delete imgArrow1;

	if(imgArrow2 != null)
		delete imgArrow2;
}

void VehicleSelectionShowroomLayoutState::initialize()
{
	fontTitle = new Font(game.sharedResources->font2Path);
	fontSubtitle = new Font(game.sharedResources->font3Path);
	fontInfo = new Font(game.sharedResources->font1Path);
	fontGui = new Font(game.sharedResources->font1Path);

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;

	menu.cursorWrapAroundEnabled = true;

	const vector<Pseudo3DVehicle::Spec>& vehiclesSpecs = game.logic.getVehicleList();
	const_foreach(const Pseudo3DVehicle::Spec&, vspec, vector<Pseudo3DVehicle::Spec>, vehiclesSpecs)
		menu.addEntry(vspec.name);

	previewAltIndex.clear();
	previewAltIndex.resize(vehiclesSpecs.size(), -1);

	backButton.bgColor = backButton.highlightColor = Color::AZURE;
	backButton.textColor = Color::WHITE;
	backButton.font = fontGui;
	backButton.shape = Button::SHAPE_ROUNDED_RECTANGULAR;
	backButton.label = " Back ";

	selectButton = backButton;
	selectButton.label = " Select ";

	imgBackground = new Image("assets/showroom-bg.jpg");
	imgArrow1 = new Image("assets/arrow-red.png");
	imgArrow2 = new Image("assets/arrow-blue.png");
}

void VehicleSelectionShowroomLayoutState::onEnter()
{
	Display& display = game.getDisplay();
	const unsigned dw = display.getWidth(), dh = display.getHeight();

	// reload fonts if display size changed
	if(lastDisplaySize.x != dw or lastDisplaySize.y != dh)
	{
		fontTitle->setFontSize(dip(22));
		fontSubtitle->setFontSize(dip(36));
		fontInfo->setFontSize(dip(12));
		fontGui->setFontSize(dip(18));
		lastDisplaySize.x = dw;
		lastDisplaySize.y = dh;
	}

	lastEnterSelectedVehicleIndex = menu.getSelectedIndex();
	lastEnterSelectedVehicleAltIndex = previewAltIndex[lastEnterSelectedVehicleIndex];

	previousVehicleButtonBounds.h = 0.05*dh;
	previousVehicleButtonBounds.x = 0.01*dw;
	previousVehicleButtonBounds.y = 0.5*dh - previousVehicleButtonBounds.h/2;
	previousVehicleButtonBounds.w = 0.04*dw;

	nextVehicleButtonBounds = previousVehicleButtonBounds;
	nextVehicleButtonBounds.x = 0.99*dw - nextVehicleButtonBounds.w;

	previousApperanceButtonBounds.x = 0.485*dw;
	previousApperanceButtonBounds.y = 0.295*dh;
	previousApperanceButtonBounds.w = 0.030*dw;
	previousApperanceButtonBounds.h = 0.010*dw;

	nextAppearanceButtonBounds = previousApperanceButtonBounds;
	nextAppearanceButtonBounds.y = 0.580*dh;

	backButton.bounds.x = 0.01*dw;
	backButton.bounds.y = 0.95*dh - fontGui->getHeight();
	backButton.bounds.w = fontGui->getTextWidth(" Back ");
	backButton.bounds.h = fontGui->getHeight();
	backButton.highlightSpacing = 0.007*dh;

	selectButton.bounds.x = 0.85*dw;
	selectButton.bounds.y = 0.95*dh - fontGui->getHeight();
	selectButton.bounds.w = fontGui->getTextWidth(" Select ");
	selectButton.bounds.h = fontGui->getHeight();
	selectButton.highlightSpacing = backButton.highlightSpacing;
}

void VehicleSelectionShowroomLayoutState::onLeave()
{}

void VehicleSelectionShowroomLayoutState::update(float delta)
{
	bool justFinishedTransitioning = false;
	if(isSelectionTransitioning)
	{
		selectionTransitionProgress += 6*(((int) menu.getSelectedIndex()) - previousIndex) * delta;

		if(fabs(selectionTransitionProgress) > 0.99)
		{
			isSelectionTransitioning = false;
			justFinishedTransitioning = true;
		}
	}

	if(not isSelectionTransitioning)
	{
		// small optimization
		if(justFinishedTransitioning)
		{
			if(previousIndex < (int) menu.getSelectedIndex())
			{
				Image* const tmp = previewPreviousSprite;
				previewPreviousSprite = previewCurrentSprite;
				previewPreviousSpriteFilename = previewCurrentSpriteFilename;
				previewCurrentSprite = previewNextSprite;
				previewCurrentSpriteFilename = previewNextSpriteFilename;
				previewNextSprite = tmp;  // force delete unused
				previewNextSpriteFilename.clear();  // force cache miss
			}
			if(previousIndex > (int) menu.getSelectedIndex())
			{
				Image* const tmp = previewNextSprite;
				previewNextSprite = previewCurrentSprite;
				previewNextSpriteFilename = previewCurrentSpriteFilename;
				previewCurrentSprite = previewPreviousSprite;
				previewCurrentSpriteFilename = previewPreviousSpriteFilename;
				previewPreviousSprite = tmp;  // force delete unused
				previewPreviousSpriteFilename.clear();  // force cache miss
			}
			previousIndex = -1;
		}

		if(menu.getSelectedIndex() == 0)
			reloadSpriteIfMiss(menu.getEntries().size()-1, previewPreviousSprite, previewPreviousSpriteFilename);
		else
			reloadSpriteIfMiss(menu.getSelectedIndex()-1, previewPreviousSprite, previewPreviousSpriteFilename);

		reloadSpriteIfMiss(menu.getSelectedIndex(), previewCurrentSprite, previewCurrentSpriteFilename);

		if(menu.getSelectedIndex() == menu.getEntries().size()-1)
			reloadSpriteIfMiss(0, previewNextSprite, previewNextSpriteFilename);
		else
			reloadSpriteIfMiss(menu.getSelectedIndex()+1, previewNextSprite, previewNextSpriteFilename);
	}
}

void VehicleSelectionShowroomLayoutState::reloadSpriteIfMiss(unsigned menuIndex, Image*& previewSprite, string& previewSpriteFilename)
{
	const vector<Pseudo3DVehicle::Spec>& vehiclesSpecs = game.logic.getVehicleList();
	if(previewSprite == null
		or (previewAltIndex[menuIndex] == -1 and previewSpriteFilename != vehiclesSpecs[menuIndex].sprite.sheetFilename)
		or (previewAltIndex[menuIndex] >=  0 and previewSpriteFilename != vehiclesSpecs[menuIndex].alternateSprites[previewAltIndex[menuIndex]].sheetFilename))
	{
		if(previewSprite != null)
			delete previewSprite;

		const Pseudo3DVehicle::Spec& spec = vehiclesSpecs[menuIndex];

		if(previewAltIndex[menuIndex] == -1)
			previewSpriteFilename = spec.sprite.sheetFilename;
		else
			previewSpriteFilename = spec.alternateSprites[previewAltIndex[menuIndex]].sheetFilename;

		previewSprite = new Image(previewSpriteFilename);
	}
}

void VehicleSelectionShowroomLayoutState::render()
{
	Display& display = game.getDisplay();
	display.clear();
	const float dw = display.getWidth(), dh = display.getHeight();
	const Point mousePos = Mouse::getPosition();
	const bool blinkCycle = (cos(20*fgeal::uptime()) > 0);
	const vector<Pseudo3DVehicle::Spec>& vehicles = game.logic.getVehicleList();
	const unsigned index = isSelectionTransitioning? previousIndex : menu.getSelectedIndex();
	const Pseudo3DVehicle::Spec& vehicle = vehicles[index];

	// transition effects
	const float trans = isSelectionTransitioning? selectionTransitionProgress : 0,
				doff = 0.35*sin(trans),  // dynamic offset
	            doffp = -0.15*doff,
				doffc = -0.15*fabs(doff),
				doffn = 0.15*doff;

	imgBackground->drawScaled(0, 0, scaledToSize(imgBackground, display));

	// draw previous vehicle
	if(vehicles.size() > 2 or (vehicles.size() == 2 and index == 1))
	{
		const unsigned i = index == 0? menu.getEntries().size()-1 : index-1;
		drawVehiclePreview(previewPreviousSprite, vehicles[i].sprite, (0.2-doff)*dw, (0.5-doffp)*dh, 1.05-0.05*fabs(trans), trans < -0.5? 0 : -1);
	}

	// draw next vehicle
	if(vehicles.size() > 2 or (vehicles.size() == 2 and index == 0))
	{
		const unsigned i = index == menu.getEntries().size()-1? 0 : index+1;
		drawVehiclePreview(previewNextSprite, vehicles[i].sprite, (0.8-doff)*dw, (0.5-doffn)*dh, 1.05-0.05*fabs(trans), trans > 0.5? 0 : +1);
	}

	// darkening other vehicles
	fgeal::Graphics::drawFilledRectangle(0, 0, dw, dh,Color(0, 0, 0, 128));

	// draw current vehicle
	drawVehiclePreview(previewCurrentSprite, vehicles[index].sprite, (0.5-doff)*dw, (0.45-doffc)*dh, 1.0+0.05*fabs(trans), trans > 0.5? -1 : trans < -0.5? +1 : 0);

	// draw current vehicle info
	const string lblChooseVehicle = "Choose your vehicle";
	fgeal::Graphics::drawFilledRectangle(0.9*dw - fontTitle->getTextWidth(lblChooseVehicle), 0, dw, 1.05*fontTitle->getHeight(), Color::MAROON);
	fontTitle->drawText(lblChooseVehicle, 0.95*dw - fontTitle->getTextWidth(lblChooseVehicle), 0.03*fontTitle->getHeight(), Color::WHITE);

	const string name = vehicle.name.empty()? "--" : vehicle.name;
	const unsigned nameWidth = fontSubtitle->getTextWidth(name);
	const int nameOffset = nameWidth > dw? 0.6*sin(fgeal::uptime())*(nameWidth - dw) : 0;
	const int infoX = 0.25*dw; int infoY = 0.75*dh;

	fgeal::Graphics::drawFilledRectangle(0, infoY - 1.1*fontSubtitle->getHeight(), dw, fontSubtitle->getHeight(), Color::AZURE);
	fontSubtitle->drawText(name, 0.5*(dw-fontSubtitle->getTextWidth(name)) + nameOffset, infoY - 1.1*fontSubtitle->getHeight(), Color::WHITE);
	fgeal::Graphics::drawFilledRectangle(0, infoY - 0.1*fontSubtitle->getHeight(), dw, 0.25*dh, Color::NAVY);
	drawVehicleSpec(infoX,  infoY);

	const float arrowOffset = cos(10*fgeal::uptime()) > 0? 0 : std::max(0.005f*dh, 1.0f);
	const fgeal::Point skinArrowUp1 = { 0.5f*dw, 0.295f*dh - arrowOffset },
					   skinArrowUp2 = { 0.485f*dw, 0.305f*dh - arrowOffset },
					   skinArrowUp3 = { 0.515f*dw, 0.305f*dh - arrowOffset},
					   skinArrowDown1 = { 0.5f*dw, 0.590f*dh + arrowOffset },
					   skinArrowDown2 = { 0.485f*dw, 0.580f*dh + arrowOffset },
					   skinArrowDown3 = { 0.515f*dw, 0.580f*dh + arrowOffset};

	if(not vehicles[menu.getSelectedIndex()].alternateSprites.empty())
	{
		fgeal::Graphics::drawFilledTriangle(skinArrowDown1, skinArrowDown2, skinArrowDown3, nextAppearanceButtonBounds.contains(mousePos)? Color(128  , 192, 255, 192) : Color(64  , 127, 255, 128));
		fgeal::Graphics::drawFilledTriangle(skinArrowUp1, skinArrowUp2, skinArrowUp3, previousApperanceButtonBounds.contains(mousePos)? Color(128  , 192, 255, 192) : Color(64  , 127, 255, 128));

		if(previewAltIndex[menu.getSelectedIndex()] != -1)
		{
			const string txt = "Alternate appearance" + (previewAltIndex[menu.getSelectedIndex()] == 0? "" : " " + futil::to_string(previewAltIndex[menu.getSelectedIndex()]+1) + " ");
			fontInfo->drawText(txt, 0.5*(dw - fontInfo->getTextWidth(txt)), 0.610*dh, Color::AZURE);
		}
	}

	imgArrow1->drawScaled(previousVehicleButtonBounds.x - (blinkCycle and previousVehicleButtonBounds.contains(mousePos)? 2 : 0),
		previousVehicleButtonBounds.y, scaledToRect(imgArrow1, previousVehicleButtonBounds), Image::FLIP_HORIZONTAL);
	imgArrow1->drawScaled(nextVehicleButtonBounds.x + (blinkCycle and nextVehicleButtonBounds.contains(mousePos)? 2 : 0),
		nextVehicleButtonBounds.y, scaledToRect(imgArrow1, nextVehicleButtonBounds));

	backButton.highlighted = blinkCycle and backButton.bounds.contains(mousePos);
	backButton.draw();

	selectButton.highlighted = blinkCycle and selectButton.bounds.contains(mousePos);
	selectButton.draw();
}

void VehicleSelectionShowroomLayoutState::onKeyPressed(Keyboard::Key key)
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
			if(not isSelectionTransitioning)
				changeSprite(false);
			break;

		case Keyboard::KEY_ARROW_DOWN:
			if(not isSelectionTransitioning)
				changeSprite();
			break;

		case Keyboard::KEY_ARROW_LEFT:
			if(not isSelectionTransitioning)
			{
				isSelectionTransitioning = true;
				previousIndex = menu.getSelectedIndex();
				selectionTransitionProgress = 0;

				menu.moveCursorUp();
				game.sharedResources->sndCursorMove.play();
			}
			break;

		case Keyboard::KEY_ARROW_RIGHT:
			if(not isSelectionTransitioning)
			{
				isSelectionTransitioning = true;
				previousIndex = menu.getSelectedIndex();
				selectionTransitionProgress = 0;

				menu.moveCursorDown();
				game.sharedResources->sndCursorMove.play();
			}
			break;

		case Keyboard::KEY_1:
			game.logic.currentVehicleSelectionStateId = CarseGame::VEHICLE_SELECTION_SIMPLE_LIST_STATE_ID;
			game.enterState(game.logic.currentVehicleSelectionStateId);
			break;

		default:
			break;
	}
}

void VehicleSelectionShowroomLayoutState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{
	if(button == fgeal::Mouse::BUTTON_LEFT)
	{
		if(previousVehicleButtonBounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ARROW_LEFT);

		else if(nextVehicleButtonBounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ARROW_RIGHT);

		else if(previousApperanceButtonBounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ARROW_UP);

		else if(nextAppearanceButtonBounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ARROW_DOWN);

		else if(selectButton.bounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ENTER);

		else if(backButton.bounds.contains(x, y))
			this->onKeyPressed(Keyboard::KEY_ESCAPE);
	}
}

void VehicleSelectionShowroomLayoutState::onJoystickAxisMoved(unsigned joystick, unsigned axis, float oldValue, float newValue)
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

void VehicleSelectionShowroomLayoutState::onJoystickButtonPressed(unsigned joystick, unsigned button)
{
	if(button == 0)
		this->onKeyPressed(Keyboard::KEY_ENTER);
	if(button == 1)
		this->onKeyPressed(Keyboard::KEY_ESCAPE);
}

void VehicleSelectionShowroomLayoutState::menuSelectionAction()
{
	game.logic.setPickedVehicle(menu.getSelectedIndex(), previewAltIndex[menu.getSelectedIndex()]);
	game.enterState(game.logic.currentMainMenuStateId);
}

void VehicleSelectionShowroomLayoutState::drawVehiclePreview(Image* sprite, const Pseudo3DVehicleAnimationSpec& spriteSpec, float x, float y, float scale, int angleType)
{
	Display& display = game.getDisplay();

	const Image::FlipMode flipMode = (angleType > 0 ? Image::FLIP_HORIZONTAL : Image::FLIP_NONE);
	const float scalex = display.getWidth() * 0.0048828125f * scale * spriteSpec.scale.x,
				scaley = display.getWidth() * 0.0048828125f * scale * spriteSpec.scale.y,
				posX = x - 0.5*spriteSpec.frameWidth * scalex,
				posY = y - 0.5*spriteSpec.frameHeight * scaley,
				offsetY = (angleType == 0? 0 : spriteSpec.frameHeight * (spriteSpec.stateCount/2));

	sprite->drawScaledRegion(posX, posY, scalex, scaley, flipMode, 0, offsetY, spriteSpec.frameWidth, spriteSpec.frameHeight);
}

void VehicleSelectionShowroomLayoutState::drawVehicleSpec(float infoX, float infoY, float index)
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

void VehicleSelectionShowroomLayoutState::changeSprite(bool forward)
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
