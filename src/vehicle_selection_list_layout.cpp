/*
 * vehicle_selection_list_layout.cpp
 *
 *  Created on: 24 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "vehicle_selection_state.hpp"

#include "carse_game.hpp"

#include "futil/string_actions.hpp"

typedef GenericMenuStateLayout<VehicleSelectionState> Layout;

using fgeal::Display;
using fgeal::Color;
using std::string;

VehicleSelectionState::ListLayout::ListLayout(VehicleSelectionState& state)
: Layout(state)
{}

void VehicleSelectionState::ListLayout::draw()
{
	Display& display = state.game.getDisplay();
	const float dw = display.getWidth(), dh = display.getHeight();
	state.menu->draw();
	state.fontSub->drawText("Choose your vehicle", 32, 25, Color::WHITE);
	state.drawVehiclePreview(0.7*dw, 0.35*dh);
	state.drawVehicleSpec(0.525*dw,  0.525*dh);

	VehiclePreview& preview = state.previews[state.menu->getSelectedIndex()];
	const fgeal::Point skinArrowLeft1 =  { 0.52f*dw, 0.45f*dh }, skinArrowLeft2  = { 0.53f*dw, 0.44f*dh }, skinArrowLeft3 =  { 0.53f*dw, 0.46f*dh};
	const fgeal::Point skinArrowRight1 = { 0.88f*dw, 0.45f*dh }, skinArrowRight2 = { 0.87f*dw, 0.44f*dh }, skinArrowRight3 = { 0.87f*dw, 0.46f*dh};
	if(not preview.altSprites.empty())
	{
		fgeal::Graphics::drawFilledTriangle(skinArrowLeft1, skinArrowLeft2, skinArrowLeft3, Color::AZURE);
		fgeal::Graphics::drawFilledTriangle(skinArrowRight1, skinArrowRight2, skinArrowRight3, Color::AZURE);
		if(preview.altIndex != -1)
		{
			const string txt = "Alternate appearance" + (preview.altSprites.size() == 1? " " : " " + futil::to_string(preview.altIndex+1) + " ");
			state.fontInfo->drawText(txt, dw - state.fontInfo->getTextWidth(txt), 0.5*dh, Color::AZURE);
		}
	}
	else
	{
		fgeal::Graphics::drawFilledTriangle(skinArrowLeft1, skinArrowLeft2, skinArrowLeft3, Color::DARK_GREY);
		fgeal::Graphics::drawFilledTriangle(skinArrowRight1, skinArrowRight2, skinArrowRight3, Color::DARK_GREY);
	}
}

void VehicleSelectionState::ListLayout::update(float delta)
{}

void VehicleSelectionState::ListLayout::onCursorUp()
{
	state.menu->moveCursorUp();
	state.game.sharedResources->sndCursorMove.stop();
	state.game.sharedResources->sndCursorMove.play();
}

void VehicleSelectionState::ListLayout::onCursorDown()
{
	state.menu->moveCursorDown();
	state.game.sharedResources->sndCursorMove.stop();
	state.game.sharedResources->sndCursorMove.play();
}

void VehicleSelectionState::ListLayout::onCursorLeft()
{
	state.changeSprite(false);
}

void VehicleSelectionState::ListLayout::onCursorRight()
{
	state.changeSprite();
}

void VehicleSelectionState::ListLayout::onCursorAccept()
{}
