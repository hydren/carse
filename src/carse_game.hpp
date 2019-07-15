/*
 * carse_game.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef CARSE_GAME_HPP_
#define CARSE_GAME_HPP_
#include <ciso646>

#include "carse_logic.hpp"

#include "fgeal/extra/game.hpp"

#include "futil/language.hpp"

extern const std::string CARSE_VERSION;

class CarseGame extends public fgeal::Game
{
	public:
	CarseLogic& logic;

	/** Wrapper to resources shared between states. */
	struct SharedResources
	{
		fgeal::Sound sndCursorMove, sndCursorIn, sndCursorOut;
		fgeal::Font fontDev;
		std::string font1Path, font2Path, font3Path;

		SharedResources();
	} *sharedResources;

	enum StateID
	{
		RACE_STATE_ID,
		MAIN_MENU_SIMPLE_LIST_STATE_ID,
		MAIN_MENU_CLASSIC_LAYOUT_STATE_ID,
		OPTIONS_MENU_STATE_ID,
		VEHICLE_SELECTION_SIMPLE_LIST_STATE_ID,
		VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_ID,
		COURSE_SELECTION_STATE_ID,
		COURSE_EDITOR_STATE_ID,
	};

	CarseGame();
	~CarseGame();
	virtual void initialize();
};

#endif /* CARSE_GAME_HPP_ */
