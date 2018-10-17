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
using fgeal::Font;
using fgeal::Color;
using fgeal::Image;
using fgeal::Sound;
using fgeal::Rectangle;
using fgeal::Point;
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
  background(null), imgRandom(null), imgCircuit(null), imgCourseEditor(null),
  fontMain(null), fontInfo(null),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  isLoadedCourseSelected(false), isDebugCourseSelected(false),
  status(STATUS_HOVERING_COURSE_LIST)
{}

CourseSelectionState::~CourseSelectionState()
{
	if(background != null) delete background;
	if(imgRandom != null) delete imgRandom;
	if(imgCircuit != null) delete imgCircuit;
	if(imgCourseEditor != null) delete imgCourseEditor;
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
}

void CourseSelectionState::initialize()
{
	Display& display = game.getDisplay();

	background = new Image("assets/course-menu-bg.jpg");
	imgRandom = new Image("assets/portrait-random.png");
	imgCircuit = new Image("assets/portrait-circuit.png");
	imgCourseEditor = new Image("assets/portrait-course-editor.png");

	fontMain = new Font(game.sharedResources->font2Path, dip(32));
	fontInfo = new Font(game.sharedResources->font1Path, dip(14));

	menuCourse.setFont(new Font(game.sharedResources->font1Path, dip(12)), false);
	menuCourse.setColor(Color::RED);
	menuCourse.bgColor = Color(0, 0, 0, 128);
	menuCourse.borderColor = Color(0, 0, 0, 192);
	menuCourse.focusedEntryFontColor = Color::WHITE;

	menuCourse.addEntry("<Random course>");
	menuCourse.addEntry("<Debug course>");
	const vector<Pseudo3DCourse::Spec>& courses = game.logic.getCourseList();
	for(unsigned i = 0; i < courses.size(); i++)
		menuCourse.addEntry((string) courses[i]);

	menuSettings.setFont(new Font(game.sharedResources->font1Path, dip(12)), false);
	menuSettings.setColor(Color::RED);
	menuSettings.bgColor = menuCourse.bgColor;
	menuSettings.borderColor = menuCourse.borderColor;
	menuSettings.focusedEntryFontColor = menuCourse.focusedEntryFontColor;
	menuSettings.addEntry("Race type: " + Pseudo3DRaceState::toString(game.logic.getNextRaceSettings().raceType));
	menuSettings.addEntry("Laps: " + to_string(game.logic.getNextRaceSettings().lapCountGoal));
	menuSettings.cursorWrapAroundEnabled = false;

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;
}

void CourseSelectionState::onEnter()
{
	const float displayWidth = game.getDisplay().getWidth(),
				displayHeight = game.getDisplay().getHeight(),
				focusSpacing = displayHeight*0.01;

	titleBounds.x = 0.5f*(displayWidth - fontMain->getTextWidth(TITLE_TEXT));
	titleBounds.y = (1/32.f)*displayHeight;
	titleBounds.w = fontMain->getTextWidth(TITLE_TEXT);
	titleBounds.h = fontMain->getHeight();

	paneBounds.x = (1/64.f)*displayWidth;
	paneBounds.y = titleBounds.y + titleBounds.h + (1/64.f)*displayHeight;
	paneBounds.w = (62/64.f)*displayWidth;
	paneBounds.h = displayHeight - titleBounds.h - titleBounds.y - (2/64.f)*displayHeight;

	portraitBounds.x = paneBounds.x + (1/32.f)*paneBounds.w;
	portraitBounds.y = paneBounds.y + (1/32.f)*paneBounds.h;
	portraitBounds.w = (1/4.f)*paneBounds.h;
	portraitBounds.h = (1/4.f)*paneBounds.h;

	portraitImgBounds.x = portraitBounds.x + portraitBounds.w*0.02f;
	portraitImgBounds.y = portraitBounds.y + portraitBounds.h*0.02f;
	portraitImgBounds.w = portraitBounds.w * 0.96f;
	portraitImgBounds.h = portraitBounds.h * 0.96f;

	courseEditorPortraitBounds = portraitImgBounds;
	courseEditorPortraitBounds.x = paneBounds.x + paneBounds.w - portraitImgBounds.w - 2*focusSpacing;

	menuCourse.bounds.x = portraitBounds.x;
	menuCourse.bounds.y = portraitBounds.y + portraitBounds.h + (1/32.f)*paneBounds.h;
	menuCourse.bounds.w = paneBounds.w - menuCourse.bounds.x;
	menuCourse.bounds.h = (paneBounds.w - portraitBounds.h)/4;

	menuSettings.bounds.x = menuCourse.bounds.x;
	menuSettings.bounds.y = menuCourse.bounds.y + menuCourse.bounds.h + 4*focusSpacing;
	menuSettings.bounds.w = menuCourse.bounds.w;
	menuSettings.bounds.h = (paneBounds.w - portraitBounds.h)/4;

	status = STATUS_HOVERING_COURSE_LIST;
}

