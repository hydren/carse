/*
 * race_state.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "race_state.hpp"

#include "carse_game.hpp"

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>

using std::string;
using std::map;

// xxx debug
#include <iostream>
using std::cout; using std::endl;

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

#define GRAVITY_ACCELERATION Mechanics::GRAVITY_ACCELERATION

static const float MINIMUM_SPEED_BURN_RUBBER_ON_TURN = 5.5556;  // == 20kph
static const float MAXIMUM_STRAFE_SPEED = 15000;  // undefined unit

static const float GLOBAL_VEHICLE_SCALE_FACTOR = 0.0048828125;
static const float PSEUDO_ANGLE_THRESHOLD = 0.1;

static const float BACKGROUND_POSITION_FACTOR = 0.509375;

// -------------------------------------------------------------------------------

int Pseudo3DRaceState::getId(){ return Pseudo3DCarseGame::RACE_STATE_ID; }

Pseudo3DRaceState::Pseudo3DRaceState(CarseGame* game)
: State(*game),
  font(null), font2(null), font3(null), fontDebug(null),
  imgBackground(null),
  music(null),
  sndTireBurnoutStandIntro(null), sndTireBurnoutStandLoop(null), sndTireBurnoutIntro(null), sndTireBurnoutLoop(null), sndOnDirtLoop(null), sndJumpImpact(null),

  bgColor(), bgColorHorizon(),
  spriteSmokeLeft(null), spriteSmokeRight(null),

  parallax(), backgroundScale(),

  drawParameters(), coursePositionFactor(500), isImperialUnit(), simulationType(),
  onIntro(), onEnding(), introTime(), raceType(),
  laptime(0), laptimeBest(0), lapCurrent(0),

  course(0, 0), playerVehicleSpec(), playerVehicleSpecAlternateSpriteIndex(-1), playerVehicle(),

  hudTachometer(null), hudSpeedometer(null), hudGearDisplay(null), hudTimerCurrentLap(null), hudTimerBestLap(null), hudCurrentLap(null),

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
{
	drawParameters.cameraDepth = 0.84;
	drawParameters.drawDistance = 300;
	parallax.x = parallax.y = 0;
}

Pseudo3DRaceState::~Pseudo3DRaceState()
{
	// todo delete sounds
	if(font != null) delete font;
	if(font2 != null) delete font2;
	if(font3 != null) delete font3;
	if(fontDebug != null) delete fontDebug;

	if(imgBackground != null) delete imgBackground;
	if(music != null) delete music;

	if(sndTireBurnoutStandIntro != null) delete sndTireBurnoutStandIntro;
	if(sndTireBurnoutStandLoop != null) delete sndTireBurnoutStandLoop;
	if(sndTireBurnoutIntro != null) delete sndTireBurnoutIntro;
	if(sndTireBurnoutLoop != null) delete sndTireBurnoutLoop;
	if(sndOnDirtLoop != null) delete sndOnDirtLoop;
	if(sndJumpImpact != null) delete sndJumpImpact;

	if(spriteSmokeLeft != null) delete spriteSmokeLeft;
	if(spriteSmokeRight != null) delete spriteSmokeRight;
}

void Pseudo3DRaceState::initialize()
{
	font = new Font("assets/font.ttf");
	font2 = new Font("assets/font2.ttf", 40);
	font3 = new Font("assets/font2.ttf", 24);
	fontDebug = new Font("assets/font.ttf");
	music = new Music("assets/music_sample.ogg");

	sndTireBurnoutStandIntro = new Sound("assets/sound/tire_burnout_stand1_intro.ogg");
	sndTireBurnoutStandLoop = new Sound("assets/sound/tire_burnout_stand1_loop.ogg");
	sndTireBurnoutIntro = new Sound("assets/sound/tire_burnout_normal1_intro.ogg");
	sndTireBurnoutLoop = new Sound("assets/sound/tire_burnout_normal1_loop.ogg");
	sndOnDirtLoop = new Sound("assets/sound/on_gravel.ogg");
	sndJumpImpact = new Sound("assets/sound/landing.ogg");

	Image* smokeSpriteSheet = new Image("assets/smoke-sprite.png");
	spriteSmokeLeft = new Sprite(smokeSpriteSheet, 32, 32, 0.25, -1, 0, 0, true);
	spriteSmokeRight = new Sprite(smokeSpriteSheet, 32, 32, 0.25);
	spriteSmokeRight->flipmode = Image::FLIP_HORIZONTAL;
}

void Pseudo3DRaceState::onEnter()
{
	Display& display = game.getDisplay();
	drawParameters.drawAreaWidth = display.getWidth();
	drawParameters.drawAreaHeight = display.getHeight();

	playerVehicle.clearDynamicData();
	playerVehicle = Pseudo3DVehicle(playerVehicleSpec, playerVehicleSpecAlternateSpriteIndex);
	playerVehicle.setupDynamicData();

	for(unsigned s = 0; s < playerVehicle.sprites.size(); s++)
		playerVehicle.sprites[s]->scale *= (display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR);

	if(imgBackground != null)
		delete imgBackground;

	imgBackground = new Image(course.landscapeFilename);

	backgroundScale = 0.2 * display.getHeight() / (float) imgBackground->getHeight();

	if(not drawParameters.sprites.empty())
	{
		for(unsigned i = 0; i < drawParameters.sprites.size(); i++)
			delete drawParameters.sprites[i];

		drawParameters.sprites.clear();
	}

	for(unsigned i = 0; i < course.spritesFilenames.size(); i++)
		if(not course.spritesFilenames[i].empty())
			drawParameters.sprites.push_back(new Image(course.spritesFilenames[i]));
		else
			drawParameters.sprites.push_back(null);

	bgColor = course.colorLandscape;
	bgColorHorizon = course.colorHorizon;

	playerVehicle.engineSound.setProfile(playerVehicle.engineSoundProfile, playerVehicle.body.engine.maxRpm);

	if(raceType != RACE_TYPE_DEBUG)
	{
		onIntro = true;
		onEnding = false;
		introTime = 4.5;
		debugMode = false;
	}
	else
	{
		onIntro = false;
		debugMode = true;
	}

	float gaugeDiameter = 0.15*std::max(display.getWidth(), display.getHeight());
	fgeal::Rectangle gaugeSize = { display.getWidth() - 1.1f*gaugeDiameter, display.getHeight() - 1.2f*gaugeDiameter, gaugeDiameter, gaugeDiameter };
	hudTachometer = new Hud::DialGauge<float>(playerVehicle.body.engine.rpm, 1000, playerVehicle.body.engine.maxRpm, gaugeSize);
	hudTachometer->borderThickness = 6;
	hudTachometer->graduationLevel = 2;
	hudTachometer->graduationPrimarySize = 1000;
	hudTachometer->graduationSecondarySize = 100;
	hudTachometer->graduationValueScale = 0.001;
	hudTachometer->graduationFont = font;
	hudTachometer->compile();

	gaugeSize.y = gaugeSize.y + 0.7*gaugeSize.h;
	gaugeSize.x = gaugeSize.x + 0.4*gaugeSize.w;
	gaugeSize.w = 24;
	gaugeSize.h = 1.5 * font->getHeight();
	hudGearDisplay = new Hud::NumericalDisplay<int>(playerVehicle.body.engine.gear, gaugeSize, font);
	hudGearDisplay->borderThickness = 6;
	hudGearDisplay->borderColor = fgeal::Color::LIGHT_GREY;
	hudGearDisplay->backgroundColor = fgeal::Color::BLACK;
	hudGearDisplay->specialCases[0] = "N";
	hudGearDisplay->specialCases[-1] = "R";
	hudGearDisplay->fontIsShared = true;

	gaugeSize.x = hudTachometer->bounds.x - font2->getTextWidth("---");
	gaugeSize.w *= 3;
	gaugeSize.h *= 1.7;
	hudSpeedometer = new Hud::NumericalDisplay<float>(playerVehicle.body.speed, gaugeSize, font2);
	hudSpeedometer->valueScale = isImperialUnit? 2.25 : 3.6;
	hudSpeedometer->disableBackground = true;
	hudSpeedometer->displayColor = fgeal::Color::WHITE;
	hudSpeedometer->borderThickness = 0;
	hudSpeedometer->fontIsShared = true;

	hudCurrentLap = new Hud::NumericalDisplay<unsigned>(lapCurrent, gaugeSize, font3);
	hudCurrentLap->bounds.y = display.getHeight() * 0.04;
	hudCurrentLap->disableBackground = true;
	hudCurrentLap->displayColor = Color::WHITE;
	hudCurrentLap->fontIsShared = true;

	hudTimerCurrentLap = new Hud::TimerDisplay<float>(laptime, gaugeSize, font3);
	hudTimerCurrentLap->bounds.y = hudCurrentLap->bounds.y + font3->getHeight()*1.05;
	hudTimerCurrentLap->valueScale = 1000;
	hudTimerCurrentLap->disableBackground = true;
	hudTimerCurrentLap->displayColor = Color::WHITE;
	hudTimerCurrentLap->fontIsShared = true;

	hudTimerBestLap = new Hud::TimerDisplay<float>(laptimeBest, gaugeSize, font3);
	hudTimerBestLap->bounds.y = hudTimerCurrentLap->bounds.y + font3->getHeight()*1.05;
	hudTimerBestLap->valueScale = 1000;
	hudTimerBestLap->disableBackground = true;
	hudTimerBestLap->displayColor = Color::WHITE;
	hudTimerBestLap->fontIsShared = true;

	spriteSmokeLeft->scale.x =
			spriteSmokeLeft->scale.y =
					spriteSmokeRight->scale.x =
							spriteSmokeRight->scale.y = display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR*0.75f;

	playerVehicle.corneringForceLeechFactor = (playerVehicle.body.vehicleType == Mechanics::TYPE_BIKE? 0.25 : 0.5);
	playerVehicle.corneringStiffness = 0.575 + 0.575/(1+exp(-0.4*(10.0 - (playerVehicle.body.mass*GRAVITY_ACCELERATION)/1000.0)));

	parallax.x = parallax.y = 0;
	playerVehicle.position = 0;
	playerVehicle.horizontalPosition = playerVehicle.verticalPosition = 0;
//	verticalSpeed = 0;
	playerVehicle.body.simulationType = simulationType;
	playerVehicle.body.reset();
	playerVehicle.body.automaticShiftingEnabled = true;
	playerVehicle.pseudoAngle = 0;
	laptime = laptimeBest = 0;
	lapCurrent = 1;

	playerVehicle.isBurningRubber = /*onAir = onLongAir =*/ false;

	music->loop();
	playerVehicle.engineSound.playIdle();
}

