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
  focus()
{}

CourseEditorState::~CourseEditorState()
{
	if(font != null) delete font;
}

void CourseEditorState::initialize()
{
	Display& display = game.getDisplay();
	fileMenu.setFont(&game.sharedResources->fontDev);
	fileMenu.setColor(Color::GREEN);
	font = new Font(game.sharedResources->font1Path, dip(15));

	saveDialogTextField.font = font;
	saveDialogTextField.bgColor = Color::BLACK;
	saveDialogTextField.bgColor.a = 127;
	saveDialogTextField.textColor = Color::WHITE;
	saveDialogTextField.borderColor = Color::_TRANSPARENT;

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

	mapBounds.x = 0.25*dw;
	mapBounds.y = 0;
	mapBounds.w = 0.75*dw;
	mapBounds.h = 0.95*dh;

	courseViewBounds.x = courseViewBounds.y = 0;
	courseViewBounds.w = 0.25*dw;
	courseViewBounds.h = 0.20*dh;

	toolsPanelBounds.x = 0;
	toolsPanelBounds.y = 0.20*dh;
	toolsPanelBounds.w = 0.25*dw;
	toolsPanelBounds.h = 0.75*dh;

	statusBarBounds.x = 0;
	statusBarBounds.y = 0.95*dh;
	statusBarBounds.w = dw;
	statusBarBounds.h = 0.05*dh;

	newButtonBounds.x = toolsPanelBounds.x + widgetSpacing;
	newButtonBounds.y = toolsPanelBounds.y + widgetSpacing + font->getHeight();
	newButtonBounds.w = 0.08*dh;
	newButtonBounds.h = 0.05*dh;

	loadButtonBounds = newButtonBounds;
	loadButtonBounds.x += newButtonBounds.w + widgetSpacing;

	saveButtonBounds = loadButtonBounds;
	saveButtonBounds.x += loadButtonBounds.w + widgetSpacing;

	generateButtonBounds = newButtonBounds;
	generateButtonBounds.w *= 2;
	generateButtonBounds.y += newButtonBounds.h + widgetSpacing;

	loadDialogBounds.x = 0.15*dw;
	loadDialogBounds.y = 0.20*dh;
	loadDialogBounds.w = 0.70*dw;
	loadDialogBounds.h = 0.55*dh;

	fileMenu.bounds.x = loadDialogBounds.x + widgetSpacing;
	fileMenu.bounds.y = loadDialogBounds.y + widgetSpacing;
	fileMenu.bounds.w = loadDialogBounds.w - widgetSpacing*2;
	fileMenu.bounds.h = loadDialogBounds.h - widgetSpacing*2 - font->getHeight();

	loadDialogButtonSelectBounds.w = 1.1*font->getTextWidth("Select");
	loadDialogButtonSelectBounds.h = 1.1*font->getHeight();
	loadDialogButtonSelectBounds.x = 0.5*(loadDialogBounds.x + loadDialogBounds.w - loadDialogButtonSelectBounds.w);
	loadDialogButtonSelectBounds.y = loadDialogBounds.y + loadDialogBounds.h - 1.2*loadDialogButtonSelectBounds.h;

	loadDialogButtonCancelBounds = loadDialogButtonSelectBounds;
	loadDialogButtonSelectBounds.w = 1.1*font->getTextWidth("Cancel");
	loadDialogButtonCancelBounds.x += loadDialogButtonSelectBounds.w + widgetSpacing;

	// save dialog

	saveDialogBounds.w = 0.6*dw;
	saveDialogBounds.h = 0.2*dh;
	saveDialogBounds.x = 0.5*(dw - saveDialogBounds.w);
	saveDialogBounds.y = 0.5*(dh - saveDialogBounds.h);

	saveDialogTextField.bounds.x = saveDialogBounds.x + widgetSpacing;
	saveDialogTextField.bounds.y = saveDialogBounds.y + widgetSpacing + font->getHeight();
	saveDialogTextField.bounds.w = saveDialogBounds.w - 2*widgetSpacing;
	saveDialogTextField.bounds.h = 1.1*font->getHeight();
	saveDialogTextField.content.clear();
	saveDialogTextField.caretPosition = 0;

	saveDialogSaveButtonBounds = loadDialogButtonSelectBounds;
	saveDialogSaveButtonBounds.y = saveDialogBounds.y + saveDialogBounds.h - saveDialogSaveButtonBounds.h - widgetSpacing;

	saveDialogCancelButtonBounds = loadDialogButtonCancelBounds;
	saveDialogCancelButtonBounds.y = saveDialogSaveButtonBounds.y;

	// initial values

	focus = ON_EDITOR;

	course.miniMapRoadColor = Color::RED;
	course.miniMapRoadContrastColorEnabled = true;
	course.miniMapSegmentHighlightColor = Color::YELLOW;
	course.miniMapOffset.x = 0.5*mapBounds.w;
	course.miniMapOffset.y = 0.5*mapBounds.h;
	course.miniMapScale.x = course.miniMapScale.y = 1.f;
	course.miniMapBounds = mapBounds;

	this->loadCourse(Pseudo3DCourse(Pseudo3DCourse::Spec(200, 3000)));
}

