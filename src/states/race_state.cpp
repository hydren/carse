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

static const float PSEUDO_ANGLE_THRESHOLD = 0.1;

static const float MINIMUM_SPEED_BURN_RUBBER_ON_TURN = 5.5556;  // == 20kph
static const float MAXIMUM_STRAFE_SPEED = 15000;  // undefined unit

static const float GLOBAL_VEHICLE_SCALE_FACTOR = 0.0048828125;

static const float BACKGROUND_POSITION_FACTOR = 0.509375;

#define isPlayerAccelerating() (Keyboard::isKeyPressed(controlKeyAccelerate) or (Joystick::getCount() > 0 and Joystick::isButtonPressed(0, controlJoystickKeyAccelerate)))
#define isPlayerBraking() (Keyboard::isKeyPressed(controlKeyBrake) or (Joystick::getCount() > 0 and Joystick::isButtonPressed(0, controlJoystickKeyBrake)))
#define isPlayerSteeringLeft() (Keyboard::isKeyPressed(controlKeyTurnLeft) or (Joystick::getCount() > 0 and Joystick::getAxisPosition(0, controlJoystickAxisTurn) < 0))
#define isPlayerSteeringRight() (Keyboard::isKeyPressed(controlKeyTurnRight) or (Joystick::getCount() > 0 and Joystick::getAxisPosition(0, controlJoystickAxisTurn) > 0))

// -------------------------------------------------------------------------------

int Pseudo3DRaceState::getId(){ return Pseudo3DCarseGame::RACE_STATE_ID; }

//static
Pseudo3DRaceState* Pseudo3DRaceState::getInstance(fgeal::Game& game) { return static_cast<Pseudo3DRaceState*>(game.getState(Pseudo3DCarseGame::RACE_STATE_ID)); }

Pseudo3DRaceState::Pseudo3DRaceState(CarseGame* game)  //@suppress("Class members should be properly initialized")
: State(*game),
  font(null), font2(null), font3(null), fontDebug(null), bg(null), music(null),
  sndTireBurnoutStandIntro(null), sndTireBurnoutStandLoop(null), sndTireBurnoutIntro(null), sndTireBurnoutLoop(null), sndOnDirtLoop(null), sndJumpImpact(null),
  bgColor(), bgColorHorizon(), spriteSmokeLeft(null), spriteSmokeRight(null),
  position(0), posX(0), posY(0),
  pseudoAngle(0), strafeSpeed(0), curvePull(0), corneringForceLeechFactor(0), corneringStiffness(0),
  bgParallax(), isBurningRubber(false), /* verticalSpeed(0), onAir(false), onLongAir(false), */
  drawParameters(), coursePositionFactor(500), isImperialUnit(), simulationType(),
  laptime(0), laptimeBest(0), lapCurrent(0), course(0, 0),
  hudRpmGauge(null), hudSpeedDisplay(null), hudGearDisplay(null), hudTimerCurrentLap(null), hudTimerBestLap(null), hudCurrentLap(null),
  debugMode(true)
{
	drawParameters.cameraDepth = 0.84;
	drawParameters.drawDistance = 300;
	bgParallax.x = bgParallax.y = 0;
}

Pseudo3DRaceState::~Pseudo3DRaceState()
{
	// todo delete sounds
	delete font;
	delete font2;
	delete font3;
	delete fontDebug;
	delete bg;
	delete music;
	delete spriteSmokeLeft;
	delete spriteSmokeRight;
	for(unsigned i = 0; i < spritesVehicle.size(); i++)
		delete spritesVehicle[i];
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

	controlKeyAccelerate = fgeal::Keyboard::KEY_ARROW_UP;
	controlKeyBrake = fgeal::Keyboard::KEY_ARROW_DOWN;
	controlKeyTurnLeft = fgeal::Keyboard::KEY_ARROW_LEFT;
	controlKeyTurnRight = fgeal::Keyboard::KEY_ARROW_RIGHT;
	controlKeyShiftUp = fgeal::Keyboard::KEY_LEFT_SHIFT;
	controlKeyShiftDown = fgeal::Keyboard::KEY_LEFT_CONTROL;

	controlJoystickAxisTurn = 0;
	controlJoystickKeyAccelerate = 2;
	controlJoystickKeyBrake = 3;
	controlJoystickKeyShiftDown = 4;
	controlJoystickKeyShiftUp = 5;
}

