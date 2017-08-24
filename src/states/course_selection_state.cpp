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
  fontMain(null), fontInfo(null),
  menu(null), sndCursorMove(null), sndCursorAccept(null), sndCursorOut(null),
  isLoadedCourseSelected(true), isDebugCourseSelected(false)
{}

CourseSelectionState::~CourseSelectionState()
{
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(menu != null) delete menu;
	if(sndCursorMove != null) delete sndCursorMove;
	if(sndCursorAccept != null) delete sndCursorAccept;
	if(sndCursorOut != null) delete sndCursorOut;
}

void CourseSelectionState::initialize()
{
	fontMain = new Font("assets/font2.ttf", 32);
	fontInfo = new Font("assets/font.ttf", 12);

	sndCursorMove = new Sound("assets/sound/cursor_move.ogg");
	sndCursorAccept = new Sound("assets/sound/cursor_accept.ogg");
	sndCursorOut = new Sound("assets/sound/cursor_out.ogg");

	menu = new Menu(Rectangle(), new Font("assets/font.ttf", 12), Color::DARK_GREEN, "Courses:");
	menu->fontIsOwned = true;
	menu->bgColor = Color::GREEN;
	menu->focusedEntryFontColor = Color::WHITE;
	menu->titleColor = Color::RED;

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
				displayHeight = display.getHeight();

	// update menu bounds
	{
		const Rectangle updatedBounds =
		{
				0.0625f * displayWidth,
				0.25f * displayHeight,
				0.4f * displayWidth,
				0.5f * displayHeight
		};
		menu->bounds = updatedBounds;
	}

	if(isLoadedCourseSelected)
	{
		menu->draw();
		Course& course = courses[menu->getSelectedIndex()];
		fontInfo->drawText(string("Length: ")+(course.lines.size()*course.roadSegmentLength*0.001) + "Km", menu->bounds.x + menu->bounds.w + 32, menu->bounds.y + menu->bounds.h * 0.5f, Color::WHITE);
	}
	else if(isDebugCourseSelected)
	{
		Image::drawFilledRectangle(menu->bounds.x, menu->bounds.y, menu->bounds.w, menu->bounds.h, Color::GREY);
		fontMain->drawText("Debug course", menu->bounds.x * 1.1f, menu->bounds.y * 1.1f, Color::LIGHT_GREY);
	}
	else
	{
		Image::drawFilledRectangle(menu->bounds.x, menu->bounds.y, menu->bounds.w, menu->bounds.h, menu->bgColor);
		fontMain->drawText("Random course", menu->bounds.x * 1.1f, menu->bounds.y * 1.1f, Color::RED);
	}

	fontMain->drawText("Choose a course", 84, 25, Color::WHITE);
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
					menu->cursorUp();
					break;
				case Keyboard::KEY_ARROW_DOWN:
					sndCursorMove->play();
					menu->cursorDown();
					break;
				case Keyboard::KEY_ARROW_LEFT:
					sndCursorMove->play();
					if(isLoadedCourseSelected)
						isLoadedCourseSelected = false;
					else
					{
						if(isDebugCourseSelected)
						{
							isLoadedCourseSelected = true;
							isDebugCourseSelected = false;
						}
						else
							isDebugCourseSelected = true;
					}
					break;
				case Keyboard::KEY_ARROW_RIGHT:
					sndCursorMove->play();
					if(isLoadedCourseSelected)
					{
						isLoadedCourseSelected = false;
						isDebugCourseSelected = true;
					}
					else
					{
						if(isDebugCourseSelected)
							isDebugCourseSelected = false;
						else
						{
							isLoadedCourseSelected = true;
							isDebugCourseSelected = false;
						}
					}
					break;
				default:
					break;
			}
		}
	}
}

void CourseSelectionState::onMenuSelect()
{
	Pseudo3DRaceState::getInstance(game)->setCourse(courses[menu->getSelectedIndex()]);
	game.enterState(Pseudo3DCarseGame::VEHICLE_SELECTION_STATE_ID);
}
