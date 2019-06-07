/*
 * options_menu_state.cpp
 *
 *  Created on: 5 de set de 2017
 *      Author: carlosfaruolo
 */

#include "options_menu_state.hpp"
#include "futil/string_actions.hpp"

#include "util.hpp"

using fgeal::Image;
using fgeal::Font;
using fgeal::Sound;
using fgeal::Color;
using fgeal::Menu;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Display;
using fgeal::Rectangle;

using std::string;

// these guys help giving semantics to menu indexes.
enum MenuItem
{
	MENU_ITEM_RESOLUTION,
	MENU_ITEM_FULLSCREEN,
	MENU_ITEM_UNIT,
	MENU_ITEM_SIMULATION_TYPE,
	MENU_ITEM_ENABLE_JUMP,
	MENU_ITEM_HUD_TYPE,
	MENU_ITEM_DIAL_GAUGE_POINTER_TYPE,
	MENU_ITEM_CACHE_TACHOMETER,
	MENU_ITEM_COUNT
};

int OptionsMenuState::getId() { return CarseGame::OPTIONS_MENU_STATE_ID; }

OptionsMenuState::OptionsMenuState(CarseGame* game)
: State(*game), game(*game),
  fontTitle(null), font(null), background(null),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  isResolutionMenuActive(false)
{}

OptionsMenuState::~OptionsMenuState()
{
	if(fontTitle != null) delete fontTitle;
	if(font != null) delete font;
	if(background != null) delete background;
}

void OptionsMenuState::initialize()
{
	Display& display = game.getDisplay();
	const FontSizer fs(display.getHeight());
	background = new Image("assets/options-bg.jpg");
	fontTitle = new Font(game.sharedResources->font2Path, fs(48));
	font = new Font(game.sharedResources->font1Path, fs(16));

	menu.setFont(font);
	menu.titleColor = Color(16, 24, 192);
	menu.bgColor = Color(0, 0, 0, 128);
	menu.entryColor = Color::WHITE;
	menu.focusedEntryBgColor = Color(16, 24, 192);
	menu.focusedEntryFontColor = Color::WHITE;
	menu.borderColor = Color::_TRANSPARENT;

	// DO NOT USE ':' CHARACTER FOR OTHER MEANS OTHER THAN TO SEPARATE MENU ITEM AND ITEM VALUE
	menu.addEntry("Resolution: ");
	menu.addEntry("Fullscreen: ");
	menu.addEntry("Unit: ");
	menu.addEntry("Simulation mode: ");
	menu.addEntry("Enable jumps (experimental): ");
	menu.addEntry("HUD type: ");
	menu.addEntry("Dial gauge pointer type: ");
	menu.addEntry("Cached gauge texture (experimental): ");
	menu.addEntry("Back to main menu");

	menuResolution.setFont(font);
	menuResolution.entryColor = Color::GREY;
	menuResolution.focusedEntryBgColor = Color::GREY;
	menuResolution.titleColor = Color::GREY;
	menuResolution.bgColor = Color(0, 0, 0, 96);
	menuResolution.borderColor = Color::_TRANSPARENT;
	for(unsigned i = 0; i < Display::Mode::getList().size(); i++)
	{
		Display::Mode resolution = Display::Mode::getList()[i];
		menuResolution.addEntry(futil::to_string(resolution.width)+"x"+futil::to_string(resolution.height)
			+ " ("+futil::to_string(resolution.aspectRatio.first)+":"+futil::to_string(resolution.aspectRatio.second)+")"
			+ (resolution.description.empty()? "" : " ("+resolution.description+")"));
	}

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;
}

void OptionsMenuState::onEnter()
{
	isResolutionMenuActive = false;
}

void OptionsMenuState::onLeave()
{}

void OptionsMenuState::update(float delta)
{}

void OptionsMenuState::render()
{
	Display& display = game.getDisplay();
	display.clear();

	// update menu bounds
	menu.bounds.x = 0.0625f*display.getWidth();
	menu.bounds.y = 0.25f*display.getHeight();
	menu.bounds.w = display.getWidth() - 2*menu.bounds.x;
	menu.bounds.h = 0.5f*display.getHeight();

	menuResolution.bounds.x = 0.0625f*display.getWidth();
	menuResolution.bounds.y = 0.25f*display.getHeight();
	menuResolution.bounds.w = display.getWidth() - 2*menuResolution.bounds.x;
	menuResolution.bounds.h = 0.5f*display.getHeight();

	background->drawScaled(0, 0, scaledToSize(background, display));
	updateLabels();

	if(isResolutionMenuActive)
		menuResolution.draw();
	else
		menu.draw();

	fontTitle->drawText("Options", 0.5*(display.getWidth()-fontTitle->getTextWidth("Options")), 0.2*display.getHeight()-fontTitle->getHeight(), Color::WHITE);
}

