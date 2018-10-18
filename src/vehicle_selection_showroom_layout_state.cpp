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

int VehicleSelectionShowroomLayoutState::getId() { return CarseGame::VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_ID; }

VehicleSelectionShowroomLayoutState::VehicleSelectionShowroomLayoutState(CarseGame* game)
: State(*game), game(*game),
  fontMain(null), fontInfo(null), fontSub(null), menu(null),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  lastEnterSelectedVehicleIndex(0), lastEnterSelectedVehicleAltIndex(0),
  imgBackground(null), isSelectionTransitioning(false), previousIndex(-1), selectionTransitionProgress(0)
{}

VehicleSelectionShowroomLayoutState::~VehicleSelectionShowroomLayoutState()
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

	if(imgBackground != null)
		delete imgBackground;
}

void VehicleSelectionShowroomLayoutState::initialize()
{
	Display& display = game.getDisplay();
	Rectangle menuBounds = {0.0625f*display.getWidth(), 0.25f*display.getHeight(), 0.4f*display.getWidth(), 0.5f*display.getHeight()};
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

	imgBackground = new Image("assets/showroom-bg.jpg");
}

void VehicleSelectionShowroomLayoutState::onEnter()
{
	lastEnterSelectedVehicleIndex = menu->getSelectedIndex();
	lastEnterSelectedVehicleAltIndex = previews[menu->getSelectedIndex()].altIndex;
}

void VehicleSelectionShowroomLayoutState::onLeave()
{}

void VehicleSelectionShowroomLayoutState::render()
{
	Display& display = game.getDisplay();
	display.clear();
	const float dw = display.getWidth(), dh = display.getHeight();
	const vector<Pseudo3DVehicle::Spec>& vehicles = game.logic.getVehicleList();
	const unsigned index = isSelectionTransitioning? previousIndex : menu->getSelectedIndex();
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
		const unsigned i = index == 0? menu->getEntries().size()-1 : index-1;
		drawVehiclePreview((0.2-doff)*dw, (0.5-doffp)*dh, 1.05-0.05*fabs(trans), i, trans < -0.5? 0 : -1);
	}

	// draw next vehicle
	if(vehicles.size() > 2 or (vehicles.size() == 2 and index == 0))
	{
		const unsigned i = index == menu->getEntries().size()-1? 0 : index+1;
		drawVehiclePreview((0.8-doff)*dw, (0.5-doffn)*dh, 1.05-0.05*fabs(trans), i, trans > 0.5? 0 : +1);
	}

	// darkening other vehicles
	fgeal::Graphics::drawFilledRectangle(0, 0, dw, dh,Color(0, 0, 0, 128));

	// draw current vehicle
	drawVehiclePreview((0.5-doff)*dw, (0.45-doffc)*dh, 1.0+0.05*fabs(trans), index, trans > 0.5? -1 : trans < -0.5? +1 : 0);

	// draw current vehicle info
	const string lblChooseVehicle = "Choose your vehicle";
	fgeal::Graphics::drawFilledRectangle(0.9*dw - fontMain->getTextWidth(lblChooseVehicle), 0, dw, 1.05*fontMain->getHeight(), Color::MAROON);
	fontMain->drawText(lblChooseVehicle, 0.95*dw - fontMain->getTextWidth(lblChooseVehicle), 0.03*fontMain->getHeight(), Color::WHITE);

	const string name = vehicle.name.empty()? "--" : vehicle.name;
	const unsigned nameWidth = fontSub->getTextWidth(name);
	const int nameOffset = nameWidth > dw? 0.6*sin(fgeal::uptime())*(nameWidth - dw) : 0;
	const int infoX = 0.25*dw; int infoY = 0.75*dh;

	fgeal::Graphics::drawFilledRectangle(0, infoY - 1.1*fontSub->getHeight(), dw, fontSub->getHeight(), Color::AZURE);
	fontSub->drawText(name, 0.5*(dw-fontSub->getTextWidth(name)) + nameOffset, infoY - 1.1*fontSub->getHeight(), Color::WHITE);
	fgeal::Graphics::drawFilledRectangle(0, infoY - 0.1*fontSub->getHeight(), dw, 0.25*dh, Color::NAVY);
	drawVehicleSpec(infoX,  infoY);

	VehiclePreview& preview = previews[menu->getSelectedIndex()];
	const float arrowOffset = cos(10*fgeal::uptime()) > 0? 0 : std::max(0.005f*dh, 1.0f);
	const fgeal::Point skinArrowUp1 = { 0.5f*dw, 0.295f*dh - arrowOffset },
					   skinArrowUp2 = { 0.485f*dw, 0.305f*dh - arrowOffset },
					   skinArrowUp3 = { 0.515f*dw, 0.305f*dh - arrowOffset},
					   skinArrowDown1 = { 0.5f*dw, 0.590f*dh + arrowOffset },
					   skinArrowDown2 = { 0.485f*dw, 0.580f*dh + arrowOffset },
					   skinArrowDown3 = { 0.515f*dw, 0.580f*dh + arrowOffset};

	if(not preview.altSprites.empty())
	{
		fgeal::Graphics::drawFilledTriangle(skinArrowDown1, skinArrowDown2, skinArrowDown3, Color(0  , 127, 255, 192));
		fgeal::Graphics::drawFilledTriangle(skinArrowUp1, skinArrowUp2, skinArrowUp3, Color(0  , 127, 255, 192));

		if(preview.altIndex != -1)
		{
			const string txt = "Alternate appearance" + (preview.altSprites.size() == 1? " " : " " + futil::to_string(preview.altIndex+1) + " ");
			fontInfo->drawText(txt, 0.5*(dw - fontInfo->getTextWidth(txt)), 0.610*dh, Color::AZURE);
		}
	}
}

void VehicleSelectionShowroomLayoutState::update(float delta)
{
	if(isSelectionTransitioning)
	{
		selectionTransitionProgress += 6*(((int) menu->getSelectedIndex()) - previousIndex) * delta;

		if(fabs(selectionTransitionProgress) > 0.99)
		{
			isSelectionTransitioning = false;
			previousIndex = -1;
		}
	}
}

void VehicleSelectionShowroomLayoutState::onKeyPressed(Keyboard::Key key)
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
				previousIndex = menu->getSelectedIndex();
				selectionTransitionProgress = 0;

				menu->moveCursorUp();
				game.sharedResources->sndCursorMove.play();
			}
			break;

		case Keyboard::KEY_ARROW_RIGHT:
			if(not isSelectionTransitioning)
			{
				isSelectionTransitioning = true;
				previousIndex = menu->getSelectedIndex();
				selectionTransitionProgress = 0;

				menu->moveCursorDown();
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

void VehicleSelectionShowroomLayoutState::menuSelectionAction()
{
	game.logic.setPickedVehicle(menu->getSelectedIndex(), previews[menu->getSelectedIndex()].altIndex);
	game.enterState(game.logic.currentMainMenuStateId);
}

void VehicleSelectionShowroomLayoutState::drawVehiclePreview(float x, float y, float scale, int index, int angleType)
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

void VehicleSelectionShowroomLayoutState::drawVehicleSpec(float infoX, float infoY, float index)
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

void VehicleSelectionShowroomLayoutState::changeSprite(bool forward)
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