void Pseudo3DRaceState::onLeave()
{
	playerVehicle.engineSound.haltSound();
	music->stop();
	sndTireBurnoutIntro->stop();
	sndTireBurnoutLoop->stop();
	sndTireBurnoutStandIntro->stop();
	sndTireBurnoutStandLoop->stop();
	sndOnDirtLoop->stop();

	delete hudTachometer;
	delete hudSpeedometer;
	delete hudGearDisplay;
	delete hudCurrentLap;
	delete hudTimerBestLap;
	delete hudTimerCurrentLap;
}

void Pseudo3DRaceState::render()
{
	const float displayWidth = drawParameters.drawAreaWidth,
				displayHeight = drawParameters.drawAreaHeight;

	game.getDisplay().clear();

	const float parallaxAbsoluteY = parallax.y + BACKGROUND_POSITION_FACTOR*displayHeight - imgBackground->getHeight()*backgroundScale;

	Image::drawFilledRectangle(0, 0, displayWidth, displayHeight, bgColor);
	Image::drawFilledRectangle(0, parallaxAbsoluteY + imgBackground->getHeight()*backgroundScale, displayWidth, displayHeight, bgColorHorizon);

	for(float bg = 0; bg < 2*displayWidth; bg += imgBackground->getWidth())
		imgBackground->drawScaled(parallax.x + bg, parallaxAbsoluteY, 1, backgroundScale);

	course.draw(playerVehicle.position * coursePositionFactor, playerVehicle.horizontalPosition, drawParameters);

	const fgeal::Point vehicleSpritePosition = {
			0.5f*displayWidth,  // x coord
			0.75f*displayHeight - playerVehicle.verticalPosition*0.01f  // y coord
	};

	drawVehicle(playerVehicle, vehicleSpritePosition);

	if(raceType == RACE_TYPE_LOOP_PRACTICE)
	{
		const float rightHudMargin = hudCurrentLap->bounds.x - font3->getTextWidth("______");
		font3->drawText("Lap ", 1.05*rightHudMargin, hudCurrentLap->bounds.y, Color::WHITE);
		hudCurrentLap->draw();

		font3->drawText("Time:", rightHudMargin, hudTimerCurrentLap->bounds.y, Color::WHITE);
		hudTimerCurrentLap->draw();

		font3->drawText("Best:", rightHudMargin, hudTimerBestLap->bounds.y, Color::WHITE);
		if(laptimeBest == 0)
			font3->drawText("--", hudTimerBestLap->bounds.x, hudTimerBestLap->bounds.y, Color::WHITE);
		else
			hudTimerBestLap->draw();
	}

	hudSpeedometer->draw();
	font->drawText(isImperialUnit? "mph" : "Km/h", (hudSpeedometer->bounds.x + hudTachometer->bounds.x)/2, hudSpeedometer->bounds.y+hudSpeedometer->bounds.h, fgeal::Color::WHITE);

	hudTachometer->draw();
	hudGearDisplay->draw();

	if(onIntro)
	{
		if(introTime >= 4);  //@suppress("Suspicious semicolon")
		else if(introTime > 1)
		{
			font2->drawText(futil::to_string((int) introTime),
							0.5f*(displayWidth - font2->getTextWidth("0")),
							0.4f*(displayHeight - font2->getHeight()), Color::WHITE);
		}
	}
	else if(introTime > 0)
	{
		font2->drawText("GO!", 0.5f*(displayWidth - font2->getTextWidth("GO!")),
							   0.4f*(displayHeight - font2->getHeight()), Color::WHITE);
	}

	// DEBUG
	if(debugMode)
	{
		char buffer[512];
		float offset = 25;
		fontDebug->drawText("FPS:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%d", game.getFpsCount());
		font->drawText(std::string(buffer), 55, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Position:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm", playerVehicle.position);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fkm/h", playerVehicle.body.speed*3.6);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Acc.:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s^2", playerVehicle.body.acceleration);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Height:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm", playerVehicle.verticalPosition);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

//		offset += 18;
//		fontDebug->drawText("Speed:", 25, offset, fgeal::Color::WHITE);
//		sprintf(buffer, "%2.2fm/s", verticalSpeed);
//		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Wheel turn pseudo angle:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f", playerVehicle.pseudoAngle);
		font->drawText(std::string(buffer), 250, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Slope angle:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f", playerVehicle.body.slopeAngle);
		font->drawText(std::string(buffer), 250, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Strafe speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s", playerVehicle.strafeSpeed/coursePositionFactor);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Curve pull:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s", playerVehicle.curvePull/coursePositionFactor);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Slope pull:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s^2", playerVehicle.body.slopePullForce);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Braking friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.brakingForce);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Rolling friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.rollingResistanceForce);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Air friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.airDragForce);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Combined friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", (playerVehicle.curvePull/coursePositionFactor + playerVehicle.body.slopePullForce + playerVehicle.body.brakingForce + playerVehicle.body.rollingResistanceForce + playerVehicle.body.airDragForce));
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Drive force:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.getDriveForce());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Torque:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fNm", playerVehicle.body.engine.getCurrentTorque());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Torque proportion:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f%%", 100.f*playerVehicle.body.engine.getCurrentTorque()/playerVehicle.body.engine.maximumTorque);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Power:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fhp", (playerVehicle.body.engine.getCurrentTorque()*playerVehicle.body.engine.rpm)/(5252.0 * 1.355818));
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Driven tires load:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", playerVehicle.body.getDrivenWheelsWeightLoad());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Downforce:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", -playerVehicle.body.downforce);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Wheel Ang. Speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2frad/s", playerVehicle.body.wheelAngularSpeed);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("RPM:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.f", playerVehicle.body.engine.rpm);
		font->drawText(std::string(buffer), 55, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Gear:", 25, offset, fgeal::Color::WHITE);
		const char* autoLabelTxt = (playerVehicle.body.automaticShiftingEnabled? " (auto)":"");
		sprintf(buffer, "%d %s", playerVehicle.body.engine.gear, autoLabelTxt);
		font->drawText(std::string(buffer), 60, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Slip ratio:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f%%", 100*playerVehicle.body.slipRatio);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		unsigned currentRangeIndex = playerVehicle.engineSound.getRangeIndex(playerVehicle.body.engine.rpm);
		for(unsigned i = 0; i < playerVehicle.engineSound.getSoundData().size(); i++)
		{
			const std::string format = std::string(playerVehicle.engineSound.getSoundData()[i]->isPlaying()==false? " s%u " : currentRangeIndex==i? "[s%u]" : "(s%u)") + " vol: %2.2f pitch: %2.2f";
			sprintf(buffer, format.c_str(), i, playerVehicle.engineSound.getSoundData()[i]->getVolume(), playerVehicle.engineSound.getSoundData()[i]->getPlaybackSpeed());
			font->drawText(std::string(buffer), displayWidth - 200, displayHeight/2.0 - i*font->getHeight(), fgeal::Color::WHITE);
		}
	}
}