void Pseudo3DRaceState::onEnter()
{
	Display& display = game.getDisplay();
	drawParameters.drawAreaWidth = display.getWidth();
	drawParameters.drawAreaHeight = display.getHeight();

	if(not spritesVehicle.empty())
	{
		delete spritesVehicle[0]->image;

		for(unsigned i = 0; i < spritesVehicle.size(); i++)
			delete spritesVehicle[i];

		spritesVehicle.clear();
	}

	Image* sheet = new Image(vehicle.sprite.sheetFilename);

	if(sheet->getWidth() < (int) vehicle.sprite.frameWidth)
		throw std::runtime_error("Invalid sprite width value. Value is smaller than sprite sheet width (no whole sprites could be draw)");

	for(unsigned i = 0; i < vehicle.sprite.stateCount; i++)
	{
		Sprite* sprite = new Sprite(sheet, vehicle.sprite.frameWidth, vehicle.sprite.frameHeight,
									vehicle.sprite.frameDuration, vehicle.sprite.stateFrameCount[i],
									0, i*vehicle.sprite.frameHeight);

		sprite->scale = vehicle.sprite.scale * display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR;
		sprite->referencePixelY = - (int) vehicle.sprite.contactOffset;
		spritesVehicle.push_back(sprite);
	}

	if(vehicle.sprite.asymmetrical) for(unsigned i = 1; i < vehicle.sprite.stateCount; i++)
	{
		Sprite* sprite = new Sprite(sheet, vehicle.sprite.frameWidth, vehicle.sprite.frameHeight,
									vehicle.sprite.frameDuration, vehicle.sprite.stateFrameCount[i],
									0, (vehicle.sprite.stateCount-1 + i)*vehicle.sprite.frameHeight);

		sprite->scale = vehicle.sprite.scale * display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR;
		sprite->referencePixelY = - (int) vehicle.sprite.contactOffset;
		spritesVehicle.push_back(sprite);
	}

	if(bg != null)
		delete bg;

	bg = new Image(course.landscapeFilename);

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

	engineSound.setProfile(vehicle.engineSoundProfile, vehicle.body.engine.maxRpm);

	float gaugeDiameter = 0.15*std::max(display.getWidth(), display.getHeight());
	fgeal::Rectangle gaugeSize = { display.getWidth() - 1.1f*gaugeDiameter, display.getHeight() - 1.2f*gaugeDiameter, gaugeDiameter, gaugeDiameter };
	hudRpmGauge = new Hud::DialGauge<float>(vehicle.body.engine.rpm, 1000, vehicle.body.engine.maxRpm, gaugeSize);
	hudRpmGauge->borderThickness = 6;
	hudRpmGauge->graduationLevel = 2;
	hudRpmGauge->graduationPrimarySize = 1000;
	hudRpmGauge->graduationSecondarySize = 100;
	hudRpmGauge->graduationValueScale = 0.001;
	hudRpmGauge->graduationFont = font;
	hudRpmGauge->compile();

	gaugeSize.y = gaugeSize.y + 0.7*gaugeSize.h;
	gaugeSize.x = gaugeSize.x + 0.4*gaugeSize.w;
	gaugeSize.w = 24;
	gaugeSize.h = 1.5 * font->getHeight();
	hudGearDisplay = new Hud::NumericalDisplay<int>(vehicle.body.engine.gear, gaugeSize, font);
	hudGearDisplay->borderThickness = 6;
	hudGearDisplay->borderColor = fgeal::Color::LIGHT_GREY;
	hudGearDisplay->backgroundColor = fgeal::Color::BLACK;
	hudGearDisplay->specialCases[0] = "N";
	hudGearDisplay->specialCases[-1] = "R";

	gaugeSize.x = hudRpmGauge->bounds.x - font2->getTextWidth("---");
	gaugeSize.w *= 3;
	gaugeSize.h *= 1.7;
	hudSpeedDisplay = new Hud::NumericalDisplay<float>(vehicle.body.speed, gaugeSize, font2);
	hudSpeedDisplay->valueScale = isImperialUnit? 2.25 : 3.6;
	hudSpeedDisplay->disableBackground = true;
	hudSpeedDisplay->displayColor = fgeal::Color::WHITE;
	hudSpeedDisplay->borderThickness = 0;

	hudCurrentLap = new Hud::NumericalDisplay<unsigned>(lapCurrent, gaugeSize, font3);
	hudCurrentLap->bounds.y = display.getHeight() * 0.04;
	hudCurrentLap->disableBackground = true;
	hudCurrentLap->displayColor = Color::WHITE;

	hudTimerCurrentLap = new Hud::TimerDisplay<float>(laptime, gaugeSize, font3);
	hudTimerCurrentLap->bounds.y = hudCurrentLap->bounds.y + font3->getHeight()*1.05;
	hudTimerCurrentLap->valueScale = 1000;
	hudTimerCurrentLap->disableBackground = true;
	hudTimerCurrentLap->displayColor = Color::WHITE;

	hudTimerBestLap = new Hud::TimerDisplay<float>(laptimeBest, gaugeSize, font3);
	hudTimerBestLap->bounds.y = hudTimerCurrentLap->bounds.y + font3->getHeight()*1.05;
	hudTimerBestLap->valueScale = 1000;
	hudTimerBestLap->disableBackground = true;
	hudTimerBestLap->displayColor = Color::WHITE;

	spriteSmokeLeft->scale.x =
			spriteSmokeLeft->scale.y =
					spriteSmokeRight->scale.x =
							spriteSmokeRight->scale.y = display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR*0.75f;

	corneringForceLeechFactor = (vehicle.spec->type == Mechanics::TYPE_BIKE? 0.25 : 0.5);
	corneringStiffness = 0.575 + 0.575/(1+exp(-0.4*(10.0 - (vehicle.body.mass*GRAVITY_ACCELERATION)/1000.0)));

	bgParallax.x = bgParallax.y = 0;
	position = 0;
	posX = posY = 0;
//	verticalSpeed = 0;
	vehicle.body.simulationType = simulationType;
	vehicle.body.reset();
	vehicle.body.automaticShiftingEnabled = true;
	pseudoAngle = 0;
	laptime = laptimeBest = 0;
	lapCurrent = 1;

	isBurningRubber = /*onAir = onLongAir =*/ false;

	music->loop();
	engineSound.playIdle();
}

