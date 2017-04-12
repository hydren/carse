/*
 * race_state.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef TOPDOWN_RACE_STATE_HPP_
#define TOPDOWN_RACE_STATE_HPP_
#include <ciso646>

#include "carse_game.hpp"

#include "racing/vehicle.hpp"

#include "fgeal/fgeal.hpp"

#include "futil/general/language.hpp"
#include "futil/math/rect.hpp"

#include <Box2D/Box2D.h>

#include "box2d_util.hpp"

class TopDownRaceState extends public fgeal::Game::State
{
	public:
	int getId() { return CarseGame::RACE_STATE_ID; }

	private:
	b2World* world;
	Car* player;

	bool lockOn;
	bool showDebug;

	//the race camera
	Rect camera;
	double cameraAngle;

	fgeal::Image* car_sprite, *track_bg;
	fgeal::Sound* car_sound_idle, *car_sound_high;
	fgeal::Music* music_sample;
	fgeal::Font* font, *font2;

	bool isKeyUpPressed, isKeyDownPressed,
		 isKeyRightPressed, isKeyLeftPressed;

	char buffer[256];

	public:
	TopDownRaceState(CarseGame* game);
	~TopDownRaceState();

	void initialize();

	void onEnter();
	void onLeave();

	void update(float delta);
	void render();

	private:
	void handlePhysics(float delta);
	void handleInput();
};

#endif /* TOPDOWN_RACE_STATE_HPP_ */
