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
#include "fgeal/extra/gui.hpp"

#include "futil/language.hpp"

class CarseGame;

class CourseEditorState extends public fgeal::Game::State
{
	CarseGame& game;

	fgeal::Vector2D lastDisplaySize;

	// used to store where is the current focus
	enum StateFocus
	{
		ON_EDITOR,
		ON_FILE_MENU,
		ON_SAVE_DIALOG,
	}
	focus;

	// resources
	fgeal::Font* font;
	fgeal::Sound* sndCursorMove, *sndCursorIn, *sndCursorOut;

	// the course being edited
	Pseudo3DCourse course;

	// map being displayed
	Pseudo3DCourse::Map map;

	// the area being used to draw the course map
	fgeal::Rectangle mapBounds;
	fgeal::Point courseEditorTitlePosition;

	// the area being used to draw a course preview
	fgeal::Rectangle courseViewBounds;

	// tools panel stuff
	fgeal::Rectangle toolsPanelBounds, presetsTabPanelBounds, propertiesTabPanelBounds;
	fgeal::Button presetsTabButton, propertiesTabButton, newButton, loadButton, saveButton, generateButton, exitButton;
	bool isPresetsTabActive;

	// properties tab stuff
	fgeal::TextField landscapeStyleTextField, roadStyleTextField, courseNameTextField;
	int selectedLandscapeStyleIndex, selectedRoadStyleIndex;
	fgeal::Button landscapeStyleChangeButton, roadStyleChangeButton;

	// load dialog
	fgeal::Rectangle loadDialogBounds;
	fgeal::Menu fileMenu;
	fgeal::Image* imgMenuCourseArrow;
	fgeal::Rectangle imgMenuCourseArrowUpBounds, imgMenuCourseArrowDownBounds;
	fgeal::Button loadDialogSelectButton, loadDialogCancelButton;

	// save dialog
	fgeal::Rectangle saveDialogBounds;
	fgeal::TextField saveDialogTextField;
	fgeal::Button saveDialogSaveButton, saveDialogCancelButton;

	// status bar
	fgeal::Rectangle statusBarBounds;
	fgeal::Point scaleIndicatorPosition;
	std::string scaleIndicatorText;

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
	void setPresetsTabActive(bool choice=true);
	void loadCourseSpec(const Pseudo3DCourse::Spec&);
};

#endif /* STATES_COURSE_EDITOR_STATE_HPP_ */
