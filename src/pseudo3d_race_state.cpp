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

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdlib>

using std::string;
using std::map;
using std::vector;

using fgeal::Display;
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
using fgeal::Joystick;
using fgeal::Rectangle;

#define GRAVITY_ACCELERATION Mechanics::GRAVITY_ACCELERATION

static const float MINIMUM_SPEED_TO_SIDESLIP = 5.5556;  // == 20kph
const float Pseudo3DRaceState::MAXIMUM_STRAFE_SPEED_FACTOR = 30;  // undefined unit
static const float GLOBAL_VEHICLE_SCALE_FACTOR = 0.0048828125;

static const float BACKGROUND_POSITION_FACTOR = 0.509375;

#if __cplusplus < 201103L
	double trunc(double d){ return (d>0) ? floor(d) : ceil(d) ; }
#endif

// -------------------------------------------------------------------------------

int Pseudo3DRaceState::getId(){ return CarseGame::RACE_STATE_ID; }

Pseudo3DRaceState::Pseudo3DRaceState(CarseGame* game)
: State(*game), game(*game), lastDisplaySize(),
  fontSmall(null), fontCountdown(null), font3(null), fontDev(null),
  imgBackground(null), imgCacheTachometer(null), imgStopwatch(null),
  music(null),
  sndWheelspinBurnoutIntro(null), sndWheelspinBurnoutLoop(null),
  sndSideslipBurnoutIntro(null), sndSideslipBurnoutLoop(null),
  sndRunningOnDirtLoop(null), sndJumpImpact(null),
  sndCountdownBuzzer(null), sndCountdownBuzzerFinal(null),

  bgColor(), bgColorHorizon(),
  spriteSmokeLeft(null), spriteSmokeRight(null),

  parallax(), backgroundScale(),

  coursePositionFactor(500), simulationType(),
  onSceneIntro(), onSceneFinish(), timerSceneIntro(), timerSceneFinish(), countdownBuzzerCounter(), settings(),
  lapTimeCurrent(0), lapTimeBest(0), lapCurrent(0), acc0to60clock(0), acc0to60time(0),

  hudDialTachometer(playerVehicle.body.engine.rpm, 0, 0, Rectangle()),
  hudBarTachometer(playerVehicle.body.engine.rpm, 0, 0, Rectangle()),
  hudSpeedometer(playerVehicle.body.speed, Rectangle(), null),
  hudGearDisplay(playerVehicle.body.engine.gear, Rectangle(), null),
  hudTimerCurrentLap(lapTimeCurrent, Rectangle(), null),
  hudTimerBestLap(lapTimeBest, Rectangle(), null),
  hudCurrentLap(lapCurrent, Rectangle(), null),
  hudLapCountGoal(settings.lapCountGoal, Rectangle(), null),

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
	// todo delete sounds
	if(fontSmall != null) delete fontSmall;
	if(fontCountdown != null) delete fontCountdown;
	if(font3 != null) delete font3;

	if(imgBackground != null) delete imgBackground;
	if(imgCacheTachometer != null) delete imgCacheTachometer;
	if(imgStopwatch != null) delete imgStopwatch;
	if(music != null) delete music;

	if(sndWheelspinBurnoutIntro != null) delete sndWheelspinBurnoutIntro;
	if(sndWheelspinBurnoutLoop != null) delete sndWheelspinBurnoutLoop;
	if(sndSideslipBurnoutIntro != null) delete sndSideslipBurnoutIntro;
	if(sndSideslipBurnoutLoop != null) delete sndSideslipBurnoutLoop;
	if(sndRunningOnDirtLoop != null) delete sndRunningOnDirtLoop;
	if(sndJumpImpact != null) delete sndJumpImpact;
	if(sndCountdownBuzzer != null) delete sndCountdownBuzzer;
	if(sndCountdownBuzzerFinal != null) delete sndCountdownBuzzerFinal;

	if(spriteSmokeLeft != null) delete spriteSmokeLeft;
	if(spriteSmokeRight != null) delete spriteSmokeRight;
}

