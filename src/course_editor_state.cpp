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
using fgeal::Button;
using fgeal::Point;
using fgeal::Vector2D;
using std::vector;
using std::string;
using futil::ends_with;
using futil::Properties;

int CourseEditorState::getId() { return CarseGame::COURSE_EDITOR_STATE_ID; }

CourseEditorState::CourseEditorState(CarseGame* game)
: State(*game), game(*game), lastDisplaySize(), focus(),
  font(null), sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  newButton(), loadButton(), saveButton(), generateButton(), exitButton(),
  isPresetsTabActive(),
  loadDialogSelectButton(), loadDialogCancelButton(),
  saveDialogSaveButton(), saveDialogCancelButton()
{}

CourseEditorState::~CourseEditorState()
{
	if(font != null) delete font;
}

void CourseEditorState::initialize()
{
	font = new Font(game.sharedResources->font1Path);

	fileMenu.setFont(font);
	fileMenu.setColor(Color::GREEN);

	saveDialogTextField.font = font;
	saveDialogTextField.bgColor = Color::BLACK;
	saveDialogTextField.bgColor.a = 127;
	saveDialogTextField.textColor = Color::WHITE;
	saveDialogTextField.borderColor = Color::_TRANSPARENT;

	presetsTabButton.shape = Button::SHAPE_ROUNDED_RECTANGULAR;
	presetsTabButton.bgColor = Color::GREY;
	presetsTabButton.borderColor = Color::_TRANSPARENT;
	presetsTabButton.highlightColor = Color::LIGHT_GREY;
	presetsTabButton.font = font;
	presetsTabButton.label = "Preset";

	propertiesTabButton = presetsTabButton;
	propertiesTabButton.label = "Proper.";

	newButton = presetsTabButton;
	newButton.shape = Button::SHAPE_RECTANGULAR;
	newButton.highlightColor = Color::RED;
	newButton.label = "New";

	loadButton = newButton;
	loadButton.label = "Load";

	saveButton = newButton;
	saveButton.label = "Save";

	generateButton = newButton;
	generateButton.label = "Generate";

	exitButton = newButton;
	exitButton.label = "Exit";

	loadDialogSelectButton = newButton;
	loadDialogSelectButton.bgColor = Color::LIGHT_GREY;
	loadDialogSelectButton.label = "Select";

	loadDialogCancelButton = loadDialogSelectButton;
	loadDialogCancelButton.label = "Cancel";

	saveDialogSaveButton = loadDialogSelectButton;
	saveDialogSaveButton.label = "Save";

	saveDialogCancelButton = loadDialogCancelButton;

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

	// reload fonts if display size changed
	if(lastDisplaySize.x != dw or lastDisplaySize.y != dh)
	{
		font->setFontSize(dip(15));
		lastDisplaySize.x = dw;
		lastDisplaySize.y = dh;
	}

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

	presetsTabButton.bounds = toolsPanelBounds;
	presetsTabButton.bounds.x += widgetSpacing;
	presetsTabButton.bounds.y += widgetSpacing;
	presetsTabButton.bounds.w /= 2;
	presetsTabButton.bounds.w -= 2*widgetSpacing;
	presetsTabButton.bounds.h = std::max(1.2f*presetsTabButton.font->getHeight(), 2*widgetSpacing);
	presetsTabButton.highlighted = false;

	propertiesTabButton.bounds = presetsTabButton.bounds;
	propertiesTabButton.bounds.x += presetsTabButton.bounds.w + widgetSpacing;
	propertiesTabButton.highlighted = false;

	presetsTabPanelBounds = toolsPanelBounds;
	presetsTabPanelBounds.x += widgetSpacing;
	presetsTabPanelBounds.y += widgetSpacing + 0.75 * presetsTabButton.bounds.h;
	presetsTabPanelBounds.w -= 2*widgetSpacing;
	presetsTabPanelBounds.h -= 4*widgetSpacing + presetsTabButton.bounds.h + 0.1*dh;

	propertiesTabPanelBounds = presetsTabPanelBounds;

	newButton.bounds.x = presetsTabPanelBounds.x;
	newButton.bounds.y = presetsTabPanelBounds.y + presetsTabPanelBounds.h + widgetSpacing;
	newButton.bounds.w = 0.08*dh;
	newButton.bounds.h = 0.05*dh;
	newButton.highlightSpacing = 0.4*widgetSpacing;

	loadButton.bounds = newButton.bounds;
	loadButton.bounds.x += newButton.bounds.w + widgetSpacing;
	loadButton.highlightSpacing = newButton.highlightSpacing;

	saveButton.bounds = loadButton.bounds;
	saveButton.bounds.x += loadButton.bounds.w + widgetSpacing;
	saveButton.highlightSpacing = newButton.highlightSpacing;

	generateButton.bounds = newButton.bounds;
	generateButton.bounds.w *= 2;
	generateButton.bounds.y += newButton.bounds.h + widgetSpacing;
	generateButton.highlightSpacing = newButton.highlightSpacing;

	exitButton.bounds = newButton.bounds;
	exitButton.bounds.y = generateButton.bounds.y;
	exitButton.bounds.x = generateButton.bounds.x + generateButton.bounds.w + widgetSpacing;
	exitButton.highlightSpacing = newButton.highlightSpacing;

	loadDialogBounds.x = 0.15*dw;
	loadDialogBounds.y = 0.20*dh;
	loadDialogBounds.w = 0.70*dw;
	loadDialogBounds.h = 0.55*dh;

	fileMenu.bounds.x = loadDialogBounds.x + widgetSpacing;
	fileMenu.bounds.y = loadDialogBounds.y + widgetSpacing;
	fileMenu.bounds.w = loadDialogBounds.w - widgetSpacing*2;
	fileMenu.bounds.h = loadDialogBounds.h - widgetSpacing*2 - font->getHeight();

	loadDialogSelectButton.bounds.w = 1.1*font->getTextWidth("Select");
	loadDialogSelectButton.bounds.h = 1.1*font->getHeight();
	loadDialogSelectButton.bounds.x = 0.5*(loadDialogBounds.x + loadDialogBounds.w - loadDialogSelectButton.bounds.w);
	loadDialogSelectButton.bounds.y = loadDialogBounds.y + loadDialogBounds.h - 1.2*loadDialogSelectButton.bounds.h;
	loadDialogSelectButton.highlightSpacing = newButton.highlightSpacing;

	loadDialogCancelButton.bounds = loadDialogSelectButton.bounds;
	loadDialogSelectButton.bounds.w = 1.1*font->getTextWidth("Cancel");
	loadDialogCancelButton.bounds.x += loadDialogSelectButton.bounds.w + widgetSpacing;
	loadDialogCancelButton.highlightSpacing = newButton.highlightSpacing;

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

	saveDialogSaveButton.bounds = loadDialogSelectButton.bounds;
	saveDialogSaveButton.bounds.y = saveDialogBounds.y + saveDialogBounds.h - saveDialogSaveButton.bounds.h - widgetSpacing;
	saveDialogSaveButton.highlightSpacing = newButton.highlightSpacing;

	saveDialogCancelButton.bounds = loadDialogCancelButton.bounds;
	saveDialogCancelButton.bounds.y = saveDialogSaveButton.bounds.y;
	saveDialogCancelButton.highlightSpacing = newButton.highlightSpacing;

	statusBarBounds.x = 0;
	statusBarBounds.y = 0.95*dh;
	statusBarBounds.w = dw;
	statusBarBounds.h = 0.05*dh;

	scaleIndicatorPosition.x = dw - game.sharedResources->fontDev.getTextWidth("zoom: x=00000000000, y=00000000000");
	scaleIndicatorPosition.y = statusBarBounds.y;
	scaleIndicatorText.clear();

	// initial values

	focus = ON_EDITOR;
	setPresetsTabActive();

	map.roadColor = Color::RED;
	map.roadContrastColorEnabled = true;
	map.segmentHighlightColor = Color::YELLOW;
	map.offset.x = 0.5*mapBounds.w;
	map.offset.y = 0.5*mapBounds.h;
	map.scale.x = map.scale.y = 1.f;
	map.bounds = mapBounds;

	this->loadCourseSpec(Pseudo3DCourse::Spec(200, 3000));
}