void OptionsMenuState::onKeyPressed(Keyboard::Key key)
{
	if(isResolutionMenuActive)
		updateOnResolutionMenu(key);
	else switch(key)
	{
		case Keyboard::KEY_ESCAPE:
			sndCursorOut->play();
			game.enterState(game.logic.currentMainMenuStateId);
			break;
		case Keyboard::KEY_ENTER:
			sndCursorIn->play();
			this->onMenuSelect();
			break;
		case Keyboard::KEY_ARROW_UP:
			sndCursorMove->play();
			menu.moveCursorUp();
			break;
		case Keyboard::KEY_ARROW_DOWN:
			sndCursorMove->play();
			menu.moveCursorDown();
			break;
		default:
			break;
	}
}

void OptionsMenuState::onMouseButtonPressed(fgeal::Mouse::Button button, int x, int y)
{
	if(button == fgeal::Mouse::BUTTON_LEFT)
	{
		if(isResolutionMenuActive and menuResolution.bounds.contains(x, y))
		{
			sndCursorIn->play();
			menuResolution.setSelectedIndexByLocation(x, y);
			this->setResolution();
		}
		else if(menu.bounds.contains(x, y))
		{
			sndCursorIn->play();
			this->onMenuSelect();
		}
	}
}

void OptionsMenuState::onMouseMoved(int oldx, int oldy, int x, int y)
{
	if(isResolutionMenuActive)
		menuResolution.setSelectedIndexByLocation(x, y);
	else
		menu.setSelectedIndexByLocation(x, y);
}

void OptionsMenuState::onJoystickAxisMoved(unsigned joystick, unsigned axis, float oldValue, float newValue)
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

void OptionsMenuState::onJoystickButtonPressed(unsigned joystick, unsigned button)
{
	if(button % 2 == 0)
		this->onKeyPressed(Keyboard::KEY_ENTER);
	if(button % 2 == 1)
		this->onKeyPressed(Keyboard::KEY_ESCAPE);
}

void OptionsMenuState::onMenuSelect()
{
	if(menu.getSelectedIndex() == MENU_ITEM_RESOLUTION)
		isResolutionMenuActive = true;

	if(menu.getSelectedIndex() == MENU_ITEM_FULLSCREEN)
		game.getDisplay().setFullscreen(!game.getDisplay().isFullscreen());

	if(menu.getSelectedIndex() == MENU_ITEM_UNIT)
		game.logic.getNextRaceSettings().isImperialUnit = !game.logic.getNextRaceSettings().isImperialUnit;

	if(menu.getSelectedIndex() == MENU_ITEM_SIMULATION_TYPE)
	{
		Mechanics::SimulationType newType;
		switch(game.logic.getSimulationType())
		{
			default:
			case Mechanics::SIMULATION_TYPE_SLIPLESS:       newType = Mechanics::SIMULATION_TYPE_WHEEL_LOAD_CAP; break;
			case Mechanics::SIMULATION_TYPE_WHEEL_LOAD_CAP: newType = Mechanics::SIMULATION_TYPE_PACEJKA_BASED; break;
			case Mechanics::SIMULATION_TYPE_PACEJKA_BASED:  newType = Mechanics::SIMULATION_TYPE_SLIPLESS; break;
		}
		game.logic.setSimulationType(newType);
	}

	if(menu.getSelectedIndex() == MENU_ITEM_ENABLE_JUMP)
		game.logic.setJumpSimulationEnabled(!game.logic.isJumpSimulationEnabled());

	if(menu.getSelectedIndex() == MENU_ITEM_HUD_TYPE)
		game.logic.getNextRaceSettings().hudType = static_cast<Pseudo3DRaceState::HudType>((game.logic.getNextRaceSettings().hudType+1)%Pseudo3DRaceState::HUD_TYPE_COUNT);

	if(menu.getSelectedIndex() == MENU_ITEM_DIAL_GAUGE_POINTER_TYPE)
	{
		if(game.logic.getNextRaceSettings().hudDialGaugePointerImageFilename.empty())
			game.logic.getNextRaceSettings().hudDialGaugePointerImageFilename = "assets/pointer.png";
		else
			game.logic.getNextRaceSettings().hudDialGaugePointerImageFilename.clear();
	}

	if(menu.getSelectedIndex() == MENU_ITEM_CACHE_TACHOMETER)
		game.logic.getNextRaceSettings().useCachedDialGauge = !game.logic.getNextRaceSettings().useCachedDialGauge;

	if(menu.getSelectedIndex() == menu.getEntries().size()-1)
		game.enterState(game.logic.currentMainMenuStateId);
}

