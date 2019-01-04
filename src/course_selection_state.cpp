/*
 * course_selection_state.cpp
 *
 *  Created on: 23 de mai de 2017
 *      Author: carlosfaruolo
 */

#include "course_selection_state.hpp"

#include "futil/string_actions.hpp"

#include "util.hpp"

#include "pseudo3d_race_state.hpp"

#include <cmath>

using fgeal::Display;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Mouse;
using fgeal::Font;
using fgeal::Color;
using fgeal::Image;
using fgeal::Graphics;
using fgeal::Sound;
using fgeal::Rectangle;
using fgeal::Point;
using fgeal::Vector2D;
using fgeal::Menu;
using std::vector;
using std::string;
using futil::to_string;

enum SettingsMenuIndex
{
	SETTINGS_RACE_TYPE = 0,
	SETTINGS_LAPS = 1
};

static const string TITLE_TEXT = "Choose a course";

int CourseSelectionState::getId() { return CarseGame::COURSE_SELECTION_STATE_ID; }

CourseSelectionState::CourseSelectionState(CarseGame* game)
: State(*game), game(*game),
  backgroundImage(null), imgRandom(null), imgCircuit(null), imgCourseEditor(null),
  fontMain(null), fontInfo(null), fontSmall(null),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  courseMapViewer(Pseudo3DCourse::Spec(0,0)),
  isLoadedCourseSelected(false), isDebugCourseSelected(false),
  focus(FOCUS_ON_COURSE_LIST_SELECTION)
{}

CourseSelectionState::~CourseSelectionState()
{
	if(backgroundImage != null) delete backgroundImage;
	if(imgRandom != null) delete imgRandom;
	if(imgCircuit != null) delete imgCircuit;
	if(imgCourseEditor != null) delete imgCourseEditor;
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(fontSmall != null) delete fontSmall;
}

void CourseSelectionState::initialize()
{
	Display& display = game.getDisplay();

	backgroundImage = new Image("assets/course-menu-bg.jpg");
	imgRandom = new Image("assets/portrait-random.png");
	imgCircuit = new Image("assets/portrait-circuit.png");
	imgCourseEditor = new Image("assets/portrait-course-editor.png");

	fontMain = new Font(game.sharedResources->font2Path, dip(28));
	fontInfo = new Font(game.sharedResources->font1Path, dip(14));
	fontSmall = new Font(game.sharedResources->font1Path, dip(10));

	menuCourse.setFont(new Font(game.sharedResources->font1Path, dip(12)), false);
	menuCourse.setColor(Color::RED);
	menuCourse.bgColor = Color(0, 0, 0, 128);
	menuCourse.borderColor = Color(0, 0, 0, 192);
	menuCourse.focusedEntryFontColor = Color::WHITE;

	menuSettings.setFont(new Font(game.sharedResources->font1Path, dip(12)), false);
	menuSettings.setColor(Color::RED);
	menuSettings.bgColor = menuCourse.bgColor;
	menuSettings.borderColor = menuCourse.borderColor;
	menuSettings.focusedEntryFontColor = menuCourse.focusedEntryFontColor;
	menuSettings.addEntry("Race type: " + Pseudo3DRaceState::toString(game.logic.getNextRaceSettings().raceType));
	menuSettings.addEntry("Laps: " + to_string(game.logic.getNextRaceSettings().lapCountGoal));
	menuSettings.cursorWrapAroundEnabled = false;

	courseMapViewer.roadColor = Color::WHITE;
	courseMapViewer.segmentHighlightColor = Color::YELLOW;
	courseMapViewer.geometryOtimizationEnabled = true;

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;
}