void CourseSelectionState::onLeave()
{}

void CourseSelectionState::render()
{
	Display& display = game.getDisplay();
	display.clear();

	const float focusSpacing = display.getHeight()*0.01;

	background->drawScaled(0, 0, scaledToSize(background, display));

	fontMain->drawText(TITLE_TEXT, titleBounds.x, titleBounds.y, Color::WHITE);

	// draw panel bg
	fgeal::Graphics::drawFilledRectangle(paneBounds, Color(0,0,0, 96));

	// portrait frame
	fgeal::Graphics::drawFilledRectangle(portraitBounds, Color::DARK_GREY);

	// draw portrait
	if(menuCourse.getSelectedIndex() == 0)
		imgRandom->drawScaled(portraitImgBounds.x, portraitImgBounds.y, scaledToRect(imgRandom, portraitImgBounds));
	else
		// todo choose correct portrait based on course specification
		imgCircuit->drawScaled(portraitImgBounds.x, portraitImgBounds.y, scaledToRect(imgCircuit, portraitImgBounds));

	// draw course editor portrait
	imgCourseEditor->drawScaled(courseEditorPortraitBounds.x, courseEditorPortraitBounds.y, scaledToRect(imgCourseEditor, courseEditorPortraitBounds));
	fontInfo->drawText("Course editor", courseEditorPortraitBounds.x, courseEditorPortraitBounds.y, Color::WHITE);
	if(status == STATUS_HOVERING_COURSE_EDITOR_PORTRAIT and cos(20*fgeal::uptime()) > 0)
		fgeal::Graphics::drawRectangle(getSpacedOutline(courseEditorPortraitBounds, focusSpacing), Color::RED);

	// draw course list
	menuCourse.draw();
	if(status == STATUS_ON_COURSE_LIST_SELECTION or (status == STATUS_HOVERING_COURSE_LIST and cos(20*fgeal::uptime()) > 0))
		fgeal::Graphics::drawRectangle(getSpacedOutline(menuCourse.bounds, focusSpacing), Color::RED);

	// draw info
	fontInfo->drawText(menuCourse.getSelectedEntry().label, 1.1*(portraitBounds.x + portraitBounds.w), portraitBounds.y, Color::WHITE);
	if(menuCourse.getSelectedIndex() > 1)
	{
		const Pseudo3DCourse::Spec& course = game.logic.getCourseList()[menuCourse.getSelectedIndex() - 2];
		const float courseLength = course.lines.size()*course.roadSegmentLength*0.001;
		const string txtLength = "Length: " + futil::to_string(courseLength) + "Km";
		fontInfo->drawText(txtLength, 1.1*(portraitBounds.x + portraitBounds.w), portraitBounds.y + fontInfo->getHeight(), Color::WHITE);
	}

	// draw race settings
	menuSettings.draw();
	if(status == STATUS_HOVERING_SETTINGS_LIST and cos(20*fgeal::uptime()) > 0)
		fgeal::Graphics::drawRectangle(getSpacedOutline(menuSettings.bounds, focusSpacing), Color::RED);
}

void CourseSelectionState::update(float delta) {}

void CourseSelectionState::updateLapCount()
{
	if(menuSettings.getEntryAt(1).enabled)
		menuSettings.getEntryAt(1).label = "Laps: " + to_string(game.logic.getNextRaceSettings().lapCountGoal);
	else
		menuSettings.getEntryAt(1).label = "Laps: --";
}

