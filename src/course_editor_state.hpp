/*
 * course_editor_state.hpp
 *
 *  Created on: 10 de jul de 2018
 *      Author: carlosfaruolo
 */

#ifndef STATES_COURSE_EDITOR_STATE_HPP_
#define STATES_COURSE_EDITOR_STATE_HPP_
#include <ciso646>

#include "course.hpp"

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"

class CarseGame;

class CourseEditorState extends public fgeal::Game::State
{
	CarseGame& game;

	fgeal::Menu* menuFile;

	fgeal::Font* font;

	fgeal::Rectangle boundsMap, boundsCourseView, boundsStatusBar, boundsToolsPanel,
		boundsButtonNew, boundsButtonLoad, boundsButtonSave;

	Pseudo3DCourse course;

	fgeal::Point offset;

	fgeal::Vector2D scale;

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

	private:
	void reloadFileList();
};

#endif /* STATES_COURSE_EDITOR_STATE_HPP_ */
