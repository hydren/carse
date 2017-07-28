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

static const float GRAVITY_ACCELERATION = 9.8066; // standard gravity (actual value varies with altitude, from 9.7639 to 9.8337)
static const float AIR_DENSITY = 1.2041;  // at sea level, 20ºC (68ºF) (but actually varies significantly with altitude, temperature and humidity)
static const float AIR_FRICTION_COEFFICIENT = 0.31 * 1.81;  // CdA. Hardcoded values are: 0.31 drag coefficient (Cd) and 1.81m2 reference/frontal area (A) of a Nissan 300ZX (Z32)
static const float TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT = 0.75;
static const float TIRE_FRICTION_COEFFICIENT_GRASS = 0.35;
static const float ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT = 0.013;
static const float ROLLING_RESISTANCE_COEFFICIENT_GRASS = 0.100;

// these arbitrary multipliers don't have foundation in physics, but serves as a fine-tuning for the gameplay
#define AIR_FRICTION_ARBITRARY_ADJUST 0.8
#define ROLLING_FRICTION_ARBITRARY_ADJUST 2.5

static const float CURVE_PULL_FACTOR = 0.2;
static const float STEERING_SPEED = 2.0;
static const float PSEUDO_ANGLE_MAX = 1.0;
static const float PSEUDO_ANGLE_THRESHOLD = 0.1;

static const float GLOBAL_VEHICLE_SCALE_FACTOR = 0.0048828125;

/* Tire coefficients
 *
 *          Rolling resist. | Peak static frict. | Kinetic frict.
 * dry alphalt - 0.013 ----------- 0.85 --------------- 0.75
 * wet asphalt - 0.013 ----------- 0.65 --------------- 0.52
 * concrete ---- 0.013 ----------- 0.85 --------------- 0.75
 * gravel ------ 0.020 ----------- 0.60 --------------- 0.55
 * grass ------- 0.100 ----------- 0.42 --------------- 0.35
 * dirt -------- 0.050 ----------- 0.68 --------------- 0.65
 * mud --------- 0.080 ----------- 0.55 --------------- 0.45
 * sand -------- 0.300 ----------- 0.60 --------------- 0.55
 * snow -------- 0.016 ----------- 0.20 --------------- 0.15
 * ice --------- 0.013 ----------- 0.10 --------------- 0.07
 *
 * (extrapolated)
 * water ------- 0.750 ----------- 2.00 --------------- 0.40
 *
 * */

#define sgn(x) (x > 0 ? 1 : x < 0 ? -1 : 0)

// -------------------------------------------------------------------------------

int Pseudo3DRaceState::getId(){ return Pseudo3DCarseGame::RACE_STATE_ID; }

//static
Pseudo3DRaceState* Pseudo3DRaceState::getInstance(fgeal::Game& game) { return static_cast<Pseudo3DRaceState*>(game.getState(Pseudo3DCarseGame::RACE_STATE_ID)); }

