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

class Pseudo3DRaceState extends public fgeal::Game::State
{
	fgeal::Font* font, *font2;
	fgeal::Image* bg, *car;
	fgeal::Music* music;

	std::vector< std::pair<short, fgeal::Sound*> > soundEngine;
	float position, posX, speed, strafeSpeed;

	Course course;
	Vehicle vehicle;

	public:
	int getId();

	Pseudo3DRaceState(CarseGame* game);
	~Pseudo3DRaceState();

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
