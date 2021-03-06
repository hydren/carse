/*
 * main_menu_retro_layout_state.cpp
 *
 *  Created on: 2 de ago de 2018
 *      Author: carlosfaruolo
 */

#include "main_menu_retro_layout_state.hpp"

#include "carse_game.hpp"

#include <cmath>

using std::string;
using fgeal::Display;
using fgeal::Image;
using fgeal::Font;
using fgeal::Color;
using fgeal::Rectangle;
using fgeal::Point;
using fgeal::Keyboard;
using fgeal::Mouse;

int MainMenuRetroLayoutState::getId() { return CarseGame::MAIN_MENU_CLASSIC_LAYOUT_STATE_ID; }

MainMenuRetroLayoutState::MainMenuRetroLayoutState(CarseGame* game)
: State(*game), game(*game), lastDisplaySize(),
  imgBackground(null), imgRace(null), imgExit(null), imgSettings(null), imgCourse(null), imgVehicle(null),
  fntTitle(null), fntMain(null),
  selectedItemIndex(0),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null)
{}

MainMenuRetroLayoutState::~MainMenuRetroLayoutState()
{
	if(imgBackground != null) delete imgBackground;
	if(imgRace != null) delete imgRace;
	if(imgExit != null) delete imgExit;
	if(imgSettings != null) delete imgSettings;
	if(imgCourse != null) delete imgCourse;
	if(imgVehicle != null) delete imgVehicle;
	if(fntTitle != null) delete fntTitle;
	if(fntMain != null) delete fntMain;
}

void MainMenuRetroLayoutState::initialize()
{
	vecStrItems.push_back("Race!");
	vecStrItems.push_back("Vehicle");
	vecStrItems.push_back("Course");
	vecStrItems.push_back("Options");
	vecStrItems.push_back("Exit");

	imgBackground = new Image("assets/bg-main.jpg");
	imgRace = new Image("assets/race.png");
	imgExit = new Image("assets/exit.png");
	imgSettings = new Image("assets/settings.png");

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;

	strTitle = "Carse Project";
	fntTitle = new Font(game.sharedResources->font2Path);

	fntMain = new Font(game.sharedResources->font1Path);

	strVersion = string("v")+CARSE_VERSION+" (fgeal v"+fgeal::VERSION+"/"+fgeal::ADAPTED_LIBRARY_NAME+" v"+fgeal::ADAPTED_LIBRARY_VERSION+")";
}

