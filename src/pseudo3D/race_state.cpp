/*
 * race_state.cpp
 *
 *  Created on: 29 de mar de 2017
 *      Author: carlosfaruolo
 */

#include "race_state.hpp"

// xxx debug code =======
#include <iostream>
using std::cout;
using std::endl;
// xxx debug code =======

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

//custom call to draw quad
void drawQuad(const Color& c, float x1, float y1, float w1, float x2, float y2, float w2)
{
	Image::drawQuadrangle(c, x1-w1, y1, x2-w2, y2, x2+w2, y2, x1+w1, y1);
}

// -------------------------------------------------------------------------------

int Pseudo3DRaceState::getId(){ return CarseGame::RACE_STATE_ID; }

Pseudo3DRaceState::Pseudo3DRaceState(CarseGame* game)
: State(*game),
  font(null), font2(null), bg(null), music(null),
  cameraDepth(0.84),
  position(0), posX(0), speed(0), strafeSpeed(0),
  course(Course::createDebugCourse(200, 2000)),
  autoTransmission(true)
{}

Pseudo3DRaceState::~Pseudo3DRaceState()
{
	delete font;
	delete font2;
	delete bg;
	delete music;
	for(unsigned i = 0; i < spritesVehicle.size(); i++)
		delete spritesVehicle[i];
}

void Pseudo3DRaceState::initialize()
{
	font = new Font("assets/font.ttf");
	font2 = new Font("assets/font.ttf");
	bg = new Image("assets/bg.jpg");
	music = new Music("assets/music_sample.ogg");
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
		for(unsigned i = 0; i < spritesVehicle.size(); i++)
			delete spritesVehicle[i];

		spritesVehicle.clear();
	}

	for(unsigned i = 0; i < vehicle.spriteStateCount; i++)
	{
		Image* sheet = new Image(vehicle.sheetFilename);

		if(sheet->getWidth() < static_cast<int>(vehicle.spriteWidth))
			throw std::runtime_error("Invalid sprite width value. Value is smaller than sprite sheet width (no whole sprites could be draw)");

		Sprite* sprite = new Sprite(sheet, vehicle.spriteWidth, vehicle.spriteHeight,
									vehicle.spriteFrameDuration, vehicle.spriteStateFrameCount[i],
									0, i*vehicle.spriteHeight, true);
		spritesVehicle.push_back(sprite);
	}

	engineSound.setProfile(vehicle.engineSoundProfile, vehicle.engine.maxRpm);

	vehicle.engine.gear = 1;
	vehicle.engine.rpm = 100;

	position = 0;
	posX = 0;
	speed = 0;
	strafeSpeed = 0;

	music->loop();
	engineSound.playIdle();

	cout << "race start!" << endl;
}

void Pseudo3DRaceState::onLeave()
{
	cout << "race end!" << endl;

	engineSound.haltSound();
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
		l.project(posX - x, camHeight, position - (n>N?n*course.roadSegmentLength:0), cameraDepth);
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

	const float scale = display.getWidth() * 0.0048828125f * vehicle.spriteScale;
	unsigned animationIndex = (vehicle.spriteStateCount-1)*fabs(strafeSpeed)/9000.0 + (10-vehicle.spriteStateCount)/9.0;
	if(animationIndex > vehicle.spriteStateCount-1)  animationIndex = vehicle.spriteStateCount-1;
	spritesVehicle[animationIndex]->flipmode = strafeSpeed < 0 and animationIndex > 0? Image::FLIP_HORIZONTAL : Image::FLIP_NONE;
	spritesVehicle[animationIndex]->scale.x = scale;
	spritesVehicle[animationIndex]->scale.y = scale;
	spritesVehicle[animationIndex]->draw(0.5*(display.getWidth() - scale*vehicle.spriteWidth), display.getHeight()-1.5*scale*vehicle.spriteHeight);

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
		font->drawText(std::string(buffer)+(autoTransmission? " (auto)":""), 60, display.getHeight()-100+25, fgeal::Color::WHITE);

		font2->drawText("Drive force:", 25, display.getHeight()-100+50, fgeal::Color::WHITE);
		sprintf(buffer, "%2.2fN", vehicle.engine.getDriveForce());
		font->drawText(std::string(buffer), 180, display.getHeight()-100+50, fgeal::Color::WHITE);

		unsigned currentRangeIndex = engineSound.getRangeIndex(vehicle.engine.rpm);

		for(unsigned i = 0; i < engineSound.getSoundData().size(); i++)
		{
			const std::string format = std::string(engineSound.getSoundData()[i]->isPlaying()==false? " s%u " : currentRangeIndex==i? "[s%u]" : "(s%u)") + " vol: %2.2f pitch: %2.2f";
			sprintf(buffer, format.c_str(), i, engineSound.getSoundData()[i]->getVolume(), engineSound.getSoundData()[i]->getPlaybackSpeed());
			font->drawText(std::string(buffer), display.getWidth() - 200, display.getHeight()/2.0 - i*font->getSize(), fgeal::Color::WHITE);
		}
	}

	fgeal::rest(0.01);
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
				case Keyboard::Key::T:
					autoTransmission = !autoTransmission;
					break;
				case Keyboard::Key::M:
					if(music->isPlaying())
						music->pause();
					else
						music->resume();
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

	vehicle.engine.rpm = (speed/vehicle.engine.tireRadius) * vehicle.engine.gearRatio[vehicle.engine.gear] * vehicle.engine.gearRatio[0] * (30.0f/M_PI) * 0.002;
	if(vehicle.engine.rpm < 1000)
		vehicle.engine.rpm = 1000;

	if(autoTransmission)
	{
		if(vehicle.engine.gear < vehicle.engine.gearCount
		and vehicle.engine.rpm > 0.95f*vehicle.engine.maxRpm)
			vehicle.engine.gear++;

		if(vehicle.engine.gear > 1
		and vehicle.engine.rpm < 0.57f*vehicle.engine.maxRpm)
			vehicle.engine.gear--;
	}

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
}
