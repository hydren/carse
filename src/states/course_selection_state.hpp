/*
 * course_selection_state.hpp
 *
 *  Created on: 23 de mai de 2017
 *      Author: carlosfaruolo
 */

#ifndef COURSE_SELECTION_STATE_HPP_
#define COURSE_SELECTION_STATE_HPP_
#include <ciso646>

#include <vector>

#include "carse_game.hpp"

#include "course.hpp"

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"

class CourseSelectionState extends public fgeal::Game::State
{
	CarseSharedResources& shared;
	CarseGameLogic& gameLogic;

	fgeal::Image* background, *imgRandom, *imgCircuit;
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

#endif /* COURSE_SELECTION_STATE_HPP_ */
