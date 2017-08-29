/*
 * race_state.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "race_state.hpp"

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>

using std::string;
using std::map;

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

static const float PSEUDO_ANGLE_THRESHOLD = 0.1;

static const float MINIMUM_SPEED_BURN_RUBBER_ON_TURN = 5.5556;  // == 20kph
static const float MAXIMUM_STRAFE_SPEED = 15000;  // undefined unit

static const float GLOBAL_VEHICLE_SCALE_FACTOR = 0.0048828125;

// -------------------------------------------------------------------------------

int Pseudo3DRaceState::getId(){ return Pseudo3DCarseGame::RACE_STATE_ID; }

//static
Pseudo3DRaceState* Pseudo3DRaceState::getInstance(fgeal::Game& game) { return static_cast<Pseudo3DRaceState*>(game.getState(Pseudo3DCarseGame::RACE_STATE_ID)); }

Pseudo3DRaceState::Pseudo3DRaceState(CarseGame* game)
: State(*game),
  font(null), font2(null), fontDebug(null), bg(null), music(null),
  sndTireBurnoutStandIntro(null), sndTireBurnoutStandLoop(null), sndTireBurnoutIntro(null), sndTireBurnoutLoop(null),
  bgColor(136, 204, 238), spriteSmokeLeft(null), spriteSmokeRight(null),
  position(0), posX(0), pseudoAngle(0), strafeSpeed(0), curvePull(0), bgParallax(),
  rollingFriction(0), airFriction(0), brakingFriction(0), corneringForceLeechFactor(0), isBurningRubber(false), fakeBrakeBuildUp(0),
  drawParameters(), coursePositionFactor(500), laptime(0),
  course(0, 0),
  hudRpmGauge(null), hudSpeedDisplay(null), hudGearDisplay(null), hudTimer(null),
  debugMode(true)
{
	drawParameters.cameraDepth = 0.84;
	drawParameters.drawDistance = 300;
	bgParallax.x = bgParallax.y = 0;
}

Pseudo3DRaceState::~Pseudo3DRaceState()
{
	delete font;
	delete font2;
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
	fontDebug = new Font("assets/font.ttf");
	bg = new Image("assets/bg.png");
	music = new Music("assets/music_sample.ogg");

	sndTireBurnoutStandIntro = new Sound("assets/sound/tire_burnout_stand1_intro.ogg");
	sndTireBurnoutStandLoop = new Sound("assets/sound/tire_burnout_stand1_loop.ogg");
	sndTireBurnoutIntro = new Sound("assets/sound/tire_burnout_normal1_intro.ogg");
	sndTireBurnoutLoop = new Sound("assets/sound/tire_burnout_normal1_loop.ogg");

	Image* smokeSpriteSheet = new Image("assets/smoke-sprite.png");
	spriteSmokeLeft = new Sprite(smokeSpriteSheet, 32, 32, 0.25, -1, 0, 0, true);
	spriteSmokeRight = new Sprite(smokeSpriteSheet, 32, 32, 0.25);
	spriteSmokeRight->flipmode = Image::FLIP_HORIZONTAL;

	drawParameters.drawAreaWidth = Display::getInstance().getWidth();
	drawParameters.drawAreaHeight = Display::getInstance().getHeight();
}

void Pseudo3DRaceState::setVehicle(const Vehicle& v)
{
	vehicle = v;
}

void Pseudo3DRaceState::setCourse(const Course& c)
{
	course = c;
}

void Pseudo3DRaceState::onEnter()
{
	if(not spritesVehicle.empty())
	{
		delete spritesVehicle[0]->image;

		for(unsigned i = 0; i < spritesVehicle.size(); i++)
			delete spritesVehicle[i];

		spritesVehicle.clear();
	}

	Display& display = Display::getInstance();
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

	engineSound.setProfile(vehicle.engineSoundProfile, vehicle.engine.maxRpm);

	float gaugeDiameter = 0.15*std::max(display.getWidth(), display.getHeight());
	fgeal::Rectangle gaugeSize = { display.getWidth() - 1.1f*gaugeDiameter, display.getHeight() - 1.2f*gaugeDiameter, gaugeDiameter, gaugeDiameter };
	hudRpmGauge = new Hud::DialGauge<float>(vehicle.engine.rpm, 1000, vehicle.engine.maxRpm, gaugeSize);
	hudRpmGauge->borderThickness = 6;
	hudRpmGauge->graduationLevel = 2;
	hudRpmGauge->graduationPrimarySize = 1000;
	hudRpmGauge->graduationSecondarySize = 100;
	hudRpmGauge->graduationValueScale = 0.001;
	hudRpmGauge->graduationFont = font;

	gaugeSize.y = gaugeSize.y + 0.7*gaugeSize.h;
	gaugeSize.x = gaugeSize.x + 0.4*gaugeSize.w;
	gaugeSize.w = 24;
	gaugeSize.h = 1.5 * font->getHeight();
	hudGearDisplay = new Hud::NumericalDisplay<int>(vehicle.engine.gear, gaugeSize, font);
	hudGearDisplay->borderThickness = 6;
	hudGearDisplay->borderColor = fgeal::Color::LIGHT_GREY;
	hudGearDisplay->backgroundColor = fgeal::Color::BLACK;

	gaugeSize.x = hudRpmGauge->bounds.x - font2->getTextWidth("---");
	gaugeSize.w *= 3;
	gaugeSize.h *= 1.7;
	hudSpeedDisplay = new Hud::NumericalDisplay<float>(vehicle.speed, gaugeSize, font2);
	hudSpeedDisplay->valueScale = 3.6;
	hudSpeedDisplay->disableBackground = true;
	hudSpeedDisplay->displayColor = fgeal::Color::WHITE;
	hudSpeedDisplay->borderThickness = 0;

	hudTimer = new Hud::TimerDisplay<float>(laptime, gaugeSize, font2);
	hudTimer->bounds.y = display.getHeight() * 0.1;
	hudTimer->valueScale = 1000;
	hudTimer->disableBackground = true;
	hudTimer->displayColor = Color::WHITE;

	spriteSmokeLeft->scale.x =
			spriteSmokeLeft->scale.y =
					spriteSmokeRight->scale.x =
							spriteSmokeRight->scale.y = display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR*0.75f;

	corneringForceLeechFactor = (vehicle.type == Vehicle::TYPE_BIKE? 0.25 : 0.5);
	vehicle.engine.minRpm = 1000;
	vehicle.engine.automaticShiftingEnabled = true;
	vehicle.engine.automaticShiftingLowerThreshold = 0.5*vehicle.engine.maximumTorqueRpm/vehicle.engine.maxRpm;
	vehicle.engine.automaticShiftingUpperThreshold = vehicle.engine.maximumPowerRpm/vehicle.engine.maxRpm;
	vehicle.engine.gear = 1;
	vehicle.engine.rpm = 100;

	bgParallax.x = bgParallax.y = 0;
	position = 0;
	posX = 0;
	vehicle.speed = 0;
	vehicle.acceleration = 0;
	pseudoAngle = 0;
	laptime = 0;

	isBurningRubber = false;
	fakeBrakeBuildUp = false;

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
}

void Pseudo3DRaceState::render()
{
	Display& display = Display::getInstance();
	display.clear();

	Image::drawFilledRectangle(0, 0, display.getWidth(), display.getHeight(), bgColor);
	bg->draw(bgParallax.x, bgParallax.y + 0.55*display.getHeight() - bg->getHeight());
	bg->draw(bgParallax.x + bg->getWidth(), bgParallax.y + 0.55*display.getHeight() - bg->getHeight());

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
//	sprite.duration = vehicle.speed != 0? 0.1*400.0/(vehicle.speed*sprite.numberOfFrames) : 999;  // sometimes work, sometimes don't
	sprite.duration = vehicle.sprite.frameDuration / sqrt(vehicle.speed);  // this formula doesn't present good tire animation results.
//	sprite.duration = vehicle.speed != 0? 2.0*M_PI*vehicle.engine.tireRadius/(vehicle.speed*sprite.numberOfFrames) : -1;  // this formula should be the physically correct, but still not good visually.
	sprite.computeCurrentFrame();

	const Point vehicleSpritePosition = { 0.5f*(display.getWidth() - sprite.scale.x*vehicle.sprite.frameWidth),
										0.825f*(display.getHeight()- sprite.scale.y*vehicle.sprite.frameHeight) - sprite.scale.y*vehicle.sprite.contactOffset };

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

	hudSpeedDisplay->draw();

	hudRpmGauge->draw();
	hudGearDisplay->draw();
	hudTimer->draw();
	font->drawText("Km/h", (hudSpeedDisplay->bounds.x + hudRpmGauge->bounds.x)/2, hudSpeedDisplay->bounds.y+hudSpeedDisplay->bounds.h, fgeal::Color::WHITE);

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
		sprintf(buffer, "%2.2fkm/h", vehicle.speed*3.6);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Acc.:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s^2", vehicle.acceleration);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Wheel turn pseudo angle:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f", pseudoAngle);
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
		fontDebug->drawText("Braking friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", brakingFriction);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Rolling friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", rollingFriction);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Air friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", airFriction);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Combined friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", (curvePull/coursePositionFactor + brakingFriction + rollingFriction + airFriction));
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Drive force:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", getDriveForce());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Torque:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fNm", vehicle.engine.getCurrentTorque());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Torque proportion:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f%%", 100.f*vehicle.engine.getCurrentTorque()/vehicle.engine.maximumTorque);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Power:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fhp", (vehicle.engine.getCurrentTorque()*vehicle.engine.rpm)/(5252.0 * 1.355818));
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Driven tires load:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", getDrivenWheelsTireLoad());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Longit. Slip Ratio:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f%%", 100.0*getLongitudinalSlipRatio());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Normaliz. Traction Force:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f", getNormalizedTractionForce());
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);


		offset += 25;
		fontDebug->drawText("Wheel Ang. Speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2frad/s", vehicle.engine.getAngularSpeed());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("RPM:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.f", vehicle.engine.rpm);
		font->drawText(std::string(buffer), 55, offset, fgeal::Color::WHITE);

		offset += 18;
		fontDebug->drawText("Gear:", 25, offset, fgeal::Color::WHITE);
		const char* autoLabelTxt = (vehicle.engine.automaticShiftingEnabled? " (auto)":"");
		sprintf(buffer, "%d %s", vehicle.engine.gear, autoLabelTxt);
		font->drawText(std::string(buffer), 60, offset, fgeal::Color::WHITE);


		unsigned currentRangeIndex = engineSound.getRangeIndex(vehicle.engine.rpm);
		for(unsigned i = 0; i < engineSound.getSoundData().size(); i++)
		{
			const std::string format = std::string(engineSound.getSoundData()[i]->isPlaying()==false? " s%u " : currentRangeIndex==i? "[s%u]" : "(s%u)") + " vol: %2.2f pitch: %2.2f";
			sprintf(buffer, format.c_str(), i, engineSound.getSoundData()[i]->getVolume(), engineSound.getSoundData()[i]->getPlaybackSpeed());
			font->drawText(std::string(buffer), display.getWidth() - 200, display.getHeight()/2.0 - i*font->getHeight(), fgeal::Color::WHITE);
		}
	}
}

void Pseudo3DRaceState::update(float delta)
{
	handleInput();
	handlePhysics(delta);

	// course looping control
	const unsigned N = course.lines.size();
	while(position * coursePositionFactor >= N*course.roadSegmentLength) position -= N*course.roadSegmentLength / coursePositionFactor;
	while(position < 0) position += N*course.roadSegmentLength / coursePositionFactor;

	// xxx this should be removed once the simulation allows tire slipping, and thus, car slides when braking when its tires are slipping
	if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN) and fabs(vehicle.speed) > 5.0)
		fakeBrakeBuildUp += delta;
	else
		fakeBrakeBuildUp = 0;

	engineSound.updateSound(vehicle.engine.rpm);

//	if(vehicle.engine.gear == 1 and vehicle.engine.rpm < 0.5*vehicle.engine.maxRpm and Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP))
	if(getDriveForce() == getDrivenWheelsTireLoad())
	{
		if(sndTireBurnoutIntro->isPlaying()) sndTireBurnoutIntro->stop();
		if(sndTireBurnoutLoop->isPlaying()) sndTireBurnoutLoop->stop();

		if(not isBurningRubber)
			sndTireBurnoutStandIntro->play();
		else if(not sndTireBurnoutStandIntro->isPlaying() and not sndTireBurnoutStandLoop->isPlaying())
			sndTireBurnoutStandLoop->loop();

		isBurningRubber = true;
	}
	else if(fabs(vehicle.speed) > MINIMUM_SPEED_BURN_RUBBER_ON_TURN
			and (/*fabs(pseudoAngle) == PSEUDO_ANGLE_MAX*/ fabs(strafeSpeed) == MAXIMUM_STRAFE_SPEED
				            or fakeBrakeBuildUp > 0.75))  // xxx fake braking buildup
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

	laptime += delta;
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
			switch(event.getEventKeyCode())
			{
				case Keyboard::KEY_ESCAPE:
					game.enterState(Pseudo3DCarseGame::MAIN_MENU_STATE_ID);
					break;
				case Keyboard::KEY_R:
					position = 0;
					posX = 0;
					vehicle.speed = 0;
					pseudoAngle = 0;
					bgParallax.x = bgParallax.y = 0;
					vehicle.acceleration = 0;
					laptime = 0;
					break;
				case Keyboard::KEY_T:
					vehicle.engine.automaticShiftingEnabled = !vehicle.engine.automaticShiftingEnabled;
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
				case Keyboard::KEY_LEFT_SHIFT:
					if(vehicle.engine.gear < vehicle.engine.gearCount)
						vehicle.engine.gear++;
					break;
				case Keyboard::KEY_LEFT_CONTROL:
					if(vehicle.engine.gear > 1)
						vehicle.engine.gear--;
					break;
				default:
					break;
			}
		}
	}
}

#include "race_state_physics.hxx"
