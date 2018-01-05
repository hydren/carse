/*
 * options_menu_state.hpp
 *
 *  Created on: 5 de set de 2017
 *      Author: carlosfaruolo
 */

#ifndef OPTIONS_MENU_STATE_HPP_
#define OPTIONS_MENU_STATE_HPP_
#include <ciso646>

#include "carse_game.hpp"

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"

class CarseGameLogic;
class CarseSharedResources;
class Pseudo3DCarseGame;

class OptionsMenuState extends public fgeal::Game::State
{
	CarseGameLogic& logic;
	CarseSharedResources& shared;
	fgeal::Menu* menu, *menuResolution;
	fgeal::Font* fontTitle, *font;
	fgeal::Image* background;

	bool isResolutionMenuActive;

	public:
	int getId();

	OptionsMenuState(Pseudo3DCarseGame* game);
	~OptionsMenuState();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	private:
	void onMenuSelect();
	void updateOnResolutionMenu(fgeal::Event&);
	void updateLabels();

	void updateFonts();
};

#endif /* OPTIONS_MENU_STATE_HPP_ */
