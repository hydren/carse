/*
 * course_selection_state.cpp
 *
 *  Created on: 23 de mai de 2017
 *      Author: carlosfaruolo
 */

#include "course_selection_state.hpp"

#include "futil/properties.hpp"
#include "futil/string_actions.hpp"
#include "futil/string_extra_operators.hpp"

#include <iostream>
#include <vector>

#include "util.hpp"

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
: State(*game), shared(*game->sharedResources), gameLogic(game->logic),
  background(null), imgRandom(null), imgCircuit(null),
  fontMain(null), fontInfo(null), fontTab(null), menu(null),
  isLoadedCourseSelected(false), isDebugCourseSelected(false)
{}

CourseSelectionState::~CourseSelectionState()
{
	if(background != null) delete background;
	if(imgRandom != null) delete imgRandom;
	if(imgCircuit != null) delete imgCircuit;
	if(fontMain != null) delete fontMain;
	if(fontInfo != null) delete fontInfo;
	if(fontTab  != null) delete fontTab;
	if(menu != null) delete menu;
}

void CourseSelectionState::initialize()
{
	Display& display = game.getDisplay();

	background = new Image("assets/course-menu-bg.jpg");
	imgRandom = new Image("assets/portrait-random.png");
	imgCircuit = new Image("assets/portrait-circuit.png");

	fontMain = new Font("assets/font2.ttf", dip(32));
	fontInfo = new Font("assets/font.ttf",  dip(12));
	fontTab  = new Font("assets/font2.ttf", dip(16));

	menu = new Menu(Rectangle(), new Font("assets/font.ttf", dip(12)), Color::RED);
	menu->fontIsOwned = true;
	menu->bgColor = Color(0, 0, 0, 128);
	menu->borderColor = Color(0, 0, 0, 192);
	menu->focusedEntryFontColor = Color::WHITE;

	const vector<Course>& courses = gameLogic.getCourseList();
	for(unsigned i = 0; i < courses.size(); i++)
		menu->addEntry((string) courses[i]);
}

void CourseSelectionState::onEnter()
{}

void CourseSelectionState::onLeave()
{}

void CourseSelectionState::render()
{
	Display& display = game.getDisplay();
	display.clear();

	background->drawScaled(0, 0, scaledToSize(background, display));

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
			menu->bounds.y - 1.5f*fontTab->getHeight(),
			0.4f*menu->bounds.w,
			1.5f*fontTab->getHeight()
	};

	Image::drawFilledRectangle(tabSettingsBounds, not isLoadedCourseSelected? Color::AZURE : Color::NAVY);
	fontTab->drawText("Custom",tabSettingsBounds.x + xspacing, tabSettingsBounds.y + yspacing, not isLoadedCourseSelected? Color::WHITE : Color::AZURE);

	const Rectangle tabLoadedCoursesBounds = {
			menu->bounds.x + xspacing + tabSettingsBounds.w,
			tabSettingsBounds.y,
			tabSettingsBounds.w,
			tabSettingsBounds.h
	};

	Image::drawFilledRectangle(tabLoadedCoursesBounds, isLoadedCourseSelected? Color::AZURE : Color::NAVY);
	fontTab->drawText("Courses",tabLoadedCoursesBounds.x + xspacing, tabLoadedCoursesBounds.y + yspacing, isLoadedCourseSelected? Color::WHITE : Color::AZURE);

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
		imgCircuit->drawScaled(portraitImgBounds.x, portraitImgBounds.y, scaledToRect(imgCircuit, portraitImgBounds));
		const Course& course = gameLogic.getCourseList()[menu->getSelectedIndex()];
		fontInfo->drawText(string("Length: ")+(course.lines.size()*course.roadSegmentLength*0.001) + "Km", menu->bounds.x + menu->bounds.w + 32, menu->bounds.y + menu->bounds.h, Color::WHITE);
	}
	else
	{
		Image::drawFilledRectangle(menu->bounds, menu->bgColor);
		Image::drawRectangle(menu->bounds, menu->borderColor);

		Image::drawFilledRectangle(menu->bounds.x, menu->bounds.y * 1.1f, menu->bounds.w, fontTab->getHeight(), not isDebugCourseSelected? Color::RED : Color::_TRANSPARENT);
		fontTab->drawText("Random course", menu->bounds.x * 1.1f, menu->bounds.y * 1.1f,                        not isDebugCourseSelected? Color::WHITE : Color::RED);

		Image::drawFilledRectangle(menu->bounds.x, menu->bounds.y * 1.1f + fontTab->getHeight(), menu->bounds.w, fontTab->getHeight(), isDebugCourseSelected? Color::RED : Color::_TRANSPARENT);
		fontTab->drawText("Debug course",  menu->bounds.x * 1.1f, menu->bounds.y * 1.1f + fontTab->getHeight(), isDebugCourseSelected? Color::WHITE : Color::RED);

		if(isDebugCourseSelected)
			imgCircuit->drawScaled(portraitImgBounds.x, portraitImgBounds.y, scaledToRect(imgCircuit, portraitImgBounds));
		else
			imgRandom->drawScaled(portraitImgBounds.x, portraitImgBounds.y, scaledToRect(imgRandom, portraitImgBounds));
	}

	fontMain->drawText("Choose a course", menu->bounds.x, 0.0625f*displayHeight, Color::WHITE);
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
					shared.sndCursorOut.stop();
					shared.sndCursorOut.play();
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::KEY_ENTER:
					shared.sndCursorIn.stop();
					shared.sndCursorIn.play();
					this->onMenuSelect();
					break;
				case Keyboard::KEY_ARROW_UP:
					shared.sndCursorMove.stop();
					shared.sndCursorMove.play();
					if(isLoadedCourseSelected)
						menu->cursorUp();
					else
						isDebugCourseSelected = !isDebugCourseSelected;
					break;
				case Keyboard::KEY_ARROW_DOWN:
					shared.sndCursorMove.stop();
					shared.sndCursorMove.play();
					if(isLoadedCourseSelected)
						menu->cursorDown();
					else
						isDebugCourseSelected = !isDebugCourseSelected;
					break;
				case Keyboard::KEY_ARROW_LEFT:
					shared.sndCursorMove.stop();
					shared.sndCursorMove.play();
					isLoadedCourseSelected = !isLoadedCourseSelected;
					break;
				case Keyboard::KEY_ARROW_RIGHT:
					shared.sndCursorMove.stop();
					shared.sndCursorMove.play();
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
		gameLogic.setNextCourse(menu->getSelectedIndex());
	else
		if(isDebugCourseSelected)
			gameLogic.setNextCourseDebug();
		else
			gameLogic.setNextCourseRandom();

	game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
}

Image* CourseSelectionState::getSelectedCoursePreview()
{
	return isLoadedCourseSelected or isDebugCourseSelected? imgCircuit : imgRandom;
}
