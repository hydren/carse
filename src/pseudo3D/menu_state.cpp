/*
 * menu_state.cpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "menu_state.hpp"

int MenuState::getId() { return CarseGame::MENU_STATE_ID; }

MenuState::MenuState(CarseGame* game)
: State(*game)
{}

MenuState::~MenuState()
{
	// todo
}

void MenuState::initialize()
{
	// todo
}

void MenuState::onEnter()
{
	// todo
}

void MenuState::onLeave()
{
	// todo
}

void MenuState::render()
{
	// todo
}

void MenuState::update(float delta)
{
	// todo
}

void MenuState::handleInput()
{
	// todo
}
