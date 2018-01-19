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

int OptionsMenuState::getId() { return Pseudo3DCarseGame::OPTIONS_MENU_STATE_ID; }

OptionsMenuState::OptionsMenuState(Pseudo3DCarseGame* game)
: State(*game), logic(game->logic), shared(*game->sharedResources), menu(null), menuResolution(null),
  fontTitle(null), font(null), background(null), isResolutionMenuActive(false)
{}

OptionsMenuState::~OptionsMenuState()
{
	if(fontTitle != null) delete fontTitle;
	if(font != null) delete font;
	if(menu != null) delete menu;
	if(menuResolution != null) delete menuResolution;
	if(background != null) delete background;
}

void OptionsMenuState::initialize()
{
	Display& display = game.getDisplay();
	background = new Image("assets/options-bg.jpg");
	fontTitle = new Font(shared.font2Path, dip(48));
	font = new Font(shared.font1Path, dip(16));

	menu = new Menu(Rectangle(), font, Color(16, 24, 192));
	menu->bgColor = Color(0, 0, 0, 128);
	menu->entryColor = Color::WHITE;
	menu->focusedEntryFontColor = Color::WHITE;
	menu->borderColor = Color::_TRANSPARENT;

	menu->addEntry("Resolution: ");
	menu->addEntry("Fullscreen: ");
	menu->addEntry("Unit: ");
	menu->addEntry("Simulation mode: ");
	menu->addEntry("Tachometer type: ");
	menu->addEntry("Back to main menu");

	menuResolution = new Menu(Rectangle(), font, Color::GREY);
	menuResolution->bgColor = Color(0, 0, 0, 96);
	menuResolution->borderColor = Color::_TRANSPARENT;
	for(unsigned i = 0; i < Display::Resolution::getList().size(); i++)
	{
		Display::Resolution resolution = Display::Resolution::getList()[i];
		menuResolution->addEntry(futil::to_string(resolution.width)+"x"+futil::to_string(resolution.height)
			+ " ("+futil::to_string(resolution.aspect.first)+":"+futil::to_string(resolution.aspect.second)+")"
			+ (resolution.description.empty()? "" : " ("+resolution.description+")"));
	}
}

void OptionsMenuState::onEnter()
{
	isResolutionMenuActive = false;
}

void OptionsMenuState::onLeave()
{}

void OptionsMenuState::render()
{
	Display& display = game.getDisplay();
	display.clear();

	// update menu bounds
	menu->bounds.x = 0.0625f*display.getWidth();
	menu->bounds.y = 0.25f*display.getHeight();
	menu->bounds.w = display.getWidth() - 2*menu->bounds.x;
	menu->bounds.h = 0.5f*display.getHeight();

	menuResolution->bounds.x = 0.25f*display.getWidth();
	menuResolution->bounds.y = 0.25f*display.getHeight();
	menuResolution->bounds.w = display.getWidth() - 2*menuResolution->bounds.x;
	menuResolution->bounds.h = 0.5f*display.getHeight();

	background->drawScaled(0, 0, scaledToSize(background, display));
	updateLabels();

	if(isResolutionMenuActive)
		menuResolution->draw();
	else
		menu->draw();

	fontTitle->drawText("Options", 0.5*(display.getWidth()-fontTitle->getTextWidth("Options")), 0.2*display.getHeight()-fontTitle->getHeight(), Color::WHITE);
}

void OptionsMenuState::update(float delta)
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
		else if(isResolutionMenuActive)
			updateOnResolutionMenu(event);
		else if(event.getEventType() == Event::TYPE_KEY_PRESS)
		{
			switch(event.getEventKeyCode())
			{
				case Keyboard::KEY_ESCAPE:
					shared.sndCursorOut.stop();
					shared.sndCursorOut.play();
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::KEY_ENTER:
					shared.sndCursorIn.stop();
					shared.sndCursorIn.play();
					this->onMenuSelect();
					break;
				case Keyboard::KEY_ARROW_UP:
					shared.sndCursorMove.stop();
					shared.sndCursorMove.play();
					menu->moveCursorUp();
					break;
				case Keyboard::KEY_ARROW_DOWN:
					shared.sndCursorMove.stop();
					shared.sndCursorMove.play();
					menu->moveCursorDown();
					break;
				default:
					break;
			}
		}
	}
}

