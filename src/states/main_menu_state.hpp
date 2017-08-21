/*
 * menu_state.hpp
 *
 *  Created on: 31 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_MAIN_MENU_STATE_HPP_
#define PSEUDO3D_MAIN_MENU_STATE_HPP_
#include <ciso646>

#include "carse_game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"
#include "fgeal/fgeal.hpp"

class MainMenuState extends public fgeal::Game::State
{
	struct Layout
	{
		fgeal::Menu menu;
		Layout();
		virtual void draw() abstract;
		virtual void updateBounds(fgeal::Display&) abstract;
		virtual void onCursorChange() abstract;
		virtual void onCursorAccept() abstract;
		virtual ~Layout();
	};

	Layout* layout;
	fgeal::Font* fontDev;

	struct PrototypeSimpleLayout extends Layout
	{
		fgeal::Font fontMain;
		fgeal::Sound sndCursorMove, sndCursorAccept;
		PrototypeSimpleLayout();
		void draw();
		void updateBounds(fgeal::Display&);
		void onCursorChange();
		void onCursorAccept();
	};

	public:
	int getId();

	MainMenuState(CarseGame* game);
	~MainMenuState();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	private:
	void handleInput();
	void onMenuSelect();
};

#endif /* PSEUDO3D_MAIN_MENU_STATE_HPP_ */