void Pseudo3DRaceState::onLeave()
{
	engineSound.haltSound();
	music->stop();
	sndTireBurnoutIntro->stop();
	sndTireBurnoutLoop->stop();
	sndTireBurnoutStandIntro->stop();
	sndTireBurnoutStandLoop->stop();
	sndOnDirtLoop->stop();
//	delete hudRpmGauge;
//	delete hudSpeedDisplay;
//	delete hudGearDisplay;
//	delete hudCurrentLap;
//	delete hudTimer;
}

void Pseudo3DRaceState::render()
{
	const float displayWidth = drawParameters.drawAreaWidth,
				displayHeight = drawParameters.drawAreaHeight;

	game.getDisplay().clear();

	const float parallaxAbsoluteY = bgParallax.y + BACKGROUND_POSITION_FACTOR*displayHeight - bg->getHeight();

	Image::drawFilledRectangle(0, 0, displayWidth, displayHeight, bgColor);
	Image::drawFilledRectangle(0, parallaxAbsoluteY + bg->getHeight(), displayWidth, displayHeight, bgColorHorizon);
	bg->draw(bgParallax.x, parallaxAbsoluteY);
	bg->draw(bgParallax.x + bg->getWidth(), parallaxAbsoluteY);

	course.draw(position * coursePositionFactor, posX, drawParameters);

	// the ammount of pseudo angle that will trigger the last sprite
//	const float PSEUDO_ANGLE_LAST_STATE = PSEUDO_ANGLE_MAX;  // show last sprite when the pseudo angle is at its max
	const float PSEUDO_ANGLE_LAST_STATE = vehicle.sprite.maxDepictedTurnAngle;  // show last sprite when the pseudo angle is at the specified ammount in the .properties

	// linear sprite progression
//	const unsigned animationIndex = (vehicle.gfx.spriteStateCount-1)*fabs(pseudoAngle)/PSEUDO_ANGLE_LAST_STATE;

	// exponential sprite progression. may be slower.
//	const unsigned animationIndex = (vehicle.gfx.spriteStateCount-1)*(exp(fabs(pseudoAngle))-1)/(exp(PSEUDO_ANGLE_LAST_STATE)-1);

	// linear sprite progression with 1-index advance at threshold angle
	unsigned animationIndex = 0;
	if(vehicle.sprite.stateCount > 1 and fabs(pseudoAngle) > PSEUDO_ANGLE_THRESHOLD)
		animationIndex = 1 + (vehicle.sprite.stateCount-2)*(fabs(pseudoAngle) - PSEUDO_ANGLE_THRESHOLD)/(PSEUDO_ANGLE_LAST_STATE - PSEUDO_ANGLE_THRESHOLD);

	// cap index to max possible
	if(animationIndex > vehicle.sprite.stateCount - 1)
		animationIndex = vehicle.sprite.stateCount - 1;

	const bool isLeanRight = (pseudoAngle > 0 and animationIndex != 0);

	// if asymmetrical, right-leaning sprites are after all left-leaning ones
	if(isLeanRight and vehicle.sprite.asymmetrical)
		animationIndex += (vehicle.sprite.stateCount-1);

	Sprite& sprite = *spritesVehicle[animationIndex];
	sprite.flipmode = isLeanRight and not vehicle.sprite.asymmetrical? Image::FLIP_HORIZONTAL : Image::FLIP_NONE;
//	sprite.duration = vehicle.body.speed != 0? 0.1*400.0/(vehicle.body.speed*sprite.numberOfFrames) : 999;  // sometimes work, sometimes don't
	sprite.duration = vehicle.sprite.frameDuration / sqrt(vehicle.body.speed);  // this formula doesn't present good tire animation results.
//	sprite.duration = vehicle.body.speed != 0? 2.0*M_PI*vehicle.body.engine.tireRadius/(vehicle.body.speed*sprite.numberOfFrames) : -1;  // this formula should be the physically correct, but still not good visually.
	sprite.computeCurrentFrame();

	const Point vehicleSpritePosition = {
			// x coord
			0.5f*(displayWidth - sprite.scale.x*vehicle.sprite.frameWidth),

			// y coord
			0.825f*(displayHeight- sprite.scale.y*vehicle.sprite.frameHeight)
			- sprite.scale.y*vehicle.sprite.contactOffset
			- posY*0.01f
	};

	sprite.draw(vehicleSpritePosition.x, vehicleSpritePosition.y);

	if(isBurningRubber)
	{
		const Point smokeSpritePosition = {
				vehicleSpritePosition.x + 0.5f*(sprite.scale.x*(sprite.width - vehicle.sprite.depictedVehicleWidth) - spriteSmokeLeft->width*spriteSmokeLeft->scale.x)
				+ ((pseudoAngle > 0? -1.f : 1.f)*10.f*animationIndex*vehicle.sprite.maxDepictedTurnAngle),
				vehicleSpritePosition.y + sprite.height*sprite.scale.y - spriteSmokeLeft->height*spriteSmokeLeft->scale.y  // should have included ` - sprite.offset*sprite.scale.x`, but don't look good
		};

		spriteSmokeLeft->computeCurrentFrame();
		spriteSmokeLeft->draw(smokeSpritePosition.x, smokeSpritePosition.y);

		spriteSmokeRight->computeCurrentFrame();
		spriteSmokeRight->draw(smokeSpritePosition.x + vehicle.sprite.depictedVehicleWidth*sprite.scale.x, smokeSpritePosition.y);
	}

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

	hudSpeedDisplay->draw();
	font->drawText(isImperialUnit? "mph" : "Km/h", (hudSpeedDisplay->bounds.x + hudRpmGauge->bounds.x)/2, hudSpeedDisplay->bounds.y+hudSpeedDisplay->bounds.h, fgeal::Color::WHITE);

	hudRpmGauge->draw();
	hudGearDisplay->draw();

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
		sprintf(buffer, "%2.2fm", position);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fkm/h", vehicle.body.speed*3.6);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Acc.:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s^2", vehicle.body.acceleration);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Height:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm", posY);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