void MainMenuRetroLayoutState::onEnter()
{
	Display& display = game.getDisplay();
	const float w = display.getWidth(),
				h = display.getHeight(),
				titleHeaderHeight = 0.2 * w,
				marginX = w * 0.01, marginY = h * 0.01;

	const Rectangle slotSize = {0, 0, (w - 4*marginX)/3.0f, (h - titleHeaderHeight - 3*marginY)/2.0f};

	// reload fonts if display size changed
	if(lastDisplaySize.x != w or lastDisplaySize.y != h)
	{
		const FontSizer fs(display.getHeight());
		fntTitle->setSize(fs(32));
		fntMain->setSize(fs(18));
		lastDisplaySize.x = w;
		lastDisplaySize.y = h;
	}

	slotMenuItemRace.x = 0.5f*(w - slotSize.w);
	slotMenuItemRace.y = titleHeaderHeight + 0.5f*(h - titleHeaderHeight - slotSize.h);
	slotMenuItemRace.w = slotSize.w;
	slotMenuItemRace.h = slotSize.h;

	slotMenuItemVehicle.x = marginX;
	slotMenuItemVehicle.y = titleHeaderHeight + marginY;
	slotMenuItemVehicle.w = slotSize.w;
	slotMenuItemVehicle.h = slotSize.h;

	slotMenuItemCourse.x = marginX;
	slotMenuItemCourse.y = slotMenuItemVehicle.y + slotMenuItemVehicle.h + marginY;
	slotMenuItemCourse.w = slotSize.w;
	slotMenuItemCourse.h = slotSize.h;

	slotMenuItemSettings.x = slotMenuItemRace.x + slotMenuItemRace.w + marginX;
	slotMenuItemSettings.y = titleHeaderHeight + marginY;
	slotMenuItemSettings.w = slotSize.w;
	slotMenuItemSettings.h = slotSize.h;

	slotMenuItemExit.x = slotMenuItemRace.x + slotMenuItemRace.w + marginX;
	slotMenuItemExit.y = slotMenuItemSettings.y + slotMenuItemSettings.h + marginY;
	slotMenuItemExit.w = slotSize.w;
	slotMenuItemExit.h = slotSize.h;

	selectedItemIndex = 0;

	if(imgCourse != null)
		delete imgCourse;

	imgCourse = new Image(game.logic.getNextCourse().previewFilename.empty()? "assets/portrait-circuit.png" : game.logic.getNextCourse().previewFilename);

	if(imgVehicle != null)
		delete imgVehicle;

	const Pseudo3DVehicleAnimationSpec& vspec = (
		game.logic.getPickedVehicleAlternateSpriteIndex() == -1? game.logic.getPickedVehicle().sprite
															   : game.logic.getPickedVehicle().alternateSprites[game.logic.getPickedVehicleAlternateSpriteIndex()]
	);

	imgVehicle = new Image(vspec.sheetFilename);
	scaleVehiclePreview.x = vspec.scale.x * display.getWidth() * 0.0037f;
	scaleVehiclePreview.y = vspec.scale.y * display.getWidth() * 0.0037f;
	rtSrcVehiclePreview.x = rtSrcVehiclePreview.y = 0;
	rtSrcVehiclePreview.w = vspec.frameWidth;
	rtSrcVehiclePreview.h = vspec.frameHeight;
	ptVehiclePreview.x = slotMenuItemVehicle.x + 0.5f * slotMenuItemVehicle.w - scaleVehiclePreview.x * 0.5f * vspec.frameWidth;
	ptVehiclePreview.y = slotMenuItemVehicle.y + 0.8f * slotMenuItemVehicle.h - scaleVehiclePreview.y * (vspec.frameHeight - vspec.contactOffset);
}

void MainMenuRetroLayoutState::onLeave()
{}

