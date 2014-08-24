/*
 * game_engine.cpp
 *
 *  Created on: 24/08/2014
 *      Author: felipe
 */

#include "game_engine.hpp"

GameEngine::Display* GameEngine::display; //declaration of display (because it is an EXTERN variable)

namespace GameEngine
{
	//******************* COLOR

	Color::Color(unsigned char r, unsigned char g, unsigned char b)
	: r(r), g(g), b(b)
	{}

	const Color
	Color::WHITE		(255, 255, 255),
	Color::GREY			(127, 127, 127),
	Color::BLACK		(  0,   0,   0),
	Color::RED			(255,   0,   0),
	Color::MAROON		(127,   0,   0),
	Color::YELLOW		(255, 255,   0),
	Color::OLIVE		(127, 127,   0),
	Color::GREEN		(  0, 255,   0),
	Color::DARK_GREEN	(  0, 127,   0),
	Color::CYAN			(  0, 255, 255),
	Color::TEAL			(  0, 127, 127),
	Color::BLUE			(  0,   0, 255),
	Color::NAVY			(  0,   0, 127),
	Color::MAGENTA		(255,   0, 255),
	Color::PURPLE		(127,   0, 127),

	Color::LIGHT_GREY	(192, 192, 192),
	Color::DARK_GREY	( 96,  96,  96),
	Color::ORANGE		(255, 127,   0),
	Color::PINK			(255, 192, 192),
	Color::BROWN		(144,  92,  48);
	//TODO add more colors

}


