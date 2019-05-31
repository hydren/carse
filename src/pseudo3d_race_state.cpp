/*
 * race_state.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "pseudo3d_race_state.hpp"

#include "carse_game.hpp"

#include "util.hpp"

#include "futil/random.h"
#include "futil/snprintf.h"

using futil::snprintf;

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdlib>

using std::string;
using std::map;
using std::vector;

using fgeal::Display;
using fgeal::Graphics;
using fgeal::Image;
using fgeal::Font;
using fgeal::Color;
using fgeal::Keyboard;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Sound;
using fgeal::Music;
using fgeal::Sprite;
using fgeal::Point;
using fgeal::Vector2D;
using fgeal::Joystick;
using fgeal::Rectangle;

#define GRAVITY_ACCELERATION Mechanics::GRAVITY_ACCELERATION

const float Pseudo3DRaceState::MAXIMUM_STRAFE_SPEED_FACTOR = 30;  // undefined unit

static const float MINIMUM_SPEED_TO_SIDESLIP = 5.5556,  // == 20kph
		GLOBAL_VEHICLE_SCALE_FACTOR = 0.0048828125,
		HORIZON_DISTANCE = 7.14,  // == 7.14km TODO use camera height instead in the horizon formula: 3.57*sqrt(cameraHeightInMeters)
		BACKGROUND_HORIZONTAL_PARALLAX_FACTOR = 0.35 * HORIZON_DISTANCE,
		BACKGROUND_VERTICAL_PARALLAX_FACTOR = 0.509375,
		MPS_TO_MPH = 2.236936,  // m/s to mph conversion factor
		MPS_TO_KPH = 3.6;  // m/s to km/h conversion factor

// -------------------------------------------------------------------------------

int Pseudo3DRaceState::getId(){ return CarseGame::RACE_STATE_ID; }

Pseudo3DRaceState::Pseudo3DRaceState(CarseGame* game)
: State(*game), game(*game), lastDisplaySize(),
  fontSmall(null), fontTiny(null), fontCountdown(null), font3(null), fontDev(null),
  imgCacheTachometer(null), imgStopwatch(null),
  music(null),
  sndWheelspinBurnoutIntro(null), sndWheelspinBurnoutLoop(null),
  sndSideslipBurnoutIntro(null), sndSideslipBurnoutLoop(null),
  sndRunningOnDirtLoop(null),
  sndCrashImpact(null), sndJumpImpact(null),
  sndCountdownBuzzer(null), sndCountdownBuzzerFinal(null),

  bgColor(), bgColorHorizon(),
  spriteSmoke(null), spriteBackground(null), verticalBackgroundParallax(),

  coursePositionFactor(500), playerVehicleProjectionOffset(6), courseStartPositionOffset(0), simulationType(), enableJumpSimulation(),
  onSceneIntro(), onSceneFinish(), timerSceneIntro(), timerSceneFinish(), countdownBuzzerCounter(), settings(),
  lapTimeCurrent(0), lapTimeBest(0), lapCurrent(0), acc0to60clock(0), acc0to60time(0),

  course(), playerVehicle(),

  hudDialTachometer(playerVehicle.body.engine.rpm),
  hudDialSpeedometer(playerVehicle.body.speed),
  hudBarTachometer(playerVehicle.body.engine.rpm),
  hudSpeedometer(playerVehicle.body.speed),
  hudGearDisplay(playerVehicle.body.engine.gear),
  hudTimerCurrentLap(lapTimeCurrent),
  hudTimerBestLap(lapTimeBest),
  hudCurrentLap(lapCurrent),
  hudLapCountGoal(settings.lapCountGoal),

  rightHudMargin(), offsetHudLapGoal(), posHudCountdown(), posHudFinishedCaption(),

  controlKeyAccelerate(fgeal::Keyboard::KEY_ARROW_UP),
  controlKeyBrake(fgeal::Keyboard::KEY_ARROW_DOWN),
  controlKeyTurnLeft(fgeal::Keyboard::KEY_ARROW_LEFT),
  controlKeyTurnRight(fgeal::Keyboard::KEY_ARROW_RIGHT),
  controlKeyShiftUp(fgeal::Keyboard::KEY_LEFT_SHIFT),
  controlKeyShiftDown(fgeal::Keyboard::KEY_LEFT_CONTROL),
  controlJoystickKeyAccelerate(2),
  controlJoystickKeyBrake(3),
  controlJoystickKeyShiftUp(5),
  controlJoystickKeyShiftDown(4),
  controlJoystickAxisTurn(0),

  debugMode(true)
{}

Pseudo3DRaceState::~Pseudo3DRaceState()
{
	if(fontSmall != null) delete fontSmall;
	if(fontTiny != null) delete fontTiny;
	if(fontCountdown != null) delete fontCountdown;
	if(font3 != null) delete font3;

	if(spriteBackground != null) delete spriteBackground;
	if(imgCacheTachometer != null) delete imgCacheTachometer;
	if(imgStopwatch != null) delete imgStopwatch;
	if(music != null) delete music;

	if(sndWheelspinBurnoutIntro != null) delete sndWheelspinBurnoutIntro;
	if(sndWheelspinBurnoutLoop != null) delete sndWheelspinBurnoutLoop;
	if(sndSideslipBurnoutIntro != null) delete sndSideslipBurnoutIntro;
	if(sndSideslipBurnoutLoop != null) delete sndSideslipBurnoutLoop;
	if(sndRunningOnDirtLoop != null) delete sndRunningOnDirtLoop;
	if(sndCrashImpact != null) delete sndCrashImpact;
	if(sndJumpImpact != null) delete sndJumpImpact;
	if(sndCountdownBuzzer != null) delete sndCountdownBuzzer;
	if(sndCountdownBuzzerFinal != null) delete sndCountdownBuzzerFinal;

	if(spriteSmoke != null) delete spriteSmoke;
	playerVehicle.smokeSprite = null;
}

void Pseudo3DRaceState::initialize()
{
	fontSmall = new Font(game.sharedResources->font1Path);
	fontTiny = new Font(game.sharedResources->font1Path);
	fontCountdown = new Font(game.sharedResources->font2Path);
	font3 = new Font(game.sharedResources->font1Path);

	imgStopwatch = new Image("assets/stopwatch.png");

	sndWheelspinBurnoutIntro = new Sound("assets/sound/tire_burnout_stand1_intro.ogg", game.logic.masterVolume);
	sndWheelspinBurnoutLoop = new Sound("assets/sound/tire_burnout_stand1_loop.ogg", game.logic.masterVolume);
	sndSideslipBurnoutIntro = new Sound("assets/sound/tire_burnout_normal1_intro.ogg", game.logic.masterVolume);
	sndSideslipBurnoutLoop = new Sound("assets/sound/tire_burnout_normal1_loop.ogg", game.logic.masterVolume);
	sndRunningOnDirtLoop = new Sound("assets/sound/on_gravel.ogg", game.logic.masterVolume);
	sndCrashImpact = new Sound("assets/sound/crash.ogg", game.logic.masterVolume);
	sndJumpImpact = new Sound("assets/sound/landing.ogg", game.logic.masterVolume);
	sndCountdownBuzzer = new Sound("assets/sound/countdown-buzzer.ogg", 0.8 * game.logic.masterVolume);
	sndCountdownBuzzerFinal = new Sound("assets/sound/countdown-buzzer-final.ogg", 0.8 * game.logic.masterVolume);

	spriteSmoke = new Sprite(new Image("assets/smoke-sprite.png"), 32, 32, 0.036, true);

	hudDialTachometer.graduationValueScale = 0.001;
	hudDialTachometer.graduationFont = fontSmall;

	hudDialSpeedometer.graduationFont = fontTiny;
	hudDialSpeedometer.angleMax = 1.5*M_PI;

	hudBarTachometer.fillColor = Color::RED;

	hudGearDisplay.font = fontSmall;
	hudGearDisplay.fontIsShared = true;
	hudGearDisplay.borderColor = Color::LIGHT_GREY;
	hudGearDisplay.backgroundColor = Color::BLACK;
	hudGearDisplay.specialCases[0] = "N";
	hudGearDisplay.specialCases[-1] = "R";

	hudSpeedometer.font = new Font(game.sharedResources->font2Path);
	hudSpeedometer.fontIsShared = false;
	hudSpeedometer.borderColor = Color::LIGHT_GREY;
	hudSpeedometer.backgroundColor = Color::BLACK;

	hudCurrentLap.font = font3;
	hudCurrentLap.fontIsShared = true;
	hudCurrentLap.disableBackground = true;
	hudCurrentLap.displayColor = Color::WHITE;

	hudLapCountGoal.font = font3;
	hudLapCountGoal.fontIsShared = true;
	hudLapCountGoal.disableBackground = true;
	hudLapCountGoal.displayColor = Color::WHITE;

	hudTimerCurrentLap.font = font3;
	hudTimerCurrentLap.fontIsShared = true;
	hudTimerCurrentLap.disableBackground = true;
	hudTimerCurrentLap.displayColor = Color::WHITE;

	hudTimerBestLap.font = font3;
	hudTimerBestLap.fontIsShared = true;
	hudTimerBestLap.disableBackground = true;
	hudTimerBestLap.displayColor = Color::WHITE;
	hudTimerBestLap.valueScale = 1000;

	hudMiniMapBgColor = Color::BLACK;
	hudMiniMapBgColor.a = 128;

	// loan some shared resources
	fontDev = &game.sharedResources->fontDev;
}

void Pseudo3DRaceState::onEnter()
{
	Display& display = game.getDisplay();
	const float displayWidth = display.getWidth(),
				displayHeight = display.getHeight();

	// reload fonts if display size changed
	if(lastDisplaySize.x != displayWidth or lastDisplaySize.y != displayHeight)
	{
		fontSmall->setFontSize(dip(10));
		fontTiny->setFontSize(dip(8));
		fontCountdown->setFontSize(dip(36));
		font3->setFontSize(dip(24));
		hudSpeedometer.font->setFontSize(dip(24));
		lastDisplaySize.x = displayWidth;
		lastDisplaySize.y = displayHeight;
	}

	settings = game.logic.getNextRaceSettings();
	simulationType = game.logic.getSimulationType();
	enableJumpSimulation = game.logic.isJumpSimulationEnabled();

	course.loadSpec(game.logic.getNextCourse());
	course.drawAreaWidth = displayWidth;
	course.drawAreaHeight = displayHeight;
	course.drawDistance = 300;
	course.cameraDepth = 0.84;
	course.lengthScale = coursePositionFactor;
	course.vehicles.clear();

	minimap = Pseudo3DCourse::Map(course.spec);

	if(spriteBackground != null)
		delete spriteBackground;

	Image* imgBackground = new Image(course.spec.landscapeFilename);
	spriteBackground = new Sprite(imgBackground, imgBackground->getWidth(), imgBackground->getHeight(), 0.25, true);
	spriteBackground->scale *= 0.2 * displayHeight / spriteBackground->height;

	bgColor = course.spec.colorLandscape;
	bgColorHorizon = course.spec.colorHorizon;

	if(music != null)
		delete music;

	if(not course.spec.musicFilename.empty())
	{
		music = new fgeal::Music(course.spec.musicFilename);
		music->setVolume(game.logic.masterVolume);
	}
	else
		music = null;

	if(not trafficVehicles.empty())
		trafficVehicles.clear();

	const unsigned trafficCount = settings.trafficDensity * (course.spec.lines.size() * course.spec.roadSegmentLength)/1000.f;
	if(trafficCount > 0)
	{
		trafficVehicles.resize(trafficCount);

		const vector<Pseudo3DVehicle::Spec>& trafficVehicleSpecs = game.logic.getTrafficVehicleList();

		// used to point to the vehicle instances that will "own" its respective assets and share with other vehicles with same spec/skin
		vector< vector<Pseudo3DVehicle*> > allSharedVehicles(trafficVehicleSpecs.size());
		for(unsigned i = 0; i < allSharedVehicles.size(); i++)
			allSharedVehicles[i].resize(trafficVehicleSpecs[i].alternateSprites.size()+1, null);

		for(unsigned i = 0; i < trafficCount; i++)
		{
			const unsigned trafficVehicleIndex = futil::random_between(0, trafficVehicleSpecs.size());
			const Pseudo3DVehicle::Spec& spec = trafficVehicleSpecs[trafficVehicleIndex];  // grab randomly chosen spec
			vector<Pseudo3DVehicle*>& sharedVehicles = allSharedVehicles[trafficVehicleIndex];  // grab list of "base" vehicles to use their assets
			const int skinIndex = spec.alternateSprites.empty()? -1 : futil::random_between(-1, spec.alternateSprites.size());
			Pseudo3DVehicle& trafficVehicle = trafficVehicles[i];
			trafficVehicle.setSpec(spec, skinIndex);

			// if first instance of this spec/skin, load assets and record a pointer
			if(sharedVehicles[skinIndex+1] == null)
			{
				trafficVehicle.loadAssetsData();
				sharedVehicles[skinIndex+1] = &trafficVehicle;
			}
			else  // if repeated spec/skin, use assets from other ("base") vehicle
				trafficVehicle.loadAssetsData(sharedVehicles[skinIndex+1]);

			// random parameters
			//FIXME number of lanes should be accounted for when deciding horizontal positions
			//FIXME road shoulder size should be accounted for when deciding horizontal positions
			trafficVehicle.position = futil::random_between_decimal(0.1, 0.9) * course.spec.lines.size() * course.spec.roadSegmentLength / coursePositionFactor;
			trafficVehicle.horizontalPosition = (futil::random_between(-2, 3)/2.f) * 0.825 * course.spec.roadWidth / coursePositionFactor;
			trafficVehicle.body.simulationType = simulationType;
			trafficVehicle.body.reset();
			trafficVehicle.body.engine.throttlePosition = futil::random_between_decimal(0.1, 0.4);
			trafficVehicle.body.automaticShiftingEnabled = true;
		}

		// apply screen scale to traffic sprites
		foreach(vector<Pseudo3DVehicle*>&, sharedVehiclesOfSpec, vector< vector<Pseudo3DVehicle*> >, allSharedVehicles)
			foreach(Pseudo3DVehicle*, sharedVehicle, vector<Pseudo3DVehicle*>, sharedVehiclesOfSpec)
				if(sharedVehicle != null)
					foreach(Sprite*, sprite, vector<Sprite*>, sharedVehicle->sprites)
						sprite->scale *= GLOBAL_VEHICLE_SCALE_FACTOR;

		foreach(Pseudo3DVehicle&, vehicle, vector<Pseudo3DVehicle>, trafficVehicles)
			course.vehicles.push_back(&vehicle);
	}

	playerVehicle.smokeSprite = null;
	playerVehicle.setSpec(game.logic.getPickedVehicle(), game.logic.getPickedVehicleAlternateSpriteIndex());
	playerVehicle.loadAssetsData();
	playerVehicle.smokeSprite = spriteSmoke;
	playerVehicle.engineSound.setVolume(game.logic.masterVolume);

	for(unsigned s = 0; s < playerVehicle.sprites.size(); s++)
		playerVehicle.sprites[s]->scale *= (displayWidth * GLOBAL_VEHICLE_SCALE_FACTOR);

	if(playerVehicle.brakelightSprite != null)
		playerVehicle.brakelightSprite->scale *= (displayWidth * GLOBAL_VEHICLE_SCALE_FACTOR);

	if(playerVehicle.shadowSprite != null)
		playerVehicle.shadowSprite->scale *= (displayWidth * GLOBAL_VEHICLE_SCALE_FACTOR);

	spriteSmoke->scale.x = spriteSmoke->scale.y = displayWidth * GLOBAL_VEHICLE_SCALE_FACTOR*0.75f;

	float gaugeDiameter = 0.15*std::max(displayWidth, displayHeight);
	Rectangle bounds = { displayWidth - 1.1f*gaugeDiameter, displayHeight - 1.2f*gaugeDiameter, gaugeDiameter, gaugeDiameter };

	hudDialTachometer.min = playerVehicle.body.engine.minRpm;
//	hudDialTachometer.max = playerVehicle.body.engine.maxRpm;
	hudDialTachometer.max = 1000.f * static_cast<int>((playerVehicle.body.engine.maxRpm+1000.f)/1000.f);
	hudDialTachometer.bounds = bounds;
	hudDialTachometer.graduationLevel = 3;
	hudDialTachometer.graduationPrimarySize = 1000.f * static_cast<int>(1+hudDialTachometer.max/13000.f);
	hudDialTachometer.graduationPrimaryLineSize = 0.5;
	hudDialTachometer.graduationValueOffset = (hudDialTachometer.graduationPrimarySize > 1000.f? -0.5f * hudDialTachometer.graduationPrimarySize : 0);
	hudDialTachometer.graduationSecondarySize = 0.5 * hudDialTachometer.graduationPrimarySize;
	hudDialTachometer.graduationSecondaryLineSize = 0.55;
	hudDialTachometer.graduationTertiarySize = 0.1 * hudDialTachometer.graduationPrimarySize;
	hudDialTachometer.graduationTertiaryLineSize = 0.3;
	hudDialTachometer.backgroundImage = null;
	hudDialTachometer.borderThickness = 0.01 * displayHeight;
	hudDialTachometer.boltRadius = 0.025 * displayHeight;
	if(hudDialTachometer.pointerImage != null)
	{
		delete hudDialTachometer.pointerImage;
		hudDialTachometer.pointerImage = null;
		hudDialTachometer.pointerOffset = 0;
	}
	if(not settings.hudTachometerPointerImageFilename.empty())
	{
		hudDialTachometer.pointerImage = new Image(settings.hudTachometerPointerImageFilename);
		hudDialTachometer.pointerOffset = 45;
	}
	hudDialTachometer.compile();

	hudDialSpeedometer.graduationValueScale = settings.isImperialUnit? MPS_TO_MPH : MPS_TO_KPH;
	hudDialSpeedometer.min = 0;
	hudDialSpeedometer.max = (((playerVehicle.body.getMaximumWheelAngularSpeed() * playerVehicle.body.tireRadius * hudDialSpeedometer.graduationValueScale) / 10 + 1) * 10) / hudDialSpeedometer.graduationValueScale;
	hudDialSpeedometer.bounds = bounds;
	hudDialSpeedometer.bounds.x -= hudDialTachometer.bounds.w + 0.05 * displayWidth;
	hudDialSpeedometer.bounds.w *= 1.33;
	hudDialSpeedometer.bounds.h *= 1.33;
	hudDialSpeedometer.graduationLevel = 2;
	hudDialSpeedometer.graduationPrimarySize = 20 / hudDialSpeedometer.graduationValueScale;
	hudDialSpeedometer.graduationPrimaryLineSize = 0.25;
	hudDialSpeedometer.graduationSecondarySize = 10 / hudDialSpeedometer.graduationValueScale;
	hudDialSpeedometer.graduationSecondaryLineSize = 0.4;
	hudDialSpeedometer.graduationValuePositionOffset = 0.375;
	hudDialSpeedometer.backgroundImage = null;
	hudDialSpeedometer.borderThickness = 0.01 * displayHeight;
	hudDialSpeedometer.boltRadius = 0.025 * displayHeight;
	if(hudDialSpeedometer.pointerImage != null)
	{
		delete hudDialSpeedometer.pointerImage;
		hudDialSpeedometer.pointerImage = null;
		hudDialSpeedometer.pointerOffset = 0;
	}
	if(not settings.hudTachometerPointerImageFilename.empty())
	{
		hudDialSpeedometer.pointerImage = new Image(settings.hudTachometerPointerImageFilename);
		hudDialSpeedometer.pointerOffset = 45;
	}
	hudDialSpeedometer.compile();

	hudBarTachometer.min = playerVehicle.body.engine.minRpm;
	hudBarTachometer.max = playerVehicle.body.engine.maxRpm;
	hudBarTachometer.bounds = bounds;
	hudBarTachometer.bounds.x *= 0.8;
	hudBarTachometer.bounds.y *= 1.15;
	hudBarTachometer.bounds.w *= 2;
	hudBarTachometer.bounds.h *= 0.125;
	hudBarTachometer.borderThickness = 0.01 * displayHeight;

	bounds.y = bounds.y + 0.7*bounds.h;
	bounds.x = bounds.x + 0.4*bounds.w;
	bounds.w = 0.04 * displayHeight;
	bounds.h = 1.5 * fontSmall->getHeight();
	hudGearDisplay.bounds = bounds;
	hudGearDisplay.borderThickness = 0.01 * displayHeight;

	if(true)
	{
		hudSpeedometer.font->setFontSize(dip(13));
		hudSpeedometer.bounds.w = hudSpeedometer.font->getTextWidth("0000");
		hudSpeedometer.bounds.h = 1.75f * hudSpeedometer.font->getHeight();
		hudSpeedometer.bounds.x = hudDialSpeedometer.bounds.x + 0.425f * hudDialSpeedometer.bounds.w;
		hudSpeedometer.bounds.y = hudDialSpeedometer.bounds.y + 0.6f * hudDialSpeedometer.bounds.h;
		hudSpeedometer.disableBackground = false;
		hudSpeedometer.displayColor = Color::GREEN;
		hudSpeedometer.borderThickness = 0.01f * displayHeight;
	}
	else
	{
		hudSpeedometer.font->setFontSize(dip(24));
		hudSpeedometer.bounds.x = hudDialTachometer.bounds.x - hudSpeedometer.font->getTextWidth("000");
		hudSpeedometer.bounds.y = bounds.y;
		hudSpeedometer.bounds.w = 3*bounds.w;
		hudSpeedometer.bounds.h = 1.7*bounds.h;
		hudSpeedometer.disableBackground = true;
		hudSpeedometer.displayColor = Color::WHITE;
		hudSpeedometer.borderThickness = 0;
	}
	hudSpeedometer.valueScale = settings.isImperialUnit? MPS_TO_MPH : MPS_TO_KPH;

	bounds.x = displayWidth - 1.1*hudTimerCurrentLap.font->getTextWidth("00:00:000");
	bounds.w *= 3;
	bounds.h *= 1.7;
	hudTimerCurrentLap.bounds = bounds;
	hudTimerCurrentLap.bounds.y = displayHeight * 0.01;
	hudTimerCurrentLap.valueScale = 1000;

	hudTimerBestLap.bounds = bounds;
	hudTimerBestLap.bounds.y = hudTimerCurrentLap.bounds.y + font3->getHeight()*1.05;

	hudCurrentLap.bounds = bounds;
	hudCurrentLap.bounds.y = hudTimerBestLap.bounds.y + font3->getHeight()*1.05;
	hudCurrentLap.bounds.w = hudCurrentLap.font->getTextWidth("999");

	hudLapCountGoal.bounds = bounds;
	hudLapCountGoal.bounds.x = hudCurrentLap.bounds.x + hudCurrentLap.bounds.w + hudCurrentLap.font->getTextWidth("/");
	hudLapCountGoal.bounds.y = hudCurrentLap.bounds.y;

	rightHudMargin = hudCurrentLap.bounds.x - font3->getTextWidth("999/999");
	offsetHudLapGoal = font3->getTextWidth("Laps: 999");

	stopwatchIconBounds.w = 0.032*displayWidth;
	stopwatchIconBounds.h = imgStopwatch->getHeight()*(stopwatchIconBounds.w/imgStopwatch->getWidth());
	stopwatchIconBounds.x = rightHudMargin - 1.2*stopwatchIconBounds.w;
	stopwatchIconBounds.y = hudTimerCurrentLap.bounds.y;

	posHudCountdown.x = 0.5f*(displayWidth - fontCountdown->getTextWidth("0"));
	posHudCountdown.y = 0.4f*(displayHeight - fontCountdown->getHeight());
	posHudFinishedCaption.x = 0.5f*(displayWidth - fontCountdown->getTextWidth("FINISHED"));
	posHudFinishedCaption.y = 0.4f*(displayHeight - fontCountdown->getHeight());

	if(settings.useCachedTachometer and not settings.useBarTachometer)
	{
		if(imgCacheTachometer != null)
		{
			delete imgCacheTachometer;
			imgCacheTachometer = null;
		}

		imgCacheTachometer = new Image(hudDialTachometer.bounds.w, hudDialTachometer.bounds.h);
		Graphics::setDrawTarget(imgCacheTachometer);
		Graphics::drawFilledRectangle(0, 0, imgCacheTachometer->getWidth(), imgCacheTachometer->getHeight(), Color::_TRANSPARENT);
		float oldx = hudDialTachometer.bounds.x, oldy = hudDialTachometer.bounds.y;
		hudDialTachometer.bounds.x = 0;
		hudDialTachometer.bounds.y = 0;
		hudDialTachometer.compile();
		hudDialTachometer.drawBackground();
		Graphics::setDefaultDrawTarget();
		hudDialTachometer.backgroundImage = imgCacheTachometer;
		hudDialTachometer.imagesAreShared = true;
		hudDialTachometer.graduationLevel = 0;
		hudDialTachometer.bounds.x = oldx;
		hudDialTachometer.bounds.y = oldy;
	}
	else
	{
		hudDialTachometer.backgroundImage = null;
	}
	hudDialTachometer.compile();

	minimap.roadColor = Color::GREY;
	minimap.bounds.x = hudTimerCurrentLap.bounds.x;
	minimap.bounds.y = 0.3*displayHeight;
	minimap.bounds.w = 0.1*displayWidth;
	minimap.bounds.h = 0.1*displayWidth;
	minimap.scale = fgeal::Vector2D();
	minimap.segmentHighlightColor = Color::YELLOW;
	minimap.segmentHighlightSize = 0.005f*displayWidth;
	minimap.geometryOtimizationEnabled = true;

	if(settings.raceType != RACE_TYPE_DEBUG)
	{
		onSceneIntro = true;
		onSceneFinish = false;
		timerSceneIntro = 4.5;
		countdownBuzzerCounter = 5;
		debugMode = false;
	}
	else
	{
		onSceneIntro = false;
		debugMode = true;
	}

	playerVehicle.corneringStiffness = 0.575 + 0.575/(1+exp(-0.4*(10.0 - (playerVehicle.body.mass*GRAVITY_ACCELERATION)/1000.0)));

	verticalBackgroundParallax = 0;
	playerVehicle.position = courseStartPositionOffset;
	playerVehicle.horizontalPosition = playerVehicle.verticalPosition = 0;
	playerVehicle.verticalSpeed = playerVehicle.strafeSpeed = 0;
	playerVehicle.virtualOrientation = 0;
	playerVehicle.body.simulationType = simulationType;
	playerVehicle.body.reset();
	playerVehicle.body.automaticShiftingEnabled = true;
	playerVehicle.pseudoAngle = 0;
	lapTimeCurrent = lapTimeBest = 0;
	lapCurrent = 1;
	acc0to60time = acc0to60clock = 0;

	playerVehicle.isTireBurnoutOccurring = playerVehicle.onAir = playerVehicle.onLongAir = false;

	if(music != null) music->loop();
	playerVehicle.engineSound.play();
}

void Pseudo3DRaceState::onLeave()
{
	playerVehicle.engineSound.halt();
	if(music != null) music->stop();
	sndSideslipBurnoutIntro->stop();
	sndSideslipBurnoutLoop->stop();
	sndWheelspinBurnoutIntro->stop();
	sndWheelspinBurnoutLoop->stop();
	sndRunningOnDirtLoop->stop();
	sndCountdownBuzzer->stop();
	sndCountdownBuzzerFinal->stop();
}

void Pseudo3DRaceState::render()
{
	const float displayWidth = course.drawAreaWidth,
				displayHeight = course.drawAreaHeight;

	game.getDisplay().clear();

	const Vector2D backgroundSize = { spriteBackground->width * spriteBackground->scale.x, spriteBackground->height * spriteBackground->scale.y };
	const float parallaxAbsoluteX = playerVehicle.virtualOrientation * BACKGROUND_HORIZONTAL_PARALLAX_FACTOR,
				parallaxAbsoluteY = verticalBackgroundParallax - backgroundSize.y + BACKGROUND_VERTICAL_PARALLAX_FACTOR * displayHeight,
				courseLength = course.spec.lines.size() * course.spec.roadSegmentLength / coursePositionFactor;

	Graphics::drawFilledRectangle(0, 0, displayWidth, displayHeight, bgColor);
	Graphics::drawFilledRectangle(0, parallaxAbsoluteY + backgroundSize.y, displayWidth, displayHeight, bgColorHorizon);

	for(float bgx = -backgroundSize.x * fractional_part(fabs(parallaxAbsoluteX)/backgroundSize.x); bgx < displayWidth; bgx += backgroundSize.x)
		spriteBackground->draw(bgx, parallaxAbsoluteY);

//	float cameraPosition = playerVehicle.position;  // gives better visual results regarding cornering, but causes glitch in collision, making it occur on visually wrong positions
	float cameraPosition = playerVehicle.position - playerVehicleProjectionOffset;
	while(cameraPosition < 0)  // course drawing method cannot receive negative position, take position modulus
		cameraPosition += courseLength;

	course.draw(cameraPosition * coursePositionFactor, playerVehicle.horizontalPosition * coursePositionFactor);

	playerVehicle.draw(0.5f * displayWidth, 0.83f * displayHeight - 0.01f * playerVehicle.verticalPosition, playerVehicle.pseudoAngle);

	Graphics::drawFilledRoundedRectangle(minimap.bounds, 5, hudMiniMapBgColor);
	minimap.drawMap(playerVehicle.position*coursePositionFactor/course.spec.roadSegmentLength);

	imgStopwatch->drawScaled(stopwatchIconBounds.x, stopwatchIconBounds.y, scaledToRect(imgStopwatch, stopwatchIconBounds));
	font3->drawText("Time:", rightHudMargin, hudTimerCurrentLap.bounds.y, Color::WHITE);
	hudTimerCurrentLap.draw();

	if(isRaceTypeLoop(settings.raceType))
	{
		font3->drawText("Lap", rightHudMargin, hudCurrentLap.bounds.y, Color::WHITE);
		hudCurrentLap.draw();

		if(settings.raceType == RACE_TYPE_LOOP_TIME_ATTACK)
		{
			font3->drawText("/", 1.025*rightHudMargin + offsetHudLapGoal, hudCurrentLap.bounds.y, Color::WHITE);
			hudLapCountGoal.draw();
		}

		font3->drawText("Best:", rightHudMargin, hudTimerBestLap.bounds.y, Color::WHITE);
		if(lapTimeBest == 0)
			font3->drawText("--", hudTimerBestLap.bounds.x, hudTimerBestLap.bounds.y, Color::WHITE);
		else
			hudTimerBestLap.draw();
	}
	else if(isRaceTypePointToPoint(settings.raceType))
	{
		const float progress = onSceneFinish? 100 : trunc(100.0 * (playerVehicle.position / courseLength));  // @suppress("Function cannot be resolved")
		font3->drawText("Complete " + futil::to_string(progress) + "%", rightHudMargin, hudTimerBestLap.bounds.y, Color::WHITE);
	}

	if(true)
	{
		hudDialSpeedometer.draw();
		fontSmall->drawText(settings.isImperialUnit? "mph" : "kph", hudDialSpeedometer.bounds.x + 0.5f*hudDialSpeedometer.bounds.w, hudDialSpeedometer.bounds.y + 0.75f*hudDialSpeedometer.bounds.h, fgeal::Color::BLACK);
	}
	else
		fontSmall->drawText(settings.isImperialUnit? "mph" : "kph", (hudSpeedometer.bounds.x + hudDialTachometer.bounds.x)/2, hudSpeedometer.bounds.y+hudSpeedometer.font->getHeight()*1.2f, fgeal::Color::WHITE);

	hudSpeedometer.draw();

	if(settings.useBarTachometer)
		hudBarTachometer.draw();
	else
		hudDialTachometer.draw();

	hudGearDisplay.draw();

	if(onSceneIntro)
	{
		if(timerSceneIntro >= 4);  //@suppress("Suspicious semicolon")
		else if(timerSceneIntro > 1)
			fontCountdown->drawText(futil::to_string((int) timerSceneIntro), posHudCountdown.x, posHudCountdown.y, Color::WHITE);
	}
	else if(timerSceneIntro > 0)
		fontCountdown->drawText("GO", posHudCountdown.x, posHudCountdown.y, Color::WHITE);

	else if(onSceneFinish)
		fontCountdown->drawText("FINISHED", posHudFinishedCaption.x, posHudFinishedCaption.y, Color::WHITE);

	// DEBUG
	if(debugMode)
		drawDebugInfo();
}

#define DEBUG_BUFFER_SIZE 512
void Pseudo3DRaceState::drawDebugInfo()
{
	static char buffer[DEBUG_BUFFER_SIZE]; static const unsigned size = DEBUG_BUFFER_SIZE;
	static string text;
	const float spacing = fontDev->getHeight(), spacingBig = 1.4f * spacing;
	Point offset = {spacing/2, spacing/2};

	fontDev->drawText("FPS:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%d", game.getFpsCount());
	fontDev->drawText(text=buffer, offset.x+30, offset.y, fgeal::Color::WHITE);


	offset.y += spacingBig;
	fontDev->drawText("Position:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fm", playerVehicle.position);
	fontDev->drawText(text=buffer, offset.x+65, offset.y, fgeal::Color::WHITE);

	fontDev->drawText("Horiz. position: ", offset.x+150, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fm", playerVehicle.horizontalPosition);
	fontDev->drawText(text=buffer, offset.x+265, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Speed:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fkm/h", playerVehicle.body.speed*3.6);
	fontDev->drawText(text=buffer, offset.x+65, offset.y, fgeal::Color::WHITE);

	fontDev->drawText("0-60mph: ", offset.x+150, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fs", acc0to60time);
	fontDev->drawText(text=buffer, offset.x+225, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Acc.:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fm/s^2", playerVehicle.body.acceleration);
	fontDev->drawText(text=buffer, offset.x+65, offset.y, fgeal::Color::WHITE);

	offset.y += spacingBig;
	fontDev->drawText("Height:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fm", playerVehicle.verticalPosition);
	fontDev->drawText(text=buffer, offset.x+65, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Vertical speed:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fm/s", playerVehicle.verticalSpeed);
	fontDev->drawText(text=buffer, offset.x+115, offset.y, fgeal::Color::WHITE);

	fontDev->drawText(playerVehicle.onAir? "(On air)" : "(On ground)", offset.x+195, offset.y, fgeal::Color::WHITE);

	offset.y += spacingBig;
	fontDev->drawText("Wheel turn pseudo angle:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2f", playerVehicle.pseudoAngle);
	fontDev->drawText(text=buffer, offset.x+225, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Slope angle:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2f", playerVehicle.body.slopeAngle);
	fontDev->drawText(text=buffer, offset.x+225, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Strafe speed:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fm/s", playerVehicle.strafeSpeed);
	fontDev->drawText(text=buffer, offset.x+155, offset.y, fgeal::Color::WHITE);


	offset.y += spacingBig;
	fontDev->drawText("Curve pull:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fm/s", playerVehicle.curvePull);
	fontDev->drawText(text=buffer, offset.x+175, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Slope pull:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fm/s^2", playerVehicle.body.slopePullForce);
	fontDev->drawText(text=buffer, offset.x+175, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Braking friction:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fN", playerVehicle.body.brakingForce);
	fontDev->drawText(text=buffer, offset.x+175, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Rolling friction:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fN", playerVehicle.body.rollingResistanceForce);
	fontDev->drawText(text=buffer, offset.x+175, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Air friction:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fN", playerVehicle.body.airDragForce);
	fontDev->drawText(text=buffer, offset.x+175, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Combined friction:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fN", (playerVehicle.curvePull + playerVehicle.body.slopePullForce + playerVehicle.body.brakingForce + playerVehicle.body.rollingResistanceForce + playerVehicle.body.airDragForce));
	fontDev->drawText(text=buffer, offset.x+175, offset.y, fgeal::Color::WHITE);


	offset.y += spacingBig;
	fontDev->drawText("Drive force:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fN", playerVehicle.body.getDriveForce());
	fontDev->drawText(text=buffer, offset.x+155, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Torque:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fNm", playerVehicle.body.engine.getCurrentTorque());
	fontDev->drawText(text=buffer, offset.x+155, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Torque proportion:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2f%%", 100.f*playerVehicle.body.engine.getCurrentTorque()/playerVehicle.body.engine.maximumTorque);
	fontDev->drawText(text=buffer, offset.x+155, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Power:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fhp", (playerVehicle.body.engine.getCurrentTorque()*playerVehicle.body.engine.rpm)/(5252.0 * 1.355818));
	fontDev->drawText(text=buffer, offset.x+155, offset.y, fgeal::Color::WHITE);


	offset.y += spacingBig;
	fontDev->drawText("Driven tires load:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fN", playerVehicle.body.getDrivenWheelsWeightLoad());
	fontDev->drawText(text=buffer, offset.x+155, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Downforce:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2fN", -playerVehicle.body.downforce);
	fontDev->drawText(text=buffer, offset.x+155, offset.y, fgeal::Color::WHITE);

	offset.y += spacingBig;
	fontDev->drawText("Wheel Ang. Speed:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2frad/s", playerVehicle.body.wheelAngularSpeed);
	fontDev->drawText(text=buffer, offset.x+155, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("RPM:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.f", playerVehicle.body.engine.rpm);
	fontDev->drawText(text=buffer, offset.x+30, offset.y, fgeal::Color::WHITE);

	offset.y += spacing;
	fontDev->drawText("Gear:", offset.x, offset.y, fgeal::Color::WHITE);
	const char* autoLabelTxt = (playerVehicle.body.automaticShiftingEnabled? " (auto)":"");
	snprintf(buffer, size, "%d %s", playerVehicle.body.engine.gear, autoLabelTxt);
	fontDev->drawText(text=buffer, offset.x+35, offset.y, fgeal::Color::WHITE);

	offset.y += spacingBig;
	fontDev->drawText("Slip ratio:", offset.x, offset.y, fgeal::Color::WHITE);
	snprintf(buffer, size, "%2.2f%%", 100*playerVehicle.body.slipRatio);
	fontDev->drawText(text=buffer, offset.x+155, offset.y, fgeal::Color::WHITE);


	unsigned currentRangeIndex = playerVehicle.engineSound.getRangeIndex(playerVehicle.body.engine.rpm);
	for(unsigned i = 0; i < playerVehicle.engineSound.getSoundData().size(); i++)
	{
		const std::string format = std::string(playerVehicle.engineSound.getSoundData()[i]->isPlaying()==false? " s%u " : currentRangeIndex==i? "[s%u]" : "(s%u)") + " vol: %2.2f pitch: %2.2f";
		snprintf(buffer, size, format.c_str(), i, playerVehicle.engineSound.getSoundData()[i]->getVolume(), playerVehicle.engineSound.getSoundData()[i]->getPlaybackSpeed());
		fontDev->drawText(text=buffer, game.getDisplay().getWidth() - 200, game.getDisplay().getHeight()/2.0 - i*spacing, fgeal::Color::WHITE);
	}
}

static const float LONGITUDINAL_SLIP_RATIO_BURN_RUBBER = 0.2,  // 20%
		MAXIMUM_PHYSICS_DELTA_TIME = 0.01;

void Pseudo3DRaceState::update(float delta)
{
	float physicsDelta = delta;
	while(physicsDelta > MAXIMUM_PHYSICS_DELTA_TIME)  // process delta too large in smaller delta steps
	{
		handlePhysics(MAXIMUM_PHYSICS_DELTA_TIME);
		physicsDelta -= MAXIMUM_PHYSICS_DELTA_TIME;
	}
	handlePhysics(physicsDelta);

	// course looping control
	const float courseLength = course.spec.lines.size() * course.spec.roadSegmentLength / coursePositionFactor;
	const bool courseEndReached = (playerVehicle.position >= courseLength);
	if(courseEndReached) while(playerVehicle.position >= courseLength)  // position larger than course length not allowed, take position modulus
		playerVehicle.position -= courseLength;

	while(playerVehicle.position < 0)  // negative position is not allowed, take backwards position modulus
		playerVehicle.position += courseLength;

	// update bg parallax
	verticalBackgroundParallax -= 2*playerVehicle.body.slopeAngle;

	// scene control
	if(onSceneIntro)
	{
		timerSceneIntro -= delta;

		if(countdownBuzzerCounter - timerSceneIntro > 1)
		{
			sndCountdownBuzzer->play();
			countdownBuzzerCounter--;

			if(countdownBuzzerCounter == 2)  // do not play at last call
				countdownBuzzerCounter = 0;
		}

		if(timerSceneIntro < 1)
		{
			onSceneIntro = false;
			playerVehicle.body.shiftGear(1);
			sndCountdownBuzzerFinal->play();
		}
	}
	else
	{
		lapTimeCurrent += delta;
		if(timerSceneIntro > 0)
			timerSceneIntro -= delta;
	}

	if(onSceneFinish)
	{
		timerSceneFinish -= delta;
		if(timerSceneFinish < 1)
		{
			onSceneFinish = false;
			if(game.logic.raceOnlyMode)
				game.running = false;
			else
				game.enterState(game.logic.currentMainMenuStateId);
		}
	}

	if(courseEndReached and not onSceneFinish)
	{
		if(isRaceTypeLoop(settings.raceType))
		{
			lapCurrent++;
			if(lapTimeCurrent < lapTimeBest or lapTimeBest == 0)
				lapTimeBest = lapTimeCurrent;
			lapTimeCurrent = 0;

			if(settings.raceType == RACE_TYPE_LOOP_TIME_ATTACK)
			{
				if(lapCurrent > settings.lapCountGoal)
				{
					onSceneFinish = true;
					timerSceneFinish = 8.0;
					lapCurrent--;
				}
			}
		}
		else if(isRaceTypePointToPoint(settings.raceType))
		{
			onSceneFinish = true;
			timerSceneFinish = 8.0;
		}
	}

	// engine sound control
	playerVehicle.engineSound.update(playerVehicle.body.engine.rpm);

	// 0-60 time control (debug)
	if(acc0to60time == 0)
	{
		if(playerVehicle.body.engine.throttlePosition > 0 and playerVehicle.body.speed > 0 and acc0to60clock == 0)
			acc0to60clock = fgeal::uptime();
		else if(playerVehicle.body.engine.throttlePosition < 0 and acc0to60clock != 0)
			acc0to60clock = 0;
		else if(playerVehicle.body.engine.throttlePosition > 0 and playerVehicle.body.speed * 3.6 > 96)
			acc0to60time = fgeal::uptime() - acc0to60clock;
	}

	// wheelspin logic control
	const bool isPlayerWheelspinOccurring = (
		(playerVehicle.body.simulationType == Mechanics::SIMULATION_TYPE_SLIPLESS
			and
			(
				// fake burnout mode
				playerVehicle.body.engine.gear == 1
				and playerVehicle.body.engine.rpm < 0.5*playerVehicle.body.engine.maxRpm
				and isPlayerAccelerating()
			)
		)
		or
		(playerVehicle.body.simulationType == Mechanics::SIMULATION_TYPE_WHEEL_LOAD_CAP
			and
			(
				// burnout based on capped drive force
				(playerVehicle.body.getDriveForce() < 0.75 * playerVehicle.body.engine.getDriveTorque() / playerVehicle.body.tireRadius)
				and playerVehicle.body.engine.gear == 1  // but limited to first gear.
			)
		)
		or
		(playerVehicle.body.simulationType == Mechanics::SIMULATION_TYPE_PACEJKA_BASED
			and
			(
				// burnout based on real slip ratio
				fabs(playerVehicle.body.slipRatio) > LONGITUDINAL_SLIP_RATIO_BURN_RUBBER
				and fabs(playerVehicle.body.speed) > 1.0
			)
		)
	);

	const bool isPlayerSideslipOccurring = (
			fabs(playerVehicle.body.speed) > MINIMUM_SPEED_TO_SIDESLIP
		and MAXIMUM_STRAFE_SPEED_FACTOR * playerVehicle.corneringStiffness - fabs(playerVehicle.strafeSpeed) < 1
	);

	if(isPlayerWheelspinOccurring and getCurrentSurfaceType() == SURFACE_TYPE_DRY_ASPHALT)
	{
		if(sndSideslipBurnoutIntro->isPlaying()) sndSideslipBurnoutIntro->stop();
		if(sndSideslipBurnoutLoop->isPlaying()) sndSideslipBurnoutLoop->stop();

		if(not playerVehicle.isTireBurnoutOccurring)
			sndWheelspinBurnoutIntro->play();
		else if(not sndWheelspinBurnoutIntro->isPlaying() and not sndWheelspinBurnoutLoop->isPlaying())
			sndWheelspinBurnoutLoop->loop();

		playerVehicle.isTireBurnoutOccurring = true;
	}
	else if(isPlayerSideslipOccurring and getCurrentSurfaceType() == SURFACE_TYPE_DRY_ASPHALT)
	{
		if(sndWheelspinBurnoutIntro->isPlaying()) sndWheelspinBurnoutIntro->stop();
		if(sndWheelspinBurnoutLoop->isPlaying()) sndWheelspinBurnoutLoop->stop();

		if(not playerVehicle.isTireBurnoutOccurring)
			sndSideslipBurnoutIntro->play();
		else if(not sndSideslipBurnoutIntro->isPlaying() and not sndSideslipBurnoutLoop->isPlaying())
			sndSideslipBurnoutLoop->loop();

		playerVehicle.isTireBurnoutOccurring = true;
	}
	else
	{
		if(sndWheelspinBurnoutIntro->isPlaying()) sndWheelspinBurnoutIntro->stop();
		if(sndWheelspinBurnoutLoop->isPlaying()) sndWheelspinBurnoutLoop->stop();
		if(sndSideslipBurnoutIntro->isPlaying()) sndSideslipBurnoutIntro->stop();
		if(sndSideslipBurnoutLoop->isPlaying()) sndSideslipBurnoutLoop->stop();
		playerVehicle.isTireBurnoutOccurring = false;
	}

	if(getCurrentSurfaceType() != SURFACE_TYPE_DRY_ASPHALT and fabs(playerVehicle.body.speed) > 1)
	{
		if(not sndRunningOnDirtLoop->isPlaying())
			sndRunningOnDirtLoop->loop();
	}
	else if(sndRunningOnDirtLoop->isPlaying())
		sndRunningOnDirtLoop->stop();

	if(playerVehicle.isCrashing)
	{
		if(not sndCrashImpact->isPlaying())
			sndCrashImpact->play();

		playerVehicle.isCrashing = false;
	}
}

void Pseudo3DRaceState::onKeyPressed(Keyboard::Key key)
{
	if(key == controlKeyShiftUp)
		shiftGear(playerVehicle.body.engine.gear+1);
	else if(key == controlKeyShiftDown)
		shiftGear(playerVehicle.body.engine.gear-1);

	else switch(key)
	{
		case Keyboard::KEY_ESCAPE:
			if(game.logic.raceOnlyMode)
				game.running = false;
			else
				game.enterState(game.logic.currentMainMenuStateId);
			break;
		case Keyboard::KEY_R:
			playerVehicle.position = courseStartPositionOffset;
			playerVehicle.horizontalPosition = playerVehicle.verticalPosition = 0;
			playerVehicle.verticalSpeed = 0;
			playerVehicle.body.reset();
			playerVehicle.pseudoAngle = 0;
			verticalBackgroundParallax = 0;
			lapTimeCurrent = 0;
			lapCurrent = 1;
			onSceneIntro = true;
			timerSceneIntro = 4.5;
			countdownBuzzerCounter = 5;
			playerVehicle.onAir = playerVehicle.onLongAir = false;
			acc0to60time = acc0to60clock = 0;
			break;
		case Keyboard::KEY_T:
			playerVehicle.body.automaticShiftingEnabled = !playerVehicle.body.automaticShiftingEnabled;
			break;
		case Keyboard::KEY_M:
			if(music != null)
			{
				if(music->isPlaying())
					music->pause();
				else
					music->resume();
			}
			break;
		case Keyboard::KEY_D:
			debugMode = !debugMode;
			break;
		case Keyboard::KEY_PAGE_UP:
			course.drawDistance++;
			break;
		case Keyboard::KEY_PAGE_DOWN:
			course.drawDistance--;
			break;
		case Keyboard::KEY_O:
			course.cameraDepth += 0.1;
			break;
		case Keyboard::KEY_L:
			course.cameraDepth -= 0.1;
			break;
		default:
			break;
	}
}

void Pseudo3DRaceState::onJoystickButtonPressed(unsigned joystick, unsigned button)
{
	if(button == controlJoystickKeyShiftUp)
		shiftGear(playerVehicle.body.engine.gear+1);
	else if(button == controlJoystickKeyShiftDown)
		shiftGear(playerVehicle.body.engine.gear-1);
}

bool Pseudo3DRaceState::isPlayerAccelerating()
{
	if(onSceneFinish) return false;
	return Keyboard::isKeyPressed(controlKeyAccelerate)
			or (Joystick::getCount() > 0 and Joystick::isButtonPressed(0, controlJoystickKeyAccelerate));
}

bool Pseudo3DRaceState::isPlayerBraking()
{
	if(onSceneFinish) return true;
	return Keyboard::isKeyPressed(controlKeyBrake)
			or (Joystick::getCount() > 0 and Joystick::isButtonPressed(0, controlJoystickKeyBrake));
}

bool Pseudo3DRaceState::isPlayerSteeringLeft()
{
	return Keyboard::isKeyPressed(controlKeyTurnLeft)
			or (Joystick::getCount() > 0 and Joystick::getAxisPosition(0, controlJoystickAxisTurn) < -0.2);
}

bool Pseudo3DRaceState::isPlayerSteeringRight()
{
	return Keyboard::isKeyPressed(controlKeyTurnRight)
			or (Joystick::getCount() > 0 and Joystick::getAxisPosition(0, controlJoystickAxisTurn) > 0.2);
}
