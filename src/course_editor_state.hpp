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

	fgeal::Menu fileMenu;

	fgeal::Font* font;

	fgeal::Sound* sndCursorMove, *sndCursorIn, *sndCursorOut;

	fgeal::Rectangle mapBounds, courseViewBounds, statusBarBounds, toolsPanelBounds,
		newButtonBounds, loadButtonBounds, saveButtonBounds,
		generateButtonBounds,
		loadDialogBounds, loadDialogButtonSelectBounds, loadDialogButtonCancelBounds,
		saveDialogBounds, saveDialogFilenameTextFieldBounds, saveDialogSaveButtonBounds, saveDialogCancelButtonBounds;

	Pseudo3DCourse course;

	fgeal::Point offset;

	fgeal::Vector2D scale;

	std::string saveDialogFilename;

	enum StateFocus
	{
		ON_EDITOR,
		ON_FILE_MENU,
		ON_SAVE_DIALOG,
	}
	focus;

	int saveDialogFilenameTextFieldCaretPosition;

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
	void loadCourse(const Pseudo3DCourse& course);
};

#endif /* STATES_COURSE_EDITOR_STATE_HPP_ */
