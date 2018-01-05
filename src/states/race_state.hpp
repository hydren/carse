/*
 * race_state.hpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_RACE_STATE_HPP_
#define PSEUDO3D_RACE_STATE_HPP_
#include <ciso646>

#include "course.hpp"
#include "vehicle.hpp"

#include "automotive/engine_sound.hpp"

#include "gui/race_hud.hpp"

#include "futil/language.hpp"

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/sprite.hpp"

#include <vector>
#include <utility>

class Pseudo3DCarseGame;
class CarseSharedResources;
class CarseGameLogic;

class Pseudo3DRaceState extends public fgeal::Game::State
{
	friend class CarseGameLogic;

	CarseSharedResources& shared;

	fgeal::Font* fontSmall, *fontCountdown, *font3, *fontDebug;
	fgeal::Image* imgBackground;
	fgeal::Music* music;

	fgeal::Sound* sndTireBurnoutStandIntro, *sndTireBurnoutStandLoop,
				 *sndTireBurnoutIntro, *sndTireBurnoutLoop, *sndOnDirtLoop,
				 *sndJumpImpact;

	fgeal::Color bgColor, bgColorHorizon;
	fgeal::Sprite* spriteSmokeLeft, *spriteSmokeRight;

	fgeal::Point parallax;
	float backgroundScale;

	Course::DrawParameters drawParameters;
	float coursePositionFactor;
	bool isImperialUnit;
	Mechanics::SimulationType simulationType;

	bool onSceneIntro, onSceneFinish;
	float timerSceneIntro, timerSceneFinish;

	public:
	enum RaceType
	{
		RACE_TYPE_DEBUG,
		RACE_TYPE_LOOP_PRACTICE,
		RACE_TYPE_LOOP_TIME_TRIAL,
		RACE_TYPE_LOOP_TIME_ATTACK,
//		RACE_TYPE_LOOP_AGAINST_OPPOSITION,
//		RACE_TYPE_KNOCK_OUT,
		RACE_TYPE_POINT_TO_POINT_PRACTICE,
		RACE_TYPE_POINT_TO_POINT_TIME_TRIAL,
//		RACE_TYPE_POINT_TO_POINT_AGAINST_OPPOSITION,
//		RACE_TYPE_DRAG_PRACTICE,
//		RACE_TYPE_DRAG_AGAINST_OPPOSITION,
//		RACE_TYPE_SIDE_DRAG_PRACTICE,
//		RACE_TYPE_SIDE_DRAG_AGAINST_OPPOSITION,
//		RACE_TYPE_DRIFT_PRACTICE,
//		RACE_TYPE_DRIFT_AGAINST_OPPOSITION,
//		RACE_TYPE_OVERALL_SPEED_COMPETITION,
//		RACE_TYPE_AVERAGE_SPEED_COMPETITION,
//		RACE_TYPE_OVERTAKING_DUEL,
//		RACE_TYPE_PURSUIT
		RACE_TYPE_COUNT
	};

	struct RaceSettings
	{
		RaceType raceType;
		unsigned lapCountGoal;
	};

	private:
	RaceSettings settings;

	float lapTimeCurrent, lapTimeBest;
	unsigned lapCurrent;

	Course course;
	Pseudo3DVehicle::Spec playerVehicleSpec;
	int playerVehicleSpecAlternateSpriteIndex;
	Pseudo3DVehicle playerVehicle;

	Hud::DialGauge<float> hudTachometer;
	Hud::NumericalDisplay<float> hudSpeedometer;
	Hud::NumericalDisplay<int> hudGearDisplay;
	Hud::TimerDisplay<float> hudTimerCurrentLap, hudTimerBestLap;
	Hud::NumericalDisplay<unsigned> hudCurrentLap, hudLapCountGoal;

	float rightHudMargin, offsetHudLapGoal;
	fgeal::Point posHudCountdown, posHudFinishedCaption;

	// keybindings
	fgeal::Keyboard::Key controlKeyAccelerate,
						 controlKeyBrake,
						 controlKeyTurnLeft,
						 controlKeyTurnRight,
						 controlKeyShiftUp,
						 controlKeyShiftDown;

	unsigned controlJoystickKeyAccelerate,
	 	 	 controlJoystickKeyBrake,
			 controlJoystickKeyShiftUp,
			 controlJoystickKeyShiftDown;

	unsigned controlJoystickAxisTurn;

	//todo make these types parametrizable (and thus, not hardcoded) so they can be externally specified to allow custom, user-defined types.
	enum SurfaceType
	{
		SURFACE_TYPE_DRY_ASPHALT,
		SURFACE_TYPE_WET_ASPHALT,

		SURFACE_TYPE_CONCRETE,
		SURFACE_TYPE_GRAVEL,

		SURFACE_TYPE_GRASS,
		SURFACE_TYPE_DIRT,
		SURFACE_TYPE_MUD,

		SURFACE_TYPE_SAND,
		SURFACE_TYPE_SNOW,
		SURFACE_TYPE_ICE,

		SURFACE_TYPE_WATER
	};

	bool debugMode;

	bool isPlayerAccelerating();
	bool isPlayerBraking();
	bool isPlayerSteeringLeft();
	bool isPlayerSteeringRight();

	public:
	int getId();

	Pseudo3DRaceState(Pseudo3DCarseGame* game);
	~Pseudo3DRaceState();

	void initialize();

	void onEnter();
	void onLeave();

	void update(float delta);
	void render();

	// returns true if the given race type is a loop-type
	static bool isRaceTypeLoop(RaceType type);

	// returns true if the given race type is a point-to-point-type
	static bool isRaceTypePointToPoint(RaceType type);

	private:
	void handleInput();
	void handlePhysics(float delta);

	void drawVehicle(const Pseudo3DVehicle&, const fgeal::Point&);

	void shiftGear(int gear);

	SurfaceType getCurrentSurfaceType();
	float getTireKineticFrictionCoefficient();
	float getTireRollingResistanceCoefficient();
};

std::string to_string(Pseudo3DRaceState::RaceType);

#endif /* PSEUDO3D_RACE_STATE_HPP_ */
