/*
 * course_editor_state.hpp
 *
 *  Created on: 10 de jul de 2018
 *      Author: carlosfaruolo
 */

#ifndef STATES_COURSE_EDITOR_STATE_HPP_
#define STATES_COURSE_EDITOR_STATE_HPP_
#include <ciso646>

#include "carse_game.hpp"

#include "course.hpp"

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"

class CourseEditorState extends public fgeal::Game::State
{
	CarseSharedResources& shared;
	CarseGame::Logic& logic;

	fgeal::Menu* menuFile;

	fgeal::Rectangle boundsMap;

	fgeal::Rectangle boundsCourseView;

	fgeal::Rectangle boundsStatusBar;

	fgeal::Rectangle boundsToolsPanel;

	Pseudo3DCourse course;

	enum StateFocus
	{
		ON_EDITOR,
		ON_FILE_MENU,
	}
	focus;

	public:
	virtual int getId();

	CourseEditorState(CarseGame* game);
	~CourseEditorState();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void onKeyPressed(fgeal::Keyboard::Key k);
	virtual void onMouseButtonPressed(fgeal::Mouse::Button button, int x, int y);

	virtual void render();
	virtual void update(float delta);
};

#endif /* STATES_COURSE_EDITOR_STATE_HPP_ */
