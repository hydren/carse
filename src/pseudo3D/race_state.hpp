/*
 * race_state.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_RACE_STATE_HPP_
#define PSEUDO3D_RACE_STATE_HPP_
#include <ciso646>

#include <vector>
#include <utility>

#include "carse_game.hpp"

#include "futil/general/language.hpp"
#include "fgeal/fgeal.hpp"

#include "course.hpp"
#include "vehicle.hpp"

class RaceState extends public fgeal::Game::State
{
	public:
	int getId() { return CarseGame::RACE_STATE_ID; }

	private:
	fgeal::Font* font, *font2;
	fgeal::Image* bg, *car;
	fgeal::Music* music;

	std::vector< std::pair<short, fgeal::Sound*> > soundEngine;
	float position, posX, speed, strafeSpeed;

	Course course;
	Vehicle vehicle;

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


#endif /* PSEUDO3D_RACE_STATE_HPP_ */
