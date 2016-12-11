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

#include "racing/vehicle.hpp"

#include "fgeal/fgeal.hpp"

#include "futil/general/language.hpp"

#include <Box2D/Box2D.h>

#include "util/box2d_util.hpp"

class RaceState extends public virtual fgeal::GenericGame::State
{
	public:
	int getId() { return CarseGame::RACE_STATE_ID; }

	private:
	b2World* world;
	Car* player;

	bool lockOn;  // glitches on SDL adapters
	bool showDebug; // crashes on allegro adapter

	//the race camera
	Rect camera;
	double cameraAngle;

	fgeal::Image* car_sprite, *track_bg;
	fgeal::Sound* car_sound_idle, *car_sound_high;
	fgeal::Music* music_sample;
	fgeal::Font* font, *font2;

	fgeal::EventQueue* eventQueue;

	bool isKeyUpPressed, isKeyDownPressed,
		 isKeyRightPressed, isKeyLeftPressed;

	char buffer[256];

	public:
	RaceState(fgeal::GenericGame& game);
	~RaceState();

	void initialize();

	void onEnter();
	void onLeave();

	void update(float delta);
	void render();

	private:
	void handlePhysics();
	void handleInput();
};

#endif /* RACE_STATE_HPP_ */
