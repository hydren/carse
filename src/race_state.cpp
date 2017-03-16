/*
 * race_state.cpp
 *
 *  Created on: 5 de dez de 2016
 *      Author: Felipe
 */

#include "race_state.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdio>

using std::cout;
using std::endl;
using fgeal::Display;
using fgeal::Image;
using fgeal::Font;
using fgeal::Color;
using fgeal::Keyboard;
using fgeal::Event;
using fgeal::EventQueue;
using fgeal::Sound;
using fgeal::Music;

RaceState::RaceState(CarseGame* game)
: State(*game),
  font(null), font2(null), bg(null), car(null),
  music(null), soundEngine(null), soundEngineCount(6),
  roadSegmentLength(200), roadWidth(2000), cameraDepth(0.84),
  position(0), posX(0), speed(0), strafeSpeed(0),
  carWeight(1500)
{}

RaceState::~RaceState()
{
	delete font;
	delete font2;
	delete bg;
	delete car;
	delete music;
	for(unsigned i = 0; i < soundEngineCount; i++) delete soundEngine[i];
	delete[] soundEngine;
}

void RaceState::initialize()
{
	font = new Font("font.ttf");
	font2 = new Font("font.ttf");
	bg = new Image("bg.jpg");
	car = new Image("car.png");
	music = new Music("music_sample.ogg");
	soundEngine = new Sound*[soundEngineCount];
	soundEngine[0] = new Sound("rev_idle_300zx.ogg");
	soundEngine[1] = new Sound("rev_low_300zx.ogg");
	soundEngine[2] = new Sound("rev_midlow_300zx.ogg");
	soundEngine[3] = new Sound("rev_midhigh_300zx.ogg");
	soundEngine[4] = new Sound("rev_high_300zx.ogg");
	soundEngine[5] = new Sound("rev_over_300zx.ogg");

	engine.gearCount = 5;
	engine.gearRatio = new float[engine.gearCount];
	engine.gearRatio[0] = 3.69f;  // differential
	engine.gearRatio[1] = 3.214f; // 1st gear
	engine.gearRatio[2] = 1.925f; // 2nd gear
	engine.gearRatio[3] = 1.302f; // 3th gear
	engine.gearRatio[4] = 1.000f; // 4th gear
	engine.gearRatio[5] = 0.752;  // 5th gear
	engine.reverseGearRatio = 3.369f;
	engine.maxRpm = 7000;
	engine.torque = 500;
	engine.wheelRadius = 0.34;

	engine.gear = 1;
	engine.rpm = 100;
}

void RaceState::onEnter()
{
	cout << "race start!" << endl;

	lines.clear();
	for(unsigned i = 0; i < 1600; i++)
	{
		Segment line(*this);
		line.z = i*roadSegmentLength;
		if(i > 300 && i < 500) line.curve = 0.3;
		if(i > 500 && i < 700) line.curve = -0.3;
		if(i > 900 && i < 1300) line.curve = -2.2;
		if(i > 750) line.y = sin(i/30.0)*1500;
		lines.push_back(line);
	}

	position = 0;
	posX = 0;
	speed = 0;
	strafeSpeed = 0;

	music->loop();
	soundEngine[0]->loop();
}

void RaceState::onLeave()
{
	cout << "race end!" << endl;
}

RaceState::Segment::Segment(RaceState& state) // @suppress("Class members should be properly initialized")
: state(state) {curve=x=y=z=0;}

RaceState::Segment& RaceState::Segment::operator= (const Segment& s)
{
	if(this != &s)
	{
		delete this;
		new (this) Segment(s.state);
		x = s.x; y = s.y; z = s.z;
		X = s.X; Y = s.Y; W = s.W;
		scale = s.scale;
	}
	return *this;
}

void RaceState::Segment::project(int camX, int camY, int camZ)
{
	fgeal::Display& display = fgeal::Display::getInstance();
	scale = state.cameraDepth / (z - camZ);
	X = (1 + scale*(x + camX)) * display.getWidth()/2;
	Y = (1 - scale*(y - camY)) * display.getHeight()/2;
	W = scale * state.roadWidth * display.getWidth()/2;
}

void drawQuad(const Color& c, float x1, float y1, float w1, float x2, float y2, float w2)
{
	Image::drawQuadrangle(c, x1-w1, y1, x2-w2, y2, x2+w2, y2, x1+w1, y1);
}

