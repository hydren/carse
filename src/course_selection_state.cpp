/*
 * course_selection_state.cpp
 *
 *  Created on: 23 de mai de 2017
 *      Author: carlosfaruolo
 */

#include "course_selection_state.hpp"

#include "futil/string_actions.hpp"

#include "util.hpp"

#include "race_state.hpp"

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

int CourseSelectionState::getId() { return CarseGame::COURSE_SELECTION_STATE_ID; }

CourseSelectionState::CourseSelectionState(CarseGame* game)
: State(*game), game(*game),
  background(null), imgRandom(null), imgCircuit(null),
  fontMain(null), fontInfo(null),
  menuCourse(null), menuSettings(null),
  sndCursorMove(null), sndCursorIn(null), sndCursorOut(null),
  isLoadedCourseSelected(false), isDebugCourseSelected(false),
  status(STATUS_HOVERING_COURSE_LIST)
{}

CourseSelectionState::~CourseSelectionState()
{
	if(background != null) delete background;
	if(imgRandom != null) delete imgRandom;
	if(imgCircuit != null) delete imgCircuit;
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(menuCourse != null) delete menuCourse;
	if(menuSettings != null) delete menuSettings;
}

void CourseSelectionState::initialize()
{
	Display& display = game.getDisplay();

	background = new Image("assets/course-menu-bg.jpg");
	imgRandom = new Image("assets/portrait-random.png");
	imgCircuit = new Image("assets/portrait-circuit.png");

	fontMain = new Font(game.sharedResources->font2Path, dip(32));
	fontInfo = new Font(game.sharedResources->font1Path, dip(14));

	menuCourse = new Menu();
	menuCourse->setFont(new Font(game.sharedResources->font1Path, dip(12)), false);
	menuCourse->setColor(Color::RED);
	menuCourse->bgColor = Color(0, 0, 0, 128);
	menuCourse->borderColor = Color(0, 0, 0, 192);
	menuCourse->focusedEntryFontColor = Color::WHITE;

	menuCourse->addEntry("<Random course>");
	menuCourse->addEntry("<Debug course>");
	const vector<Pseudo3DCourse::Spec>& courses = game.logic.getCourseList();
	for(unsigned i = 0; i < courses.size(); i++)
		menuCourse->addEntry((string) courses[i]);

	menuSettings = new Menu();
	menuSettings->setFont(new Font(game.sharedResources->font1Path, dip(12)), false);
	menuSettings->setColor(Color::RED);
	menuSettings->bgColor = menuCourse->bgColor;
	menuSettings->borderColor = menuCourse->borderColor;
	menuSettings->focusedEntryFontColor = menuCourse->focusedEntryFontColor;
	menuSettings->addEntry("Race type: " + Pseudo3DRaceState::toString(game.logic.getNextRaceSettings().raceType));
	menuSettings->addEntry("Laps: " + to_string(game.logic.getNextRaceSettings().lapCountGoal));
	menuSettings->cursorWrapAroundEnabled = false;

	// loan some shared resources
	sndCursorMove = &game.sharedResources->sndCursorMove;
	sndCursorIn   = &game.sharedResources->sndCursorIn;
	sndCursorOut  = &game.sharedResources->sndCursorOut;
}

void CourseSelectionState::onEnter()
{
	status = STATUS_HOVERING_COURSE_LIST;
}

void CourseSelectionState::onLeave()
{}

