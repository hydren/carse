/*
 * course_selection_state.hpp
 *
 *  Created on: 23 de mai de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_COURSE_SELECTION_STATE_HPP_
#define PSEUDO3D_COURSE_SELECTION_STATE_HPP_
#include <ciso646>

#include <vector>

#include "carse_game.hpp"
#include "fgeal/extra/menu.hpp"
#include "course.hpp"

#include "futil/general/language.hpp"
#include "fgeal/fgeal.hpp"

class CourseSelectionState extends public fgeal::Game::State
{
	fgeal::Font* fontMain, *fontInfo;
	fgeal::Menu* menu;

	std::vector<Course> courses;

	public:
	int getId();

	CourseSelectionState(Pseudo3DCarseGame* game);
	~CourseSelectionState();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	private:
	void handleInput();
	void onMenuSelect();
};

#endif /* PSEUDO3D_COURSE_SELECTION_STATE_HPP_ */