void CourseSelectionState::onKeyPressed(Keyboard::Key key)
{
	if(key == Keyboard::KEY_ESCAPE and status != STATUS_ON_COURSE_LIST_SELECTION)
	{
		sndCursorOut->stop();
		sndCursorOut->play();
		if(isLoadedCourseSelected)
			game.logic.setNextCourse(menuCourse.getSelectedIndex()-2);
		else if(isDebugCourseSelected)
			game.logic.setNextCourseDebug();
		else
			game.logic.setNextCourseRandom();

		game.enterState(game.logic.currentMainMenuStateId);
	}
	else switch(status)
	{
		case STATUS_HOVERING_COURSE_LIST:
			if(key == Keyboard::KEY_ENTER)
			{
				sndCursorIn->stop();
				sndCursorIn->play();
				status = STATUS_ON_COURSE_LIST_SELECTION;
			}
			else if(key == Keyboard::KEY_ARROW_DOWN)
			{
				sndCursorMove->stop();
				sndCursorMove->play();
				status = STATUS_HOVERING_SETTINGS_LIST;
			}
			else if(key == Keyboard::KEY_ARROW_UP)
			{
				sndCursorMove->stop();
				sndCursorMove->play();
				status = STATUS_HOVERING_COURSE_EDITOR_PORTRAIT;
			}
			break;

		case STATUS_HOVERING_COURSE_EDITOR_PORTRAIT:
			if(key == Keyboard::KEY_ENTER)
				game.enterState(CarseGame::COURSE_EDITOR_STATE_ID);
			else if(key == Keyboard::KEY_ARROW_DOWN)
			{
				sndCursorMove->stop();
				sndCursorMove->play();
				status = STATUS_HOVERING_COURSE_LIST;
			}
			break;

		case STATUS_HOVERING_SETTINGS_LIST: handleInputOnSettings(key); break;
		case STATUS_ON_COURSE_LIST_SELECTION: handleInputOnCourseList(key); break;
	}
}

void CourseSelectionState::handleInputOnCourseList(fgeal::Keyboard::Key key)
{
	switch(key)
	{
		case Keyboard::KEY_ARROW_UP:
			sndCursorMove->stop();
			sndCursorMove->play();
			menuCourse.moveCursorUp();
			break;
		case Keyboard::KEY_ARROW_DOWN:
			sndCursorMove->stop();
			sndCursorMove->play();
			menuCourse.moveCursorDown();
			break;
		case Keyboard::KEY_ENTER:
		case Keyboard::KEY_ESCAPE:
			sndCursorIn->stop();
			sndCursorIn->play();
			status = STATUS_HOVERING_COURSE_LIST;
			break;
		default:
			break;
	}

	isDebugCourseSelected = (menuCourse.getSelectedIndex() == 1);
	isLoadedCourseSelected = (menuCourse.getSelectedIndex() > 1);
}

void CourseSelectionState::handleInputOnSettings(fgeal::Keyboard::Key key)
{
	switch(key)
	{
		case Keyboard::KEY_ARROW_LEFT:
		case Keyboard::KEY_ARROW_RIGHT:
		{
			const bool isCursorLeft = (key == Keyboard::KEY_ARROW_LEFT);
			sndCursorMove->stop();
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
			break;
		}
		case Keyboard::KEY_ARROW_UP:
			sndCursorMove->stop();
			sndCursorMove->play();
			if(menuSettings.getSelectedIndex() == 0)
				status = STATUS_HOVERING_COURSE_LIST;
			else
				menuSettings.moveCursorUp();
			break;
		case Keyboard::KEY_ARROW_DOWN:
			sndCursorMove->stop();
			sndCursorMove->play();
			menuSettings.moveCursorDown();
			break;
		default:
			break;
	}
}

Image* CourseSelectionState::getSelectedCoursePreview()
{
	return isLoadedCourseSelected or isDebugCourseSelected? imgCircuit : imgRandom;  // todo choose correct portrait based on course specification
}