void CourseSelectionState::render()
{
	Display& display = game.getDisplay();
	display.clear();

	background->drawScaled(0, 0, scaledToSize(background, display));

	const float displayWidth = display.getWidth(),
				displayHeight = display.getHeight(),
				focusSpacing = display.getHeight()*0.01;

	const string txtChooseCourse = "Choose a course";
	const Rectangle titleBounds = {0.5f*(displayWidth - fontMain->getTextWidth(txtChooseCourse)), (1/32.f)*displayHeight,
								   fontMain->getTextWidth(txtChooseCourse), fontMain->getHeight()};
	fontMain->drawText(txtChooseCourse, titleBounds.x, titleBounds.y, Color::WHITE);

	const Rectangle paneBounds = {(1/64.f)*displayWidth, titleBounds.y + titleBounds.h + (1/64.f)*displayHeight,
								 (62/64.f)*displayWidth, displayHeight - titleBounds.h - titleBounds.y - (2/64.f)*displayHeight};

	// draw panel bg
	fgeal::Graphics::drawFilledRectangle(paneBounds, Color(0,0,0, 96));

	const Rectangle portraitBounds = {
			paneBounds.x + (1/32.f)*paneBounds.w,
			paneBounds.y + (1/32.f)*paneBounds.h,
			(1/3.f)*paneBounds.h,
			(1/3.f)*paneBounds.h
	};
	// portrait frame
	fgeal::Graphics::drawFilledRectangle(portraitBounds, Color::DARK_GREY);

	const Rectangle portraitImgBounds = {
			portraitBounds.x + portraitBounds.w*0.02f,
			portraitBounds.y + portraitBounds.h*0.02f,
			portraitBounds.w * 0.96f,
			portraitBounds.h * 0.96f
	};

	// draw portrait
	if(menuCourse->getSelectedIndex() == 0)
		imgRandom->drawScaled(portraitImgBounds.x, portraitImgBounds.y, scaledToRect(imgRandom, portraitImgBounds));
	else
		// todo choose correct portrait based on course specification
		imgCircuit->drawScaled(portraitImgBounds.x, portraitImgBounds.y, scaledToRect(imgCircuit, portraitImgBounds));

	// update menu bounds
	menuCourse->bounds.x = portraitBounds.x + portraitBounds.w + (1/32.f)*paneBounds.w;
	menuCourse->bounds.y = portraitBounds.y;
	menuCourse->bounds.w = paneBounds.w - menuCourse->bounds.x;
	menuCourse->bounds.h = portraitBounds.h;

	menuCourse->draw();

	// draw info
	fontInfo->drawText(menuCourse->getSelectedEntry().label, portraitBounds.x, 1.1*(portraitBounds.y + portraitBounds.h), Color::WHITE);
	if(menuCourse->getSelectedIndex() > 1)
	{
		const Pseudo3DCourse::Spec& course = game.logic.getCourseList()[menuCourse->getSelectedIndex() - 2];
		const float courseLength = course.lines.size()*course.roadSegmentLength*0.001;
		const string txtLength = "Length: " + futil::to_string(courseLength) + "Km";
		fontInfo->drawText(txtLength, portraitBounds.x, 1.1*(portraitBounds.y + portraitBounds.h) + fontInfo->getHeight(), Color::WHITE);
	}

	// draw race settings
	menuSettings->bounds.x = menuCourse->bounds.x;
	menuSettings->bounds.y = menuCourse->bounds.y + menuCourse->bounds.h + 4*focusSpacing;
	menuSettings->bounds.w = menuCourse->bounds.w;
	menuSettings->bounds.h = (4/7.f)*paneBounds.h;
	menuSettings->draw();

	if(cos(20*fgeal::uptime()) > 0 or status == STATUS_ON_COURSE_LIST_SELECTION)
	{
		const Rectangle courseListFocusArea = {
			menuCourse->bounds.x - focusSpacing,
			menuCourse->bounds.y - focusSpacing,
			menuCourse->bounds.w + 2*focusSpacing,
			menuCourse->bounds.h + 2*focusSpacing
		},
		settingsFocusArea = {
			courseListFocusArea.x,
			courseListFocusArea.y + courseListFocusArea.h + focusSpacing,
			courseListFocusArea.w,
			menuSettings->bounds.h + 3*focusSpacing,
		};

		switch(status)
		{
			case STATUS_ON_COURSE_LIST_SELECTION:
			case STATUS_HOVERING_COURSE_LIST: fgeal::Graphics::drawRectangle(courseListFocusArea, Color::RED); break;
			case STATUS_HOVERING_SETTINGS_LIST: fgeal::Graphics::drawRectangle(settingsFocusArea, Color::RED); break;
			default:break;
		}
	}
}

void CourseSelectionState::update(float delta) {}

void CourseSelectionState::updateLapCount()
{
	if(menuSettings->at(1).enabled)
		menuSettings->at(1).label = "Laps: " + to_string(game.logic.getNextRaceSettings().lapCountGoal);
	else
		menuSettings->at(1).label = "Laps: --";
}

void CourseSelectionState::onKeyPressed(Keyboard::Key key)
{
	if(key == Keyboard::KEY_ESCAPE and status != STATUS_ON_COURSE_LIST_SELECTION)
	{
		sndCursorOut->stop();
		sndCursorOut->play();
		if(isLoadedCourseSelected)
			game.logic.setNextCourse(menuCourse->getSelectedIndex()-2);
		else if(isDebugCourseSelected)
			game.logic.setNextCourseDebug();
		else
			game.logic.setNextCourseRandom();

		game.enterState(game.logic.getCurrentMainMenuStateId());
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
			else if(key == Keyboard::KEY_SPACE)
				game.enterState(CarseGame::COURSE_EDITOR_STATE_ID);  //xxx to debug editor state
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
			menuCourse->moveCursorUp();
			break;
		case Keyboard::KEY_ARROW_DOWN:
			sndCursorMove->stop();
			sndCursorMove->play();
			menuCourse->moveCursorDown();
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

	isDebugCourseSelected = (menuCourse->getSelectedIndex() == 1);
	isLoadedCourseSelected = (menuCourse->getSelectedIndex() > 1);
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
			switch(menuSettings->getSelectedIndex())
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
					menuSettings->at(SETTINGS_RACE_TYPE).label = "Race type: " + Pseudo3DRaceState::toString(game.logic.getNextRaceSettings().raceType);
					menuSettings->at(SETTINGS_LAPS).enabled = Pseudo3DRaceState::isRaceTypeLoop(game.logic.getNextRaceSettings().raceType);
					updateLapCount();
					break;
				}
				case SETTINGS_LAPS:  // laps
				{
					// if not a loop type race, do nothing
					if(not menuSettings->at(SETTINGS_LAPS).enabled)
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
			if(menuSettings->getSelectedIndex() == 0)
				status = STATUS_HOVERING_COURSE_LIST;
			else
				menuSettings->moveCursorUp();
			break;
		case Keyboard::KEY_ARROW_DOWN:
			sndCursorMove->stop();
			sndCursorMove->play();
			menuSettings->moveCursorDown();
			break;
		default:
			break;
	}
}

Image* CourseSelectionState::getSelectedCoursePreview()
{
	return isLoadedCourseSelected or isDebugCourseSelected? imgCircuit : imgRandom;  // todo choose correct portrait based on course specification
}
