/*
 * course_editor_state.cpp
 *
 *  Created on: 10 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "course_editor_state.hpp"

#include "carse_game.hpp"

using fgeal::Display;
using fgeal::Color;
using fgeal::Graphics;
using fgeal::Image;
using fgeal::Keyboard;
using fgeal::Mouse;
using fgeal::Rectangle;
using fgeal::Point;

int CourseEditorState::getId() { return CarseGame::COURSE_EDITOR_STATE_ID; }

CourseEditorState::CourseEditorState(CarseGame* game)
: State(*game), game(*game),
  menuFile(null), focus(ON_EDITOR)
{}

CourseEditorState::~CourseEditorState()
{
	if(menuFile != null) delete menuFile;
}

void CourseEditorState::initialize()
{
	menuFile = new fgeal::Menu(fgeal::Rectangle(), &game.sharedResources->fontDev, Color::GREEN);
}

void CourseEditorState::onEnter()
{
	Display& display = game.getDisplay();
	const unsigned dw = display.getWidth(), dh = display.getHeight();
	menuFile->bounds.x = 0.25*dw;
	menuFile->bounds.y = 0.25*dh;
	menuFile->bounds.w = 0.50*dw;
	menuFile->bounds.h = 0.50*dh;

	boundsMap.x = 0.25*dw;
	boundsMap.y = 0;
	boundsMap.w = 0.75*dw;
	boundsMap.h = 0.95*dh;

	boundsCourseView.x = boundsCourseView.y = 0;
	boundsCourseView.w = 0.25*dw;
	boundsCourseView.h = 0.20*dh;

	boundsToolsPanel.x = 0;
	boundsToolsPanel.y = 0.20*dh;
	boundsToolsPanel.w = 0.25*dw;
	boundsToolsPanel.h = 0.75*dh;

	boundsStatusBar.x = 0;
	boundsStatusBar.y = 0.95*dh;
	boundsStatusBar.w = dw;
	boundsStatusBar.h = 0.05*dh;

	focus = ON_EDITOR;

	offset.x = boundsMap.x;
	offset.y = boundsMap.y;
	scale.x = scale.y = 1.f;

	//xxx debug
	course.clearDynamicData();
	course = Pseudo3DCourse(Pseudo3DCourse::generateRandomCourseSpec(200, 3000, 6400, 1.5));
	course.setupDynamicData();
	course.drawAreaWidth = boundsCourseView.w;
	course.drawAreaHeight = boundsCourseView.h;
	course.drawDistance = 300;
	course.cameraDepth = 0.84;
}

void CourseEditorState::onLeave()
{}

void CourseEditorState::render()
{
	Graphics::drawFilledRectangle(boundsCourseView, Color::CYAN);
	course.draw(0, 0.5*course.drawAreaWidth);
	Graphics::drawRectangle(boundsCourseView, Color::AZURE);

	Graphics::drawFilledRectangle(boundsMap, Color::DARK_GREEN);
	Graphics::drawFilledRectangle(boundsToolsPanel, Color::DARK_GREY);
	Graphics::drawFilledRectangle(boundsStatusBar, Color::GREY);


	course.drawMap(Color::RED, offset, scale, boundsMap);

	if(focus == ON_FILE_MENU)
		menuFile->draw();
}

void CourseEditorState::update(float delta)
{
	if(Keyboard::isKeyPressed(Keyboard::KEY_Q))
		scale.y *= 1+delta;

	if(Keyboard::isKeyPressed(Keyboard::KEY_A))
		scale.y *= 1-delta;

	if(Keyboard::isKeyPressed(Keyboard::KEY_X))
		scale.x *= 1+delta;

	if(Keyboard::isKeyPressed(Keyboard::KEY_Z))
		scale.x *= 1-delta;

	if(Keyboard::isKeyPressed(Keyboard::KEY_E))
		scale *= 1+delta;

	if(Keyboard::isKeyPressed(Keyboard::KEY_S))
		scale *= 1-delta;

	if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP))
		offset.y -= 100*delta;

	if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN))
		offset.y += 100*delta;

	if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_LEFT))
		offset.x -= 100*delta;

	if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_RIGHT))
		offset.x += 100*delta;
}

void CourseEditorState::onKeyPressed(Keyboard::Key key)
{
	if(key == Keyboard::KEY_ESCAPE)
		game.enterState(CarseGame::COURSE_SELECTION_STATE_ID);
}

void CourseEditorState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{}

/*
	// remove all entries and repopulate file list
	while(menuFile->getEntryCount() != 0)
		menuFile->removeEntry(0);

 */
