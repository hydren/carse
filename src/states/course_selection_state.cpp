/*
 * course_selection_state.cpp
 *
 *  Created on: 23 de mai de 2017
 *      Author: carlosfaruolo
 */

#include "course_selection_state.hpp"

#include "race_state.hpp"
#include "futil/properties.hpp"

#include "futil/string_actions.hpp"
#include "futil/string_extra_operators.hpp"

#include <iostream>
#include <vector>

using fgeal::Display;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Font;
using fgeal::Color;
using fgeal::Image;
using fgeal::Sound;
using fgeal::Rectangle;
using fgeal::Menu;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using futil::Properties;
using futil::ends_with;

int CourseSelectionState::getId() { return Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID; }

CourseSelectionState::CourseSelectionState(Pseudo3DCarseGame* game)
: State(*game),
  imgRandom(null), imgCircuit(null),
  fontMain(null), fontInfo(null), fontTab(null),
  menu(null), sndCursorMove(null), sndCursorAccept(null), sndCursorOut(null),
  isLoadedCourseSelected(true), isDebugCourseSelected(false)
{}

CourseSelectionState::~CourseSelectionState()
{
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(fontTab  != null) delete fontTab;
	if(menu != null) delete menu;
	if(sndCursorMove != null) delete sndCursorMove;
	if(sndCursorAccept != null) delete sndCursorAccept;
	if(sndCursorOut != null) delete sndCursorOut;
}

void CourseSelectionState::initialize()
{
	imgRandom = new Image("assets/portrait-random.png");
	imgCircuit = new Image("assets/portrait-circuit.png");

	fontMain = new Font("assets/font2.ttf", 32);
	fontInfo = new Font("assets/font.ttf", 12);
	fontTab  = new Font("assets/font2.ttf", 16);

	sndCursorMove = new Sound("assets/sound/cursor_move.ogg");
	sndCursorAccept = new Sound("assets/sound/cursor_accept.ogg");
	sndCursorOut = new Sound("assets/sound/cursor_out.ogg");

	menu = new Menu(Rectangle(), new Font("assets/font.ttf", 12), Color::RED);
	menu->fontIsOwned = true;
	menu->bgColor = Color::GREEN;
	menu->focusedEntryFontColor = Color::WHITE;

	cout << "reading courses..." << endl;

	vector<string> courseFiles = fgeal::filesystem::getFilenamesWithinDirectory("data/courses");
	for(unsigned i = 0; i < courseFiles.size(); i++)
	{
		if(ends_with(courseFiles[i], ".properties"))
		{
			Properties prop;
			prop.load(courseFiles[i]);
			courses.push_back(Course::createCourseFromFile(prop));
			cout << "read course: " << courseFiles[i] << endl;
			menu->addEntry(courseFiles[i]);
		}
	}
}

void CourseSelectionState::onEnter()
{}

void CourseSelectionState::onLeave()
{}

