/*
 * course_editor_state.cpp
 *
 *  Created on: 10 de jul de 2018
 *      Author: carlosfaruolo
 */

#include "course_editor_state.hpp"

#include "carse_game.hpp"

#include "util.hpp"

#include "futil/string_actions.hpp"

using fgeal::Display;
using fgeal::Color;
using fgeal::Graphics;
using fgeal::Image;
using fgeal::Font;
using fgeal::Keyboard;
using fgeal::Mouse;
using fgeal::Rectangle;
using fgeal::Point;
using fgeal::Vector2D;
using std::vector;
using std::string;
using futil::ends_with;
using futil::Properties;

int CourseEditorState::getId() { return CarseGame::COURSE_EDITOR_STATE_ID; }

CourseEditorState::CourseEditorState(CarseGame* game)
: State(*game), game(*game), focus(),
  font(null), sndCursorMove(null), sndCursorIn(null), sndCursorOut(null)
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

	// reset geometry

	mapBounds.x = 0.25*dw;
	mapBounds.y = 0;
	mapBounds.w = 0.75*dw;
	mapBounds.h = 0.95*dh;

	courseEditorTitlePosition.x = 0.5*(dw - font->getTextWidth("Course Editor"));
	courseEditorTitlePosition.y = 0;

	courseViewBounds.x = courseViewBounds.y = 0;
	courseViewBounds.w = 0.25*dw;
	courseViewBounds.h = 0.20*dh;

	toolsPanelBounds.x = 0;
	toolsPanelBounds.y = 0.20*dh;
	toolsPanelBounds.w = 0.25*dw;
	toolsPanelBounds.h = 0.75*dh;

	presetsPanelBounds = toolsPanelBounds;
	presetsPanelBounds.x += widgetSpacing;
	presetsPanelBounds.y += widgetSpacing;
	presetsPanelBounds.w -= 2*widgetSpacing;
	presetsPanelBounds.h -= 4*widgetSpacing + 0.1*dh;

	newButtonBounds.x = presetsPanelBounds.x;
	newButtonBounds.y = presetsPanelBounds.y + presetsPanelBounds.h + widgetSpacing;
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

	statusBarBounds.x = 0;
	statusBarBounds.y = 0.95*dh;
	statusBarBounds.w = dw;
	statusBarBounds.h = 0.05*dh;

	scaleIndicatorPosition.x = dw - game.sharedResources->fontDev.getTextWidth("zoom: x=00000000000, y=00000000000");
	scaleIndicatorPosition.y = statusBarBounds.y;
	scaleIndicatorText.clear();

	// initial values

	focus = ON_EDITOR;

	course.minimap.roadColor = Color::RED;
	course.minimap.roadContrastColorEnabled = true;
	course.minimap.segmentHighlightColor = Color::YELLOW;
	course.minimap.offset.x = 0.5*mapBounds.w;
	course.minimap.offset.y = 0.5*mapBounds.h;
	course.minimap.scale.x = course.minimap.scale.y = 1.f;
	course.minimap.bounds = mapBounds;

	this->loadCourse(Pseudo3DCourse(Pseudo3DCourse::Spec(200, 3000)));
}

void CourseEditorState::onLeave()
{}

void CourseEditorState::render()
{
	const float widgetSpacing = 0.007*game.getDisplay().getHeight();

	// Course preview
	Graphics::drawFilledRectangle(courseViewBounds, Color::CYAN);
	course.draw(0, 0.5*course.drawAreaWidth);
	Graphics::drawRectangle(courseViewBounds, Color::AZURE);
	game.sharedResources->fontDev.drawText("Preview", courseViewBounds.x, courseViewBounds.y, Color::RED);

	// Map
	Graphics::drawFilledRectangle(mapBounds, Color::DARK_GREEN);
	course.minimap.drawMap();
	font->drawText("Course editor", courseEditorTitlePosition, Color::WHITE);

	// Tools panel
	Graphics::drawFilledRectangle(toolsPanelBounds, Color::DARK_GREY);
	Graphics::drawFilledRectangle(presetsPanelBounds, Color::BLACK);

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

	// load file dialog
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

	// save file dialog
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

	// status bar
	Graphics::drawFilledRectangle(statusBarBounds, Color::GREY);
	game.sharedResources->fontDev.drawText(scaleIndicatorText, scaleIndicatorPosition, Color::WHITE);
}

void CourseEditorState::update(float delta)
{
	if(focus == ON_EDITOR)
	{
		const Point oldOffset = course.minimap.offset;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP))
			course.minimap.offset.y -= 100*delta/course.minimap.scale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN))
			course.minimap.offset.y += 100*delta/course.minimap.scale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_LEFT))
			course.minimap.offset.x -= 100*delta/course.minimap.scale.x;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_RIGHT))
			course.minimap.offset.x += 100*delta/course.minimap.scale.x;

		const Vector2D oldScale = course.minimap.scale;

		if(Keyboard::isKeyPressed(Keyboard::KEY_Q))
			course.minimap.scale.y *= 1+delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_A))
			course.minimap.scale.y *= 1-delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_X))
			course.minimap.scale.x *= 1+delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_Z))
			course.minimap.scale.x *= 1-delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_E))
			course.minimap.scale *= 1+delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_S))
			course.minimap.scale *= 1-delta;

		if(course.minimap.scale.x != oldScale.x)
			course.minimap.offset.x *= oldScale.x/course.minimap.scale.x;

		if(course.minimap.scale.y != oldScale.y)
			course.minimap.offset.y *= oldScale.y/course.minimap.scale.y;

		if(course.minimap.scale != oldScale or scaleIndicatorText.empty() or course.minimap.offset != oldOffset)
		{
			scaleIndicatorText = "Zoom: x=";
			scaleIndicatorText.append(futil::to_string(course.minimap.scale.x));
			scaleIndicatorText.append(", y=");
			scaleIndicatorText.append(futil::to_string(course.minimap.scale.y));
			course.minimap.compile();
		}
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
			try
			{
				course.spec.saveToFile(CarseGame::Logic::COURSES_FOLDER+"/"+saveDialogTextField.content);
				game.logic.updateCourseList();
				reloadFileList();
			}
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

	const vector<Pseudo3DCourse::Spec>& courses = game.logic.getCourseList();
	for(unsigned i = 0; i < courses.size(); i++)
		fileMenu.addEntry(courses[i].filename);
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

	course.minimap.roadColor = Color::RED;
	course.minimap.roadContrastColorEnabled = true;
	course.minimap.segmentHighlightColor = Color::YELLOW;
	course.minimap.offset.x = 0.5*mapBounds.w;
	course.minimap.offset.y = 0.5*mapBounds.h;
	course.minimap.scale = Point();
	course.minimap.bounds = mapBounds;
	scaleIndicatorText.clear();

	focus = ON_EDITOR;
}