//		offset += 18;
//		fontDebug->drawText("Speed:", 25, offset, fgeal::Color::WHITE);
//		sprintf(buffer, "%2.2fm/s", verticalSpeed);
//		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Wheel turn pseudo angle:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f", pseudoAngle);
		font->drawText(std::string(buffer), 250, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Slope angle:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f", vehicle.body.slopeAngle);
		font->drawText(std::string(buffer), 250, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Strafe speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s", strafeSpeed/coursePositionFactor);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Curve pull:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s", curvePull/coursePositionFactor);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Slope pull:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s^2", vehicle.body.slopePullForce);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Braking friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", vehicle.body.brakingForce);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Rolling friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", vehicle.body.rollingResistanceForce);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Air friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", vehicle.body.airDragForce);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Combined friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", (curvePull/coursePositionFactor + vehicle.body.slopePullForce + vehicle.body.brakingForce + vehicle.body.rollingResistanceForce + vehicle.body.airDragForce));
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Drive force:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", vehicle.body.getDriveForce());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Torque:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fNm", vehicle.body.engine.getCurrentTorque());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Torque proportion:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f%%", 100.f*vehicle.body.engine.getCurrentTorque()/vehicle.body.engine.maximumTorque);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Power:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fhp", (vehicle.body.engine.getCurrentTorque()*vehicle.body.engine.rpm)/(5252.0 * 1.355818));
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Driven tires load:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", vehicle.body.getDrivenWheelsWeightLoad());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Downforce:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", -vehicle.body.downforce);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Wheel Ang. Speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2frad/s", vehicle.body.wheelAngularSpeed);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("RPM:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.f", vehicle.body.engine.rpm);
		font->drawText(std::string(buffer), 55, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Gear:", 25, offset, fgeal::Color::WHITE);
		const char* autoLabelTxt = (vehicle.body.automaticShiftingEnabled? " (auto)":"");
		sprintf(buffer, "%d %s", vehicle.body.engine.gear, autoLabelTxt);
		font->drawText(std::string(buffer), 60, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Slip ratio:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f%%", 100*vehicle.body.slipRatio);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		unsigned currentRangeIndex = engineSound.getRangeIndex(vehicle.body.engine.rpm);
		for(unsigned i = 0; i < engineSound.getSoundData().size(); i++)
		{
			const std::string format = std::string(engineSound.getSoundData()[i]->isPlaying()==false? " s%u " : currentRangeIndex==i? "[s%u]" : "(s%u)") + " vol: %2.2f pitch: %2.2f";
			sprintf(buffer, format.c_str(), i, engineSound.getSoundData()[i]->getVolume(), engineSound.getSoundData()[i]->getPlaybackSpeed());
			font->drawText(std::string(buffer), displayWidth - 200, displayHeight/2.0 - i*font->getHeight(), fgeal::Color::WHITE);
		}
	}
}

