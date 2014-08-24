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
	Color::BROWN		(144,  92,  48),

	Color::BEIGE		(245, 245, 220),
	Color::LIME			(191, 255,   0),
	Color::SCARLET		(253,  14,  53),
	Color::MINT			(116, 195, 101),
	Color::AZURE		(  0, 127, 255),
	Color::TURQUOISE	( 64, 224, 208),
	Color::VIOLET		(127,   0, 255),
	Color::GOLD			(255, 215,   0),
	Color::SALMON		(250, 127, 114),
	Color::BRONZE		(205, 127,  50),
	Color::WINE			(196,  30,  58),
	Color::INDIGO		( 75,   0, 130),
	Color::CELESTE		(  0, 191, 255),
	Color::FLAME		(226,  88,  34),
	Color::CREAM		(253, 252, 143),
	Color::CARAMEL		(193, 154, 107),
	Color::RUBY			(255, 255, 255),
	Color::JADE			(224,  17,  95),
	Color::CERULEAN		(  0, 123, 167),
	Color::AQUA			(176, 224, 230),
	Color::FUSCHIA 		(193,  84, 193);
	//TODO add more colors

}