void Pseudo3DRaceState::initialize()
{
	fontSmall = new Font(game.sharedResources->font1Path);
	fontCountdown = new Font(game.sharedResources->font2Path);
	font3 = new Font(game.sharedResources->font1Path);

	imgStopwatch = new Image("assets/stopwatch.png");

	sndWheelspinBurnoutIntro = new Sound("assets/sound/tire_burnout_stand1_intro.ogg");
	sndWheelspinBurnoutLoop = new Sound("assets/sound/tire_burnout_stand1_loop.ogg");
	sndSideslipBurnoutIntro = new Sound("assets/sound/tire_burnout_normal1_intro.ogg");
	sndSideslipBurnoutLoop = new Sound("assets/sound/tire_burnout_normal1_loop.ogg");
	sndRunningOnDirtLoop = new Sound("assets/sound/on_gravel.ogg");
	sndJumpImpact = new Sound("assets/sound/landing.ogg");
	sndCountdownBuzzer = new Sound("assets/sound/countdown-buzzer.ogg");
	sndCountdownBuzzerFinal = new Sound("assets/sound/countdown-buzzer-final.ogg");

	sndCountdownBuzzer->setVolume(0.8);
	sndCountdownBuzzerFinal->setVolume(0.8);

	Image* smokeSpriteSheet = new Image("assets/smoke-sprite.png");
	spriteSmokeLeft = new Sprite(smokeSpriteSheet, 32, 32, 0.25, -1, 0, 0, true);
	spriteSmokeRight = new Sprite(smokeSpriteSheet, 32, 32, 0.25);
	spriteSmokeRight->flipmode = Image::FLIP_HORIZONTAL;

	hudDialTachometer.borderThickness = 6;
	hudDialTachometer.graduationLevel = 2;
	hudDialTachometer.graduationPrimarySize = 1000;
	hudDialTachometer.graduationSecondarySize = 100;
	hudDialTachometer.graduationValueScale = 0.001;
	hudDialTachometer.graduationFont = fontSmall;

	hudBarTachometer.borderThickness = 6;
	hudBarTachometer.fillColor = Color::RED;

	hudGearDisplay.font = fontSmall;
	hudGearDisplay.fontIsShared = true;
	hudGearDisplay.borderThickness = 6;
	hudGearDisplay.borderColor = Color::LIGHT_GREY;
	hudGearDisplay.backgroundColor = Color::BLACK;
	hudGearDisplay.specialCases[0] = "N";
	hudGearDisplay.specialCases[-1] = "R";

	hudSpeedometer.font = new Font(game.sharedResources->font2Path);
	hudSpeedometer.fontIsShared = false;
	hudSpeedometer.disableBackground = true;
	hudSpeedometer.displayColor = Color::WHITE;
	hudSpeedometer.borderThickness = 0;

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

	// reload fonts if display size changed
	if(lastDisplaySize.x != display.getWidth() or lastDisplaySize.y != display.getHeight())
	{
		fontSmall->setFontSize(dip(10));
		fontCountdown->setFontSize(dip(36));
		font3->setFontSize(dip(24));
		hudSpeedometer.font->setFontSize(dip(24));
		lastDisplaySize.x = display.getWidth();
		lastDisplaySize.y = display.getHeight();
	}

	settings = game.logic.getNextRaceSettings();
	simulationType = game.logic.getSimulationType();

	course.freeAssetsData();
	course = Pseudo3DCourse(game.logic.getNextCourse());
	course.loadAssetsData();

	course.drawAreaWidth = display.getWidth();
	course.drawAreaHeight = display.getHeight();
	course.drawDistance = 300;
	course.cameraDepth = 0.84;
	course.coursePositionFactor = coursePositionFactor;

	minimap = Pseudo3DCourse::Map(course.spec);

	if(imgBackground != null)
		delete imgBackground;

	imgBackground = new Image(course.spec.landscapeFilename);

	backgroundScale = 0.2 * display.getHeight() / (float) imgBackground->getHeight();

	bgColor = course.spec.colorLandscape;
	bgColorHorizon = course.spec.colorHorizon;

	if(music != null)
		delete music;

	if(not course.spec.musicFilename.empty())
		music = new fgeal::Music(course.spec.musicFilename);
	else
		music = null;

	if(not trafficVehicles.empty())
		trafficVehicles.clear();
	course.trafficVehicles = null;

	if(course.spec.trafficCount > 0)
	{
		trafficVehicles.reserve(course.spec.trafficCount);  // NEEDED TO AVOID THE VEHICLE'S DESTRUCTOR BEING CALLED BY STD::VECTOR (INSERTING ELEMENTS CAN CAUSE REALOCATION)
		Pseudo3DVehicle::Spec carSpec, suvSpec;
		carSpec.loadFromFile("data/traffic/car1.properties");
		suvSpec.loadFromFile("data/traffic/suv1.properties");

		// used to point to the vehicle instances that will "own" its respective assets and share with other vehicles with same spec/skin
		vector<Pseudo3DVehicle*> sharedVehicleCar(carSpec.alternateSprites.size()+1, null),
								 sharedVehicleSuv(suvSpec.alternateSprites.size()+1, null);

		for(unsigned i = 0; i < course.spec.trafficCount; i++)
		{
			const bool useSUV = (rand() % 2 == 0);
			const Pseudo3DVehicle::Spec& spec = useSUV? suvSpec : carSpec;  // grab chosen spec
			vector<Pseudo3DVehicle*>& sharedVehicles = useSUV? sharedVehicleSuv : sharedVehicleCar;  // grab list of "base" vehicles to use their assets
			const int skinIndex = spec.alternateSprites.empty()? -1 : futil::random_between(-1, spec.alternateSprites.size());
			trafficVehicles.push_back(Pseudo3DVehicle(spec, skinIndex));
			Pseudo3DVehicle& trafficVehicle = trafficVehicles.back();

			// if first instance of this spec/skin, load assets and record a pointer
			if(sharedVehicles[skinIndex+1] == null)
			{
				trafficVehicle.loadAssetsData();
				sharedVehicles[skinIndex+1] = &trafficVehicle;
			}
			else  // if repeated spec/skin, use assets from other ("base") vehicle
				trafficVehicle.loadAssetsData(sharedVehicles[skinIndex+1]);

			trafficVehicle.position = futil::random_between(500, course.spec.lines.size());
			trafficVehicle.horizontalPosition = futil::random_between_decimal(-1, 1)*coursePositionFactor;
			trafficVehicle.body.engine.throttlePosition = futil::random_between_decimal(0.1, 0.3);

			for(unsigned s = 0; s < trafficVehicle.sprites.size(); s++)
				trafficVehicle.sprites[s]->scale *= (display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR);
		}

		course.trafficVehicles = &trafficVehicles;
	}

	playerVehicle.freeAssetsData();
	playerVehicle = Pseudo3DVehicle(game.logic.getPickedVehicle(), game.logic.getPickedVehicleAlternateSpriteIndex());
	playerVehicle.loadAssetsData();

	for(unsigned s = 0; s < playerVehicle.sprites.size(); s++)
		playerVehicle.sprites[s]->scale *= (display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR);

	if(playerVehicle.brakelightSprite != null)
		playerVehicle.brakelightSprite->scale *= (display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR);

	if(playerVehicle.shadowSprite != null)
		playerVehicle.shadowSprite->scale *= (display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR);

	spriteSmokeLeft->scale.x =
			spriteSmokeLeft->scale.y =
					spriteSmokeRight->scale.x =
							spriteSmokeRight->scale.y = display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR*0.75f;

	float gaugeDiameter = 0.15*std::max(display.getWidth(), display.getHeight());
	Rectangle gaugeSize = { display.getWidth() - 1.1f*gaugeDiameter, display.getHeight() - 1.2f*gaugeDiameter, gaugeDiameter, gaugeDiameter };

	hudDialTachometer.min = playerVehicle.body.engine.minRpm;
//	hudTachometer.max = playerVehicle.body.engine.maxRpm;
	hudDialTachometer.max = 1000.f * static_cast<int>((playerVehicle.body.engine.maxRpm+1000.f)/1000.f);
	hudDialTachometer.bounds = gaugeSize;
	hudDialTachometer.graduationLevel = 2;
	hudDialTachometer.backgroundImage = null;
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

	hudBarTachometer.min = playerVehicle.body.engine.minRpm;
	hudBarTachometer.max = playerVehicle.body.engine.maxRpm;
	hudBarTachometer.bounds = gaugeSize;
	hudBarTachometer.bounds.x *= 0.8;
	hudBarTachometer.bounds.y *= 1.15;
	hudBarTachometer.bounds.w *= 2;
	hudBarTachometer.bounds.h *= 0.125;

	gaugeSize.y = gaugeSize.y + 0.7*gaugeSize.h;
	gaugeSize.x = gaugeSize.x + 0.4*gaugeSize.w;
	gaugeSize.w = 24;
	gaugeSize.h = 1.5 * fontSmall->getHeight();
	hudGearDisplay.bounds = gaugeSize;

	gaugeSize.x = hudDialTachometer.bounds.x - hudSpeedometer.font->getTextWidth("000");
	gaugeSize.w *= 3;
	gaugeSize.h *= 1.7;
	hudSpeedometer.bounds = gaugeSize;
	hudSpeedometer.valueScale = settings.isImperialUnit? 2.25 : 3.6;

	gaugeSize.x = display.getWidth() - 1.1*hudTimerCurrentLap.font->getTextWidth("00:00:000");
	hudTimerCurrentLap.bounds = gaugeSize;
	hudTimerCurrentLap.bounds.y = display.getHeight() * 0.01;
	hudTimerCurrentLap.valueScale = 1000;

	hudTimerBestLap.bounds = gaugeSize;
	hudTimerBestLap.bounds.y = hudTimerCurrentLap.bounds.y + font3->getHeight()*1.05;

	hudCurrentLap.bounds = gaugeSize;
	hudCurrentLap.bounds.y = hudTimerBestLap.bounds.y + font3->getHeight()*1.05;
	hudCurrentLap.bounds.w = hudCurrentLap.font->getTextWidth("999");

	hudLapCountGoal.bounds = gaugeSize;
	hudLapCountGoal.bounds.x = hudCurrentLap.bounds.x + hudCurrentLap.bounds.w + hudCurrentLap.font->getTextWidth("/");
	hudLapCountGoal.bounds.y = hudCurrentLap.bounds.y;

	rightHudMargin = hudCurrentLap.bounds.x - font3->getTextWidth("999/999");
	offsetHudLapGoal = font3->getTextWidth("Laps: 999");

	stopwatchIconBounds.w = 0.032*display.getWidth();
	stopwatchIconBounds.h = imgStopwatch->getHeight()*(stopwatchIconBounds.w/imgStopwatch->getWidth());
	stopwatchIconBounds.x = rightHudMargin - 1.2*stopwatchIconBounds.w;
	stopwatchIconBounds.y = hudTimerCurrentLap.bounds.y;

	posHudCountdown.x = 0.5f*(display.getWidth() - fontCountdown->getTextWidth("0"));
	posHudCountdown.y = 0.4f*(display.getHeight() - fontCountdown->getHeight());
	posHudFinishedCaption.x = 0.5f*(display.getWidth() - fontCountdown->getTextWidth("FINISHED"));
	posHudFinishedCaption.y = 0.4f*(display.getHeight() - fontCountdown->getHeight());

	if(settings.useCachedTachometer and not settings.useBarTachometer)
	{
		if(imgCacheTachometer != null)
		{
			delete imgCacheTachometer;
			imgCacheTachometer = null;
		}

		imgCacheTachometer = new Image(hudDialTachometer.bounds.w, hudDialTachometer.bounds.h);
		fgeal::Graphics::setDrawTarget(imgCacheTachometer);
		fgeal::Graphics::drawFilledRectangle(0, 0, imgCacheTachometer->getWidth(), imgCacheTachometer->getHeight(), Color::_TRANSPARENT);
		float oldx = hudDialTachometer.bounds.x, oldy = hudDialTachometer.bounds.y;
		hudDialTachometer.bounds.x = 0;
		hudDialTachometer.bounds.y = 0;
		hudDialTachometer.compile();
		hudDialTachometer.drawBackground();
		fgeal::Graphics::setDefaultDrawTarget();
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
	minimap.bounds.y = 0.3*display.getHeight();
	minimap.bounds.w = 0.1*display.getWidth();
	minimap.bounds.h = 0.1*display.getWidth();
	minimap.scale = fgeal::Vector2D();
	minimap.segmentHighlightColor = Color::YELLOW;
	minimap.segmentHighlightSize = 0.005f*display.getWidth();
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

	parallax.x = parallax.y = 0;
	playerVehicle.position = 0;
	playerVehicle.horizontalPosition = playerVehicle.verticalPosition = 0;
//	verticalSpeed = 0;
	playerVehicle.body.simulationType = simulationType;
	playerVehicle.body.reset();
	playerVehicle.body.automaticShiftingEnabled = true;
	playerVehicle.pseudoAngle = 0;
	lapTimeCurrent = lapTimeBest = 0;
	lapCurrent = 1;
	acc0to60time = acc0to60clock = 0;

	playerVehicle.isTireBurnoutOccurring = /*onAir = onLongAir =*/ false;

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

	const float parallaxAbsoluteY = parallax.y + BACKGROUND_POSITION_FACTOR*displayHeight - imgBackground->getHeight()*backgroundScale;

	fgeal::Graphics::drawFilledRectangle(0, 0, displayWidth, displayHeight, bgColor);
	fgeal::Graphics::drawFilledRectangle(0, parallaxAbsoluteY + imgBackground->getHeight()*backgroundScale, displayWidth, displayHeight, bgColorHorizon);

	for(float bg = 0; bg < 3*displayWidth; bg += imgBackground->getWidth())
		imgBackground->drawScaled(parallax.x + bg, parallaxAbsoluteY, 1, backgroundScale);

	course.draw(playerVehicle.position * coursePositionFactor, playerVehicle.horizontalPosition);

	const fgeal::Point vehicleSpritePosition = {
			0.5f*displayWidth,  // x coord
			0.75f*displayHeight - playerVehicle.verticalPosition*0.01f  // y coord
	};

	playerVehicle.draw(vehicleSpritePosition, playerVehicle.pseudoAngle);

	fgeal::Graphics::drawFilledRoundedRectangle(minimap.bounds, 5, hudMiniMapBgColor);
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
		const float courseLength = (course.spec.lines.size()*course.spec.roadSegmentLength)/coursePositionFactor,
					progress = onSceneFinish? 100 : trunc(100.0 * (playerVehicle.position / courseLength));
		font3->drawText("Complete " + futil::to_string(progress) + "%", rightHudMargin, hudTimerBestLap.bounds.y, Color::WHITE);
	}

	hudSpeedometer.draw();
	fontSmall->drawText(settings.isImperialUnit? "mph" : "kph", (hudSpeedometer.bounds.x + hudDialTachometer.bounds.x)/2, hudSpeedometer.bounds.y+hudSpeedometer.font->getHeight()*1.2f, fgeal::Color::WHITE);

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
	{
		char buffer[512];
		float offset = 25;
		fontDev->drawText("FPS:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%d", game.getFpsCount());
		fontSmall->drawText(std::string(buffer), 55, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDev->drawText("Position:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm", playerVehicle.position);
		fontSmall->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fkm/h", playerVehicle.body.speed*3.6);
		fontSmall->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		fontDev->drawText("0-60mph: ", 175, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fs", acc0to60time);
		fontSmall->drawText(std::string(buffer), 250, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Acc.:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s^2", playerVehicle.body.acceleration);
		fontSmall->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDev->drawText("Height:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm", playerVehicle.verticalPosition);
		fontSmall->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

//		offset += 18;
//		fontDev->drawText("Speed:", 25, offset, fgeal::Color::WHITE);
//		sprintf(buffer, "%2.2fm/s", verticalSpeed);
//		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDev->drawText("Wheel turn pseudo angle:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f", playerVehicle.pseudoAngle);
		fontSmall->drawText(std::string(buffer), 250, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Slope angle:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f", playerVehicle.body.slopeAngle);
		fontSmall->drawText(std::string(buffer), 250, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Strafe speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s", playerVehicle.strafeSpeed/coursePositionFactor);
		fontSmall->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDev->drawText("Curve pull:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s", playerVehicle.curvePull/coursePositionFactor);
		fontSmall->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Slope pull:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s^2", playerVehicle.body.slopePullForce);
		fontSmall->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Braking friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.brakingForce);
		fontSmall->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Rolling friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.rollingResistanceForce);
		fontSmall->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Air friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.airDragForce);
		fontSmall->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Combined friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", (playerVehicle.curvePull/coursePositionFactor + playerVehicle.body.slopePullForce + playerVehicle.body.brakingForce + playerVehicle.body.rollingResistanceForce + playerVehicle.body.airDragForce));
		fontSmall->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDev->drawText("Drive force:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.getDriveForce());
		fontSmall->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Torque:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fNm", playerVehicle.body.engine.getCurrentTorque());
		fontSmall->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Torque proportion:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f%%", 100.f*playerVehicle.body.engine.getCurrentTorque()/playerVehicle.body.engine.maximumTorque);
		fontSmall->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Power:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fhp", (playerVehicle.body.engine.getCurrentTorque()*playerVehicle.body.engine.rpm)/(5252.0 * 1.355818));
		fontSmall->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDev->drawText("Driven tires load:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.getDrivenWheelsWeightLoad());
		fontSmall->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Downforce:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", -playerVehicle.body.downforce);
		fontSmall->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDev->drawText("Wheel Ang. Speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2frad/s", playerVehicle.body.wheelAngularSpeed);
		fontSmall->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("RPM:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.f", playerVehicle.body.engine.rpm);
		fontSmall->drawText(std::string(buffer), 55, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDev->drawText("Gear:", 25, offset, fgeal::Color::WHITE);
		const char* autoLabelTxt = (playerVehicle.body.automaticShiftingEnabled? " (auto)":"");
		sprintf(buffer, "%d %s", playerVehicle.body.engine.gear, autoLabelTxt);
		fontSmall->drawText(std::string(buffer), 60, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDev->drawText("Slip ratio:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f%%", 100*playerVehicle.body.slipRatio);
		fontSmall->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		unsigned currentRangeIndex = playerVehicle.engineSound.getRangeIndex(playerVehicle.body.engine.rpm);
		for(unsigned i = 0; i < playerVehicle.engineSound.getSoundData().size(); i++)
		{
			const std::string format = std::string(playerVehicle.engineSound.getSoundData()[i]->isPlaying()==false? " s%u " : currentRangeIndex==i? "[s%u]" : "(s%u)") + " vol: %2.2f pitch: %2.2f";
			sprintf(buffer, format.c_str(), i, playerVehicle.engineSound.getSoundData()[i]->getVolume(), playerVehicle.engineSound.getSoundData()[i]->getPlaybackSpeed());
			fontSmall->drawText(std::string(buffer), displayWidth - 200, displayHeight/2.0 - i*fontSmall->getHeight(), fgeal::Color::WHITE);
		}
	}
}

static const float LONGITUDINAL_SLIP_RATIO_BURN_RUBBER = 0.2;  // 20%

void Pseudo3DRaceState::update(float delta)
{
	handlePhysics(delta);

	if(onSceneIntro)
	{
		timerSceneIntro -= delta;

		if(countdownBuzzerCounter - timerSceneIntro > 1)
		{
			sndCountdownBuzzer->play();
			countdownBuzzerCounter--;

			if(countdownBuzzerCounter == 2)  // dont play at last call
				countdownBuzzerCounter = 0;
		}

		if(timerSceneIntro < 1)
		{
			onSceneIntro = false;
			playerVehicle.body.engine.gear = 1;
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

	if(acc0to60time == 0)
	{
		if(playerVehicle.body.engine.throttlePosition > 0 and playerVehicle.body.speed > 0 and acc0to60clock == 0)
			acc0to60clock = fgeal::uptime();
		else if(playerVehicle.body.engine.throttlePosition < 0 and acc0to60clock != 0)
			acc0to60clock = 0;
		else if(playerVehicle.body.engine.throttlePosition > 0 and playerVehicle.body.speed * 3.6 > 96)
			acc0to60time = fgeal::uptime() - acc0to60clock;
	}

	// course looping control
	const unsigned N = course.spec.lines.size();
	while(playerVehicle.position * coursePositionFactor >= N*course.spec.roadSegmentLength)
	{
		playerVehicle.position -= N*course.spec.roadSegmentLength / coursePositionFactor;

		if(not onSceneFinish)
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

	}
	while(playerVehicle.position < 0)
		playerVehicle.position += N*course.spec.roadSegmentLength / coursePositionFactor;

	playerVehicle.engineSound.update(playerVehicle.body.engine.rpm);

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
		and MAXIMUM_STRAFE_SPEED_FACTOR * coursePositionFactor * playerVehicle.corneringStiffness - fabs(playerVehicle.strafeSpeed) < 1
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
			playerVehicle.position = 0;
			playerVehicle.horizontalPosition = playerVehicle.verticalPosition = 0;
//					verticalSpeed = 0;
			playerVehicle.body.reset();
			playerVehicle.pseudoAngle = 0;
			parallax.x = parallax.y = 0;
			lapTimeCurrent = 0;
			lapCurrent = 1;
			onSceneIntro = true;
			timerSceneIntro = 4.5;
			countdownBuzzerCounter = 5;
//					isBurningRubber = onAir = onLongAir = false;
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
