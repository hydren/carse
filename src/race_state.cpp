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

RaceState::RaceState(CarseGame* game)
: State(*game),
  font(null), font2(null), bg(null), car(null),
  roadSegmentLength(200), roadWidth(2000), cameraDepth(0.84),
  accelPower(9000.0f),
  position(0), posX(0), speed(0)
{}

RaceState::~RaceState()
{}

void RaceState::initialize()
{
	font = new Font("font.ttf");
	font2 = new Font("font.ttf");
	bg = new Image("bg.jpg");
	car = new Image("car.png");
}

void RaceState::onEnter()
{
	cout << "race start!" << endl;

	lines.clear();
	for(unsigned i = 0; i < 1600; i++)
	{
		Segment line(*this);
		line.z = i*roadSegmentLength;
		if(i > 300 && i < 700) line.curve = 0.5;
		if(i > 750) line.y = sin(i/30.0)*1500;
		lines.push_back(line);
	}

	position = 0;
	posX = 0;
	speed = 0;
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

	const float scale = 5.0;
	float spriteOffset = 0;
	Image::FlipMode flip = Image::FLIP_NONE;

	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_LEFT))
		spriteOffset = 40;

	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_RIGHT))
	{
		spriteOffset = 40;
		flip = Image::FLIP_HORIZONTAL;
	}

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

	posX -= lines[((int)(position/roadSegmentLength))%N].curve * speed * 0.5 * delta;

	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_UP))   speed += accelPower*delta;
	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_DOWN)) speed -= accelPower*delta;
	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_LEFT))  posX += std::min(speed, 5000.0f)*delta;
	if(Keyboard::isKeyPressed(Keyboard::Key::ARROW_RIGHT)) posX -= std::min(speed, 5000.0f)*delta;

	const float rollingFriction = 0.02 * 1500 * 9.81 * (speed==0? 0 : speed > 0? 1 : -1),
			    airFriction = 0.5 * 1.2 * 0.31 * (5e-6 * speed * speed) * 1.81;

	speed -= (rollingFriction + airFriction)*delta;

	position += speed*delta;

	while(position >= N*roadSegmentLength) position -= N*roadSegmentLength;
	while(position < 0) position += N*roadSegmentLength;

}
