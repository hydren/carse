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
class CarseGameLogic;

class Pseudo3DRaceState extends public fgeal::Game::State
{
	friend class CarseGameLogic;

	fgeal::Font* font, *font2, *font3, *fontDebug;
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

	float laptime, laptimeBest;

	unsigned lapCurrent;

	Course course;
	Pseudo3DVehicle::Spec playerVehicleSpec;
	int playerVehicleSpecAlternateSpriteIndex;
	Pseudo3DVehicle vehicle;

	Hud::DialGauge<float>* hudRpmGauge;
	Hud::NumericalDisplay<float>* hudSpeedDisplay;
	Hud::NumericalDisplay<int>* hudGearDisplay;
	Hud::TimerDisplay<float>* hudTimerCurrentLap, *hudTimerBestLap;
	Hud::NumericalDisplay<unsigned>* hudCurrentLap;

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

	public:
	int getId();

	static Pseudo3DRaceState* getInstance(fgeal::Game& game);

	Pseudo3DRaceState(Pseudo3DCarseGame* game);
	~Pseudo3DRaceState();

	void initialize();

	void onEnter();
	void onLeave();

	void update(float delta);
	void render();

	private:
	void handleInput();
	void handlePhysics(float delta);

	void drawVehicle(const Pseudo3DVehicle&, const fgeal::Point&);

	void shiftGear(int gear);

	SurfaceType getCurrentSurfaceType();
	float getTireKineticFrictionCoefficient();
	float getTireRollingResistanceCoefficient();
};

#endif /* PSEUDO3D_RACE_STATE_HPP_ */
