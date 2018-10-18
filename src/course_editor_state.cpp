/*
 * course_editor_state.cpp
 *
 *  Created on: 10 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "course_editor_state.hpp"

#include "carse_game.hpp"

#include "util.hpp"

using fgeal::Display;
using fgeal::Color;
using fgeal::Graphics;
using fgeal::Image;
using fgeal::Font;
using fgeal::Keyboard;
using fgeal::Mouse;
using fgeal::Rectangle;
using fgeal::Point;
using std::vector;
using std::string;
using futil::ends_with;
using futil::Properties;

int CourseEditorState::getId() { return CarseGame::COURSE_EDITOR_STATE_ID; }

CourseEditorState::CourseEditorState(CarseGame* game)
: State(*game), game(*game),
  font(null), sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  focus(ON_EDITOR)
{}

CourseEditorState::~CourseEditorState()
{
	if(font != null) delete font;
}

void CourseEditorState::initialize()
{
	Display& display = game.getDisplay();
	menuFile.setFont(&game.sharedResources->fontDev);
	menuFile.setColor(Color::GREEN);
	font = new Font(game.sharedResources->font1Path, dip(15));

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;
}

void CourseEditorState::onEnter()
{
	Display& display = game.getDisplay();
	const unsigned dw = display.getWidth(), dh = display.getHeight();
	const float widgetSpacing = 0.02*dh;

	reloadFileList();

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

	boundsButtonNew.x = boundsToolsPanel.x + widgetSpacing;
	boundsButtonNew.y = boundsToolsPanel.y + widgetSpacing + font->getHeight();
	boundsButtonNew.w = 0.08*dh;
	boundsButtonNew.h = 0.05*dh;

	boundsButtonLoad = boundsButtonNew;
	boundsButtonLoad.x += boundsButtonNew.w + widgetSpacing;

	boundsButtonSave = boundsButtonLoad;
	boundsButtonSave.x += boundsButtonLoad.w + widgetSpacing;

	boundsButtonGenerate = boundsButtonNew;
	boundsButtonGenerate.w *= 2;
	boundsButtonGenerate.y += boundsButtonNew.h + widgetSpacing;

	boundsFileDialog.x = 0.15*dw;
	boundsFileDialog.y = 0.20*dh;
	boundsFileDialog.w = 0.70*dw;
	boundsFileDialog.h = 0.55*dh;

	menuFile.bounds.x = boundsFileDialog.x + widgetSpacing;
	menuFile.bounds.y = boundsFileDialog.y + widgetSpacing;
	menuFile.bounds.w = boundsFileDialog.w - widgetSpacing*2;
	menuFile.bounds.h = boundsFileDialog.h - widgetSpacing*2 - font->getHeight();

	boundsFileDialogButtonSelect.w = 1.1*font->getTextWidth("Select");
	boundsFileDialogButtonSelect.h = 1.1*font->getHeight();
	boundsFileDialogButtonSelect.x = 0.5*(boundsFileDialog.x + boundsFileDialog.w - boundsFileDialogButtonSelect.w);
	boundsFileDialogButtonSelect.y = boundsFileDialog.y + boundsFileDialog.h - 1.2*boundsFileDialogButtonSelect.h;

	boundsFileDialogButtonCancel = boundsFileDialogButtonSelect;
	boundsFileDialogButtonSelect.w = 1.1*font->getTextWidth("Cancel");
	boundsFileDialogButtonCancel.x += boundsFileDialogButtonSelect.w + widgetSpacing;

	focus = ON_EDITOR;

	offset.x = boundsMap.x;
	offset.y = boundsMap.y;
	scale.x = scale.y = 1.f;

	this->loadCourse(Pseudo3DCourse(Pseudo3DCourse::Spec(200, 3000)));
}

void CourseEditorState::onLeave()
{}

void CourseEditorState::render()
{
	const float widgetSpacing = 0.007*game.getDisplay().getHeight();

	Graphics::drawFilledRectangle(boundsCourseView, Color::CYAN);
	course.draw(0, 0.5*course.drawAreaWidth);
	Graphics::drawRectangle(boundsCourseView, Color::AZURE);


	Graphics::drawFilledRectangle(boundsMap, Color::DARK_GREEN);


	Graphics::drawFilledRectangle(boundsToolsPanel, Color::DARK_GREY);
	font->drawText("Course editor", boundsToolsPanel.x, boundsToolsPanel.y, Color::WHITE);

	Graphics::drawFilledRectangle(boundsButtonNew, Color::GREY);
	font->drawText("New", boundsButtonNew.x, boundsButtonNew.y, Color::BLACK);

	Graphics::drawFilledRectangle(boundsButtonLoad, Color::GREY);
	font->drawText("Load", boundsButtonLoad.x, boundsButtonLoad.y, Color::BLACK);

	Graphics::drawFilledRectangle(boundsButtonSave, Color::GREY);
	font->drawText("Save", boundsButtonSave.x, boundsButtonSave.y, Color::DARK_GREY);

	Graphics::drawFilledRectangle(boundsButtonGenerate, Color::GREY);
	font->drawText("Generate", boundsButtonGenerate.x, boundsButtonGenerate.y, Color::BLACK);

	if(focus == ON_EDITOR and cos(20*fgeal::uptime()) > 0)
	{
		if(boundsButtonNew.contains(Mouse::getPosition()))
			Graphics::drawRectangle(getSpacedOutline(boundsButtonNew, widgetSpacing), Color::RED);
		else if(boundsButtonLoad.contains(Mouse::getPosition()))
			Graphics::drawRectangle(getSpacedOutline(boundsButtonLoad, widgetSpacing), Color::RED);
//		else if(boundsButtonSave.contains(Mouse::getPosition()))
//			Graphics::drawRectangle(getSpacedOutline(boundsButtonSave, widgetSpacing), Color::RED);
		else if(boundsButtonGenerate.contains(Mouse::getPosition()))
			Graphics::drawRectangle(getSpacedOutline(boundsButtonGenerate, widgetSpacing), Color::RED);
	}


	Graphics::drawFilledRectangle(boundsStatusBar, Color::GREY);


	course.drawMap(Color::RED, offset, scale, boundsMap);

	if(focus == ON_FILE_MENU)
	{
		Graphics::drawFilledRoundedRectangle(boundsFileDialog, 10, Color::GREY);
		Graphics::drawRoundedRectangle(boundsFileDialog, 10, Color::DARK_GREY);

		menuFile.draw();

		Graphics::drawFilledRectangle(boundsFileDialogButtonSelect, Color::LIGHT_GREY);
		font->drawText("Select", boundsFileDialogButtonSelect.x, boundsFileDialogButtonSelect.y, Color::BLACK);

		Graphics::drawFilledRectangle(boundsFileDialogButtonCancel, Color::LIGHT_GREY);
		font->drawText("Cancel", boundsFileDialogButtonCancel.x, boundsFileDialogButtonCancel.y, Color::BLACK);

		if(cos(20*fgeal::uptime()) > 0)
		{
			if(boundsFileDialogButtonSelect.contains(Mouse::getPosition()))
				Graphics::drawRectangle(getSpacedOutline(boundsFileDialogButtonSelect, widgetSpacing), Color::RED);
			else if(boundsFileDialogButtonCancel.contains(Mouse::getPosition()))
				Graphics::drawRectangle(getSpacedOutline(boundsFileDialogButtonCancel, widgetSpacing), Color::RED);
		}
	}
}

void CourseEditorState::update(float delta)
{
	if(focus == ON_EDITOR)
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
			offset.y -= 100*delta/scale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN))
			offset.y += 100*delta/scale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_LEFT))
			offset.x -= 100*delta/scale.x;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_RIGHT))
			offset.x += 100*delta/scale.x;
	}
}

void CourseEditorState::onKeyPressed(Keyboard::Key key)
{
	if(focus == ON_EDITOR)
	{
		if(key == Keyboard::KEY_ESCAPE)
			game.enterState(CarseGame::COURSE_SELECTION_STATE_ID);
	}
	else if(focus == ON_FILE_MENU)
	{
		if(key == Keyboard::KEY_ESCAPE)
		{
			sndCursorOut->play();
			focus = ON_EDITOR;
		}

		if(key == Keyboard::KEY_ARROW_UP)
		{
			sndCursorMove->play();
			menuFile.moveCursorUp();
		}

		if(key == Keyboard::KEY_ARROW_DOWN)
		{
			sndCursorMove->play();
			menuFile.moveCursorDown();
		}

		if(key == Keyboard::KEY_ENTER)
		{
			sndCursorIn->play();
			this->loadCourse(Pseudo3DCourse::parseCourseSpecFromFile(menuFile.getSelectedEntry().label));
		}
	}
}

void CourseEditorState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{
	if(focus == ON_EDITOR)
	{
		if(boundsButtonNew.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourse(Pseudo3DCourse(Pseudo3DCourse::Spec(200, 3000)));
		}

		if(boundsButtonLoad.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_FILE_MENU;
		}

		if(boundsButtonGenerate.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourse(Pseudo3DCourse::generateRandomCourseSpec(200, 3000, 6400, 1.5));
		}
	}
	else if(focus == ON_FILE_MENU)
	{
		if(menuFile.bounds.contains(x, y))
		{
			sndCursorMove->play();
			menuFile.setSelectedIndexByLocation(x, y);
		}

		if(boundsFileDialogButtonSelect.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourse(Pseudo3DCourse::parseCourseSpecFromFile(menuFile.getSelectedEntry().label));
		}

		if(boundsFileDialogButtonCancel.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_EDITOR;
		}
	}
}

void CourseEditorState::reloadFileList()
{
	// clear menu
	while(not menuFile.getEntries().empty())
		menuFile.removeEntry(0);

	// populate menu
	vector<string> courseFiles = fgeal::filesystem::getFilenamesWithinDirectory(CarseGame::Logic::COURSES_FOLDER);
	for(unsigned i = 0; i < courseFiles.size(); i++)
		if(ends_with(courseFiles[i], ".properties"))
			menuFile.addEntry(courseFiles[i]);
}

void CourseEditorState::loadCourse(const Pseudo3DCourse& c)
{
	course.clearDynamicData();
	course = c;
	course.setupDynamicData();
	course.drawAreaWidth = boundsCourseView.w;
	course.drawAreaHeight = boundsCourseView.h;
	course.drawDistance = 300;
	course.cameraDepth = 0.84;
	focus = ON_EDITOR;
}
