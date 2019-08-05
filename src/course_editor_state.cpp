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
#include "futil/random.h"
#include "futil/collection_actions.hpp"

#include <algorithm>

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

//#define checkHighlightedOnEditor(button) button.highlighted = blinkCycle and focus == ON_EDITOR and button.bounds.contains(mousePosition)
//#define checkHighlighted(button) button.highlighted = blinkCycle and button.bounds.contains(mousePosition)

int CourseEditorState::getId() { return CarseGame::COURSE_EDITOR_STATE_ID; }

CourseEditorState::CourseEditorState(CarseGame* game)
: State(*game), game(*game), lastDisplaySize(), focus(),
  font(null), sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  newButton(), loadButton(), saveButton(), generateButton(), exitButton(),
  selectedLandscapeStyleIndex(), selectedRoadStyleIndex(), landscapeStyleChangeButton(), roadStyleChangeButton(),
  imgMenuCourseArrow(null), loadDialogSelectButton(), loadDialogCancelButton(),
  saveDialogSaveButton(), saveDialogCancelButton()
{}

CourseEditorState::~CourseEditorState()
{
	if(font != null) delete font;
}

void CourseEditorState::initialize()
{
	imgMenuCourseArrow = new Image("assets/arrow-blue.png");
	font = new Font(game.sharedResources->font1Path);

	fileMenu.setFont(font);
	fileMenu.setColor(Color::GREEN);

	saveDialogTextField.font = font;
	saveDialogTextField.bgColor = Color::BLACK;
	saveDialogTextField.bgColor.a = 127;
	saveDialogTextField.textColor = Color::WHITE;
	saveDialogTextField.borderColor = Color::_TRANSPARENT;

	toolsPanel.bgColor = Color::DARK_GREY;

	toolsTabbedPane.bgColor = Color::_TRANSPARENT;
	toolsTabbedPane.font = font;
	toolsPanel.addComponent(static_cast<fgeal::Panel&>(toolsTabbedPane));

	presetsTabPanel.bgColor = Color::BLACK;
	toolsTabbedPane.addComponent(presetsTabPanel, " Presets");

	shortStraightPresetButton.shape = Button::SHAPE_ROUNDED_RECTANGULAR;
	shortStraightPresetButton.bgColor = Color::GREY;
	shortStraightPresetButton.borderColor = Color::_TRANSPARENT;
	shortStraightPresetButton.highlightColor = Color::RED;
	shortStraightPresetButton.font = font;
	shortStraightPresetButton.label = " ' ";
	presetsTabPanel.addHighlightableComponent(shortStraightPresetButton);

	longStraightPresetButton = shortStraightPresetButton;
	longStraightPresetButton.label = " | ";
	presetsTabPanel.addHighlightableComponent(longStraightPresetButton);

	short90LeftCurvePresetButton = shortStraightPresetButton;
	short90LeftCurvePresetButton.label = " '-";
	presetsTabPanel.addHighlightableComponent(short90LeftCurvePresetButton);

	long90LeftCurvePresetButton = shortStraightPresetButton;
	long90LeftCurvePresetButton.label = " |_";
	presetsTabPanel.addHighlightableComponent(long90LeftCurvePresetButton);

	short90RightCurvePresetButton = shortStraightPresetButton;
	short90RightCurvePresetButton.label = "-' ";
	presetsTabPanel.addHighlightableComponent(short90RightCurvePresetButton);

	long90RightCurvePresetButton = shortStraightPresetButton;
	long90RightCurvePresetButton.label = "_| ";
	presetsTabPanel.addHighlightableComponent(long90RightCurvePresetButton);

	short45LeftCurvePresetButton = shortStraightPresetButton;
	short45LeftCurvePresetButton.label = " \\ ";
	presetsTabPanel.addHighlightableComponent(short45LeftCurvePresetButton);

	short45RightCurvePresetButton = shortStraightPresetButton;
	short45RightCurvePresetButton.label = " / ";
	presetsTabPanel.addHighlightableComponent(short45RightCurvePresetButton);

	straight20UpSlopePresetButton = shortStraightPresetButton;
	straight20UpSlopePresetButton.label = "/ \\";
	presetsTabPanel.addHighlightableComponent(straight20UpSlopePresetButton);

	straight20DownSlopePresetButton = shortStraightPresetButton;
	straight20DownSlopePresetButton.label = "\\ /";
	presetsTabPanel.addHighlightableComponent(straight20DownSlopePresetButton);

	propertiesTabPanel.bgColor = Color::GREY;
	toolsTabbedPane.addComponent(propertiesTabPanel, " Propert.");

	courseNameTextField = saveDialogTextField;
	courseNameTextField.caretHidden = true;
	propertiesTabPanel.addComponent(courseNameTextField);

	roadStyleTextField = landscapeStyleTextField = courseNameTextField;
	propertiesTabPanel.addComponent(roadStyleTextField);
	propertiesTabPanel.addComponent(landscapeStyleTextField);

	landscapeStyleChangeButton = shortStraightPresetButton;
	landscapeStyleChangeButton.shape = Button::SHAPE_RECTANGULAR;
	landscapeStyleChangeButton.bgColor = Color::LIGHT_GREY;
	landscapeStyleChangeButton.label = ">>";
	propertiesTabPanel.addComponent(landscapeStyleChangeButton);

	roadStyleChangeButton = landscapeStyleChangeButton;
	propertiesTabPanel.addComponent(roadStyleChangeButton);

	newButton = shortStraightPresetButton;
	newButton.shape = Button::SHAPE_RECTANGULAR;
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

	eraseButton = landscapeStyleChangeButton;
	eraseButton.font = &game.sharedResources->fontDev;
	eraseButton.label = "Erase";

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
		font->setSize(FontSizer(dh)(14));
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

	toolsPanel.bounds.x = 0;
	toolsPanel.bounds.y = 0.20*dh;
	toolsPanel.bounds.w = 0.25*dw;
	toolsPanel.bounds.h = 0.75*dh;

	toolsTabbedPane.bounds = toolsPanel.bounds;
	toolsTabbedPane.bounds.x += widgetSpacing;
	toolsTabbedPane.bounds.y += widgetSpacing;
	toolsTabbedPane.bounds.w -= 2*widgetSpacing;
	toolsTabbedPane.bounds.h -= 4*widgetSpacing + font->getTextHeight() + 0.1*dh;
	toolsTabbedPane.pack();

	presetsTabPanel.bounds = toolsTabbedPane.bounds;
	presetsTabPanel.bounds.y += 0.9*toolsTabbedPane.font->getTextHeight();

	shortStraightPresetButton.bounds.x = presetsTabPanel.bounds.x + widgetSpacing;
	shortStraightPresetButton.bounds.y = presetsTabPanel.bounds.y + widgetSpacing;
	shortStraightPresetButton.bounds.w = shortStraightPresetButton.bounds.h = 2.5*widgetSpacing;
	shortStraightPresetButton.highlightSpacing = 0.3*widgetSpacing;

	longStraightPresetButton.bounds = shortStraightPresetButton.bounds;
	longStraightPresetButton.bounds.x += shortStraightPresetButton.bounds.w + 2*widgetSpacing;
	longStraightPresetButton.highlightSpacing = shortStraightPresetButton.highlightSpacing;

	short90LeftCurvePresetButton.bounds = shortStraightPresetButton.bounds;
	short90LeftCurvePresetButton.bounds.y += shortStraightPresetButton.bounds.h + 2*widgetSpacing;
	short90LeftCurvePresetButton.highlightSpacing = shortStraightPresetButton.highlightSpacing;

	long90LeftCurvePresetButton.bounds = short90LeftCurvePresetButton.bounds;
	long90LeftCurvePresetButton.bounds.x += short90LeftCurvePresetButton.bounds.w + 2*widgetSpacing;
	long90LeftCurvePresetButton.highlightSpacing = short90LeftCurvePresetButton.highlightSpacing;

	short90RightCurvePresetButton.bounds = short90LeftCurvePresetButton.bounds;
	short90RightCurvePresetButton.bounds.y += short90LeftCurvePresetButton.bounds.h + 2*widgetSpacing;
	short90RightCurvePresetButton.highlightSpacing = short90LeftCurvePresetButton.highlightSpacing;

	long90RightCurvePresetButton.bounds = short90RightCurvePresetButton.bounds;
	long90RightCurvePresetButton.bounds.x += short90RightCurvePresetButton.bounds.w + 2*widgetSpacing;
	long90RightCurvePresetButton.highlightSpacing = short90RightCurvePresetButton.highlightSpacing;

	short45LeftCurvePresetButton.bounds = short90RightCurvePresetButton.bounds;
	short45LeftCurvePresetButton.bounds.y += short90RightCurvePresetButton.bounds.h + 2*widgetSpacing;
	short45LeftCurvePresetButton.highlightSpacing = short90RightCurvePresetButton.highlightSpacing;

	short45RightCurvePresetButton.bounds = short45LeftCurvePresetButton.bounds;
	short45RightCurvePresetButton.bounds.x += short45LeftCurvePresetButton.bounds.w + 2*widgetSpacing;
	short45RightCurvePresetButton.highlightSpacing = short45LeftCurvePresetButton.highlightSpacing;

	straight20UpSlopePresetButton.bounds = short45LeftCurvePresetButton.bounds;
	straight20UpSlopePresetButton.bounds.y += short45LeftCurvePresetButton.bounds.h + 2*widgetSpacing;
	straight20UpSlopePresetButton.highlightSpacing = short45LeftCurvePresetButton.highlightSpacing;

	straight20DownSlopePresetButton.bounds = straight20UpSlopePresetButton.bounds;
	straight20DownSlopePresetButton.bounds.x += straight20UpSlopePresetButton.bounds.w + 2*widgetSpacing;
	straight20DownSlopePresetButton.highlightSpacing = straight20UpSlopePresetButton.highlightSpacing;

	propertiesTabPanel.bounds = presetsTabPanel.bounds;

	courseNameTextField.content.clear();
	courseNameTextField.caretPosition = 0;
	courseNameTextField.caretHidden = true;
	courseNameTextField.bounds.x = propertiesTabPanel.bounds.x + widgetSpacing;
	courseNameTextField.bounds.y = propertiesTabPanel.bounds.y + font->getTextHeight() + widgetSpacing;
	courseNameTextField.bounds.w = propertiesTabPanel.bounds.w - 2*widgetSpacing;
	courseNameTextField.bounds.h = 1.1f*courseNameTextField.font->getTextHeight();

	landscapeStyleTextField.bounds = courseNameTextField.bounds;
	landscapeStyleTextField.bounds.y += courseNameTextField.bounds.h + font->getTextHeight() + widgetSpacing;
	landscapeStyleTextField.bounds.w *= 0.75;

	roadStyleTextField.bounds = landscapeStyleTextField.bounds;
	roadStyleTextField.bounds.y += landscapeStyleTextField.bounds.h + font->getTextHeight() + widgetSpacing;

	landscapeStyleChangeButton.bounds.x = landscapeStyleTextField.bounds.x + landscapeStyleTextField.bounds.w + widgetSpacing;
	landscapeStyleChangeButton.bounds.y = landscapeStyleTextField.bounds.y;
	landscapeStyleChangeButton.bounds.w = propertiesTabPanel.bounds.w - landscapeStyleTextField.bounds.w - 3*widgetSpacing;
	landscapeStyleChangeButton.bounds.h = font->getTextHeight();

	roadStyleChangeButton = landscapeStyleChangeButton;
	roadStyleChangeButton.bounds.x = roadStyleTextField.bounds.x + roadStyleTextField.bounds.w + widgetSpacing;
	roadStyleChangeButton.bounds.y = roadStyleTextField.bounds.y;

	newButton.bounds.x = presetsTabPanel.bounds.x;
	newButton.bounds.y = presetsTabPanel.bounds.y + presetsTabPanel.bounds.h + widgetSpacing;
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
	fileMenu.bounds.h = loadDialogBounds.h - widgetSpacing*2 - font->getTextHeight();

	imgMenuCourseArrowUpBounds.w = imgMenuCourseArrowUpBounds.h = dh*0.04;
	imgMenuCourseArrowUpBounds.x = fileMenu.bounds.x + fileMenu.bounds.w - imgMenuCourseArrowUpBounds.w;
	imgMenuCourseArrowUpBounds.y = fileMenu.bounds.y;

	imgMenuCourseArrowDownBounds = imgMenuCourseArrowUpBounds;
	imgMenuCourseArrowDownBounds.y = fileMenu.bounds.y + fileMenu.bounds.h - imgMenuCourseArrowDownBounds.h;

	loadDialogSelectButton.bounds.w = 1.1*font->getTextWidth("Select");
	loadDialogSelectButton.bounds.h = 1.1*font->getTextHeight();
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
	saveDialogTextField.bounds.y = saveDialogBounds.y + widgetSpacing + font->getTextHeight();
	saveDialogTextField.bounds.w = saveDialogBounds.w - 2*widgetSpacing;
	saveDialogTextField.bounds.h = 1.1*font->getTextHeight();
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

	eraseButton.bounds.w = 4.0 * widgetSpacing;
	eraseButton.bounds.h = 0.8 * statusBarBounds.h;
	eraseButton.bounds.x = dw - eraseButton.bounds.w - widgetSpacing;
	eraseButton.bounds.y = dh - eraseButton.bounds.h - 0.5*(statusBarBounds.h - eraseButton.bounds.h);
	eraseButton.highlightSpacing = 0.5*(statusBarBounds.h - eraseButton.bounds.h);

	// initial values

	focus = ON_EDITOR;
	toolsTabbedPane.setActiveTab(presetsTabPanel);

	selectedLandscapeStyleIndex = selectedRoadStyleIndex = 0;
	landscapeStyleTextField.content = roadStyleTextField.content = "default";
	course.spec.assignStyle(Pseudo3DCourse::Spec::RoadStyle::DEFAULT);
	course.spec.assignStyle(Pseudo3DCourse::Spec::LandscapeStyle::DEFAULT);

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

	const bool isBlinkCycle = (cos(20*fgeal::uptime()) > 0);
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

	toolsPanel.hoveringDisabled = not isBlinkCycle or focus != ON_EDITOR;
	toolsPanel.draw();

	if(toolsTabbedPane.isActiveTab(propertiesTabPanel))
	{
		font->drawText("name:", courseNameTextField.bounds.x, courseNameTextField.bounds.y - font->getTextHeight());
		font->drawText("landscape:", landscapeStyleTextField.bounds.x, landscapeStyleTextField.bounds.y - font->getTextHeight());
		font->drawText("road style:", roadStyleTextField.bounds.x, roadStyleTextField.bounds.y - font->getTextHeight());
	}

	newButton.highlighted = isBlinkCycle and focus == ON_EDITOR and newButton.bounds.contains(mousePosition);
	newButton.draw();

	loadButton.highlighted = isBlinkCycle and focus == ON_EDITOR and loadButton.bounds.contains(mousePosition);
	loadButton.draw();

	saveButton.highlighted = isBlinkCycle and focus == ON_EDITOR and saveButton.bounds.contains(mousePosition);
	saveButton.draw();

	generateButton.highlighted = isBlinkCycle and focus == ON_EDITOR and generateButton.bounds.contains(mousePosition);
	generateButton.draw();

	exitButton.highlighted = isBlinkCycle and focus == ON_EDITOR and exitButton.bounds.contains(mousePosition);
	exitButton.draw();

	// load file dialog
	if(focus == ON_FILE_MENU)
	{
		Graphics::drawFilledRoundedRectangle(loadDialogBounds, 10, Color::GREY);
		Graphics::drawRoundedRectangle(loadDialogBounds, 10, Color::DARK_GREY);

		fileMenu.draw();

		{
			const Rectangle& bounds = imgMenuCourseArrowUpBounds;
			imgMenuCourseArrow->drawScaledRotated(bounds.x+bounds.w/4, bounds.y+bounds.h*0.8, scaledToRect(imgMenuCourseArrow, bounds), M_PI/2, bounds.w/2, bounds.h/2);
		}
		{
			const Rectangle& bounds = imgMenuCourseArrowDownBounds;
			imgMenuCourseArrow->drawScaledRotated(bounds.x+bounds.w/4, bounds.y+bounds.h*0.8, scaledToRect(imgMenuCourseArrow, bounds), M_PI/2, bounds.w/2, bounds.h/2, Image::FLIP_HORIZONTAL);
		}

		loadDialogSelectButton.highlighted = isBlinkCycle and loadDialogSelectButton.bounds.contains(mousePosition);
		loadDialogSelectButton.draw();

		loadDialogCancelButton.highlighted = isBlinkCycle and loadDialogCancelButton.bounds.contains(mousePosition);
		loadDialogCancelButton.draw();
	}

	// save file dialog
	if(focus == ON_SAVE_DIALOG)
	{
		Graphics::drawFilledRoundedRectangle(saveDialogBounds, 10, Color::GREY);
		Graphics::drawRoundedRectangle(saveDialogBounds, 10, Color::DARK_GREY);

		font->drawText("Enter a filename for the course:", saveDialogBounds.x + widgetSpacing, saveDialogBounds.y + widgetSpacing, Color::BLACK);

		saveDialogTextField.draw();

		saveDialogSaveButton.highlighted = isBlinkCycle and saveDialogSaveButton.bounds.contains(mousePosition);
		saveDialogSaveButton.draw();

		saveDialogCancelButton.highlighted = isBlinkCycle and saveDialogCancelButton.bounds.contains(mousePosition);
		saveDialogCancelButton.draw();
	}

	// status bar
	Graphics::drawFilledRectangle(statusBarBounds, Color::GREY);
	eraseButton.highlightColor = Keyboard::isKeyPressed(Keyboard::KEY_LEFT_SHIFT)? Color::MAGENTA:
			 	 	 	 	 	 Keyboard::isKeyPressed(Keyboard::KEY_LEFT_CONTROL)? Color::ROSE: Color::RED;
	eraseButton.highlighted = isBlinkCycle and focus == ON_EDITOR and eraseButton.bounds.contains(mousePosition);
	eraseButton.draw();
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
		{
			sndCursorOut->play();
			game.enterState(CarseGame::COURSE_SELECTION_STATE_ID);
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
			this->loadCourseSpec(Pseudo3DCourse::Spec::createFromFile(fileMenu.getSelectedEntry().label));
			focus = ON_EDITOR;
		}
	}
	else if(focus == ON_SAVE_DIALOG)
	{
		if(key == Keyboard::KEY_ESCAPE)
		{
			sndCursorOut->play();
			focus = ON_EDITOR;
		}
		else
		{
			saveDialogTextField.onKeyPressed(key);
			sndCursorMove->play();
		}
	}
	else if(focus == ON_NAME_TEXTFIELD)
	{
		if(key == Keyboard::KEY_ESCAPE)
		{
			sndCursorOut->play();
			focus = ON_EDITOR;
			courseNameTextField.caretHidden = true;
		}
		else
		{
			courseNameTextField.onKeyPressed(key);
			sndCursorMove->play();
		}
	}
}