Pseudo3DRaceState::Pseudo3DRaceState(CarseGame* game)
: State(*game),
  font(null), font2(null), fontDebug(null), bg(null), music(null), spriteSmokeLeft(null), spriteSmokeRight(null),
  position(0), posX(0), speed(0), pseudoAngle(0), strafeSpeed(0), curvePull(0),
  rollingFriction(0), airFriction(0), brakingFriction(0), corneringForceLeechFactor(0),
  drawParameters(), coursePositionFactor(500),
  course(Course::createDebugCourse(200, 2000)),
  hudRpmGauge(null), hudSpeedDisplay(null), hudGearDisplay(null),
  debugMode(true)
{
	drawParameters.cameraDepth = 0.84;
	drawParameters.drawDistance = 300;
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
	bg = new Image("assets/bg.jpg");
	music = new Music("assets/music_sample.ogg");

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
	course.updateReferences();
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

	fgeal::Display& display = fgeal::Display::getInstance();
	Image* sheet = new Image(vehicle.sheetFilename);
	for(unsigned i = 0; i < vehicle.spriteStateCount; i++)
	{
		if(sheet->getWidth() < static_cast<int>(vehicle.spriteWidth))
			throw std::runtime_error("Invalid sprite width value. Value is smaller than sprite sheet width (no whole sprites could be draw)");

		Sprite* sprite = new Sprite(sheet, vehicle.spriteWidth, vehicle.spriteHeight,
									vehicle.spriteFrameDuration, vehicle.spriteStateFrameCount[i],
									0, i*vehicle.spriteHeight);

		sprite->scale = vehicle.spriteScale * display.getWidth() * GLOBAL_VEHICLE_SCALE_FACTOR;
		sprite->referencePixelY = - (int) vehicle.offset;
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
	gaugeSize.h = 1.5 * font->getFontHeight();
	hudGearDisplay = new Hud::NumericalDisplay<int>(vehicle.engine.gear, gaugeSize, font);
	hudGearDisplay->borderThickness = 6;
	hudGearDisplay->borderColor = fgeal::Color::LIGHT_GREY;
	hudGearDisplay->backgroundColor = fgeal::Color::BLACK;

	gaugeSize.x = hudRpmGauge->bounds.x - font2->getTextWidth("---");
	gaugeSize.w *= 3;
	gaugeSize.h *= 1.7;
	hudSpeedDisplay = new Hud::NumericalDisplay<float>(speed, gaugeSize, font2);
	hudSpeedDisplay->valueScale = 3.6;
	hudSpeedDisplay->disableBackground = true;
	hudSpeedDisplay->displayColor = fgeal::Color::WHITE;
	hudSpeedDisplay->borderThickness = 0;

	corneringForceLeechFactor = (vehicle.type == Vehicle::TYPE_BIKE? 0.25 : 0.5);
	vehicle.engine.minRpm = 1000;
	vehicle.engine.automaticShiftingEnabled = true;
	vehicle.engine.automaticShiftingLowerThreshold = 0.5*vehicle.engine.maximumTorqueRpm/vehicle.engine.maxRpm;
	vehicle.engine.automaticShiftingUpperThreshold = vehicle.engine.maximumPowerRpm/vehicle.engine.maxRpm;
	vehicle.engine.gear = 1;
	vehicle.engine.rpm = 100;

	position = 0;
	posX = 0;
	speed = 0;
	pseudoAngle = 0;

	music->loop();
	engineSound.playIdle();
}

void Pseudo3DRaceState::onLeave()
{
	engineSound.haltSound();
	music->stop();
}

void Pseudo3DRaceState::render()
{
	fgeal::Display& display = fgeal::Display::getInstance();

	display.clear();

	bg->draw();

	course.draw(position * coursePositionFactor, posX, drawParameters);

	// the ammount of pseudo angle that will trigger the last sprite
//	const float PSEUDO_ANGLE_LAST_STATE = PSEUDO_ANGLE_MAX;  // show last sprite when the pseudo angle is at its max
	const float PSEUDO_ANGLE_LAST_STATE = vehicle.spriteMaxDepictedTurnAngle;  // show last sprite when the pseudo angle is at the specified ammount in the .properties

	// linear sprite progression
//	const unsigned animationIndex = (vehicle.spriteStateCount-1)*fabs(pseudoAngle)/PSEUDO_ANGLE_LAST_STATE;

	// exponential sprite progression. may be slower.
//	const unsigned animationIndex = (vehicle.spriteStateCount-1)*(exp(fabs(pseudoAngle))-1)/(exp(PSEUDO_ANGLE_LAST_STATE)-1);

	// linear sprite progression with 1-index advance at threshold angle
	unsigned animationIndex = 0;
	if(vehicle.spriteStateCount > 1 and fabs(pseudoAngle) > PSEUDO_ANGLE_THRESHOLD)
		animationIndex = 1 + (vehicle.spriteStateCount-2)*(fabs(pseudoAngle) - PSEUDO_ANGLE_THRESHOLD)/(PSEUDO_ANGLE_LAST_STATE - PSEUDO_ANGLE_THRESHOLD);

	// cap index to max possible
	if(animationIndex > vehicle.spriteStateCount - 1)
		animationIndex = vehicle.spriteStateCount - 1;

	Sprite& sprite = *spritesVehicle[animationIndex];
	sprite.flipmode = strafeSpeed < 0 and animationIndex > 0? Image::FLIP_HORIZONTAL : Image::FLIP_NONE;
//	sprite.duration = speed != 0? 0.1*400.0/(speed*sprite.numberOfFrames) : 999;  // sometimes work, sometimes don't
	sprite.duration = vehicle.spriteFrameDuration / sqrt(speed);  // this formula doesn't present good tire animation results.
//	sprite.duration = speed != 0? 2.0*M_PI*vehicle.engine.tireRadius/(speed*sprite.numberOfFrames) : -1;  // this formula should be the physically correct, but still not good visually.
	sprite.computeCurrentFrame();

	const Point vehicleSpritePosition = { 0.5f*(display.getWidth() - sprite.scale.x*vehicle.spriteWidth),
										0.825f*(display.getHeight()- sprite.scale.y*vehicle.spriteHeight) - sprite.scale.y*vehicle.offset };

	sprite.draw(vehicleSpritePosition.x, vehicleSpritePosition.y);

	if(fabs(pseudoAngle) == PSEUDO_ANGLE_MAX or
	  (vehicle.engine.gear == 1 and vehicle.engine.rpm < 0.5*vehicle.engine.maxRpm and Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP)))
	{
		const float smokeTurnOffset = (pseudoAngle < 0? -1.f : 1.f)*10.f*animationIndex*vehicle.spriteMaxDepictedTurnAngle;
		spriteSmokeLeft->computeCurrentFrame();
		spriteSmokeLeft->draw(1.25f*vehicleSpritePosition.x - 0.5f*spriteSmokeLeft->width + smokeTurnOffset,
									vehicleSpritePosition.y + sprite.height*sprite.scale.y*0.67f);

		spriteSmokeRight->computeCurrentFrame();
		spriteSmokeRight->draw(2.02f*vehicleSpritePosition.x - 0.5f*spriteSmokeRight->width + smokeTurnOffset,
									 vehicleSpritePosition.y + sprite.height*sprite.scale.y*0.67f);
	}

	hudSpeedDisplay->draw();

	hudRpmGauge->draw();
	hudGearDisplay->draw();
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

		offset += 25;
		fontDebug->drawText("Speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fkm/h", speed*3.6);
		font->drawText(std::string(buffer), 90, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Strafe speed:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s", strafeSpeed/coursePositionFactor);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Curve pull:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm/s", curvePull/coursePositionFactor);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Braking friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", brakingFriction);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Rolling friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", rollingFriction);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Air friction:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", airFriction);
		font->drawText(std::string(buffer), 200, offset, fgeal::Color::WHITE);

		offset += 25;
		fontDebug->drawText("Wheel turn pseudo angle:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f", pseudoAngle);
		font->drawText(std::string(buffer), 250, offset, fgeal::Color::WHITE);

		offset = display.getHeight()-100;
		fontDebug->drawText("RPM:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.f", vehicle.engine.rpm);
		font->drawText(std::string(buffer), 55, offset, fgeal::Color::WHITE);

		offset -= 25;
		fontDebug->drawText("Gear:", 25, offset, fgeal::Color::WHITE);
		const char* autoLabelTxt = (vehicle.engine.automaticShiftingEnabled? " (auto)":"");
		sprintf(buffer, "%d %s", vehicle.engine.gear, autoLabelTxt);
		font->drawText(std::string(buffer), 60, offset, fgeal::Color::WHITE);

		offset -= 25;
		fontDebug->drawText("Drive force:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", vehicle.engine.getDriveForce());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset -= 25;
		fontDebug->drawText("Torque:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fNm", vehicle.engine.getCurrentTorque());
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		offset -= 25;
		fontDebug->drawText("Torque proportion:", 25, offset, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2f%%", 100.f*vehicle.engine.getCurrentTorque()/vehicle.engine.maximumTorque);
		font->drawText(std::string(buffer), 180, offset, fgeal::Color::WHITE);

		unsigned currentRangeIndex = engineSound.getRangeIndex(vehicle.engine.rpm);
		for(unsigned i = 0; i < engineSound.getSoundData().size(); i++)
		{
			const std::string format = std::string(engineSound.getSoundData()[i]->isPlaying()==false? " s%u " : currentRangeIndex==i? "[s%u]" : "(s%u)") + " vol: %2.2f pitch: %2.2f";
			sprintf(buffer, format.c_str(), i, engineSound.getSoundData()[i]->getVolume(), engineSound.getSoundData()[i]->getPlaybackSpeed());
			font->drawText(std::string(buffer), display.getWidth() - 200, display.getHeight()/2.0 - i*font->getFontHeight(), fgeal::Color::WHITE);
		}
	}
}