void CourseEditorState::onLeave()
{}

void CourseEditorState::render()
{
	const float widgetSpacing = 0.007*game.getDisplay().getHeight();
	const bool blinkCycle = (cos(20*fgeal::uptime()) > 0);
	const Point mousePosition = Mouse::getPosition();

	// Course preview
	Graphics::drawFilledRectangle(courseViewBounds, course.spec.colorLandscape);
	course.draw(0, 0.5*course.drawAreaWidth);
	Graphics::drawRectangle(courseViewBounds, Color::AZURE);
	game.sharedResources->fontDev.drawText("Preview", courseViewBounds.x, courseViewBounds.y, Color::RED);

	// Map
	Graphics::drawFilledRectangle(mapBounds, Color::DARK_GREEN);
	map.drawMap();
	font->drawText("Course editor", courseEditorTitlePosition, Color::WHITE);

	// Tools panel
	Graphics::drawFilledRectangle(toolsPanelBounds, Color::DARK_GREY);

	presetsTabButton.highlighted = focus == ON_EDITOR and presetsTabButton.bounds.contains(mousePosition);
	presetsTabButton.draw();

	propertiesTabButton.highlighted = focus == ON_EDITOR and propertiesTabButton.bounds.contains(mousePosition);
	propertiesTabButton.draw();

	if(isPresetsTabActive)
	{
		Graphics::drawFilledRectangle(presetsTabPanelBounds, Color::BLACK);
		// todo draw presets
	}
	else
	{
		Graphics::drawFilledRectangle(propertiesTabPanelBounds, Color::GREY);
	}

	newButton.highlighted = blinkCycle and focus == ON_EDITOR and newButton.bounds.contains(mousePosition);
	newButton.draw();

	loadButton.highlighted = blinkCycle and focus == ON_EDITOR and loadButton.bounds.contains(mousePosition);
	loadButton.draw();

	saveButton.highlighted = blinkCycle and focus == ON_EDITOR and saveButton.bounds.contains(mousePosition);
	saveButton.draw();

	generateButton.highlighted = blinkCycle and focus == ON_EDITOR and generateButton.bounds.contains(mousePosition);
	generateButton.draw();

	exitButton.highlighted = blinkCycle and focus == ON_EDITOR and exitButton.bounds.contains(mousePosition);
	exitButton.draw();

	// load file dialog
	if(focus == ON_FILE_MENU)
	{
		Graphics::drawFilledRoundedRectangle(loadDialogBounds, 10, Color::GREY);
		Graphics::drawRoundedRectangle(loadDialogBounds, 10, Color::DARK_GREY);

		fileMenu.draw();

		loadDialogSelectButton.highlighted = blinkCycle and loadDialogSelectButton.bounds.contains(mousePosition);
		loadDialogSelectButton.draw();

		loadDialogCancelButton.highlighted = blinkCycle and loadDialogCancelButton.bounds.contains(mousePosition);
		loadDialogCancelButton.draw();
	}

	// save file dialog
	if(focus == ON_SAVE_DIALOG)
	{
		Graphics::drawFilledRoundedRectangle(saveDialogBounds, 10, Color::GREY);
		Graphics::drawRoundedRectangle(saveDialogBounds, 10, Color::DARK_GREY);

		font->drawText("Enter a filename for the course:", saveDialogBounds.x + widgetSpacing, saveDialogBounds.y + widgetSpacing, Color::BLACK);

		saveDialogTextField.draw();

		saveDialogSaveButton.highlighted = blinkCycle and saveDialogSaveButton.bounds.contains(mousePosition);
		saveDialogSaveButton.draw();

		saveDialogCancelButton.highlighted = blinkCycle and saveDialogCancelButton.bounds.contains(mousePosition);
		saveDialogCancelButton.draw();
	}

	// status bar
	Graphics::drawFilledRectangle(statusBarBounds, Color::GREY);
	game.sharedResources->fontDev.drawText(scaleIndicatorText, scaleIndicatorPosition, Color::WHITE);
}