void CourseEditorState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{
	if(focus == ON_EDITOR)
	{
		if(toolsTabbedPane.setActiveTabByButtonPosition(x, y))
		{
			sndCursorMove->play();
		}
		else if(toolsTabbedPane.isActiveTab(presetsTabPanel))
		{
			const unsigned shortSegmentAmount = 100, longSegmentAmount = 3*shortSegmentAmount;
			unsigned segmentAmountToAdd = 0;
			float totalAngle = 0, slope = 0;

			if(shortStraightPresetButton.bounds.contains(x, y))
				segmentAmountToAdd = shortSegmentAmount;

			else if(longStraightPresetButton.bounds.contains(x, y))
				segmentAmountToAdd = longSegmentAmount;

			else if(short90LeftCurvePresetButton.bounds.contains(x, y))
			{
				totalAngle = -M_PI_2;
				segmentAmountToAdd = shortSegmentAmount;
			}
			else if(long90LeftCurvePresetButton.bounds.contains(x, y))
			{
				totalAngle = -M_PI_2;
				segmentAmountToAdd = longSegmentAmount;
			}
			else if(short90RightCurvePresetButton.bounds.contains(x, y))
			{
				totalAngle = M_PI_2;
				segmentAmountToAdd = shortSegmentAmount;
			}
			else if(long90RightCurvePresetButton.bounds.contains(x, y))
			{
				totalAngle = M_PI_2;
				segmentAmountToAdd = longSegmentAmount;
			}
			else if(short45LeftCurvePresetButton.bounds.contains(x, y))
			{
				totalAngle = -M_PI_4;
				segmentAmountToAdd = shortSegmentAmount;
			}
			else if(short45RightCurvePresetButton.bounds.contains(x, y))
			{
				totalAngle = M_PI_4;
				segmentAmountToAdd = shortSegmentAmount;
			}
			else if(straight20UpSlopePresetButton.bounds.contains(x, y))
			{
				slope = 0.5*M_PI_4;  // 22.5 degrees instead of 20
				segmentAmountToAdd = shortSegmentAmount;
			}
			else if(straight20DownSlopePresetButton.bounds.contains(x, y))
			{
				slope = -0.5*M_PI_4;  // 22.5 degrees instead of 20
				segmentAmountToAdd = shortSegmentAmount;
			}

			if(segmentAmountToAdd > 0)
			{
				CourseSpec::Segment segment;
				if(totalAngle != 0)
				{
					segmentAmountToAdd *= fabs(totalAngle);
					segment.curve = totalAngle * course.spec.roadSegmentLength/segmentAmountToAdd;
				}
				for(unsigned i = 0; i < segmentAmountToAdd; i++)
				{
					segment.z = course.spec.lines.size()*course.spec.roadSegmentLength;
					segment.y = (course.spec.lines.empty()? 0 : course.spec.lines.back().y) + course.spec.roadSegmentLength * sin(slope);
					course.spec.lines.push_back(segment);
				}
				this->loadCourseSpec(course.spec);
			}
		}
		else
		{
			if(landscapeStyleChangeButton.bounds.contains(x, y))
			{
				sndCursorIn->play();
				vector<string> presetLandscaleStylesNames = game.logic.getPresetLandscapeStylesNames();
				selectedLandscapeStyleIndex++;
				if(selectedLandscapeStyleIndex >= (int) presetLandscaleStylesNames.size())
					selectedLandscapeStyleIndex = 0;

				landscapeStyleTextField.content = presetLandscaleStylesNames[selectedLandscapeStyleIndex];
				course.spec.props.clear();
				course.spec.spritesFilenames.clear();
				course.spec.assignStyle(game.logic.getPresetLandscapeStyle(landscapeStyleTextField.content));
				course.loadSpec(course.spec);  // reloads its own spec so it reload the new sprites
			}
			if(roadStyleChangeButton.bounds.contains(x, y))
			{
				sndCursorIn->play();
				vector<string> presetLandscaleStylesNames = game.logic.getPresetRoadStylesNames();
				selectedRoadStyleIndex++;
				if(selectedRoadStyleIndex >= (int) presetLandscaleStylesNames.size())
					selectedRoadStyleIndex = 0;

				roadStyleTextField.content = presetLandscaleStylesNames[selectedRoadStyleIndex];
				course.spec.assignStyle(game.logic.getPresetRoadStyle(roadStyleTextField.content));
			}
			if(courseNameTextField.bounds.contains(x, y))
			{
				sndCursorIn->play();
				focus = ON_NAME_TEXTFIELD;
				courseNameTextField.caretHidden = false;
			}
		}

		if(newButton.bounds.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourseSpec(Pseudo3DCourse::Spec(200, 3000));
			selectedLandscapeStyleIndex = selectedRoadStyleIndex = 0;
			landscapeStyleTextField.content = roadStyleTextField.content = "default";
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
			Pseudo3DCourse::Spec spec = Pseudo3DCourse::Spec::createRandom(200, 3000, 6400, 1.5);

			const vector<string> roadStyleList = game.logic.getPresetRoadStylesNames();
			selectedRoadStyleIndex = futil::random_between(0, roadStyleList.size());
			roadStyleTextField.content = roadStyleList[selectedRoadStyleIndex];
			spec.assignStyle(game.logic.getPresetRoadStyle(roadStyleTextField.content));

			const vector<string> landscapeStyleList = game.logic.getPresetLandscapeStylesNames();
			selectedLandscapeStyleIndex = futil::random_between(0, landscapeStyleList.size());
			landscapeStyleTextField.content = landscapeStyleList[selectedLandscapeStyleIndex];
			spec.assignStyle(game.logic.getPresetLandscapeStyle(landscapeStyleTextField.content));

			this->loadCourseSpec(spec);
		}

		if(exitButton.bounds.contains(x, y))
		{
			sndCursorOut->play();
			game.enterState(CarseGame::COURSE_SELECTION_STATE_ID);
		}

		if(eraseButton.bounds.contains(x, y))
		{
			const unsigned ammountToRemove = std::min(Keyboard::isKeyPressed(Keyboard::KEY_LEFT_SHIFT)?  100:
													  Keyboard::isKeyPressed(Keyboard::KEY_LEFT_CONTROL)? 10: 1, (int) course.spec.lines.size());
			sndCursorIn->play();
			course.spec.lines.resize(course.spec.lines.size() - ammountToRemove);
			this->loadCourseSpec(course.spec);
		}
	}
	else if(focus == ON_FILE_MENU)
	{
		if(imgMenuCourseArrowUpBounds.contains(x, y))
		{
			sndCursorMove->play();
			fileMenu.moveCursorUp();
		}

		else if(imgMenuCourseArrowDownBounds.contains(x, y))
		{
			sndCursorMove->play();
			fileMenu.moveCursorDown();
		}

		else if(fileMenu.bounds.contains(x, y))
		{
			sndCursorMove->play();
			fileMenu.setSelectedIndexByLocation(x, y);
		}

		if(loadDialogSelectButton.bounds.contains(x, y))
		{
			sndCursorIn->play();
			this->loadCourseSpec(Pseudo3DCourse::Spec::createFromFile(fileMenu.getSelectedEntry().label));
			if(course.spec.presetRoadStyleName.empty())
			{
				roadStyleTextField.content = "custom";
				selectedRoadStyleIndex = -1;
			}
			else
			{
				roadStyleTextField.content = course.spec.presetRoadStyleName;
				selectedRoadStyleIndex = futil::index_of(game.logic.getPresetRoadStylesNames(), roadStyleTextField.content);
			}
			if(course.spec.presetLandscapeStyleName.empty())
			{
				landscapeStyleTextField.content = "custom";
				selectedLandscapeStyleIndex = -1;
			}
			else
			{
				landscapeStyleTextField.content = course.spec.presetLandscapeStyleName;
				selectedLandscapeStyleIndex = futil::index_of(game.logic.getPresetLandscapeStylesNames(), landscapeStyleTextField.content);
			}
			focus = ON_EDITOR;
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
			course.spec.name = courseNameTextField.content.empty()? saveDialogTextField.content : courseNameTextField.content;
			course.spec.comments = "Generated using carse v" + CARSE_VERSION;

			try { course.spec.saveToFile(CarseLogic::COURSES_FOLDER+"/"+saveDialogTextField.content); }
			catch(const std::exception& e) { /* TODO show error dialog */ }

			game.logic.updateCourseList();
			reloadFileList();
			focus = ON_EDITOR;
		}

		if(saveDialogCancelButton.bounds.contains(x, y))
		{
			sndCursorIn->play();
			focus = ON_EDITOR;
		}
	}
	else if(focus == ON_NAME_TEXTFIELD)
	{
		if(not courseNameTextField.bounds.contains(x, y))
		{
			sndCursorOut->play();
			focus = ON_EDITOR;
			courseNameTextField.caretHidden = true;
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
}
