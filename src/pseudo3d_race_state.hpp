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

class CarseGame;

class Pseudo3DRaceState extends public fgeal::Game::State
{
	CarseGame& game;

	fgeal::Vector2D lastDisplaySize;

	fgeal::Font* fontSmall, *fontCountdown, *font3, *fontDev;
	fgeal::Image* imgBackground, *imgCacheTachometer, *imgStopwatch;
	fgeal::Music* music;

	fgeal::Sound* sndWheelspinBurnoutIntro, *sndWheelspinBurnoutLoop,
				 *sndSideslipBurnoutIntro, *sndSideslipBurnoutLoop, *sndRunningOnDirtLoop,
				 *sndJumpImpact, *sndCountdownBuzzer, *sndCountdownBuzzerFinal;

	fgeal::Color bgColor, bgColorHorizon;
	fgeal::Sprite* spriteSmokeLeft, *spriteSmokeRight;

	fgeal::Point parallax;
	float backgroundScale;

	float coursePositionFactor;
	Mechanics::SimulationType simulationType;

	bool onSceneIntro, onSceneFinish;
	float timerSceneIntro, timerSceneFinish;

	unsigned countdownBuzzerCounter;

	static const float MAXIMUM_STRAFE_SPEED_FACTOR;

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

	inline static std::string toString(Pseudo3DRaceState::RaceType type)
	{
		switch(type)
		{
			case Pseudo3DRaceState::RACE_TYPE_DEBUG: 						return "Debug";
			case Pseudo3DRaceState::RACE_TYPE_LOOP_PRACTICE: 				return "Circuit - Practice";
			case Pseudo3DRaceState::RACE_TYPE_LOOP_TIME_TRIAL: 				return "Circuit - Time Trial";
			case Pseudo3DRaceState::RACE_TYPE_LOOP_TIME_ATTACK:				return "Circuit - Time Attack";
			case Pseudo3DRaceState::RACE_TYPE_POINT_TO_POINT_PRACTICE:		return "Sprint - Practice";
			case Pseudo3DRaceState::RACE_TYPE_POINT_TO_POINT_TIME_TRIAL:	return "Sprint - Time Trial";
			default: return "???";
		}
	}

	// returns true if the given race type is a loop-type
	inline static bool isRaceTypeLoop(RaceType type)
	{
		return type == RACE_TYPE_LOOP_PRACTICE
			or type == RACE_TYPE_LOOP_TIME_TRIAL
			or type == RACE_TYPE_LOOP_TIME_ATTACK;
	}

	// returns true if the given race type is a point-to-point-type
	inline static bool isRaceTypePointToPoint(RaceType type)
	{
		return type == RACE_TYPE_POINT_TO_POINT_PRACTICE
			or type == RACE_TYPE_POINT_TO_POINT_TIME_TRIAL;
	}

	struct RaceSettings
	{
		RaceType raceType;
		unsigned lapCountGoal;
		bool isImperialUnit;
		bool useBarTachometer;
		bool useCachedTachometer;
		std::string hudTachometerPointerImageFilename;
	};

	private:
	RaceSettings settings;

	float lapTimeCurrent, lapTimeBest;
	unsigned lapCurrent;

	//debug
	float acc0to60clock, acc0to60time;

	Pseudo3DCourse course;

	Pseudo3DVehicle playerVehicle;

	std::vector<Pseudo3DVehicle> trafficVehicles;

	Pseudo3DCourse::Map minimap;

	Hud::DialGauge<float> hudDialTachometer;
	Hud::BarGauge<float> hudBarTachometer;
	Hud::NumericalDisplay<float> hudSpeedometer;
	Hud::NumericalDisplay<int> hudGearDisplay;
	Hud::TimerDisplay<float> hudTimerCurrentLap, hudTimerBestLap;
	Hud::NumericalDisplay<unsigned> hudCurrentLap, hudLapCountGoal;

	fgeal::Color hudMiniMapBgColor;

	float rightHudMargin, offsetHudLapGoal;
	fgeal::Point posHudCountdown, posHudFinishedCaption;

	fgeal::Rectangle stopwatchIconBounds;

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
	virtual int getId();

	Pseudo3DRaceState(CarseGame* game);
	~Pseudo3DRaceState();

	virtual void initialize();

	virtual void onEnter();
	virtual void onLeave();

	virtual void update(float delta);
	virtual void render();

	virtual void onKeyPressed(fgeal::Keyboard::Key);
	virtual void onJoystickButtonPressed(unsigned joystick, unsigned button);

	private:
	void handlePhysics(float delta);

	void shiftGear(int gear);

	SurfaceType getCurrentSurfaceType();
	float getTireKineticFrictionCoefficient();
	float getTireRollingResistanceCoefficient();
};

#endif /* PSEUDO3D_RACE_STATE_HPP_ */