void OptionsMenuState::onMenuSelect()
{
	if(menu->getSelectedIndex() == 0)
		isResolutionMenuActive = true;

	if(menu->getSelectedIndex() == 1)
		game.getDisplay().setFullscreen(!game.getDisplay().isFullscreen());

	if(menu->getSelectedIndex() == 2)
		logic.getNextRaceSettings().isImperialUnit = !logic.getNextRaceSettings().isImperialUnit;

	if(menu->getSelectedIndex() == 3)
	{
		Mechanics::SimulationType newType;
		switch(logic.getSimulationType())
		{
			default:
			case Mechanics::SIMULATION_TYPE_SLIPLESS:	newType = Mechanics::SIMULATION_TYPE_FAKESLIP; break;
			case Mechanics::SIMULATION_TYPE_FAKESLIP:	newType = Mechanics::SIMULATION_TYPE_PACEJKA_BASED; break;
			case Mechanics::SIMULATION_TYPE_PACEJKA_BASED:	newType = Mechanics::SIMULATION_TYPE_SLIPLESS; break;
		}
		logic.setSimulationType(newType);
	}

	if(menu->getSelectedIndex() == 4)
		logic.getNextRaceSettings().useBarTachometer = !logic.getNextRaceSettings().useBarTachometer;

	if(menu->getSelectedIndex() == menu->getEntryCount()-1)
		game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
}

void OptionsMenuState::updateLabels()
{
	Display& display = game.getDisplay();
	menu->at(0).label = string("Resolution: ") + futil::to_string(display.getWidth()) + "x" + futil::to_string(display.getHeight());
	menu->at(1).label = string("Fullscreen: ") + (display.isFullscreen()? "yes" : "no");
	menu->at(2).label = string("Unit: ") + (logic.getNextRaceSettings().isImperialUnit? "imperial" : "metric");

	string strSimType;
	switch(logic.getSimulationType())
	{
		default:
		case Mechanics::SIMULATION_TYPE_SLIPLESS:		strSimType = "Arcade (slipless)"; break;
		case Mechanics::SIMULATION_TYPE_FAKESLIP:		strSimType = "Intermediate (wheel load-capped power)"; break;
		case Mechanics::SIMULATION_TYPE_PACEJKA_BASED:	strSimType = "Advanced (slip ratio simulation)"; break;
	}
	menu->at(3).label = "Simulation mode: " + strSimType;
	menu->at(4).label = "Tachometer type: " + string(logic.getNextRaceSettings().useBarTachometer? "bar" : "gauge");
}

void OptionsMenuState::updateOnResolutionMenu(Event& event)
{
	if(event.getEventType() == Event::TYPE_KEY_PRESS)
	{
		switch(event.getEventKeyCode())
		{
			case Keyboard::KEY_ESCAPE:
				isResolutionMenuActive = false;
				break;
			case Keyboard::KEY_ENTER:
			{
				Display::Resolution resolution = Display::Resolution::getList()[menuResolution->getSelectedIndex()];
				game.getDisplay().setSize(resolution.width, resolution.height);
				updateFonts();
				isResolutionMenuActive = false;
				break;
			}
			case Keyboard::KEY_ARROW_UP:
				menuResolution->moveCursorUp();
				break;
			case Keyboard::KEY_ARROW_DOWN:
				menuResolution->moveCursorDown();
				break;
			default:
				break;
		}
	}
}

void OptionsMenuState::updateFonts()
{
	Display& display = game.getDisplay();

	if(fontTitle != null) delete fontTitle;
	fontTitle = new Font(shared.font2Path, dip(48));
}
