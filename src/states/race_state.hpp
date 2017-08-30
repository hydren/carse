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

	fgeal::Color bgColor;
	std::vector<fgeal::Sprite*> spritesVehicle;
	fgeal::Sprite* spriteSmokeLeft, *spriteSmokeRight;

	EngineSoundSimulator engineSound;
	float position, posX, pseudoAngle, strafeSpeed, curvePull;
	fgeal::Point bgParallax;
	float rollingFriction, airFriction, brakingFriction, corneringForceLeechFactor;
	bool isBurningRubber;
	float fakeBrakeBuildUp;  // xxx this should be removed once the simulation allows tire slipping, and thus, car slides when braking when its tires are slipping

	Course::DrawParameters drawParameters;
	float coursePositionFactor;

	float laptime, laptimeBest;

	unsigned lapCurrent;

	Course course;
	Vehicle vehicle;

	Hud::DialGauge<float>* hudRpmGauge;
	Hud::NumericalDisplay<float>* hudSpeedDisplay;
	Hud::NumericalDisplay<int>* hudGearDisplay;
	Hud::TimerDisplay<float>* hudTimerCurrentLap, *hudTimerBestLap;
	Hud::NumericalDisplay<unsigned>* hudCurrentLap;

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
	void handleInput();
	void handlePhysics(float delta);

	void updateDrivetrain(float delta);
	float getDriveForce();
	float getLongitudinalSlipRatio();
	float getDrivenWheelsTireLoad();
	float getNormalizedTractionForce();

	public://menu accessed methods
	void setVehicle(const Vehicle& v);
	void setCourse(const Course& c);
};


#endif /* PSEUDO3D_RACE_STATE_HPP_ */