#define setMenuItemValueText(item, valueTxt) menu.getEntryAt(item).label.erase(menu.getEntryAt(item).label.find(':')).append(": ").append(valueTxt)

void OptionsMenuState::updateLabels()
{
	Display& display = game.getDisplay();
	setMenuItemValueText(MENU_ITEM_RESOLUTION, futil::to_string(display.getWidth()) + "x" + futil::to_string(display.getHeight()));
	setMenuItemValueText(MENU_ITEM_FULLSCREEN, display.isFullscreen()? "yes" : "no");
	setMenuItemValueText(MENU_ITEM_UNIT, game.logic.getNextRaceSettings().isImperialUnit? "imperial" : "metric");

	string auxStr;
	switch(game.logic.getSimulationType())
	{
		default:
		case Mechanics::SIMULATION_TYPE_SLIPLESS:       auxStr = "slipless"; break;
		case Mechanics::SIMULATION_TYPE_WHEEL_LOAD_CAP: auxStr = "slipless with wheel-load-capped power"; break;
		case Mechanics::SIMULATION_TYPE_PACEJKA_BASED:  auxStr = "longitudinal-only slip (Pacejka)"; break;
	}
	setMenuItemValueText(MENU_ITEM_SIMULATION_TYPE, auxStr);
	setMenuItemValueText(MENU_ITEM_ENABLE_JUMP, game.logic.isJumpSimulationEnabled()? "enabled" : "disabled");
	switch(game.logic.getNextRaceSettings().hudType)
	{
		default:
		case Pseudo3DRaceState::HUD_TYPE_DIALGAUGE_TACHO_NUMERIC_SPEEDO: auxStr = "dial tacho / numeric speedometer"; break;
		case Pseudo3DRaceState::HUD_TYPE_BAR_TACHO_NUMERIC_SPEEDO:       auxStr = "bar tacho / numeric speedometer"; break;
		case Pseudo3DRaceState::HUD_TYPE_DIALGAUGE_TACHO_AND_SPEEDO:     auxStr = "dial tacho / dial speedometer"; break;
	}
	setMenuItemValueText(MENU_ITEM_HUD_TYPE, auxStr);
	setMenuItemValueText(MENU_ITEM_DIAL_GAUGE_POINTER_TYPE, game.logic.getNextRaceSettings().hudDialGaugePointerImageFilename.empty()? "built-in" : "custom");
	setMenuItemValueText(MENU_ITEM_CACHE_TACHOMETER, game.logic.getNextRaceSettings().useCachedDialGauge? "yes" : "no");
}

void OptionsMenuState::updateOnResolutionMenu(Keyboard::Key key)
{
	switch(key)
	{
		case Keyboard::KEY_ESCAPE:
			isResolutionMenuActive = false;
			break;
		case Keyboard::KEY_ENTER:
		{
			sndCursorIn->play();
			this->setResolution();
			break;
		}
		case Keyboard::KEY_ARROW_UP:
			menuResolution.moveCursorUp();
			break;
		case Keyboard::KEY_ARROW_DOWN:
			menuResolution.moveCursorDown();
			break;
		default:
			break;
	}
}

void OptionsMenuState::setResolution()
{
	Display::Mode resolution = Display::Mode::getList()[menuResolution.getSelectedIndex()];
	Display& display = game.getDisplay();
	display.setSize(resolution.width, resolution.height);
	const FontSizer fs(display.getHeight());
	font->setFontSize(fs(16));
	fontTitle->setFontSize(fs(48));
	isResolutionMenuActive = false;
}