void CourseSelectionState::onEnter()
{
	const float dw = game.getDisplay().getWidth(), dh = game.getDisplay().getHeight();

	paneBounds.x = (1/64.f)*dw;
	paneBounds.y = (3/64.f)*dh + fontMain->getHeight();
	paneBounds.w = (62/64.f)*dw;
	paneBounds.h = dh - fontMain->getHeight() - (1/16.f)*dh;

	portraitBounds.x = paneBounds.x + (1/64.f)*paneBounds.w;
	portraitBounds.y = paneBounds.y + (1/64.f)*paneBounds.h;
	portraitBounds.w = (1/4.f)*paneBounds.h;
	portraitBounds.h = (1/4.f)*paneBounds.h;

	portraitImgBounds.x = portraitBounds.x + portraitBounds.w*0.02f;
	portraitImgBounds.y = portraitBounds.y + portraitBounds.h*0.02f;
	portraitImgBounds.w = portraitBounds.w * 0.96f;
	portraitImgBounds.h = portraitBounds.h * 0.96f;

	courseMapBounds.w = courseMapBounds.h = 0.42*paneBounds.h;
	courseMapBounds.x = paneBounds.x + paneBounds.w - courseMapBounds.w - (1/64.f)*paneBounds.w;
	courseMapBounds.y = portraitBounds.y;

	courseEditorPortraitBounds.x = portraitBounds.x;
	courseEditorPortraitBounds.y = portraitBounds.y + portraitBounds.h + 0.03*dh;
	courseEditorPortraitBounds.w = courseEditorPortraitBounds.h = (1/8.f)*paneBounds.h;

	menuCourse.bounds.x = portraitBounds.x;
	menuCourse.bounds.y = 0.5*dh;
	menuCourse.bounds.w = 0.5*paneBounds.w;
	menuCourse.bounds.h = (paneBounds.w - portraitBounds.h)/3;

	string previouslySelectedEntryLabel;
	if(not menuCourse.getEntries().empty())
	{
		previouslySelectedEntryLabel = menuCourse.getSelectedEntry().label;

		// clear course list
		while(not menuCourse.getEntries().empty())
			menuCourse.removeEntry(0);
	}

	menuCourse.addEntry("<Random course>");
	menuCourse.addEntry("<Debug course>");
	const vector<Pseudo3DCourse::Spec>& courses = game.logic.getCourseList();
	for(unsigned i = 0; i < courses.size(); i++)
		menuCourse.addEntry((string) courses[i]);

	if(not previouslySelectedEntryLabel.empty())
		for(unsigned i = 0; i < menuCourse.getEntries().size(); i++)
			if(menuCourse.getEntryAt(i).label == previouslySelectedEntryLabel)
			{
				menuCourse.setSelectedIndex(i);
				break;
			}

	menuSettings.bounds.x = menuCourse.bounds.x + menuCourse.bounds.w + dw*0.02;
	menuSettings.bounds.y = menuCourse.bounds.y;
	menuSettings.bounds.w = 0.45*paneBounds.w;
	menuSettings.bounds.h = menuCourse.bounds.h;

	courseMapViewer.segmentHighlightSize = 0.005*dh;
	courseMapViewer.bounds = courseMapBounds;

	backButtonBounds.x = 0.03*dw;
	backButtonBounds.y = 0.95*dh - fontInfo->getHeight();
	backButtonBounds.w = fontInfo->getTextWidth(" Back ");
	backButtonBounds.h = fontInfo->getHeight();

	selectButtonBounds.x = 0.85*dw;
	selectButtonBounds.y = 0.95*dh - fontInfo->getHeight();
	selectButtonBounds.w = fontInfo->getTextWidth(" Select ");
	selectButtonBounds.h = fontInfo->getHeight();

	focus = FOCUS_ON_COURSE_LIST_SELECTION;
}

void CourseSelectionState::onLeave()
{
	if(isLoadedCourseSelected)
		game.logic.setNextCourse(menuCourse.getSelectedIndex()-2);
	else if(isDebugCourseSelected)
		game.logic.setNextCourseDebug();
	else
		game.logic.setNextCourseRandom();
}

