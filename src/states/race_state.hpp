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
	friend class Pseudo3DCarseGame;

	fgeal::Font* font, *font2, *font3, *fontDebug;
	fgeal::Image* bg;
	fgeal::Music* music;

	fgeal::Sound* sndTireBurnoutStandIntro, *sndTireBurnoutStandLoop,
				 *sndTireBurnoutIntro, *sndTireBurnoutLoop, *sndOnDirtLoop,
				 *sndJumpImpact;

	fgeal::Color bgColor, bgColorHorizon;
	std::vector<fgeal::Sprite*> spritesVehicle;
	fgeal::Sprite* spriteSmokeLeft, *spriteSmokeRight;

	EngineSoundSimulator engineSound;
	float position, posX, posY, pseudoAngle, strafeSpeed, curvePull, corneringForceLeechFactor;
	fgeal::Point bgParallax;
	bool isBurningRubber;
//	float verticalSpeed;
//	bool onAir, onLongAir;

	Course::DrawParameters drawParameters;
	float coursePositionFactor;
	bool isImperialUnit;
	Mechanics::SimulationType simulationType;

	float laptime, laptimeBest;

	unsigned lapCurrent;

	Course course;
	Vehicle vehicle;

	Hud::DialGauge<float>* hudRpmGauge;
	Hud::NumericalDisplay<float>* hudSpeedDisplay;
	Hud::NumericalDisplay<int>* hudGearDisplay;
	Hud::TimerDisplay<float>* hudTimerCurrentLap, *hudTimerBestLap;
	Hud::NumericalDisplay<unsigned>* hudCurrentLap;

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

	void shiftGear(int gear);

	SurfaceType getCurrentSurfaceType();
	float getTireKineticFrictionCoefficient();
	float getTireRollingResistanceCoefficient();
};

#endif /* PSEUDO3D_RACE_STATE_HPP_ */
