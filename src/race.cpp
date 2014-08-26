/*
 * race.cpp
 *
 *  Created on: 25/08/2014
 *      Author: carlosfaruolo
 */

#include "race.hpp"

#include "util.hpp"
#include "game_engine.hpp"

using GameEngine::Image;

//the race camera
Rect camera;
bool running = true;

Image* car_sprite;

int posx=20, posy=20;
float angle = 0;

GameEngine::EventQueue* eventQueue;
GameEngine::Event* ev;

bool isKeyUpPressed = false,
	isKeyDownPressed = false,
	isKeyRightPressed = false,
	isKeyLeftPressed = false;

Race::Race()
{
	camera.w = GameEngine::display->getWidth();
	camera.h = GameEngine::display->getHeight();
	camera.x = camera.y = 0;

	car_sprite = new Image("car-delorean-dmc12.png");
	eventQueue = new GameEngine::EventQueue;
}

Race::~Race()
{
	delete car_sprite;
}

void Race::start()
{
	cout << "race start!" << endl;
	do
	{
		handleInput();
		handlePhysics();
		handleRender();
	}
	while(running);
}

void Race::handleInput()
{
	while(not eventQueue->isEmpty())
	{
		ev = eventQueue->waitForEvent();

		if(ev->getEventType() == GameEngine::Event::Type::DISPLAY_CLOSURE)
		{
			running=false;
		}
		else if(ev->getEventType() == GameEngine::Event::Type::KEY_PRESS)
		{
			switch(ev->getEventKeyCode())
			{
			case GameEngine::Event::Key::ARROW_UP:
				isKeyUpPressed = true;
				break;
			case GameEngine::Event::Key::ARROW_DOWN:
				isKeyDownPressed = true;
				break;
			case GameEngine::Event::Key::ARROW_RIGHT:
				isKeyRightPressed = true;
				break;
			case GameEngine::Event::Key::ARROW_LEFT:
				isKeyLeftPressed = true;
				break;
			case GameEngine::Event::Key::ESCAPE:
				break;
			case GameEngine::Event::Key::ENTER:
				break;
			default:
				break;
			}
		}
		else if(ev->getEventType() == GameEngine::Event::Type::KEY_RELEASE)
		{
			switch(ev->getEventKeyCode())
			{
			case GameEngine::Event::Key::ARROW_UP:
				isKeyUpPressed = false;
				break;
			case GameEngine::Event::Key::ARROW_DOWN:
				isKeyDownPressed = false;
				break;
			case GameEngine::Event::Key::ARROW_RIGHT:
				isKeyRightPressed = false;
				break;
			case GameEngine::Event::Key::ARROW_LEFT:
				isKeyLeftPressed = false;
				break;
			default:
				break;
			}
		}

	}
}

void Race::handleRender()
{
	GameEngine::display->clear();

	car_sprite->draw_rotated(posx-camera.x, posy-camera.y, 23, 48, angle, 0, 0);
	GameEngine::rest(0.01);
	GameEngine::display->refresh();
}

void Race::handlePhysics()
{
	int speed = 20;
	if(    isKeyUpPressed and     isKeyDownPressed and     isKeyLeftPressed and     isKeyRightPressed)
	{
		//clear
	}
	else if(    isKeyUpPressed and     isKeyDownPressed and     isKeyLeftPressed and not isKeyRightPressed)
	{
		posx -= speed;
		angle = Math::PI/2.0;
	}
	else if(    isKeyUpPressed and     isKeyDownPressed and not isKeyLeftPressed and     isKeyRightPressed)
	{
		posx += speed;
		angle = 3.0*Math::PI/2.0;
	}

	else if(    isKeyUpPressed and     isKeyDownPressed and not isKeyLeftPressed and not isKeyRightPressed)
	{
		//clear
	}

	else if(    isKeyUpPressed and not isKeyDownPressed and     isKeyLeftPressed and     isKeyRightPressed)
	{
		posy -= speed;
		angle = 0;
	}

	else if(    isKeyUpPressed and not isKeyDownPressed and     isKeyLeftPressed and not isKeyRightPressed)
	{
		posy -= speed/2;
		posx -= speed/2;
		angle = Math::PI/4.0;
	}

	else if(    isKeyUpPressed and not isKeyDownPressed and not isKeyLeftPressed and     isKeyRightPressed)
	{
		posy -= speed/2;
		posx += speed/2;
		angle = 7.0*Math::PI/4.0;
	}

	else if(    isKeyUpPressed and not isKeyDownPressed and not isKeyLeftPressed and not isKeyRightPressed)
	{
		posy -= speed;
		angle = 0;
	}

	else if(not isKeyUpPressed and     isKeyDownPressed and     isKeyLeftPressed and     isKeyRightPressed)
	{
		posy += speed;
		angle = Math::PI;
	}

	else if(not isKeyUpPressed and     isKeyDownPressed and     isKeyLeftPressed and not isKeyRightPressed)
	{
		posy += speed/2;
		posx -= speed/2;
		angle = 3.0*Math::PI/4.0;
	}
	else if(not isKeyUpPressed and     isKeyDownPressed and not isKeyLeftPressed and     isKeyRightPressed)
	{
		posy += speed/2;
		posx += speed/2;
		angle = 5.0*Math::PI/4.0;
	}

	else if(not isKeyUpPressed and     isKeyDownPressed and not isKeyLeftPressed and not isKeyRightPressed)
	{
		posy += speed;
		angle = Math::PI;
	}

	else if(not isKeyUpPressed and not isKeyDownPressed and     isKeyLeftPressed and     isKeyRightPressed)
	{
		//clear
	}

	else if(not isKeyUpPressed and not isKeyDownPressed and     isKeyLeftPressed and not isKeyRightPressed)
	{
		posx -= speed;
		angle = Math::PI/2.0;
	}

	else if(not isKeyUpPressed and not isKeyDownPressed and not isKeyLeftPressed and     isKeyRightPressed)
	{
		posx += speed;
		angle = 3.0*Math::PI/2.0;
	}

	else if(not isKeyUpPressed and not isKeyDownPressed and not isKeyLeftPressed and not isKeyRightPressed)
	{
		//clear
	}

}