void CourseEditorState::update(float delta)
{
	if(focus == ON_EDITOR)
	{
		const Point oldOffset = map.offset;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP))
			map.offset.y -= 100*delta/map.scale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN))
			map.offset.y += 100*delta/map.scale.y;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_LEFT))
			map.offset.x -= 100*delta/map.scale.x;

		if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_RIGHT))
			map.offset.x += 100*delta/map.scale.x;

		const Vector2D oldScale = map.scale;

		if(Keyboard::isKeyPressed(Keyboard::KEY_Q))
			map.scale.y *= 1+delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_A))
			map.scale.y *= 1-delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_X))
			map.scale.x *= 1+delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_Z))
			map.scale.x *= 1-delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_E))
			map.scale *= 1+delta;

		if(Keyboard::isKeyPressed(Keyboard::KEY_S))
			map.scale *= 1-delta;

		if(map.scale.x != oldScale.x)
			map.offset.x *= oldScale.x/map.scale.x;

		if(map.scale.y != oldScale.y)
			map.offset.y *= oldScale.y/map.scale.y;

		if(map.scale != oldScale or scaleIndicatorText.empty() or map.offset != oldOffset)
		{
			scaleIndicatorText = "Zoom: x=";
			scaleIndicatorText.append(futil::to_string(map.scale.x));
			scaleIndicatorText.append(", y=");
			scaleIndicatorText.append(futil::to_string(map.scale.y));
			map.compile();
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
			this->loadCourseSpec(Pseudo3DCourse::Spec::createFromFile(fileMenu.getSelectedEntry().label));
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
		if(presetsTabButton.bounds.contains(x, y) and not isPresetsTabActive)
		{
			sndCursorMove->play();
			setPresetsTabActive();
		}

		if(propertiesTabButton.bounds.contains(x, y) and isPresetsTabActive)
		{
			sndCursorMove->play();
			setPresetsTabActive(false);
		}

		if(newButton.bounds.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourseSpec(Pseudo3DCourse::Spec(200, 3000));
		}

		if(loadButton.bounds.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_FILE_MENU;
		}

		if(saveButton.bounds.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_SAVE_DIALOG;
		}

		if(generateButton.bounds.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourseSpec(Pseudo3DCourse::Spec::generateRandomCourseSpec(200, 3000, 6400, 1.5));
		}

		if(exitButton.bounds.contains(x, y))
		{
			sndCursorOut->play();
			game.enterState(CarseGame::COURSE_SELECTION_STATE_ID);
		}
	}
	else if(focus == ON_FILE_MENU)
	{
		if(fileMenu.bounds.contains(x, y))
		{
			sndCursorMove->play();
			fileMenu.setSelectedIndexByLocation(x, y);
		}

		if(loadDialogSelectButton.bounds.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourseSpec(Pseudo3DCourse::Spec::createFromFile(fileMenu.getSelectedEntry().label));
		}

		if(loadDialogCancelButton.bounds.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_EDITOR;
		}
	}
	else if(focus == ON_SAVE_DIALOG)
	{
		if(saveDialogSaveButton.bounds.contains(x, y))
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

		if(saveDialogCancelButton.bounds.contains(x, y))
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

void CourseEditorState::setPresetsTabActive(bool choice)
{
	isPresetsTabActive = choice;
	Button &selectedButton = isPresetsTabActive? presetsTabButton : propertiesTabButton,
		   &unselectedButton = isPresetsTabActive? propertiesTabButton : presetsTabButton;

	selectedButton.bgColor = Color::GREY;
	selectedButton.textColor = Color::BLACK;
	unselectedButton.bgColor = Color(112, 112, 112);
	unselectedButton.textColor = Color::DARK_GREY.getDarker();
}

void CourseEditorState::loadCourseSpec(const Pseudo3DCourse::Spec& spec)
{
	course.loadSpec(spec);
	course.drawAreaWidth = courseViewBounds.w;
	course.drawAreaHeight = courseViewBounds.h;
	course.drawDistance = 300;
	course.cameraDepth = 0.84;

	map.spec = course.spec;
	map.roadColor = Color::RED;
	map.roadContrastColorEnabled = true;
	map.segmentHighlightColor = Color::YELLOW;
	map.offset.x = 0.5*mapBounds.w;
	map.offset.y = 0.5*mapBounds.h;
	map.scale = Point();
	map.bounds = mapBounds;
	scaleIndicatorText.clear();

	focus = ON_EDITOR;
}
