/*
 * options_menu_state.cpp
 *
 *  Created on: 5 de set de 2017
 *      Author: carlosfaruolo
 */

#include "options_menu_state.hpp"
#include "futil/string_actions.hpp"

using fgeal::Font;
using fgeal::Sound;
using fgeal::Color;
using fgeal::Menu;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Display;
using fgeal::Rectangle;

int OptionsMenuState::getId() { return Pseudo3DCarseGame::OPTIONS_MENU_STATE_ID; }

OptionsMenuState::OptionsMenuState(Pseudo3DCarseGame* game)
: State(*game), shared(*game->sharedResources), menu(null),
  fontTitle(null), font(null)
{}

OptionsMenuState::~OptionsMenuState()
{
	if(fontTitle != null) delete fontTitle;
	if(font != null) delete font;
	if(menu != null) delete menu;
}

void OptionsMenuState::initialize()
{
	fontTitle = new Font("assets/font2.ttf", 32);
	font = new Font("assets/font.ttf", 12);

	menu = new Menu(Rectangle(), font, Color::NAVY);
	menu->bgColor = Color::BLUE;
	menu->focusedEntryFontColor = Color::WHITE;

	menu->addEntry("Resolution: ");
	menu->addEntry("Back to main menu");
}

void OptionsMenuState::onEnter()
{}

void OptionsMenuState::onLeave()
{}

void OptionsMenuState::render()
{
	Display& display = game.getDisplay();
	display.clear();

	// update menu bounds
	menu->bounds.x = 0.0625f*display.getWidth();
	menu->bounds.y = 0.25f*display.getHeight();
	menu->bounds.w = 0.4f*display.getWidth();
	menu->bounds.h = 0.5f*display.getHeight();

	menu->draw();

	fontTitle->drawText("Options", 0.125*display.getWidth(), 0.125*display.getHeight(), Color::WHITE);

	font->drawText(futil::to_string(display.getWidth()) + "x" + futil::to_string(display.getHeight()), menu->bounds.x + 128, menu->bounds.y, Color::WHITE);
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
					menu->cursorUp();
					break;
				case Keyboard::KEY_ARROW_DOWN:
					shared.sndCursorMove.stop();
					shared.sndCursorMove.play();
					menu->cursorDown();
					break;
				default:
					break;
			}
		}
	}
}

void OptionsMenuState::onMenuSelect()
{
	if(menu->getSelectedIndex() == menu->getEntryCount()-1)
		game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
}