void CourseSelectionState::render()
{
	Display& display = Display::getInstance();
	display.clear();

	const float displayWidth = display.getWidth(),
				displayHeight = display.getHeight(),
				xspacing = 0.01f*displayWidth,
				yspacing = 0.01f*displayHeight;;

	// update menu bounds
	{
		const Rectangle updatedBounds = {0.0625f*displayWidth, 0.25f*displayHeight, 0.4f*displayWidth, 0.5f*displayHeight};
		menu->bounds = updatedBounds;
	}

	const Rectangle tabSettingsBounds = {
			menu->bounds.x,
			menu->bounds.y - 0.1f*displayHeight,
			0.4f*menu->bounds.w,
			0.1f*displayHeight
	};
	const Rectangle tabLoadedCoursesBounds = {
			menu->bounds.x + xspacing + tabSettingsBounds.w,
			tabSettingsBounds.y,
			tabSettingsBounds.w,
			tabSettingsBounds.h
	};

	Image::drawFilledRectangle(tabSettingsBounds, not isLoadedCourseSelected? Color::GREEN : Color::DARK_GREEN);
	fontTab->drawText("Custom",tabSettingsBounds.x + xspacing, tabSettingsBounds.y + yspacing, not isLoadedCourseSelected? Color::BLACK : Color::GREEN);
	Image::drawFilledRectangle(tabLoadedCoursesBounds, isLoadedCourseSelected? Color::GREEN : Color::DARK_GREEN);
	fontTab->drawText("Courses",tabLoadedCoursesBounds.x + xspacing, tabLoadedCoursesBounds.y + yspacing, isLoadedCourseSelected? Color::BLACK : Color::GREEN);

	const Rectangle portraitBounds = {
			menu->bounds.x + menu->bounds.w + 32,
			1.1f *menu->bounds.y,
			0.75f*menu->bounds.h,
			0.75f*menu->bounds.h
	};
	Image::drawFilledRectangle(portraitBounds, Color::DARK_GREY);  // portrait frame

	const Rectangle portraitImgBounds = {
			portraitBounds.x + portraitBounds.w*0.02f,
			portraitBounds.y + portraitBounds.h*0.02f,
			portraitBounds.w * 0.96f,
			portraitBounds.h * 0.96f
	};

	if(isLoadedCourseSelected)
	{
		menu->draw();
		imgCircuit->drawScaled(portraitImgBounds.x, portraitImgBounds.y, portraitImgBounds.h/imgCircuit->getWidth(), portraitImgBounds.h/imgCircuit->getHeight());
		Course& course = courses[menu->getSelectedIndex()];
		fontInfo->drawText(string("Length: ")+(course.lines.size()*course.roadSegmentLength*0.001) + "Km", menu->bounds.x + menu->bounds.w + 32, menu->bounds.y + menu->bounds.h, Color::WHITE);
	}
	else
	{
		Image::drawFilledRectangle(menu->bounds, isDebugCourseSelected? Color::GREY : menu->bgColor);
		Image::drawRectangle(menu->bounds, Color::RED);
		if(isDebugCourseSelected)
		{
			fontMain->drawText("Debug course", menu->bounds.x * 1.1f, menu->bounds.y * 1.1f, isDebugCourseSelected? Color::LIGHT_GREY : Color::BLACK);
			imgCircuit->drawScaled(portraitImgBounds.x, portraitImgBounds.y, portraitImgBounds.h/imgCircuit->getWidth(), portraitImgBounds.h/imgCircuit->getHeight());
		}
		else
		{
			fontMain->drawText("Random course", menu->bounds.x * 1.1f, menu->bounds.y * 1.1f, isDebugCourseSelected? Color::BLACK : Color::RED);
			imgRandom->drawScaled(portraitImgBounds.x, portraitImgBounds.y, portraitImgBounds.h/imgCircuit->getWidth(), portraitImgBounds.h/imgCircuit->getHeight());
		}
	}

	fontMain->drawText("Choose a course", menu->bounds.x, 0.025f*displayHeight, Color::WHITE);
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
			switch(event.getEventKeyCode())
			{
				case Keyboard::KEY_ESCAPE:
					sndCursorOut->play();
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::KEY_ENTER:
					sndCursorAccept->play();
					this->onMenuSelect();
					break;
				case Keyboard::KEY_ARROW_UP:
					sndCursorMove->play();
					if(isLoadedCourseSelected)
						menu->cursorUp();
					else
						isDebugCourseSelected = !isDebugCourseSelected;
					break;
				case Keyboard::KEY_ARROW_DOWN:
					sndCursorMove->play();
					if(isLoadedCourseSelected)
						menu->cursorDown();
					else
						isDebugCourseSelected = !isDebugCourseSelected;
					break;
				case Keyboard::KEY_ARROW_LEFT:
					sndCursorMove->play();
					isLoadedCourseSelected = !isLoadedCourseSelected;
					break;
				case Keyboard::KEY_ARROW_RIGHT:
					sndCursorMove->play();
					isLoadedCourseSelected = !isLoadedCourseSelected;
					break;
				default:
					break;
			}
		}
	}
}

void CourseSelectionState::onMenuSelect()
{
	if(isLoadedCourseSelected)
		Pseudo3DRaceState::getInstance(game)->setCourse(courses[menu->getSelectedIndex()]);
	else
		if(isDebugCourseSelected)
			Pseudo3DRaceState::getInstance(game)->setCourse(Course::createDebugCourse(200, 3000));
		else
			Pseudo3DRaceState::getInstance(game)->setCourse(Course::createRandomCourse(200, 3000, 6400, 1.5));

	game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
}
