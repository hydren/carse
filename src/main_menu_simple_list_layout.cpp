/*
 * main_menu_simple_list_layout.cpp
 *
 *  Created on: 23 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "main_menu_state.hpp"

#include "carse_game.hpp"

using std::string;
using fgeal::Display;
using fgeal::Color;

typedef GenericMenuStateLayout<MainMenuState> Layout;

MainMenuState::SimpleListLayout::SimpleListLayout(MainMenuState& state)
: Layout(state),
  fontMain(state.shared.font1Path, 32 * (state.game.getDisplay().getHeight()/480.0))
{}

void MainMenuState::SimpleListLayout::draw()
{
	Display& display = state.game.getDisplay();

	state.menu->bounds.x = 0.25*display.getWidth();
	state.menu->bounds.y = 0.25*display.getHeight();
	state.menu->bounds.w = 0.5*display.getWidth();
	state.menu->bounds.h = 0.5*display.getHeight();

	state.menu->draw();
	const string title("Carse Project");
	fontMain.drawText(title, 0.5*(display.getWidth() - fontMain.getTextWidth(title)),
							  0.05*(display.getHeight() - fontMain.getHeight()), Color::WHITE);
}

void MainMenuState::SimpleListLayout::update(float delta)
{}

void MainMenuState::SimpleListLayout::onCursorUp()
{
	state.menu->moveCursorUp();
	state.shared.sndCursorMove.stop();
	state.shared.sndCursorMove.play();
}

void MainMenuState::SimpleListLayout::onCursorDown()
{
	state.menu->moveCursorDown();
	state.shared.sndCursorMove.stop();
	state.shared.sndCursorMove.play();
}

void MainMenuState::SimpleListLayout::onCursorAccept()
{
	state.shared.sndCursorIn.stop();
	state.shared.sndCursorIn.play();
	Layout::onCursorAccept();
}