void MainMenuRetroLayoutState::render()
{
	Display& display = game.getDisplay();
	display.clear();
	imgBackground->drawScaled(0, 0, scaledToSize(imgBackground, display));

	const float w = display.getWidth(),
				h = display.getHeight(),
				marginX = w * 0.01, marginY = h * 0.01;

	const fgeal::Vector2D margin = {marginX, marginY};

	drawGridSlot(slotMenuItemRace, margin, MENU_ITEM_RACE);
	imgRace->drawScaled(slotMenuItemRace.x*1.01, slotMenuItemRace.y*1.01,
		slotMenuItemRace.w * 0.98f / imgRace->getWidth(), slotMenuItemRace.h * 0.98f / imgRace->getHeight());

	drawGridSlot(slotMenuItemVehicle, margin, MENU_ITEM_VEHICLE);

	imgVehicle->drawScaledRegion(ptVehiclePreview, scaleVehiclePreview, Image::FLIP_NONE, rtSrcVehiclePreview);

	drawGridSlot(slotMenuItemCourse, margin, MENU_ITEM_COURSE);
	const float portraitX = slotMenuItemCourse.x + 0.125f*slotMenuItemCourse.w*(1 - 0.75f/imgCourse->getWidth()),
				portraitY = slotMenuItemCourse.y + 0.1750f*slotMenuItemCourse.h*(1 - 0.75f/imgCourse->getHeight()),
				portraitScaleX = 0.75f*slotMenuItemCourse.w/imgCourse->getWidth(),
				portraitScaleY = 0.75f*slotMenuItemCourse.h/imgCourse->getHeight();
	imgCourse->drawScaled(portraitX, portraitY, portraitScaleX, portraitScaleY);

	drawGridSlot(slotMenuItemSettings, margin, MENU_ITEM_SETTINGS);
	const float imgSettingsX = slotMenuItemSettings.x + 0.125f*slotMenuItemSettings.w*(1 - 0.75f/imgSettings->getWidth()),
				imgSettingsY = slotMenuItemSettings.y + 0.1750f*slotMenuItemSettings.h*(1 - 0.75f/imgSettings->getHeight()),
				imgSettingsScaleX = 0.75f*slotMenuItemSettings.w/imgSettings->getWidth(),
				imgSettingsScaleY = 0.75f*slotMenuItemSettings.h/imgSettings->getHeight();
	imgSettings->drawScaled(imgSettingsX, imgSettingsY, imgSettingsScaleX, imgSettingsScaleY);

	drawGridSlot(slotMenuItemExit, margin, MENU_ITEM_EXIT);
	const float imgExitX = slotMenuItemExit.x + 0.125f*slotMenuItemExit.w*(1 - 0.75f/imgExit->getWidth()),
				imgExitY = slotMenuItemExit.y + 0.1750f*slotMenuItemExit.h*(1 - 0.75f/imgExit->getHeight()),
				imgExitScaleX = 0.75f*slotMenuItemExit.w/imgExit->getWidth(),
				imgExitScaleY = 0.75f*slotMenuItemExit.h/imgExit->getHeight();
	imgExit->drawScaled(imgExitX, imgExitY, imgExitScaleX, imgExitScaleY);

	fntTitle->drawText(strTitle, 0.5*(display.getWidth() - fntTitle->getTextWidth(strTitle)), 0.1*(display.getHeight() - fntTitle->getTextHeight()), Color::WHITE);

	game.sharedResources->fontDev.drawText(strVersion, 4, 4, Color::CREAM);
}

void MainMenuRetroLayoutState::drawGridSlot(const fgeal::Rectangle& slot, const fgeal::Vector2D& margin, int index)
{
	const bool isSelected = (index == (int) selectedItemIndex);
	fgeal::Graphics::drawFilledRectangle(slot.x, slot.y, slot.w, slot.h, Color::DARK_GREY);
	fgeal::Graphics::drawFilledRectangle(slot.x + margin.x, slot.y + margin.y, slot.w - margin.x*2, slot.h - margin.y*2, isSelected? Color::LIGHT_GREY : Color::GREY);
	fntMain->drawText(vecStrItems[index], slot.x + 0.5*(slot.w - fntMain->getTextWidth(vecStrItems[index])), slot.y * 1.02f, isSelected? selectedSlotColor : Color::WHITE);
	if(isSelected) fgeal::Graphics::drawRectangle(slot.x, slot.y, slot.w, slot.h, selectedSlotColor);
}

void MainMenuRetroLayoutState::onMenuAccept()
{
	switch(selectedItemIndex)
	{
		case MENU_ITEM_RACE:     game.enterState(CarseGame::RACE_STATE_ID); break;
		case MENU_ITEM_VEHICLE:  game.enterState(game.logic.currentVehicleSelectionStateId); break;
		case MENU_ITEM_COURSE:   game.enterState(CarseGame::COURSE_SELECTION_STATE_ID); break;
		case MENU_ITEM_SETTINGS: game.enterState(CarseGame::OPTIONS_MENU_STATE_ID); break;
		case MENU_ITEM_EXIT:     game.running = false; break;
		default: break;
	}
}

void MainMenuRetroLayoutState::update(float delta)
{
	selectedSlotColor = cos(20*fgeal::uptime()) > 0? Color::RED : Color::MAROON;
}

