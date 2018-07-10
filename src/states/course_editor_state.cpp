/*
 * course_editor_state.cpp
 *
 *  Created on: 10 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "course_editor_state.hpp"

using fgeal::Display;
using fgeal::Color;
using fgeal::Image;
using fgeal::Keyboard;
using fgeal::Mouse;

int CourseEditorState::getId() { return Pseudo3DCarseGame::COURSE_EDITOR_STATE_ID; }

CourseEditorState::CourseEditorState(Pseudo3DCarseGame* game)
: State(*game), shared(*game->sharedResources), logic(game->logic),
  menuFile(null), focus(ON_EDITOR)
{}

CourseEditorState::~CourseEditorState()
{
	if(menuFile != null) delete menuFile;
}

void CourseEditorState::initialize()
{
	menuFile = new fgeal::Menu(fgeal::Rectangle(), shared.fontDev, Color::GREEN);
}

void CourseEditorState::onEnter()
{
	Display& display = game.getDisplay();
	const unsigned dw = display.getWidth(), dh = display.getHeight();
	menuFile->bounds.x = 0.25*dw;
	menuFile->bounds.y = 0.25*dh;
	menuFile->bounds.w = 0.50*dw;
	menuFile->bounds.h = 0.50*dh;

	focus = ON_EDITOR;
}

void CourseEditorState::onLeave()
{}

void CourseEditorState::render()
{}

void CourseEditorState::update(float delta)
{}

void CourseEditorState::onKeyPressed(Keyboard::Key key)
{}

void CourseEditorState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{}

/*
	// remove all entries and repopulate file list
	while(menuFile->getEntryCount() != 0)
		menuFile->removeEntry(0);

 */
