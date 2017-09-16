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

#include "futil/language.hpp"
#include "fgeal/fgeal.hpp"

class CourseSelectionState extends public fgeal::Game::State
{
	CarseGame::SharedResources& shared;
	CarseGame::Logic& gameLogic;

	fgeal::Image* imgRandom, *imgCircuit;
	fgeal::Font* fontMain, *fontInfo, *fontTab;
	fgeal::Menu* menu;

	bool isLoadedCourseSelected;

	bool isDebugCourseSelected;

	public:
	int getId();

	CourseSelectionState(Pseudo3DCarseGame* game);
	~CourseSelectionState();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	fgeal::Image* getSelectedCoursePreview();

	private:
	void handleInput();
	void onMenuSelect();
};

#endif /* PSEUDO3D_COURSE_SELECTION_STATE_HPP_ */
