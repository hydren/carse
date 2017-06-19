/*
 * course_selection_mode.hpp
 *
 *  Created on: 23 de mai de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_COURSE_SELECTION_MODE_HPP_
#define PSEUDO3D_COURSE_SELECTION_MODE_HPP_
#include <ciso646>

#include <vector>

#include "carse_game.hpp"
#include "fgeal/extra/menu.hpp"
#include "course.hpp"

#include "futil/general/language.hpp"
#include "fgeal/fgeal.hpp"

class CourseSelectionMode extends public fgeal::Game::State
{
	fgeal::Font* fontMain, *fontInfo;
	fgeal::Menu* menu;

	std::vector<Course> courses;

	public:
	int getId();

	CourseSelectionMode(Pseudo3DCarseGame* game);
	~CourseSelectionMode();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	private:
	void handleInput();
	void onMenuSelect();
};

#endif /* PSEUDO3D_COURSE_SELECTION_MODE_HPP_ */