void CourseEditorState::onLeave()
{}

void CourseEditorState::render()
{
	const float widgetSpacing = 0.007*game.getDisplay().getHeight();

	Graphics::drawFilledRectangle(courseViewBounds, Color::CYAN);
	course.draw(0, 0.5*course.drawAreaWidth);
	Graphics::drawRectangle(courseViewBounds, Color::AZURE);


	Graphics::drawFilledRectangle(mapBounds, Color::DARK_GREEN);


	Graphics::drawFilledRectangle(toolsPanelBounds, Color::DARK_GREY);
	font->drawText("Course editor", toolsPanelBounds.x, toolsPanelBounds.y, Color::WHITE);

	Graphics::drawFilledRectangle(newButtonBounds, Color::GREY);
	font->drawText("New", newButtonBounds.x, newButtonBounds.y, Color::BLACK);

	Graphics::drawFilledRectangle(loadButtonBounds, Color::GREY);
	font->drawText("Load", loadButtonBounds.x, loadButtonBounds.y, Color::BLACK);

	Graphics::drawFilledRectangle(saveButtonBounds, Color::GREY);
	font->drawText("Save", saveButtonBounds.x, saveButtonBounds.y, Color::BLACK);

	Graphics::drawFilledRectangle(generateButtonBounds, Color::GREY);
	font->drawText("Generate", generateButtonBounds.x, generateButtonBounds.y, Color::BLACK);

	if(focus == ON_EDITOR and cos(20*fgeal::uptime()) > 0)
	{
		if(newButtonBounds.contains(Mouse::getPosition()))
			Graphics::drawRectangle(getSpacedOutline(newButtonBounds, widgetSpacing), Color::RED);
		else if(loadButtonBounds.contains(Mouse::getPosition()))
			Graphics::drawRectangle(getSpacedOutline(loadButtonBounds, widgetSpacing), Color::RED);
		else if(saveButtonBounds.contains(Mouse::getPosition()))
			Graphics::drawRectangle(getSpacedOutline(saveButtonBounds, widgetSpacing), Color::RED);
		else if(generateButtonBounds.contains(Mouse::getPosition()))
			Graphics::drawRectangle(getSpacedOutline(generateButtonBounds, widgetSpacing), Color::RED);
	}


	Graphics::drawFilledRectangle(statusBarBounds, Color::GREY);


	course.drawMap();

	if(focus == ON_FILE_MENU)
	{
		Graphics::drawFilledRoundedRectangle(loadDialogBounds, 10, Color::GREY);
		Graphics::drawRoundedRectangle(loadDialogBounds, 10, Color::DARK_GREY);

		fileMenu.draw();

		Graphics::drawFilledRectangle(loadDialogButtonSelectBounds, Color::LIGHT_GREY);
		font->drawText("Select", loadDialogButtonSelectBounds.x, loadDialogButtonSelectBounds.y, Color::BLACK);

		Graphics::drawFilledRectangle(loadDialogButtonCancelBounds, Color::LIGHT_GREY);
		font->drawText("Cancel", loadDialogButtonCancelBounds.x, loadDialogButtonCancelBounds.y, Color::BLACK);

		if(cos(20*fgeal::uptime()) > 0)
		{
			if(loadDialogButtonSelectBounds.contains(Mouse::getPosition()))
				Graphics::drawRectangle(getSpacedOutline(loadDialogButtonSelectBounds, widgetSpacing), Color::RED);
			else if(loadDialogButtonCancelBounds.contains(Mouse::getPosition()))
				Graphics::drawRectangle(getSpacedOutline(loadDialogButtonCancelBounds, widgetSpacing), Color::RED);
		}
	}

	if(focus == ON_SAVE_DIALOG)
	{
		Graphics::drawFilledRoundedRectangle(saveDialogBounds, 10, Color::GREY);
		Graphics::drawRoundedRectangle(saveDialogBounds, 10, Color::DARK_GREY);

		font->drawText("Enter a filename for the course:", saveDialogBounds.x + widgetSpacing, saveDialogBounds.y + widgetSpacing, Color::BLACK);

		saveDialogTextField.draw();

		Graphics::drawFilledRectangle(saveDialogSaveButtonBounds, Color::LIGHT_GREY);
		font->drawText("Save", saveDialogSaveButtonBounds.x, saveDialogSaveButtonBounds.y, Color::BLACK);

		Graphics::drawFilledRectangle(saveDialogCancelButtonBounds, Color::LIGHT_GREY);
		font->drawText("Cancel", saveDialogCancelButtonBounds.x, saveDialogCancelButtonBounds.y, Color::BLACK);

		if(cos(20*fgeal::uptime()) > 0)
		{
			if(saveDialogSaveButtonBounds.contains(Mouse::getPosition()))
				Graphics::drawRectangle(getSpacedOutline(saveDialogSaveButtonBounds, widgetSpacing), Color::RED);
			else if(saveDialogCancelButtonBounds.contains(Mouse::getPosition()))
				Graphics::drawRectangle(getSpacedOutline(saveDialogCancelButtonBounds, widgetSpacing), Color::RED);
		}
	}
}