void RaceState::render()
{
	fgeal::Display& display = fgeal::Display::getInstance();

	display.clear();

	bg->draw();

	const unsigned N = lines.size(), fromPos = position/roadSegmentLength;
	float camHeight = 1500 + lines[fromPos].y;
	float x = 0, dx = 0;
	float maxY = display.getHeight();

	for(unsigned n = fromPos+1; n < fromPos+300; n++)
	{
		Segment& l = lines[n%N];
		l.project(posX - x, camHeight, position - (n>N?n*roadSegmentLength:0));
		x += dx;
		dx += l.curve;

		if(l.Y > maxY) continue;
		maxY = l.Y;

		Color grass  = (n/3)%2? Color(16, 200, 16) : Color(  0, 154,   0);
		Color rumble = (n/3)%2? Color(255,255,255) : Color(255,   0,   0);
		Color road   = (n/3)%2? Color(107,107,107) : Color(105, 105, 105);

		Segment& p = lines[(n-1)%N]; // previous line

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
		sprintf(buffer, "%2.f", engine.rpm);
		font->drawText(std::string(buffer), 55, display.getHeight()-100, fgeal::Color::WHITE);

		font2->drawText("Gear:", 25, display.getHeight()-100+25, fgeal::Color::WHITE);
		sprintf(buffer, "%d", engine.gear);
		font->drawText(std::string(buffer), 60, display.getHeight()-100+25, fgeal::Color::WHITE);

		font2->drawText("Drive force:", 25, display.getHeight()-100+50, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", engine.getDriveForce());
		font->drawText(std::string(buffer), 180, display.getHeight()-100+50, fgeal::Color::WHITE);
	}

	fgeal::rest(0.01);
}

void RaceState::update(float delta)
{
	handleInput();
	handlePhysics(delta);
}

void RaceState::handleInput()
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
					if(engine.gear < engine.gearCount)
						engine.gear++;
					break;
				case Keyboard::Key::LEFT_CONTROL:
					if(engine.gear > 1)
						engine.gear--;
					break;
				default:
					break;
			}
		}
	}
}

void RaceState::handlePhysics(float delta)
{
	const unsigned N = lines.size();

	const float curve = lines[((int)(position/roadSegmentLength))%N].curve;
	posX -= atan(curve) * speed * 0.5 * delta;

	engine.rpm = (speed/engine.wheelRadius) * engine.gearRatio[engine.gear] * engine.gearRatio[0] * (30.0f/M_PI) * 0.002;
	if(engine.rpm < 1000)
		engine.rpm = 1000;

	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_UP))   speed += (engine.getDriveForce() * delta)/carWeight;
	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_DOWN)) speed -= (engine.getDriveForce() * delta)/carWeight;

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

	const float rollingFriction = 0.02 * carWeight * 9.81 * (speed==0? 0 : speed > 0? 1 : -1),
			    airFriction = 0.5 * 1.2 * 0.31 * (5e-6 * speed * speed) * 1.81,
				turnFriction = Keyboard::isKeyPressed(Keyboard::Key::ARROW_LEFT) or Keyboard::isKeyPressed(Keyboard::Key::ARROW_RIGHT)? 2000 : 0;

	speed -= (rollingFriction + airFriction + turnFriction)*delta;

	position += speed*delta;

	while(position >= N*roadSegmentLength) position -= N*roadSegmentLength;
	while(position < 0) position += N*roadSegmentLength;

	bool rangeTouched = false;
	const unsigned soundRange = engine.maxRpm / (soundEngineCount-1);
	for(unsigned i = 0; i < soundEngineCount; i++)
	{
		if((engine.rpm > i*soundRange and engine.rpm < (i+1)*soundRange)
		   or (i == soundEngineCount-1 and not rangeTouched))
		{
			if(not rangeTouched)
			{
				rangeTouched = true;
				if(not soundEngine[i]->isPlaying())
					soundEngine[i]->loop();
			}
		}
		else soundEngine[i]->stop();
	}
}

float RaceState::Engine::getDriveForce()
{
	return (rpm < maxRpm? torque : 0) * gearRatio[gear] * gearRatio[0] * 0.765 * wheelRadius * 5000.0;
}
