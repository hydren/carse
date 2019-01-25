/*
 * carse_game_shared_resources.cpp
 *
 *  Created on: 9 de ago de 2018
 *      Author: carlosfaruolo
 */

#include "carse_game.hpp"

using std::string;
using futil::Properties;

static string getFontFilename(const string& key)
{
	Properties properties;
	properties.load("assets/fonts/fonts.properties");
	return "assets/fonts/"+properties.get(key, "default.ttf");
}

CarseGame::SharedResources::SharedResources()
: sndCursorMove("assets/sound/cursor_move.ogg"),
  sndCursorIn("assets/sound/cursor_accept.ogg"),
  sndCursorOut("assets/sound/cursor_out.ogg"),
  fontDev("assets/fonts/default.ttf", 11),
  font1Path(getFontFilename("font1")),
  font2Path(getFontFilename("font2")),
  font3Path(getFontFilename("font3"))
{}
