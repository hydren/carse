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

#include "futil/language.hpp"
#include "fgeal/fgeal.hpp"
#include "fgeal/extra/sprite.hpp"

#include "course.hpp"
#include "vehicle.hpp"
#include "automotive/engine_sound.hpp"
#include "gui/race_hud.hpp"

class Pseudo3DRaceState extends public fgeal::Game::State
{
	fgeal::Font* font, *font2, *fontDebug;
	fgeal::Image* bg;
	fgeal::Music* music;

	fgeal::Sound* sndTireBurnoutStandIntro, *sndTireBurnoutStandLoop,
				 *sndTireBurnoutIntro, *sndTireBurnoutLoop;

	std::vector<fgeal::Sprite*> spritesVehicle;
	fgeal::Sprite* spriteSmokeLeft, *spriteSmokeRight;

	EngineSoundSimulator engineSound;
	float position, posX, speed, pseudoAngle, strafeSpeed, curvePull;
	float rollingFriction, airFriction, brakingFriction, corneringForceLeechFactor;
	bool isBurningRubber;

	Course::DrawParameters drawParameters;
	float coursePositionFactor;

	Course course;
	Vehicle vehicle;

	Hud::DialGauge<float>* hudRpmGauge;
	Hud::NumericalDisplay<float>* hudSpeedDisplay;
	Hud::NumericalDisplay<int>* hudGearDisplay;

	bool debugMode;

	public:
	int getId();

	static Pseudo3DRaceState* getInstance(fgeal::Game& game);

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

	public://menu accessed methods
	void setVehicle(const Vehicle& v);
	void setCourse(const Course& c);
};


#endif /* PSEUDO3D_RACE_STATE_HPP_ */