void Pseudo3DRaceState::drawVehicle(const Pseudo3DVehicle& vehicle, const fgeal::Point& p)
{
	// the ammount of pseudo angle that will trigger the last sprite
//	const float PSEUDO_ANGLE_LAST_STATE = PSEUDO_ANGLE_MAX;  // show last sprite when the pseudo angle is at its max
	const float PSEUDO_ANGLE_LAST_STATE = vehicle.spriteSpec.maxDepictedTurnAngle;  // show last sprite when the pseudo angle is at the specified ammount in the .properties

	// linear sprite progression
//	const unsigned animationIndex = (vehicle.gfx.spriteStateCount-1)*fabs(pseudoAngle)/PSEUDO_ANGLE_LAST_STATE;

	// exponential sprite progression. may be slower.
//	const unsigned animationIndex = (vehicle.gfx.spriteStateCount-1)*(exp(fabs(pseudoAngle))-1)/(exp(PSEUDO_ANGLE_LAST_STATE)-1);

	// linear sprite progression with 1-index advance at threshold angle
	unsigned animationIndex = 0;
	if(vehicle.spriteSpec.stateCount > 1 and fabs(vehicle.pseudoAngle) > PSEUDO_ANGLE_THRESHOLD)
		animationIndex = 1 + (vehicle.spriteSpec.stateCount-2)*(fabs(vehicle.pseudoAngle) - PSEUDO_ANGLE_THRESHOLD)/(PSEUDO_ANGLE_LAST_STATE - PSEUDO_ANGLE_THRESHOLD);

	// cap index to max possible
	if(animationIndex > vehicle.spriteSpec.stateCount - 1)
		animationIndex = vehicle.spriteSpec.stateCount - 1;

	const bool isLeanRight = (vehicle.pseudoAngle > 0 and animationIndex != 0);

	// if asymmetrical, right-leaning sprites are after all left-leaning ones
	if(isLeanRight and vehicle.spriteSpec.asymmetrical)
		animationIndex += (vehicle.spriteSpec.stateCount-1);

	Sprite& sprite = *vehicle.sprites[animationIndex];
	sprite.flipmode = isLeanRight and not vehicle.spriteSpec.asymmetrical? Image::FLIP_HORIZONTAL : Image::FLIP_NONE;
//	sprite.duration = vehicle.body.speed != 0? 0.1*400.0/(vehicle.body.speed*sprite.numberOfFrames) : 999;  // sometimes work, sometimes don't
	sprite.duration = vehicle.spriteSpec.frameDuration / sqrt(vehicle.body.speed);  // this formula doesn't present good tire animation results.
//	sprite.duration = vehicle.body.speed != 0? 2.0*M_PI*vehicle.body.tireRadius/(vehicle.body.speed*sprite.numberOfFrames) : -1;  // this formula should be the physically correct, but still not good visually.
	sprite.computeCurrentFrame();

	const Point vehicleSpritePosition =
		{ p.x - 0.5f*sprite.scale.x*vehicle.spriteSpec.frameWidth,
		  p.y - 0.5f*sprite.scale.y*vehicle.spriteSpec.frameHeight
			  - sprite.scale.y*vehicle.spriteSpec.contactOffset };

	sprite.draw(vehicleSpritePosition.x, vehicleSpritePosition.y);

	if(vehicle.isBurningRubber)
	{
		const Point smokeSpritePosition = {
				vehicleSpritePosition.x + 0.5f*(sprite.scale.x*(sprite.width - vehicle.spriteSpec.depictedVehicleWidth) - spriteSmokeLeft->width*spriteSmokeLeft->scale.x)
				+ ((vehicle.pseudoAngle > 0? -1.f : 1.f)*10.f*animationIndex*vehicle.spriteSpec.maxDepictedTurnAngle),
				vehicleSpritePosition.y + sprite.height*sprite.scale.y - spriteSmokeLeft->height*spriteSmokeLeft->scale.y  // should have included ` - sprite.offset*sprite.scale.x`, but don't look good
		};

		spriteSmokeLeft->computeCurrentFrame();
		spriteSmokeLeft->draw(smokeSpritePosition.x, smokeSpritePosition.y);

		spriteSmokeRight->computeCurrentFrame();
		spriteSmokeRight->draw(smokeSpritePosition.x + vehicle.spriteSpec.depictedVehicleWidth*sprite.scale.x, smokeSpritePosition.y);
	}
}