void MainMenuRetroLayoutState::onKeyPressed(Keyboard::Key key)
{
	switch(key)
	{
		case Keyboard::KEY_ESCAPE:
			game.running = false;
			break;

		case Keyboard::KEY_ENTER:
			sndCursorIn->play();
			this->onMenuAccept();
			break;

		case Keyboard::KEY_ARROW_UP:
			if(selectedItemIndex == 2 or selectedItemIndex == 4)
			{
				selectedItemIndex--;
				sndCursorMove->play();
			}
			break;

		case Keyboard::KEY_ARROW_DOWN:
			if(selectedItemIndex == 1 or selectedItemIndex == 3)
			{
				selectedItemIndex++;
				sndCursorMove->play();
			}
			break;

		case Keyboard::KEY_ARROW_LEFT:
			if(selectedItemIndex == 0 or selectedItemIndex == 3 or selectedItemIndex == 4)
			{
				selectedItemIndex = (selectedItemIndex == 0? 1 : 0);
				sndCursorMove->play();
			}
			break;

		case Keyboard::KEY_ARROW_RIGHT:
			if(selectedItemIndex == 0 or selectedItemIndex == 1 or selectedItemIndex == 2)
			{
				selectedItemIndex = (selectedItemIndex == 0? 3 : 0);
				sndCursorMove->play();
			}
			break;

		case Keyboard::KEY_1:
			game.logic.currentMainMenuStateId = CarseGame::MAIN_MENU_SIMPLE_LIST_STATE_ID;
			game.enterState(game.logic.currentMainMenuStateId);
			break;

		default:break;
	}
}

void MainMenuRetroLayoutState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{
	if(button != Mouse::BUTTON_LEFT)
		return;

	const Point pt = {(float) x, (float) y};
	if(slotMenuItemRace.contains(pt))
	{
		sndCursorIn->play();
		selectedItemIndex = MENU_ITEM_RACE;
		this->onMenuAccept();
	}

	if(slotMenuItemCourse.contains(pt))
	{
		sndCursorIn->play();
		selectedItemIndex = MENU_ITEM_COURSE;
		this->onMenuAccept();
	}

	if(slotMenuItemVehicle.contains(pt))
	{
		sndCursorIn->play();
		selectedItemIndex = MENU_ITEM_VEHICLE;
		this->onMenuAccept();
	}

	if(slotMenuItemSettings.contains(pt))
	{
		sndCursorIn->play();
		selectedItemIndex = MENU_ITEM_SETTINGS;
		this->onMenuAccept();
	}

	if(slotMenuItemExit.contains(pt))
	{
		sndCursorIn->play();
		selectedItemIndex = MENU_ITEM_EXIT;
		this->onMenuAccept();
	}
}

void MainMenuRetroLayoutState::onMouseMoved(int oldx, int oldy, int newx, int newy)
{
	const Point pt = {(float) newx, (float) newy};
	if(slotMenuItemRace.contains(pt) and selectedItemIndex != MENU_ITEM_RACE)
	{
		sndCursorMove->play();
		selectedItemIndex = MENU_ITEM_RACE;
	}

	if(slotMenuItemCourse.contains(pt) and selectedItemIndex != MENU_ITEM_COURSE)
	{
		sndCursorMove->play();
		selectedItemIndex = MENU_ITEM_COURSE;
	}

	if(slotMenuItemVehicle.contains(pt) and selectedItemIndex != MENU_ITEM_VEHICLE)
	{
		sndCursorMove->play();
		selectedItemIndex = MENU_ITEM_VEHICLE;
	}

	if(slotMenuItemSettings.contains(pt) and selectedItemIndex != MENU_ITEM_SETTINGS)
	{
		sndCursorMove->play();
		selectedItemIndex = MENU_ITEM_SETTINGS;
	}

	if(slotMenuItemExit.contains(pt) and selectedItemIndex != MENU_ITEM_EXIT)
	{
		sndCursorMove->play();
		selectedItemIndex = MENU_ITEM_EXIT;
	}
}

void MainMenuRetroLayoutState::onJoystickAxisMoved(unsigned joystick, unsigned axis, float oldValue, float newValue)
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

void MainMenuRetroLayoutState::onJoystickButtonPressed(unsigned joystick, unsigned button)
{
	if(button % 2 == 0)
		this->onKeyPressed(Keyboard::KEY_ENTER);
}
