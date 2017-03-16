/*
 * race_state.hpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#ifndef RACE_STATE_HPP_
#define RACE_STATE_HPP_
#include <ciso646>

#include <vector>

#include "carse_game.hpp"
#include "fgeal/fgeal.hpp"
#include "futil/general/language.hpp"

class RaceState extends public fgeal::Game::State
{
	public:
	int getId() { return CarseGame::RACE_STATE_ID; }

	private:
	fgeal::Font* font, *font2;
	fgeal::Image* bg, *car;
	fgeal::Music* music;
	fgeal::Sound** soundEngine;
	unsigned soundEngineCount;

	char buffer[256];

	float roadSegmentLength, roadWidth;
	float cameraDepth;

	struct Segment
	{
		RaceState& state;

		Segment& operator= (const Segment& s);

		float x, y, z; // 3d center of line (delta coordinates)
		float X, Y, W; // screen coordinate
		float scale, curve;

		Segment(RaceState& state);

		// from "world" to screen coordinates
		void project(int camX, int camY, int camZ);
	};

	std::vector<Segment> lines;

	float position, posX, speed, strafeSpeed;

	struct Engine
	{
		float torque;
		float rpm, maxRpm;
		int gear, gearCount;
		float *gearRatio, reverseGearRatio;
		float wheelRadius;

		float getDriveForce();
	};

	Engine engine;

	float carWeight;

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