void CourseSelectionState::render()
{
	Display& display = game.getDisplay();
	const unsigned dw = display.getWidth(), dh = display.getHeight();
	const float focusSpacing = dh*0.01;
	const bool blinkCycle = cos(20*fgeal::uptime()) > 0;

	display.clear();

	// draw bg
	backgroundImage->drawScaled(0, 0, scaledToSize(backgroundImage, display));

	// draw panel bg
	fgeal::Graphics::drawFilledRectangle(paneBounds, Color(0,0,0, 96));

	// draw title
	fontMain->drawText(TITLE_TEXT, (1/32.f)*dw, (1/32.f)*dh, Color::WHITE);
	Graphics::drawLine(paneBounds.x, fontMain->getHeight()+(1/32.f)*dh, paneBounds.x + paneBounds.w, fontMain->getHeight()+(1/32.f)*dh, Color::WHITE);

	// portrait frame
	fgeal::Graphics::drawFilledRectangle(portraitBounds, Color::DARK_GREY);

	// draw portrait
	if(menuCourse.getSelectedIndex() == 0)
		imgRandom->drawScaled(portraitImgBounds.x, portraitImgBounds.y, scaledToRect(imgRandom, portraitImgBounds));
	else
		// todo choose correct portrait based on course specification
		imgCircuit->drawScaled(portraitImgBounds.x, portraitImgBounds.y, scaledToRect(imgCircuit, portraitImgBounds));

	// draw info
	Graphics::drawFilledRectangle(courseMapBounds, Color::BLACK);
	fontInfo->drawText(menuCourse.getSelectedEntry().label, 1.1*(portraitBounds.x + portraitBounds.w), portraitBounds.y, Color::WHITE);
	if(menuCourse.getSelectedIndex() > 1)
	{
		const Pseudo3DCourse::Spec& course = game.logic.getCourseList()[menuCourse.getSelectedIndex() - 2];
		const float courseLength = course.lines.size()*course.roadSegmentLength*0.001;
		const string txtLength = "Length: " + futil::to_string(courseLength) + "Km";
		fontInfo->drawText(txtLength, 1.1*(portraitBounds.x + portraitBounds.w), portraitBounds.y + fontInfo->getHeight(), Color::WHITE);
		courseMapViewer.drawMap(0);
	}

	// draw course editor portrait
	imgCourseEditor->drawScaled(courseEditorPortraitBounds.x, courseEditorPortraitBounds.y, scaledToRect(imgCourseEditor, courseEditorPortraitBounds));
	fontSmall->drawText("Course", courseEditorPortraitBounds.x, courseEditorPortraitBounds.y, Color::WHITE);
	fontSmall->drawText("  editor", courseEditorPortraitBounds.x, courseEditorPortraitBounds.y+fontSmall->getHeight(), Color::WHITE);
	if(focus == FOCUS_ON_COURSE_EDITOR_PORTRAIT and blinkCycle)
		fgeal::Graphics::drawRectangle(getSpacedOutline(courseEditorPortraitBounds, focusSpacing), Color::RED);

	// draw course list
	menuCourse.draw();
	if(focus == FOCUS_ON_COURSE_LIST_SELECTION and blinkCycle)
		fgeal::Graphics::drawRectangle(getSpacedOutline(menuCourse.bounds, focusSpacing), Color::RED);

	// draw race settings
	menuSettings.focusedEntryBgColor = (focus == FOCUS_ON_SETTINGS_LIST_SELECTION? Color::RED : Color::_TRANSPARENT);
	menuSettings.focusedEntryFontColor = (focus == FOCUS_ON_SETTINGS_LIST_SELECTION? Color::WHITE : Color::RED);
	menuSettings.draw();
	if(focus == FOCUS_ON_SETTINGS_LIST_SELECTION or (focus == FOCUS_ON_SETTINGS_LIST_HOVER and blinkCycle))
		fgeal::Graphics::drawRectangle(getSpacedOutline(menuSettings.bounds, focusSpacing), Color::RED);

	Graphics::drawFilledRoundedRectangle(backButtonBounds, 4, menuSettings.bgColor);
	fontInfo->drawText(" Back ", backButtonBounds.x, backButtonBounds.y, Color::WHITE);
	Graphics::drawFilledRoundedRectangle(selectButtonBounds, 4, menuSettings.bgColor);
	fontInfo->drawText(" Select ", selectButtonBounds.x, selectButtonBounds.y, Color::WHITE);
}

