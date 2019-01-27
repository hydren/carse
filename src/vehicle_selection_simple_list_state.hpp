/*
 * vehicle_selection_simple_list_state.hpp
 *
 *  Created on: 17 de set de 2018
 *      Author: carlosfaruolo
 */

#ifndef VEHICLE_SELECTION_SIMPLE_LIST_STATE_HPP_
#define VEHICLE_SELECTION_SIMPLE_LIST_STATE_HPP_
#include <ciso646>

#include "vehicle.hpp"

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"
#include "fgeal/extra/gui.hpp"

#include "futil/language.hpp"

#include <vector>

#include "util.hpp"

// fwd decl.
class CarseGame;

class VehicleSelectionSimpleListState extends public fgeal::Game::State
{
	CarseGame& game;

	fgeal::Vector2D lastDisplaySize;

	fgeal::Font* fontMain, *fontInfo, *fontSub;
	fgeal::Sound* sndCursorMove, *sndCursorIn, *sndCursorOut;

	fgeal::Menu menu;
	unsigned lastEnterSelectedVehicleIndex, lastEnterSelectedVehicleAltIndex;
	fgeal::Button menuUpButton;
	fgeal::Point menuUpButtonArrow1, menuUpButtonArrow2, menuUpButtonArrow3;
	fgeal::Button menuDownButton;
	fgeal::Point menuDownButtonArrow1, menuDownButtonArrow2, menuDownButtonArrow3;
	fgeal::Button selectButton, backButton, appearanceLeftButton, appearanceRightButton;

	struct VehiclePreview
	{
		fgeal::Image* sprite;
		std::vector<fgeal::Image*> altSprites;
		int altIndex;
	};

	std::vector<VehiclePreview> previews;

	public:
	virtual int getId();

	VehicleSelectionSimpleListState(CarseGame* game);
	~VehicleSelectionSimpleListState();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void render();
	virtual void update(float delta);

	virtual void onKeyPressed(fgeal::Keyboard::Key);
	virtual void onMouseButtonPressed(fgeal::Mouse::Button button, int x, int y);
	virtual void onJoystickAxisMoved(unsigned, unsigned, float, float);
	virtual void onJoystickButtonPressed(unsigned, unsigned);

	void drawVehiclePreview(float x, float y, float scale=1.0f, int index=-1, int angleType=0);
	void drawVehicleSpec(float x, float y, float index=-1);
	void changeSprite(bool forward=true);

	private:
	void handleInput();
	void menuSelectionAction();
};

#endif /* VEHICLE_SELECTION_SIMPLE_LIST_STATE_HPP_ */