static const float LONGITUDINAL_SLIP_RATIO_BURN_RUBBER = 0.2;  // 20%

void Pseudo3DRaceState::update(float delta)
{
	handleInput();
	handlePhysics(delta);

	if(onIntro)
	{
		introTime -= delta;
		if(introTime < 1)
		{
			onIntro = false;
			playerVehicle.body.engine.gear = 1;
		}
	}
	else
	{
		laptime += delta;
		if(introTime > 0)
			introTime -= delta;
	}

	// course looping control
	const unsigned N = course.lines.size();
	while(playerVehicle.position * coursePositionFactor >= N*course.roadSegmentLength)
	{
		playerVehicle.position -= N*course.roadSegmentLength / coursePositionFactor;

		if(raceType == RACE_TYPE_LOOP_PRACTICE)
		{
			lapCurrent++;
			if(laptime < laptimeBest or laptimeBest == 0)
				laptimeBest = laptime;
			laptime = 0;
		}
	}
	while(playerVehicle.position < 0)
		playerVehicle.position += N*course.roadSegmentLength / coursePositionFactor;

	playerVehicle.engineSound.updateSound(playerVehicle.body.engine.rpm);

	// lets decide if there is burnout animation
	const bool tireBurnoutAnimRequired = (
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
		(playerVehicle.body.simulationType == Mechanics::SIMULATION_TYPE_FAKESLIP
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

	if(tireBurnoutAnimRequired and getCurrentSurfaceType() == SURFACE_TYPE_DRY_ASPHALT)
	{
		if(sndTireBurnoutIntro->isPlaying()) sndTireBurnoutIntro->stop();
		if(sndTireBurnoutLoop->isPlaying()) sndTireBurnoutLoop->stop();

		if(not playerVehicle.isBurningRubber)
			sndTireBurnoutStandIntro->play();
		else if(not sndTireBurnoutStandIntro->isPlaying() and not sndTireBurnoutStandLoop->isPlaying())
			sndTireBurnoutStandLoop->loop();

		playerVehicle.isBurningRubber = true;
	}
	else if(fabs(playerVehicle.body.speed) > MINIMUM_SPEED_BURN_RUBBER_ON_TURN
	   and (MAXIMUM_STRAFE_SPEED*playerVehicle.corneringStiffness - fabs(playerVehicle.strafeSpeed) < 1)
	 	 	 and getCurrentSurfaceType() == SURFACE_TYPE_DRY_ASPHALT)
	{
		if(sndTireBurnoutStandIntro->isPlaying()) sndTireBurnoutStandIntro->stop();
		if(sndTireBurnoutStandLoop->isPlaying()) sndTireBurnoutStandLoop->stop();

		if(not playerVehicle.isBurningRubber)
			sndTireBurnoutIntro->play();
		else if(not sndTireBurnoutIntro->isPlaying() and not sndTireBurnoutLoop->isPlaying())
			sndTireBurnoutLoop->loop();

		playerVehicle.isBurningRubber = true;
	}
	else
	{
		if(sndTireBurnoutStandIntro->isPlaying()) sndTireBurnoutStandIntro->stop();
		if(sndTireBurnoutStandLoop->isPlaying()) sndTireBurnoutStandLoop->stop();
		if(sndTireBurnoutIntro->isPlaying()) sndTireBurnoutIntro->stop();
		if(sndTireBurnoutLoop->isPlaying()) sndTireBurnoutLoop->stop();
		playerVehicle.isBurningRubber = false;
	}

	if(getCurrentSurfaceType() != SURFACE_TYPE_DRY_ASPHALT and fabs(playerVehicle.body.speed) > 1)
	{
		if(not sndOnDirtLoop->isPlaying())
			sndOnDirtLoop->loop();
	}
	else if(sndOnDirtLoop->isPlaying())
		sndOnDirtLoop->stop();
}

void Pseudo3DRaceState::handleInput()
{
	Event event;
	EventQueue& eventQueue = EventQueue::getInstance();
	while(not eventQueue.isEmpty())
	{
		eventQueue.waitNextEvent(&event);
		if(event.getEventType() == Event::TYPE_DISPLAY_CLOSURE)
			game.running = false;

		else if(event.getEventType() == Event::TYPE_KEY_PRESS)
		{
			if(event.getEventKeyCode() == controlKeyShiftUp)
				shiftGear(playerVehicle.body.engine.gear+1);
			else if(event.getEventKeyCode() == controlKeyShiftDown)
				shiftGear(playerVehicle.body.engine.gear-1);

			else switch(event.getEventKeyCode())
			{
				case Keyboard::KEY_ESCAPE:
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::KEY_R:
					playerVehicle.position = 0;
					playerVehicle.horizontalPosition = playerVehicle.verticalPosition = 0;
//					verticalSpeed = 0;
					playerVehicle.body.reset();
					playerVehicle.pseudoAngle = 0;
					parallax.x = parallax.y = 0;
					laptime = 0;
					lapCurrent = 1;
					onIntro = true;
					introTime = 4.5;
//					isBurningRubber = onAir = onLongAir = false;
					break;
				case Keyboard::KEY_T:
					playerVehicle.body.automaticShiftingEnabled = !playerVehicle.body.automaticShiftingEnabled;
					break;
				case Keyboard::KEY_M:
					if(music->isPlaying())
						music->pause();
					else
						music->resume();
					break;
				case Keyboard::KEY_D:
					debugMode = !debugMode;
					break;
				case Keyboard::KEY_PAGE_UP:
					drawParameters.drawDistance++;
					break;
				case Keyboard::KEY_PAGE_DOWN:
					drawParameters.drawDistance--;
					break;
				case Keyboard::KEY_O:
					drawParameters.cameraDepth += 0.1;
					break;
				case Keyboard::KEY_L:
					drawParameters.cameraDepth -= 0.1;
					break;
				default:
					break;
			}
		}

		else if(event.getEventType() == Event::TYPE_JOYSTICK_BUTTON_PRESS)
		{
			if(event.getEventJoystickButtonIndex() == (int) controlJoystickKeyShiftUp)
				shiftGear(playerVehicle.body.engine.gear+1);
			else if(event.getEventJoystickButtonIndex() == (int) controlJoystickKeyShiftDown)
				shiftGear(playerVehicle.body.engine.gear-1);
		}
	}
}

bool Pseudo3DRaceState::isPlayerAccelerating()
{
	return Keyboard::isKeyPressed(controlKeyAccelerate)
			or (Joystick::getCount() > 0 and Joystick::isButtonPressed(0, controlJoystickKeyAccelerate));
}

bool Pseudo3DRaceState::isPlayerBraking()
{
	return Keyboard::isKeyPressed(controlKeyBrake)
			or (Joystick::getCount() > 0 and Joystick::isButtonPressed(0, controlJoystickKeyBrake));
}

bool Pseudo3DRaceState::isPlayerSteeringLeft()
{
	return Keyboard::isKeyPressed(controlKeyTurnLeft)
			or (Joystick::getCount() > 0 and Joystick::getAxisPosition(0, controlJoystickAxisTurn) < 0);
}

bool Pseudo3DRaceState::isPlayerSteeringRight()
{
	return Keyboard::isKeyPressed(controlKeyTurnRight)
			or (Joystick::getCount() > 0 and Joystick::getAxisPosition(0, controlJoystickAxisTurn) > 0);
}

#include "race_state_physics.hxx"