void CourseSelectionState::update(float delta)
{
	if(menuCourse.getSelectedIndex() > 1)
	{
		if(courseMapViewer.spec.filename != game.logic.getCourseList()[menuCourse.getSelectedIndex() - 2].filename)
		{
			courseMapViewer.spec = game.logic.getCourseList()[menuCourse.getSelectedIndex() - 2];
			courseMapViewer.scale.scale(0);
			courseMapViewer.offset.scale(0);
			courseMapViewer.compile();
		}
	}
	else if(not courseMapViewer.spec.filename.empty())
	{
		courseMapViewer.spec = Pseudo3DCourse::Spec(0,0);
		courseMapViewer.compile();
	}
}

void CourseSelectionState::updateLapCount()
{
	if(menuSettings.getEntryAt(1).enabled)
		menuSettings.getEntryAt(1).label = "Laps: " + to_string(game.logic.getNextRaceSettings().lapCountGoal);
	else
		menuSettings.getEntryAt(1).label = "Laps: --";
}

void CourseSelectionState::onKeyPressed(Keyboard::Key key)
{
	if(key == Keyboard::KEY_ESCAPE and focus != FOCUS_ON_SETTINGS_LIST_SELECTION)
	{
		//XXX should we ensure no changes are done to course selection this way?
		sndCursorOut->play();
		game.enterState(game.logic.currentMainMenuStateId);
	}
	else switch(focus)
	{
		case FOCUS_ON_COURSE_LIST_SELECTION:
			if(key == Keyboard::KEY_ENTER)
			{
				sndCursorIn->play();
				game.enterState(game.logic.currentMainMenuStateId);
			}
			else if(key == Keyboard::KEY_ARROW_RIGHT)
			{
				sndCursorMove->play();
				focus = FOCUS_ON_SETTINGS_LIST_HOVER;
			}
			else if(key == Keyboard::KEY_ARROW_UP)
			{
				sndCursorMove->play();
				if(menuCourse.getSelectedIndex() == 0)
					focus = FOCUS_ON_COURSE_EDITOR_PORTRAIT;
				else
					menuCourse.moveCursorUp();
			}
			else if(key == Keyboard::KEY_ARROW_DOWN)
			{
				sndCursorMove->play();
				menuCourse.moveCursorDown();
			}
			isDebugCourseSelected = (menuCourse.getSelectedIndex() == 1);
			isLoadedCourseSelected = (menuCourse.getSelectedIndex() > 1);
			break;

		case FOCUS_ON_SETTINGS_LIST_HOVER:
			if(key == Keyboard::KEY_ENTER)
			{
				sndCursorIn->play();
				focus = FOCUS_ON_SETTINGS_LIST_SELECTION;
			}
			else if(key == Keyboard::KEY_ARROW_LEFT)
			{
				sndCursorMove->play();
				focus = FOCUS_ON_COURSE_LIST_SELECTION;
			}
			break;

		case FOCUS_ON_SETTINGS_LIST_SELECTION:
			handleInputOnSettings(key);
			break;

		case FOCUS_ON_COURSE_EDITOR_PORTRAIT:
			if(key == Keyboard::KEY_ENTER)
			{
				sndCursorIn->play();
				game.enterState(CarseGame::COURSE_EDITOR_STATE_ID);
			}
			else if(key == Keyboard::KEY_ARROW_DOWN)
			{
				sndCursorMove->play();
				focus = FOCUS_ON_COURSE_LIST_SELECTION;
			}
			break;
		default: break;
	}
}

