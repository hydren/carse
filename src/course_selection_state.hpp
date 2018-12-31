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
	fgeal::Rectangle paneBounds, portraitBounds, portraitImgBounds, courseMapBounds,
		courseEditorPortraitBounds, backButtonBounds, selectButtonBounds;

	fgeal::Image* background, *imgRandom, *imgCircuit, *imgCourseEditor;
	fgeal::Font* fontMain, *fontInfo, *fontSmall;
	fgeal::Sound* sndCursorMove, *sndCursorIn, *sndCursorOut;

	bool isLoadedCourseSelected, isDebugCourseSelected;

	enum ScreenFocus
	{
		FOCUS_ON_COURSE_LIST_SELECTION,
		FOCUS_ON_SETTINGS_LIST_HOVER,
		FOCUS_ON_SETTINGS_LIST_SELECTION,
		FOCUS_ON_COURSE_EDITOR_PORTRAIT
	}
	focus;

	public:
	virtual int getId();

	CourseSelectionState(CarseGame* game);
	~CourseSelectionState();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void onKeyPressed(fgeal::Keyboard::Key k);
	virtual void onMouseButtonPressed(fgeal::Mouse::Button button, int x, int y);
	virtual void onMouseMoved(int oldx, int oldy, int x, int y);

	virtual void render();
	virtual void update(float delta);

	fgeal::Image* getSelectedCoursePreview();

	private:
	void handleInputOnSettings(fgeal::Keyboard::Key k);

	void updateLapCount();
};

#endif /* COURSE_SELECTION_STATE_HPP_ */
