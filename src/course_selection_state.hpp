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
	CarseGame& game;
	fgeal::Menu menuCourse, menuSettings;
	fgeal::Rectangle titleBounds, paneBounds, portraitBounds, portraitImgBounds, courseEditorPortraitBounds;

	fgeal::Image* background, *imgRandom, *imgCircuit, *imgCourseEditor;
	fgeal::Font* fontMain, *fontInfo;
	fgeal::Sound* sndCursorMove, *sndCursorIn, *sndCursorOut;

	bool isLoadedCourseSelected, isDebugCourseSelected;

	enum MenuStatus
	{
		STATUS_ON_COURSE_LIST_SELECTION,
		STATUS_HOVERING_COURSE_LIST,
		STATUS_HOVERING_SETTINGS_LIST,
		STATUS_HOVERING_COURSE_EDITOR_PORTRAIT
	}
	status;

	public:
	virtual int getId();

	CourseSelectionState(CarseGame* game);
	~CourseSelectionState();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void onKeyPressed(fgeal::Keyboard::Key k);

	virtual void render();
	virtual void update(float delta);

	fgeal::Image* getSelectedCoursePreview();

	private:
	void handleInputOnCourseList(fgeal::Keyboard::Key k);
	void handleInputOnSettings(fgeal::Keyboard::Key k);

	void updateLapCount();
};

#endif /* COURSE_SELECTION_STATE_HPP_ */
