/*
 * main_menu_car_panel_layout.cpp
 *
 *  Created on: 23 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "main_menu_state.hpp"

#include "carse_game.hpp"

#include "futil/string_extra_operators.hpp"

using std::string;
using fgeal::Display;
using fgeal::Color;
using fgeal::Image;
using fgeal::Rectangle;

typedef GenericMenuStateLayout<MainMenuState> Layout;

MainMenuState::PrototypeGridLayout::PrototypeGridLayout(MainMenuState& state)
: Layout(state),
  fontMain(state.shared.font1Path,   18 * (state.game.getDisplay().getHeight()/480.0)),
  fontTitle(state.shared.font2Path, 40 * (state.game.getDisplay().getHeight()/480.0))
{}

void MainMenuState::PrototypeGridLayout::drawGridSlot(const fgeal::Rectangle& slot, const fgeal::Vector2D& margin, int index)
{
	const bool isSelected = (index == (int) state.menu->getSelectedIndex());
	fgeal::Graphics::drawFilledRectangle(slot.x, slot.y, slot.w, slot.h, Color::DARK_GREY);
	fgeal::Graphics::drawFilledRectangle(slot.x + margin.x, slot.y + margin.y, slot.w - margin.x*2, slot.h - margin.y*2, isSelected? Color::LIGHT_GREY : Color::GREY);
	const float textWidth = fontMain.getTextWidth(state.menu->at(index).label);
	fontMain.drawText(state.menu->at(index).label, slot.x + 0.5*(slot.w - textWidth), slot.y * 1.02f, isSelected? selectedSlotColor : Color::WHITE);
	if(isSelected) fgeal::Graphics::drawRectangle(slot.x, slot.y, slot.w, slot.h, selectedSlotColor);
}

void MainMenuState::PrototypeGridLayout::draw()
{
	const float w = state.game.getDisplay().getWidth(),
				h = state.game.getDisplay().getHeight(),
				titleHeaderHeight = 0.2 * w,
				marginX = w * 0.01, marginY = h * 0.01;

	const fgeal::Vector2D margin = {marginX, marginY};

	const Rectangle slotSize = {0, 0, (w - 4*marginX)/3.0f, (h - titleHeaderHeight - 3*marginY)/2.0f};

	// draw MENU_ITEM_RACE
	const Rectangle slotMenuItemRace = {
		0.5f*(w - slotSize.w),
		titleHeaderHeight + 0.5f*(h - titleHeaderHeight - slotSize.h),
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemRace, margin, MENU_ITEM_RACE);
	state.imgRace->drawScaled(slotMenuItemRace.x*1.01, slotMenuItemRace.y*1.01,
		slotMenuItemRace.w * 0.98f / state.imgRace->getWidth(), slotMenuItemRace.h * 0.98f / state.imgRace->getHeight());

	// draw MENU_ITEM_VEHICLE
	const Rectangle slotMenuItemVehicle = {
		marginX,
		titleHeaderHeight + marginY,
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemVehicle, margin, MENU_ITEM_VEHICLE);
	state.logic.drawPickedVehicle(slotMenuItemVehicle.x + 0.5*slotMenuItemVehicle.w, slotMenuItemVehicle.y + 0.625*slotMenuItemVehicle.h, 0.75);

	// draw MENU_ITEM_COURSE
	const Rectangle slotMenuItemCourse = {
		marginX,
		slotMenuItemVehicle.y + slotMenuItemVehicle.h + marginY,
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemCourse, margin, MENU_ITEM_COURSE);
	Image* portrait = state.logic.getNextCoursePreviewImage();
	const float portraitX = slotMenuItemCourse.x + 0.125f*slotMenuItemCourse.w*(1 - 0.75f/portrait->getWidth()),
				portraitY = slotMenuItemCourse.y + 0.1750f*slotMenuItemCourse.h*(1 - 0.75f/portrait->getHeight()),
				portraitScaleX = 0.75f*slotMenuItemCourse.w/portrait->getWidth(),
				portraitScaleY = 0.75f*slotMenuItemCourse.h/portrait->getHeight();
	portrait->drawScaled(portraitX, portraitY, portraitScaleX, portraitScaleY);

	// draw MENU_ITEM_SETTINGS
	const Rectangle slotMenuItemSettings = {
		slotMenuItemRace.x + slotMenuItemRace.w + marginX,
		titleHeaderHeight + marginY,
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemSettings, margin, MENU_ITEM_SETTINGS);
	const float imgSettingsX = slotMenuItemSettings.x + 0.125f*slotMenuItemSettings.w*(1 - 0.75f/state.imgSettings->getWidth()),
				imgSettingsY = slotMenuItemSettings.y + 0.1750f*slotMenuItemSettings.h*(1 - 0.75f/state.imgSettings->getHeight()),
				imgSettingsScaleX = 0.75f*slotMenuItemSettings.w/state.imgSettings->getWidth(),
				imgSettingsScaleY = 0.75f*slotMenuItemSettings.h/state.imgSettings->getHeight();
	state.imgSettings->drawScaled(imgSettingsX, imgSettingsY, imgSettingsScaleX, imgSettingsScaleY);

	// draw MENU_ITEM_EXIT
	const Rectangle slotMenuItemExit = {
		slotMenuItemRace.x + slotMenuItemRace.w + marginX,
		slotMenuItemSettings.y + slotMenuItemSettings.h + marginY,
		slotSize.w, slotSize.h
	};
	drawGridSlot(slotMenuItemExit, margin, MENU_ITEM_EXIT);
	const float imgExitX = slotMenuItemExit.x + 0.125f*slotMenuItemExit.w*(1 - 0.75f/state.imgExit->getWidth()),
				imgExitY = slotMenuItemExit.y + 0.1750f*slotMenuItemExit.h*(1 - 0.75f/state.imgExit->getHeight()),
				imgExitScaleX = 0.75f*slotMenuItemExit.w/state.imgExit->getWidth(),
				imgExitScaleY = 0.75f*slotMenuItemExit.h/state.imgExit->getHeight();
	state.imgExit->drawScaled(imgExitX, imgExitY, imgExitScaleX, imgExitScaleY);

	const string title("Carse Project");
	fontTitle.drawText(title, 0.5*(state.game.getDisplay().getWidth() - fontTitle.getTextWidth(title)),
							  0.1*(state.game.getDisplay().getHeight() - fontTitle.getHeight()), Color::WHITE);
}

void MainMenuState::PrototypeGridLayout::update(float delta)
{
	selectedSlotColor = cos(20*fgeal::uptime()) > 0? Color::RED : Color::MAROON;
}

void MainMenuState::PrototypeGridLayout::onCursorUp()
{
	const unsigned index = state.menu->getSelectedIndex();
	if(index == 2 or index == 4)
	{
		state.menu->setSelectedIndex(index-1);
		state.shared.sndCursorMove.stop();
		state.shared.sndCursorMove.play();
	}
}

void MainMenuState::PrototypeGridLayout::onCursorDown()
{
	const unsigned index = state.menu->getSelectedIndex();
	if(index == 1 or index == 3)
	{
		state.menu->setSelectedIndex(index+1);
		state.shared.sndCursorMove.stop();
		state.shared.sndCursorMove.play();
	}
}

void MainMenuState::PrototypeGridLayout::onCursorLeft()
{
	const unsigned index = state.menu->getSelectedIndex();
	if(index == 0 or index == 3 or index == 4)
	{
		state.menu->setSelectedIndex(index==0? 1 : 0);
		state.shared.sndCursorMove.stop();
		state.shared.sndCursorMove.play();
	}
}

void MainMenuState::PrototypeGridLayout::onCursorRight()
{
	const unsigned index = state.menu->getSelectedIndex();
	if(index == 0 or index == 1 or index == 2)
	{
		state.menu->setSelectedIndex(index==0? 3 : 0);
		state.shared.sndCursorMove.stop();
		state.shared.sndCursorMove.play();
	}
}

void MainMenuState::PrototypeGridLayout::onCursorAccept()
{
	state.shared.sndCursorIn.play();
	Layout::onCursorAccept();
}
