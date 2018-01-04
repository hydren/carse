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

int CourseSelectionState::getId() { return Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID; }

CourseSelectionState::CourseSelectionState(Pseudo3DCarseGame* game)
: State(*game), shared(*game->sharedResources), logic(game->logic),
  background(null), imgRandom(null), imgCircuit(null),
  fontMain(null), fontInfo(null), menuCourse(null), menuSettings(null),
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

	fontMain = new Font(shared.font2Path, dip(32));
	fontInfo = new Font(shared.font1Path, dip(14));

	menuCourse = new Menu(Rectangle(), new Font(shared.font1Path, dip(12)), Color::RED);
	menuCourse->fontIsOwned = true;
	menuCourse->bgColor = Color(0, 0, 0, 128);
	menuCourse->borderColor = Color(0, 0, 0, 192);
	menuCourse->focusedEntryFontColor = Color::WHITE;

	menuCourse->addEntry("<Random course>");
	menuCourse->addEntry("<Debug course>");
	const vector<Course>& courses = logic.getCourseList();
	for(unsigned i = 0; i < courses.size(); i++)
		menuCourse->addEntry((string) courses[i]);

	menuSettings = new Menu(Rectangle(), new Font(shared.font1Path, dip(12)), Color::RED);
	menuSettings->fontIsOwned = true;
	menuSettings->bgColor = menuCourse->bgColor;
	menuSettings->borderColor = menuCourse->borderColor;
	menuSettings->focusedEntryFontColor = menuCourse->focusedEntryFontColor;
	menuSettings->addEntry("Race type: " + to_string(logic.getNextRaceSettings().raceType));
	menuSettings->addEntry("Laps: " + to_string(logic.getNextRaceSettings().lapCountGoal));
	menuSettings->cursorWrapAroundEnabled = false;
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
	Image::drawFilledRectangle(paneBounds, Color(0,0,0, 96));

	const Rectangle portraitBounds = {
			paneBounds.x + (1/32.f)*paneBounds.w,
			paneBounds.y + (1/32.f)*paneBounds.h,
			(1/3.f)*paneBounds.h,
			(1/3.f)*paneBounds.h
	};
	// portrait frame
	Image::drawFilledRectangle(portraitBounds, Color::DARK_GREY);

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
		const Course& course = logic.getCourseList()[menuCourse->getSelectedIndex() - 2];
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
			case STATUS_HOVERING_COURSE_LIST: Image::drawRectangle(courseListFocusArea, Color::RED); break;
			case STATUS_HOVERING_SETTINGS_LIST: Image::drawRectangle(settingsFocusArea, Color::RED); break;
			default:break;
		}
	}
}

void CourseSelectionState::update(float delta)
{
	this->handleInput();
}

void CourseSelectionState::handleInput()
{
	Event event;
	EventQueue& eventQueue = EventQueue::getInstance();
	while(eventQueue.hasEvents())
	{
		eventQueue.getNextEvent(&event);
		if(event.getEventType() == Event::TYPE_DISPLAY_CLOSURE)
		{
			game.running = false;
		}
		else if(event.getEventType() == Event::TYPE_KEY_PRESS)
		{
			if(event.getEventKeyCode() == Keyboard::KEY_ESCAPE and status != STATUS_ON_COURSE_LIST_SELECTION)
			{
				shared.sndCursorOut.stop();
				shared.sndCursorOut.play();
				if(isLoadedCourseSelected)
					logic.setNextCourse(menuCourse->getSelectedIndex()-2);
				else if(isDebugCourseSelected)
					logic.setNextCourseDebug();
				else
					logic.setNextCourseRandom();

				game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
			}
			else switch(status)
			{
				case STATUS_HOVERING_COURSE_LIST:
					if(event.getEventKeyCode() == Keyboard::KEY_ENTER)
					{
						shared.sndCursorIn.stop();
						shared.sndCursorIn.play();
						status = STATUS_ON_COURSE_LIST_SELECTION;
					}
					else if(event.getEventKeyCode() == Keyboard::KEY_ARROW_DOWN)
					{
						shared.sndCursorMove.stop();
						shared.sndCursorMove.play();
						status = STATUS_HOVERING_SETTINGS_LIST;
					}
					break;

				case STATUS_HOVERING_SETTINGS_LIST: handleInputOnSettings(event); break;
				case STATUS_ON_COURSE_LIST_SELECTION: handleInputOnCourseList(event); break;
			}
		}
	}
}