void CourseSelectionState::handleInputOnSettings(Keyboard::Key key)
{
	switch(key)
	{
		case Keyboard::KEY_ARROW_LEFT:
		case Keyboard::KEY_ARROW_RIGHT:
		case Keyboard::KEY_ENTER:
		{
			const bool isCursorLeft = (key == Keyboard::KEY_ARROW_LEFT);
			sndCursorMove->play();
			switch(menuSettings.getSelectedIndex())
			{
				case SETTINGS_RACE_TYPE:  // race type
				{
					unsigned nextType;
					if(isCursorLeft)
						if(game.logic.getNextRaceSettings().raceType == 0)
							nextType = Pseudo3DRaceState::RACE_TYPE_COUNT-1;
						else
							nextType = game.logic.getNextRaceSettings().raceType-1;
					else
						if(game.logic.getNextRaceSettings().raceType == Pseudo3DRaceState::RACE_TYPE_COUNT-1)
							nextType = 0;
						else
							nextType = game.logic.getNextRaceSettings().raceType+1;

					game.logic.getNextRaceSettings().raceType = static_cast<Pseudo3DRaceState::RaceType>(nextType);
					menuSettings.getEntryAt(SETTINGS_RACE_TYPE).label = "Race type: " + Pseudo3DRaceState::toString(game.logic.getNextRaceSettings().raceType);
					menuSettings.getEntryAt(SETTINGS_LAPS).enabled = Pseudo3DRaceState::isRaceTypeLoop(game.logic.getNextRaceSettings().raceType);
					updateLapCount();
					break;
				}
				case SETTINGS_LAPS:  // laps
				{
					// if not a loop type race, do nothing
					if(not menuSettings.getEntryAt(SETTINGS_LAPS).enabled)
						break;

					if(isCursorLeft)
					{
						if(game.logic.getNextRaceSettings().lapCountGoal > 2)
							game.logic.getNextRaceSettings().lapCountGoal--;
					}
					else
						game.logic.getNextRaceSettings().lapCountGoal++;

					updateLapCount();
					break;
				}
				default: break;
			}
		}
		break;

		case Keyboard::KEY_ESCAPE:
			sndCursorOut->play();
			focus = FOCUS_ON_SETTINGS_LIST_HOVER;
			break;

		case Keyboard::KEY_ARROW_UP:
			sndCursorMove->play();
			menuSettings.moveCursorUp();
			break;

		case Keyboard::KEY_ARROW_DOWN:
			sndCursorMove->play();
			menuSettings.moveCursorDown();
			break;

		default: break;
	}
}

void CourseSelectionState::onMouseButtonPressed(Mouse::Button button, int x, int y)
{
	if(button == Mouse::BUTTON_LEFT)
	{
		if(backButtonBounds.contains(x, y) or selectButtonBounds.contains(x, y))
			onKeyPressed(Keyboard::KEY_ESCAPE);

		else switch(focus)
		{
			case FOCUS_ON_COURSE_LIST_SELECTION:
				sndCursorMove->play();
				menuCourse.setSelectedIndexByLocation(x, y);
				break;

			case FOCUS_ON_SETTINGS_LIST_HOVER:
				sndCursorIn->play();
				focus = FOCUS_ON_SETTINGS_LIST_SELECTION;
				break;

			case FOCUS_ON_SETTINGS_LIST_SELECTION:
				if(menuSettings.bounds.contains(x, y))
				{
					sndCursorMove->play();
					menuSettings.setSelectedIndexByLocation(x, y);
				}
				else
				{
					sndCursorOut->play();
					focus = FOCUS_ON_SETTINGS_LIST_HOVER;
				}
				break;

			case FOCUS_ON_COURSE_EDITOR_PORTRAIT:
				sndCursorIn->play();
				game.enterState(CarseGame::COURSE_EDITOR_STATE_ID);
				break;
		}
	}
}

void CourseSelectionState::onMouseMoved(int oldx, int oldy, int x, int y)
{
	if(focus != FOCUS_ON_SETTINGS_LIST_SELECTION)
	{
		const ScreenFocus oldStatus = focus;
		if(menuCourse.bounds.contains(x, y))
			focus = FOCUS_ON_COURSE_LIST_SELECTION;

		if(menuSettings.bounds.contains(x, y))
			focus = FOCUS_ON_SETTINGS_LIST_HOVER;

		if(courseEditorPortraitBounds.contains(x, y))
			focus = FOCUS_ON_COURSE_EDITOR_PORTRAIT;

		if(focus != oldStatus)
			sndCursorMove->play();
	}
}

Image* CourseSelectionState::getSelectedCoursePreview()
{
	return isLoadedCourseSelected or isDebugCourseSelected? imgCircuit : imgRandom;  // todo choose correct portrait based on course specification
}
