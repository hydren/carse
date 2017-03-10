/*
 * race_state.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef RACE_STATE_HPP_
#define RACE_STATE_HPP_
#include <ciso646>

#include "carse_game.hpp"
#include "fgeal/fgeal.hpp"
#include "futil/general/language.hpp"

class RaceState extends public fgeal::Game::State
{
	public:
	int getId() { return CarseGame::RACE_STATE_ID; }

	private:
	fgeal::Font* font, *font2;

	char buffer[256];

	public:
	RaceState(CarseGame* game);
	~RaceState();

	void initialize();

	void onEnter();
	void onLeave();

	void update(float delta);
	void render();

	private:
	void handlePhysics(float delta);
	void handleInput();
};

#endif /* RACE_STATE_HPP_ */