void Pseudo3DRaceState::update(float delta)
{
	handleInput();
	handlePhysics(delta);
	engineSound.updateSound(vehicle.engine.rpm);
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
					speed = 0;
					pseudoAngle = 0;
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

void Pseudo3DRaceState::handlePhysics(float delta)
{
	vehicle.engine.update(speed);

	const float throttle = Keyboard::isKeyPressed(Keyboard::KEY_ARROW_UP)? 1.0 : 0.0;
	const float braking =  Keyboard::isKeyPressed(Keyboard::KEY_ARROW_DOWN)? 1.0 : 0.0;
	const float wheelAngleFactor = 1 - corneringForceLeechFactor*fabs(pseudoAngle)/PSEUDO_ANGLE_MAX;

	const bool onGrass = (fabs(posX) > 1.2*course.roadWidth);
	const float tireFrictionCoefficient = onGrass? TIRE_FRICTION_COEFFICIENT_GRASS : TIRE_FRICTION_COEFFICIENT_DRY_ASPHALT,
				rollingResistanceCoefficient = onGrass? ROLLING_RESISTANCE_COEFFICIENT_GRASS : ROLLING_RESISTANCE_COEFFICIENT_DRY_ASPHALT;

	const float tireFriction = tireFrictionCoefficient * vehicle.mass * GRAVITY_ACCELERATION * sgn(speed);
	brakingFriction = braking * tireFriction;
	rollingFriction = rollingResistanceCoefficient * vehicle.mass * GRAVITY_ACCELERATION * sgn(speed) * ROLLING_FRICTION_ARBITRARY_ADJUST;
	airFriction = 0.5 * AIR_DENSITY * AIR_FRICTION_COEFFICIENT * speed * speed * AIR_FRICTION_ARBITRARY_ADJUST;

	// update speed
	speed += delta*(wheelAngleFactor*throttle*vehicle.engine.getDriveForce() - brakingFriction - rollingFriction - airFriction)/vehicle.mass;

	// update position
	position += speed*delta;

	// update steering
	if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_LEFT))
	{
		if(pseudoAngle < 0) pseudoAngle *= 1/(1+5*delta);
		pseudoAngle += delta * STEERING_SPEED;
	}
	else if(Keyboard::isKeyPressed(Keyboard::KEY_ARROW_RIGHT))
	{
		if(pseudoAngle > 0) pseudoAngle *= 1/(1+5*delta);
		pseudoAngle -= delta * STEERING_SPEED;
	}
	else pseudoAngle *= 1/(1+5*delta);

	if(pseudoAngle > PSEUDO_ANGLE_MAX) pseudoAngle = PSEUDO_ANGLE_MAX;
	if(pseudoAngle <-PSEUDO_ANGLE_MAX) pseudoAngle =-PSEUDO_ANGLE_MAX;

	// update strafing
	strafeSpeed = pseudoAngle * speed * coursePositionFactor;

	// limit strafing speed by tire friction
//	if(strafeSpeed >  tireFriction) strafeSpeed = tireFriction;
//	if(strafeSpeed < -tireFriction) strafeSpeed =-tireFriction;

	// limit strafing speed by magic constant
	if(strafeSpeed >  15000) strafeSpeed = 15000;
	if(strafeSpeed < -15000) strafeSpeed =-15000;

	const unsigned N = course.lines.size();
	const float curve = course.lines[((int)(position*coursePositionFactor/course.roadSegmentLength))%N].curve;

	// update curve pull
	curvePull = curve * speed * coursePositionFactor * CURVE_PULL_FACTOR;

	// update strafe position
	posX += (strafeSpeed - curvePull)*delta;

	// course looping control
	while(position * coursePositionFactor >= N*course.roadSegmentLength) position -= N*course.roadSegmentLength / coursePositionFactor;
	while(position < 0) position += N*course.roadSegmentLength / coursePositionFactor;
}