void CourseEditorState::update(float delta)
{
	if(focus == ON_EDITOR)
	{
		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP))
			course.miniMapOffset.y -= 100*delta/course.miniMapScale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN))
			course.miniMapOffset.y += 100*delta/course.miniMapScale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_LEFT))
			course.miniMapOffset.x -= 100*delta/course.miniMapScale.x;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_RIGHT))
			course.miniMapOffset.x += 100*delta/course.miniMapScale.x;

		const fgeal::Vector2D oldScale = course.miniMapScale;

		if(Keyboard::isKeyPressed(Keyboard::KEY_Q))
			course.miniMapScale.y *= 1+delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_A))
			course.miniMapScale.y *= 1-delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_X))
			course.miniMapScale.x *= 1+delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_Z))
			course.miniMapScale.x *= 1-delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_E))
			course.miniMapScale *= 1+delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_S))
			course.miniMapScale *= 1-delta;

		if(course.miniMapScale.x != oldScale.x)
			course.miniMapOffset.x *= oldScale.x/course.miniMapScale.x;

		if(course.miniMapScale.y != oldScale.y)
			course.miniMapOffset.y *= oldScale.y/course.miniMapScale.y;
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
			fileMenu.moveCursorUp();
		}

		if(key == Keyboard::KEY_ARROW_DOWN)
		{
			sndCursorMove->play();
			fileMenu.moveCursorDown();
		}

		if(key == Keyboard::KEY_ENTER)
		{
			sndCursorIn->play();
			this->loadCourse(Pseudo3DCourse::Spec::createFromFile(fileMenu.getSelectedEntry().label));
		}
	}
	else if(focus == ON_SAVE_DIALOG)
	{
		if(key== Keyboard::KEY_ESCAPE)
			focus = ON_EDITOR;
		else
			saveDialogTextField.onKeyPressed(key);
	}
}

void CourseEditorState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{
	if(focus == ON_EDITOR)
	{
		if(newButtonBounds.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourse(Pseudo3DCourse(Pseudo3DCourse::Spec(200, 3000)));
		}

		if(loadButtonBounds.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_FILE_MENU;
		}

		if(saveButtonBounds.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_SAVE_DIALOG;
		}

		if(generateButtonBounds.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourse(Pseudo3DCourse::Spec::generateRandomCourseSpec(200, 3000, 6400, 1.5));
		}
	}
	else if(focus == ON_FILE_MENU)
	{
		if(fileMenu.bounds.contains(x, y))
		{
			sndCursorMove->play();
			fileMenu.setSelectedIndexByLocation(x, y);
		}

		if(loadDialogButtonSelectBounds.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourse(Pseudo3DCourse::Spec::createFromFile(fileMenu.getSelectedEntry().label));
		}

		if(loadDialogButtonCancelBounds.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_EDITOR;
		}
	}
	else if(focus == ON_SAVE_DIALOG)
	{
		if(saveDialogSaveButtonBounds.contains(x, y))
		{
			sndCursorIn->play();
			if(course.spec.name.empty())
				course.spec.name = saveDialogTextField.content;
			try { course.spec.saveToFile(CarseGame::Logic::COURSES_FOLDER+"/"+saveDialogTextField.content); }
			catch(const std::exception& e) { /* TODO show error dialog */ }
			focus = ON_EDITOR;
		}

		if(saveDialogCancelButtonBounds.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_EDITOR;
		}
	}
}

void CourseEditorState::reloadFileList()
{
	// clear menu
	while(not fileMenu.getEntries().empty())
		fileMenu.removeEntry(0);

	// populate menu
	vector<string> courseFiles = fgeal::filesystem::getFilenamesWithinDirectory(CarseGame::Logic::COURSES_FOLDER);
	for(unsigned i = 0; i < courseFiles.size(); i++)
		if(ends_with(courseFiles[i], ".properties"))
			fileMenu.addEntry(courseFiles[i]);
}

void CourseEditorState::loadCourse(const Pseudo3DCourse& c)
{
	course.clearDynamicData();
	course = c;
	course.setupDynamicData();
	course.drawAreaWidth = courseViewBounds.w;
	course.drawAreaHeight = courseViewBounds.h;
	course.drawDistance = 300;
	course.cameraDepth = 0.84;

	course.miniMapRoadColor = Color::RED;
	course.miniMapRoadContrastColorEnabled = true;
	course.miniMapSegmentHighlightColor = Color::YELLOW;
	course.miniMapOffset.x = 0.5*mapBounds.w;
	course.miniMapOffset.y = 0.5*mapBounds.h;
	course.miniMapScale.x = course.miniMapScale.y = 1.f;
	course.miniMapBounds = mapBounds;

	focus = ON_EDITOR;
}
