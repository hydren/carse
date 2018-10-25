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

static char parseKeyStroke(fgeal::Keyboard::Key key);

int CourseEditorState::getId() { return CarseGame::COURSE_EDITOR_STATE_ID; }

CourseEditorState::CourseEditorState(CarseGame* game)
: State(*game), game(*game),
  font(null), sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  focus(), saveDialogFilenameTextFieldCaretPosition()
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

	saveDialogFilenameTextFieldBounds.x = saveDialogBounds.x + widgetSpacing;
	saveDialogFilenameTextFieldBounds.y = saveDialogBounds.y + widgetSpacing + font->getHeight();
	saveDialogFilenameTextFieldBounds.w = saveDialogBounds.w - 2*widgetSpacing;
	saveDialogFilenameTextFieldBounds.h = 1.1*font->getHeight();

	saveDialogSaveButtonBounds = loadDialogButtonSelectBounds;
	saveDialogSaveButtonBounds.y = saveDialogBounds.y + saveDialogBounds.h - saveDialogSaveButtonBounds.h - widgetSpacing;

	saveDialogCancelButtonBounds = loadDialogButtonCancelBounds;
	saveDialogCancelButtonBounds.y = saveDialogSaveButtonBounds.y;

	// initial values

	focus = ON_EDITOR;
	saveDialogFilenameTextFieldCaretPosition = 0;

	offset.x = 0.5*mapBounds.w;
	offset.y = 0.5*mapBounds.h;
	scale.x = scale.y = 1.f;

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


	course.drawMap(Color::RED, offset, scale, mapBounds);

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

		Graphics::drawFilledRectangle(saveDialogFilenameTextFieldBounds, Color(0, 0, 0, 128));
		font->drawText(saveDialogFilename, saveDialogFilenameTextFieldBounds.x, saveDialogFilenameTextFieldBounds.y, Color::WHITE);

		Graphics::drawFilledRectangle(saveDialogSaveButtonBounds, Color::LIGHT_GREY);
		font->drawText("Save", saveDialogSaveButtonBounds.x, saveDialogSaveButtonBounds.y, Color::BLACK);

		Graphics::drawFilledRectangle(saveDialogCancelButtonBounds, Color::LIGHT_GREY);
		font->drawText("Cancel", saveDialogCancelButtonBounds.x, saveDialogCancelButtonBounds.y, Color::BLACK);

		if(cos(20*fgeal::uptime()) > 0)
		{
			Graphics::drawFilledRectangle(saveDialogFilenameTextFieldBounds.x+font->getTextWidth(saveDialogFilename), saveDialogFilenameTextFieldBounds.y, 4, font->getHeight(), Color::WHITE);

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
			offset.y -= 100*delta/scale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN))
			offset.y += 100*delta/scale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_LEFT))
			offset.x -= 100*delta/scale.x;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_RIGHT))
			offset.x += 100*delta/scale.x;

		const fgeal::Vector2D oldScale = scale;

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

		if(scale.x != oldScale.x)
			offset.x *= oldScale.x/scale.x;

		if(scale.y != oldScale.y)
			offset.y *= oldScale.y/scale.y;
	}
}

void CourseEditorState::onKeyPressed(Keyboard::Key key)
{
	if(focus == ON_EDITOR)
	{
		if(key == Keyboard::KEY_ESCAPE)
		{
			game.enterState(CarseGame::COURSE_SELECTION_STATE_ID);
			saveDialogFilename.clear();
			saveDialogFilenameTextFieldCaretPosition = 0;
		}
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
			this->loadCourse(Pseudo3DCourse::parseCourseSpecFromFile(fileMenu.getSelectedEntry().label));
		}
	}
	else if(focus == ON_SAVE_DIALOG)
	{
		if(key== Keyboard::KEY_ESCAPE)
			focus = ON_EDITOR;

		else if(key == Keyboard::KEY_ENTER)
		{

		}
		else if(key == Keyboard::KEY_BACKSPACE)
		{
			if(not saveDialogFilename.empty() and saveDialogFilenameTextFieldCaretPosition > 0)
			{
				saveDialogFilename.erase(saveDialogFilename.begin() + saveDialogFilenameTextFieldCaretPosition-1);
				saveDialogFilenameTextFieldCaretPosition--;
			}
		}
		else if(key == Keyboard::KEY_ARROW_LEFT)
		{
			if(saveDialogFilenameTextFieldCaretPosition > 0)
				saveDialogFilenameTextFieldCaretPosition--;
		}
		else if(key == Keyboard::KEY_ARROW_RIGHT)
		{
			if(saveDialogFilenameTextFieldCaretPosition < (int) saveDialogFilename.size())
				saveDialogFilenameTextFieldCaretPosition++;
		}
		else
		{
			const char typed = parseKeyStroke(key);
			if(typed != '\n')
			{
				saveDialogFilename.insert(saveDialogFilename.begin() + saveDialogFilenameTextFieldCaretPosition, 1, typed);
				saveDialogFilenameTextFieldCaretPosition++;
			}
		}
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
			this->loadCourse(Pseudo3DCourse::generateRandomCourseSpec(200, 3000, 6400, 1.5));
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
			this->loadCourse(Pseudo3DCourse::parseCourseSpecFromFile(fileMenu.getSelectedEntry().label));
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
//			sndCursorIn->play();
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
	focus = ON_EDITOR;
}

