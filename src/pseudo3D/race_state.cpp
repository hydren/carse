/*
 * race_state.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "race_state.hpp"

// for debug =======
#include <iostream>
using std::cout;
using std::endl;
// =================

#include <algorithm>
#include <cstdio>
#include <cmath>

using fgeal::Display;
using fgeal::Image;
using fgeal::Font;
using fgeal::Color;
using fgeal::Keyboard;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Sound;
using fgeal::Music;

//custom call to draw quad
void drawQuad(const Color& c, float x1, float y1, float w1, float x2, float y2, float w2)
{
	Image::drawQuadrangle(c, x1-w1, y1, x2-w2, y2, x2+w2, y2, x1+w1, y1);
}

//calculates engine sound pitch for given RPM and max RPM
float calculatePitch(float rpmDiff, float maxRpm)
{
	return 1 + 0.5*rpmDiff/maxRpm;
}

// -------------------------------------------------------------------------------

Pseudo3DRaceState::Pseudo3DRaceState(CarseGame* game)
: State(*game),
  font(null), font2(null), bg(null), car(null), music(null),
  position(0), posX(0), speed(0), strafeSpeed(0),
  course(*this)
{
	course.roadSegmentLength = 200;
	course.roadWidth = 2000;
	course.cameraDepth = 0.84;

	vehicle.mass = 1500;
	vehicle.engine.gearCount = 5;
	vehicle.engine.gearRatio = new float[vehicle.engine.gearCount];
	vehicle.engine.gearRatio[0] = 3.69f;  // differential
	vehicle.engine.gearRatio[1] = 3.214f; // 1st gear
	vehicle.engine.gearRatio[2] = 1.925f; // 2nd gear
	vehicle.engine.gearRatio[3] = 1.302f; // 3th gear
	vehicle.engine.gearRatio[4] = 1.000f; // 4th gear
	vehicle.engine.gearRatio[5] = 0.752;  // 5th gear
	vehicle.engine.reverseGearRatio = 3.369f;
	vehicle.engine.maxRpm = 7000;
	vehicle.engine.torque = 500;
	vehicle.engine.wheelRadius = 0.34;
}

Pseudo3DRaceState::~Pseudo3DRaceState()
{
	delete font;
	delete font2;
	delete bg;
	delete car;
	delete music;
	for(unsigned i = 0; i < soundEngine.size(); i++) delete soundEngine[i].second;
}

void Pseudo3DRaceState::initialize()
{
	font = new Font("font.ttf");
	font2 = new Font("font.ttf");
	bg = new Image("bg.jpg");
	car = new Image("car.png");
	music = new Music("music_sample.ogg");

	soundEngine.push_back(std::make_pair(0   , new Sound("rev_idle_300zx.ogg")));
	soundEngine.push_back(std::make_pair(1100, new Sound("rev_low_300zx.ogg")));
	soundEngine.push_back(std::make_pair(2000, new Sound("rev_midlow_300zx.ogg")));
	soundEngine.push_back(std::make_pair(4500, new Sound("rev_midhigh_300zx.ogg")));
	soundEngine.push_back(std::make_pair(6000, new Sound("rev_high_300zx.ogg")));
	soundEngine.push_back(std::make_pair(6950, new Sound("rev_over_300zx.ogg")));

	// generating hardcoded course
	for(unsigned i = 0; i < 1600; i++)
	{
		Course::Segment line(course);
		line.z = i*course.roadSegmentLength;
		if(i > 300 && i < 500) line.curve = 0.3;
		if(i > 500 && i < 700) line.curve = -0.3;
		if(i > 900 && i < 1300) line.curve = -2.2;
		if(i > 750) line.y = sin(i/30.0)*1500;
		course.lines.push_back(line);
	}
}

void Pseudo3DRaceState::onEnter()
{
	cout << "race start!" << endl;

	vehicle.engine.gear = 1;
	vehicle.engine.rpm = 100;

	position = 0;
	posX = 0;
	speed = 0;
	strafeSpeed = 0;

	music->loop();
	soundEngine[0].second->loop();
}

void Pseudo3DRaceState::onLeave()
{
	cout << "race end!" << endl;
	for(unsigned i = 0; i < soundEngine.size(); i++)
		soundEngine[i].second->stop();
	music->stop();
}

void Pseudo3DRaceState::render()
{
	fgeal::Display& display = fgeal::Display::getInstance();

	display.clear();

	bg->draw();

	const unsigned N = course.lines.size(), fromPos = position/course.roadSegmentLength;
	float camHeight = 1500 + course.lines[fromPos].y;
	float x = 0, dx = 0;
	float maxY = display.getHeight();

	for(unsigned n = fromPos+1; n < fromPos+300; n++)
	{
		Course::Segment& l = course.lines[n%N];
		l.project(posX - x, camHeight, position - (n>N?n*course.roadSegmentLength:0));
		x += dx;
		dx += l.curve;

		if(l.Y > maxY) continue;
		maxY = l.Y;

		Color grass  = (n/3)%2? Color(16, 200, 16) : Color(  0, 154,   0);
		Color rumble = (n/3)%2? Color(255,255,255) : Color(255,   0,   0);
		Color road   = (n/3)%2? Color(107,107,107) : Color(105, 105, 105);

		Course::Segment& p = course.lines[(n-1)%N]; // previous line

		drawQuad(grass,  0,   p.Y, display.getWidth(), 0, l.Y, display.getWidth());
		drawQuad(rumble, p.X, p.Y, p.W*1.2, l.X, l.Y, l.W*1.2);
		drawQuad(road,   p.X, p.Y, p.W, l.X, l.Y, l.W);
	}

	const float scale = display.getWidth() * 0.0048828125;
	float spriteOffset = 0;
	Image::FlipMode flip = Image::FLIP_NONE;

	if(fabs(strafeSpeed) > 10000)
		spriteOffset = 80;
	else if(fabs(strafeSpeed) > 1000)
		spriteOffset = 40;

	if(strafeSpeed < 0)
		flip = Image::FLIP_HORIZONTAL;

	car->drawScaledRegion(0.5*(display.getWidth() - scale*car->getWidth()), display.getHeight()-0.5*scale*car->getHeight(), scale, scale, flip, 0, spriteOffset, 80, 40);

	char buffer[512];

	// DEBUG
	{
		font2->drawText("FPS:", 25, 25, fgeal::Color::WHITE);
		sprintf(buffer, "%d", game.getFpsCount());
		font->drawText(std::string(buffer), 55, 25, fgeal::Color::WHITE);

		font2->drawText("Position:", 25, 50, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fm", position);
		font->drawText(std::string(buffer), 90, 50, fgeal::Color::WHITE);

		font2->drawText("Speed:", 25, 75, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fkm/h", speed/120);
		font->drawText(std::string(buffer), 90, 75, fgeal::Color::WHITE);

		font2->drawText("Strafe speed:", 25, 100, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fkm/h", strafeSpeed/120);
		font->drawText(std::string(buffer), 180, 100, fgeal::Color::WHITE);

		font2->drawText("RPM:", 25, display.getHeight()-100, fgeal::Color::WHITE);
		sprintf(buffer, "%2.f", vehicle.engine.rpm);
		font->drawText(std::string(buffer), 55, display.getHeight()-100, fgeal::Color::WHITE);

		font2->drawText("Gear:", 25, display.getHeight()-100+25, fgeal::Color::WHITE);
		sprintf(buffer, "%d", vehicle.engine.gear);
		font->drawText(std::string(buffer), 60, display.getHeight()-100+25, fgeal::Color::WHITE);

		font2->drawText("Drive force:", 25, display.getHeight()-100+50, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", vehicle.engine.getDriveForce());
		font->drawText(std::string(buffer), 180, display.getHeight()-100+50, fgeal::Color::WHITE);

		unsigned currentRangeIndex = 0;
		for(unsigned i = 0; i < soundEngine.size(); i++)
			if(vehicle.engine.rpm > soundEngine[i].first)
				currentRangeIndex = i;

		for(unsigned i = 0; i < soundEngine.size(); i++)
		{
			const std::string format = std::string(soundEngine[i].second->isPlaying()==false? " s%u " : currentRangeIndex==i? "[s%u]" : "(s%u)") + " vol: %2.2f pitch: %2.2f";
			sprintf(buffer, format.c_str(), i, soundEngine[i].second->getVolume(), soundEngine[i].second->getPlaybackSpeed());
			font->drawText(std::string(buffer), display.getWidth() - 200, display.getHeight()/2.0 - i*font->getSize(), fgeal::Color::WHITE);
		}
	}

	fgeal::rest(0.01);
}

void Pseudo3DRaceState::update(float delta)
{
	handleInput();
	handlePhysics(delta);
}

void Pseudo3DRaceState::handleInput()
{
	Event event;
	EventQueue& eventQueue = EventQueue::getInstance();
	while(not eventQueue.isEmpty())
	{
		eventQueue.waitNextEvent(&event);
		if(event.getEventType() == Event::Type::DISPLAY_CLOSURE)
		{
			//game.enterState(CarseGame::MENU_STATE_ID);
			game.running = false;
		}
		else if(event.getEventType() == Event::Type::KEY_PRESS)
		{
			switch(event.getEventKeyCode())
			{
				case Keyboard::Key::ESCAPE:
					game.running = false;
					break;
				case Keyboard::Key::R:
					position = 0;
					posX = 0;
					speed = 0;
					strafeSpeed = 0;
					break;
				case Keyboard::Key::LEFT_SHIFT:
					if(vehicle.engine.gear < vehicle.engine.gearCount)
						vehicle.engine.gear++;
					break;
				case Keyboard::Key::LEFT_CONTROL:
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
	const unsigned N = course.lines.size();

	const float curve = course.lines[((int)(position/course.roadSegmentLength))%N].curve;
	posX -= atan(curve) * speed * 0.5 * delta;

	vehicle.engine.rpm = (speed/vehicle.engine.wheelRadius) * vehicle.engine.gearRatio[vehicle.engine.gear] * vehicle.engine.gearRatio[0] * (30.0f/M_PI) * 0.002;
	if(vehicle.engine.rpm < 1000)
		vehicle.engine.rpm = 1000;

	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_UP))   speed += (vehicle.engine.getDriveForce() * delta)/vehicle.mass;
	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_DOWN)) speed -= (vehicle.engine.getDriveForce() * delta)/vehicle.mass;

	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_LEFT))
	{
		if(strafeSpeed < 0) strafeSpeed *= 1/(1+5*delta);
		strafeSpeed += speed*delta;
	}
	else if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_RIGHT))
	{
		if(strafeSpeed > 0) strafeSpeed *= 1/(1+5*delta);
		strafeSpeed -= speed*delta;
	}
	else strafeSpeed *= 1/(1+5*delta);

	if(strafeSpeed >  15000.f) strafeSpeed = 15000.f;
	if(strafeSpeed < -15000.f) strafeSpeed = -15000.f;

	posX += strafeSpeed*delta;

	const float rollingFriction = 0.02 * vehicle.mass * 9.81 * (speed==0? 0 : speed > 0? 1 : -1),
			    airFriction = 0.5 * 1.2 * 0.31 * (5e-6 * speed * speed) * 1.81,
				turnFriction = Keyboard::isKeyPressed(Keyboard::Key::ARROW_LEFT) or Keyboard::isKeyPressed(Keyboard::Key::ARROW_RIGHT)? 2000 : 0;

	speed -= (rollingFriction + airFriction + turnFriction)*delta;

	position += speed*delta;

	while(position >= N*course.roadSegmentLength) position -= N*course.roadSegmentLength;
	while(position < 0) position += N*course.roadSegmentLength;

	if(soundEngine.size() > 0 and vehicle.engine.rpm > 0) // its no use if there is no engine sound or rpm is too low
	{
		unsigned currentRangeIndex = 0;
		for(unsigned i = 0; i < soundEngine.size(); i++)
			if(vehicle.engine.rpm > soundEngine[i].first)
				currentRangeIndex = i;

		fgeal::Sound* currentSoundEngine = soundEngine[currentRangeIndex].second;
		const float lowerRpmCurrent = soundEngine[currentRangeIndex].first;
		const float upperRpmCurrent = currentRangeIndex+1 < soundEngine.size()? soundEngine[currentRangeIndex+1].first : vehicle.engine.maxRpm;
		const float rangeSizeCurrent = upperRpmCurrent - lowerRpmCurrent;

		currentSoundEngine->setVolume(1.0f);
		if(currentRangeIndex+1 < soundEngine.size())
			currentSoundEngine->setPlaybackSpeed(calculatePitch(vehicle.engine.rpm - lowerRpmCurrent, vehicle.engine.maxRpm));

		if(not currentSoundEngine->isPlaying())
			currentSoundEngine->loop();

		for(unsigned i = 0; i < soundEngine.size(); i++)
		{
			if(i == currentRangeIndex) continue;

			else if(i + 1 == currentRangeIndex and vehicle.engine.rpm - lowerRpmCurrent < 0.25*rangeSizeCurrent and currentRangeIndex > 0)  // preceding range
			{
//				soundEngine[i].second->setVolume(1.0 - 4*(engine.rpm - lowerRpmCurrent)/rangeSizeCurrent); // linear fade out
				soundEngine[i].second->setVolume(sqrt(1-16*pow((vehicle.engine.rpm - lowerRpmCurrent)/rangeSizeCurrent, 2))); // quadratic fade out

				soundEngine[i].second->setPlaybackSpeed(calculatePitch(vehicle.engine.rpm - soundEngine[i].first, vehicle.engine.maxRpm));
				if(not soundEngine[i].second->isPlaying())
					soundEngine[i].second->loop();
			}

			else if(i == currentRangeIndex + 1 and vehicle.engine.rpm - lowerRpmCurrent > 0.75*rangeSizeCurrent and currentRangeIndex < soundEngine.size()-2)  // succeeding range
			{
//				soundEngine[i].second->setVolume(-3 + 4*(engine.rpm - lowerRpmCurrent)/rangeSizeCurrent); // linear fade in
				soundEngine[i].second->setVolume(sqrt(1-pow(4*((vehicle.engine.rpm - lowerRpmCurrent)/rangeSizeCurrent)-4, 2)) ); // quadratic fade in

				soundEngine[i].second->setPlaybackSpeed(calculatePitch(vehicle.engine.rpm - soundEngine[i].first, vehicle.engine.maxRpm));
				if(not soundEngine[i].second->isPlaying())
					soundEngine[i].second->loop();
			}

			else soundEngine[i].second->stop();
		}
	}
}