static const float LONGITUDINAL_SLIP_RATIO_BURN_RUBBER = 0.2;  // 20%

void Pseudo3DRaceState::update(float delta)
{
	handleInput();
	handlePhysics(delta);

	laptime += delta;

	// course looping control
	const unsigned N = course.lines.size();
	while(position * coursePositionFactor >= N*course.roadSegmentLength)
	{
		position -= N*course.roadSegmentLength / coursePositionFactor;
		lapCurrent++;
		if(laptime < laptimeBest or laptimeBest == 0)
			laptimeBest = laptime;
		laptime = 0;
	}
	while(position < 0)
		position += N*course.roadSegmentLength / coursePositionFactor;

	engineSound.updateSound(vehicle.body.engine.rpm);

	// lets decide if there is burnout animation
	const bool tireBurnoutAnimRequired = (
		(vehicle.body.simulationType == Mechanics::SIMULATION_TYPE_SLIPLESS
			and
			(
				// fake burnout mode
				vehicle.body.engine.gear == 1
				and vehicle.body.engine.rpm < 0.5*vehicle.body.engine.maxRpm
				and isPlayerAccelerating()
			)
		)
		or
		(vehicle.body.simulationType == Mechanics::SIMULATION_TYPE_FAKESLIP
			and
			(
				// burnout based on capped drive force
				(vehicle.body.getDriveForce() < 0.75 * vehicle.body.engine.getDriveTorque() / vehicle.body.tireRadius)
				and vehicle.body.engine.gear == 1  // but limited to first gear.
			)
		)
		or
		(vehicle.body.simulationType == Mechanics::SIMULATION_TYPE_PACEJKA_BASED
			and
			(
				// burnout based on real slip ratio
				fabs(vehicle.body.slipRatio) > LONGITUDINAL_SLIP_RATIO_BURN_RUBBER
				and fabs(vehicle.body.speed) > 1.0
			)
		)
	);

	if(tireBurnoutAnimRequired and getCurrentSurfaceType() == SURFACE_TYPE_DRY_ASPHALT)
	{
		if(sndTireBurnoutIntro->isPlaying()) sndTireBurnoutIntro->stop();
		if(sndTireBurnoutLoop->isPlaying()) sndTireBurnoutLoop->stop();

		if(not isBurningRubber)
			sndTireBurnoutStandIntro->play();
		else if(not sndTireBurnoutStandIntro->isPlaying() and not sndTireBurnoutStandLoop->isPlaying())
			sndTireBurnoutStandLoop->loop();

		isBurningRubber = true;
	}
	else if(fabs(vehicle.body.speed) > MINIMUM_SPEED_BURN_RUBBER_ON_TURN
	   and (MAXIMUM_STRAFE_SPEED*corneringStiffness - fabs(strafeSpeed) < 1)
	 	 	 and getCurrentSurfaceType() == SURFACE_TYPE_DRY_ASPHALT)
	{
		if(sndTireBurnoutStandIntro->isPlaying()) sndTireBurnoutStandIntro->stop();
		if(sndTireBurnoutStandLoop->isPlaying()) sndTireBurnoutStandLoop->stop();

		if(not isBurningRubber)
			sndTireBurnoutIntro->play();
		else if(not sndTireBurnoutIntro->isPlaying() and not sndTireBurnoutLoop->isPlaying())
			sndTireBurnoutLoop->loop();

		isBurningRubber = true;
	}
	else
	{
		if(sndTireBurnoutStandIntro->isPlaying()) sndTireBurnoutStandIntro->stop();
		if(sndTireBurnoutStandLoop->isPlaying()) sndTireBurnoutStandLoop->stop();
		if(sndTireBurnoutIntro->isPlaying()) sndTireBurnoutIntro->stop();
		if(sndTireBurnoutLoop->isPlaying()) sndTireBurnoutLoop->stop();
		isBurningRubber = false;
	}

	if(getCurrentSurfaceType() != SURFACE_TYPE_DRY_ASPHALT and fabs(vehicle.body.speed) > 1)
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
				shiftGear(vehicle.body.engine.gear+1);
			else if(event.getEventKeyCode() == controlKeyShiftDown)
				shiftGear(vehicle.body.engine.gear-1);

			else switch(event.getEventKeyCode())
			{
				case Keyboard::KEY_ESCAPE:
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::KEY_R:
					position = 0;
					posX = posY = 0;
//					verticalSpeed = 0;
					vehicle.body.reset();
					pseudoAngle = 0;
					bgParallax.x = bgParallax.y = 0;
					laptime = 0;
					lapCurrent = 1;
//					isBurningRubber = onAir = onLongAir = false;
					break;
				case Keyboard::KEY_T:
					vehicle.body.automaticShiftingEnabled = !vehicle.body.automaticShiftingEnabled;
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
				default:
					break;
			}
		}

		else if(event.getEventType() == Event::TYPE_JOYSTICK_BUTTON_PRESS)
		{
			if(event.getEventJoystickButtonIndex() == (int) controlJoystickKeyShiftUp)
				shiftGear(vehicle.body.engine.gear+1);
			else if(event.getEventJoystickButtonIndex() == (int) controlJoystickKeyShiftDown)
				shiftGear(vehicle.body.engine.gear-1);
		}
	}
}

#include "race_state_physics.hxx"