// fixme this should be in fgeal package
static char parseKeyStroke(fgeal::Keyboard::Key key)
{
	char typed;
	switch(key)
	{
		default: typed = '\n'; break;
		case Keyboard::KEY_A: typed = 'a'; break;
		case Keyboard::KEY_B: typed = 'b'; break;
		case Keyboard::KEY_C: typed = 'c'; break;
		case Keyboard::KEY_D: typed = 'd'; break;
		case Keyboard::KEY_E: typed = 'e'; break;
		case Keyboard::KEY_F: typed = 'f'; break;
		case Keyboard::KEY_G: typed = 'g'; break;
		case Keyboard::KEY_H: typed = 'h'; break;
		case Keyboard::KEY_I: typed = 'i'; break;
		case Keyboard::KEY_J: typed = 'j'; break;
		case Keyboard::KEY_K: typed = 'k'; break;
		case Keyboard::KEY_L: typed = 'l'; break;
		case Keyboard::KEY_M: typed = 'm'; break;
		case Keyboard::KEY_N: typed = 'n'; break;
		case Keyboard::KEY_O: typed = 'o'; break;
		case Keyboard::KEY_P: typed = 'p'; break;
		case Keyboard::KEY_Q: typed = 'q'; break;
		case Keyboard::KEY_R: typed = 'r'; break;
		case Keyboard::KEY_S: typed = 's'; break;
		case Keyboard::KEY_T: typed = 't'; break;
		case Keyboard::KEY_U: typed = 'u'; break;
		case Keyboard::KEY_V: typed = 'v'; break;
		case Keyboard::KEY_W: typed = 'w'; break;
		case Keyboard::KEY_X: typed = 'x'; break;
		case Keyboard::KEY_Y: typed = 'y'; break;
		case Keyboard::KEY_Z: typed = 'z'; break;
		case Keyboard::KEY_NUMPAD_0:
		case Keyboard::KEY_0:       typed = '0'; break;
		case Keyboard::KEY_NUMPAD_1:
		case Keyboard::KEY_1:       typed = '1'; break;
		case Keyboard::KEY_NUMPAD_2:
		case Keyboard::KEY_2:       typed = '2'; break;
		case Keyboard::KEY_NUMPAD_3:
		case Keyboard::KEY_3:       typed = '3'; break;
		case Keyboard::KEY_NUMPAD_4:
		case Keyboard::KEY_4:       typed = '4'; break;
		case Keyboard::KEY_NUMPAD_5:
		case Keyboard::KEY_5:       typed = '5'; break;
		case Keyboard::KEY_NUMPAD_6:
		case Keyboard::KEY_6:       typed = '6'; break;
		case Keyboard::KEY_NUMPAD_7:
		case Keyboard::KEY_7:       typed = '7'; break;
		case Keyboard::KEY_NUMPAD_8:
		case Keyboard::KEY_8:       typed = '8'; break;
		case Keyboard::KEY_NUMPAD_9:
		case Keyboard::KEY_9:       typed = '9'; break;
		case Keyboard::KEY_SPACE:   typed = ' '; break;
		case Keyboard::KEY_PERIOD:  typed = '.'; break;
		case Keyboard::KEY_MINUS:   typed = '-'; break;
	}

	if(typed >= 'a' and typed <= 'z')
	if(Keyboard::isKeyPressed(Keyboard::KEY_LEFT_SHIFT)
	or Keyboard::isKeyPressed(Keyboard::KEY_RIGHT_SHIFT))
		typed -= 32;

	if(typed == '-')
	if(Keyboard::isKeyPressed(Keyboard::KEY_LEFT_SHIFT)
	or Keyboard::isKeyPressed(Keyboard::KEY_RIGHT_SHIFT))
		typed = '_';

	return typed;
}
