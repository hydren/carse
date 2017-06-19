/*
 * course_selection_state.cpp
 *
 *  Created on: 23 de mai de 2017
 *      Author: carlosfaruolo
 */

#include "course_selection_state.hpp"

#include "race_state.hpp"
#include "futil/properties.hpp"

#include "futil/string/actions.hpp"
#include "futil/string/more_operators.hpp"

#include <iostream>
#include <vector>

using fgeal::Display;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Keyboard;
using fgeal::Font;
using fgeal::Color;
using fgeal::Image;
using fgeal::Rectangle;
using fgeal::Menu;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using futil::Properties;

int CourseSelectionState::getId() { return Pseudo3DCarseGame::COURSE_SELECTION_STATE_ID; }

CourseSelectionState::CourseSelectionState(Pseudo3DCarseGame* game)
: State(*game),
  fontMain(null), fontInfo(null),
  menu(null)
{}

CourseSelectionState::~CourseSelectionState()
{
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(menu != null) delete menu;
}

void CourseSelectionState::initialize()
{
	Display& display = Display::getInstance();
	Rectangle menuBounds = {0.0625f*display.getWidth(), 0.25f*display.getHeight(), 0.4f*display.getWidth(), 0.5f*display.getHeight()};
	fontMain = new Font("assets/font.ttf", 24);
	fontInfo = new Font("assets/font.ttf", 12);

	menu = new Menu(menuBounds, new Font("assets/font2.ttf", 18), Color::WHITE);
	menu->manageFontDeletion = true;
	menu->bgColor = Color::GREEN;
	menu->selectedColor = Color::DARK_GREEN;

	cout << "reading vehicles..." << endl;

	vector<string> courseFiles = fgeal::getFilenamesWithinDirectory("data/courses");
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
	menu->draw();
	fontMain->drawText("Choose a course", 84, 25, Color::WHITE);

	Course& course = courses[menu->getSelectedIndex()];
	fontInfo->drawText(string("Length: ")+(course.lines.size()*course.roadSegmentLength*0.001) + "Km", 0.525*display.getWidth(), 0.525*display.getHeight(), Color::WHITE);
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
		if(event.getEventType() == Event::Type::DISPLAY_CLOSURE)
		{
			game.running = false;
		}
		else if(event.getEventType() == Event::Type::KEY_PRESS)
		{
			switch(event.getEventKeyCode())
			{
				case Keyboard::Key::ESCAPE:
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::Key::ENTER:
					this->onMenuSelect();
					break;
				case Keyboard::Key::ARROW_UP:
					menu->cursorUp();
					break;
				case Keyboard::Key::ARROW_DOWN:
					menu->cursorDown();
					break;
				default:
					break;
			}
		}
	}
}

void CourseSelectionState::onMenuSelect()
{
	Pseudo3DRaceState* raceState = static_cast<Pseudo3DRaceState*>(game.getState(CarseGame::RACE_STATE_ID));
	raceState->setCourse(courses[menu->getSelectedIndex()]);
	game.enterState(CarseGame::VEHICLE_SELECTION_STATE_ID);
}
