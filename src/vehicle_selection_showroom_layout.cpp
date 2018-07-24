/*
 * vehicle_selection_showroom_layout.cpp
 *
 *  Created on: 24 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "vehicle_selection_state.hpp"

#include "carse_game.hpp"

typedef GenericMenuStateLayout<VehicleSelectionState> Layout;

using fgeal::Display;
using fgeal::Color;
using fgeal::Font;
using fgeal::Menu;
using std::vector;
using std::string;

VehicleSelectionState::ShowroomLayout::ShowroomLayout(VehicleSelectionState& state)
: Layout(state), imgBackground("assets/showroom-bg.jpg"),
  isSelectionTransitioning(false), previousIndex(-1), selectionTransitionProgress(0)
{}

void VehicleSelectionState::ShowroomLayout::draw()
{
	Display& display = state.game.getDisplay();
	const float dw = display.getWidth(), dh = display.getHeight();
	Font& fontMain = *state.fontMain, &fontSub = *state.fontSub;
	Menu* const menu = state.menu;
	const vector<Pseudo3DVehicle::Spec>& vehicles = state.gameLogic.getVehicleList();
	const unsigned index = isSelectionTransitioning? previousIndex : menu->getSelectedIndex();
	const Pseudo3DVehicle::Spec& vehicle = vehicles[index];

	// transition effects
	const float trans = isSelectionTransitioning? selectionTransitionProgress : 0,
				doff = 0.35*sin(trans),  // dynamic offset
	            doffp = -0.15*doff,
				doffc = -0.15*fabs(doff),
				doffn = 0.15*doff;

	imgBackground.drawScaled(0, 0, scaledToSize(&imgBackground, display));

	// draw previous vehicle
	if(vehicles.size() > 2 or (vehicles.size() == 2 and index == 1))
	{
		const unsigned i = index == 0? menu->getNumberOfEntries()-1 : index-1;
		state.drawVehiclePreview((0.2-doff)*dw, (0.5-doffp)*dh, 1.05-0.05*fabs(trans), i, trans < -0.5? 0 : -1);
	}

	// draw next vehicle
	if(vehicles.size() > 2 or (vehicles.size() == 2 and index == 0))
	{
		const unsigned i = index == menu->getNumberOfEntries()-1? 0 : index+1;
		state.drawVehiclePreview((0.8-doff)*dw, (0.5-doffn)*dh, 1.05-0.05*fabs(trans), i, trans > 0.5? 0 : +1);
	}

	// darkening other vehicles
	fgeal::Graphics::drawFilledRectangle(0, 0, dw, dh,Color(0, 0, 0, 128));

	// draw current vehicle
	state.drawVehiclePreview((0.5-doff)*dw, (0.45-doffc)*dh, 1.0+0.05*fabs(trans), index, trans > 0.5? -1 : trans < -0.5? +1 : 0);

	// draw current vehicle info
	const string lblChooseVehicle = "Choose your vehicle";
	fgeal::Graphics::drawFilledRectangle(0.9*dw - fontMain.getTextWidth(lblChooseVehicle), 0, dw, 1.05*fontMain.getHeight(), Color::DARK_GREEN);
	fontMain.drawText(lblChooseVehicle, 0.95*dw - fontMain.getTextWidth(lblChooseVehicle), 0.03*fontMain.getHeight(), Color::WHITE);

	const string name = vehicle.name.empty()? "--" : vehicle.name;
	const unsigned nameWidth = fontSub.getTextWidth(name);
	const int nameOffset = nameWidth > dw? 0.6*sin(fgeal::uptime())*(nameWidth - dw) : 0;
	const int infoX = 0.25*dw; int infoY = 0.75*dh;

	fgeal::Graphics::drawFilledRectangle(0, infoY - 1.1*fontSub.getHeight(), dw, fontSub.getHeight(), Color::AZURE);
	fontSub.drawText(name, 0.5*(dw-fontSub.getTextWidth(name)) + nameOffset, infoY - 1.1*fontSub.getHeight(), Color::WHITE);
	fgeal::Graphics::drawFilledRectangle(0, infoY - 0.1*fontSub.getHeight(), dw, 0.25*dh, Color::NAVY);
	state.drawVehicleSpec(infoX,  infoY);

	VehiclePreview& preview = state.previews[state.menu->getSelectedIndex()];
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
			state.fontInfo->drawText(txt, 0.5*(dw - state.fontInfo->getTextWidth(txt)), 0.610*dh, Color::AZURE);
		}
	}
}

void VehicleSelectionState::ShowroomLayout::update(float delta)
{
	if(isSelectionTransitioning)
	{
		selectionTransitionProgress += 6*(((int) state.menu->getSelectedIndex()) - previousIndex) * delta;

		if(fabs(selectionTransitionProgress) > 0.99)
		{
			isSelectionTransitioning = false;
			previousIndex = -1;
		}
	}
}

void VehicleSelectionState::ShowroomLayout::onCursorUp()
{
	if(not isSelectionTransitioning)
		state.changeSprite(false);
}

void VehicleSelectionState::ShowroomLayout::onCursorDown()
{
	if(not isSelectionTransitioning)
		state.changeSprite();
}

void VehicleSelectionState::ShowroomLayout::onCursorLeft()
{
	if(not isSelectionTransitioning)
	{
		isSelectionTransitioning = true;
		previousIndex = state.menu->getSelectedIndex();
		selectionTransitionProgress = 0;

		state.menu->moveCursorUp();
		state.shared.sndCursorMove.stop();
		state.shared.sndCursorMove.play();
	}
}

void VehicleSelectionState::ShowroomLayout::onCursorRight()
{
	if(not isSelectionTransitioning)
	{
		isSelectionTransitioning = true;
		previousIndex = state.menu->getSelectedIndex();
		selectionTransitionProgress = 0;

		state.menu->moveCursorDown();
		state.shared.sndCursorMove.stop();
		state.shared.sndCursorMove.play();
	}
}

void VehicleSelectionState::ShowroomLayout::onCursorAccept()
{}