void CourseSelectionState::handleInputOnCourseList(Event& event)
{
	switch(event.getEventKeyCode())
	{
		case Keyboard::KEY_ARROW_UP:
			shared.sndCursorMove.stop();
			shared.sndCursorMove.play();
			menuCourse->cursorUp();
			break;
		case Keyboard::KEY_ARROW_DOWN:
			shared.sndCursorMove.stop();
			shared.sndCursorMove.play();
			menuCourse->cursorDown();
			break;
		case Keyboard::KEY_ENTER:
		case Keyboard::KEY_ESCAPE:
			shared.sndCursorIn.stop();
			shared.sndCursorIn.play();
			status = STATUS_HOVERING_COURSE_LIST;
			break;
		default:
			break;
	}

	isDebugCourseSelected = (menuCourse->getSelectedIndex() == 1);
	isLoadedCourseSelected = (menuCourse->getSelectedIndex() > 1);
}

void CourseSelectionState::handleInputOnSettings(Event& event)
{
	switch(event.getEventKeyCode())
	{
		case Keyboard::KEY_ARROW_LEFT:
		case Keyboard::KEY_ARROW_RIGHT:
		{
			const bool isCursorLeft = (event.getEventKeyCode() == Keyboard::KEY_ARROW_LEFT);
			shared.sndCursorMove.stop();
			shared.sndCursorMove.play();
			switch(menuSettings->getSelectedIndex())
			{
				case 0:  // race type
				{
					unsigned nextType;
					if(isCursorLeft)
						if(logic.getNextRaceSettings().raceType == 0)
							nextType = Pseudo3DRaceState::RACE_TYPE_COUNT-1;
						else
							nextType = logic.getNextRaceSettings().raceType-1;
					else
						if(logic.getNextRaceSettings().raceType == Pseudo3DRaceState::RACE_TYPE_COUNT-1)
							nextType = 0;
						else
							nextType = logic.getNextRaceSettings().raceType+1;

					logic.getNextRaceSettings().raceType = static_cast<Pseudo3DRaceState::RaceType>(nextType);
					menuSettings->getSelectedEntry().label = "Race type: " + to_string(logic.getNextRaceSettings().raceType);
					break;
				}
				case 1:  // laps
				{
					if(isCursorLeft)
					{
						if(logic.getNextRaceSettings().lapCountGoal > 2)
							logic.getNextRaceSettings().lapCountGoal--;
					}
					else
						logic.getNextRaceSettings().lapCountGoal++;

					menuSettings->getSelectedEntry().label = "Laps: " + to_string(logic.getNextRaceSettings().lapCountGoal);
					break;
				}
				default: break;
			}
			break;
		}
		case Keyboard::KEY_ARROW_UP:
			shared.sndCursorMove.stop();
			shared.sndCursorMove.play();
			if(menuSettings->getSelectedIndex() == 0)
				status = STATUS_HOVERING_COURSE_LIST;
			else
				menuSettings->cursorUp();
			break;
		case Keyboard::KEY_ARROW_DOWN:
			shared.sndCursorMove.stop();
			shared.sndCursorMove.play();
			menuSettings->cursorDown();
			break;
		default:
			break;
	}
}

Image* CourseSelectionState::getSelectedCoursePreview()
{
	return isLoadedCourseSelected or isDebugCourseSelected? imgCircuit : imgRandom;  // todo choose correct portrait based on course specification
}
