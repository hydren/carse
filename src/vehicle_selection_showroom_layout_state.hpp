/*
 * vehicle_selection_showroom_layout_state.hpp
 *
 *  Created on: 17 de set de 2018
 *      Author: carlosfaruolo
 */

#ifndef VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_HPP_
#define VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_HPP_
#include <ciso646>

#include "vehicle.hpp"

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"

#include <vector>

#include "util.hpp"

// fwd decl.
class CarseGame;

class VehicleSelectionShowroomLayoutState extends public fgeal::Game::State
{
	CarseGame& game;

	fgeal::Font* fontMain, *fontInfo, *fontSub;
	fgeal::Menu menu;

	fgeal::Rectangle selectButtonBounds, backButtonBounds, nextVehicleButtonBounds, previousVehicleButtonBounds, nextAppearanceButtonBounds, previousApperanceButtonBounds;

	fgeal::Sound* sndCursorMove, *sndCursorIn, *sndCursorOut;

	unsigned lastEnterSelectedVehicleIndex, lastEnterSelectedVehicleAltIndex;

	struct VehiclePreview
	{
		fgeal::Image* sprite;
		std::vector<fgeal::Image*> altSprites;
		int altIndex;
	};

	std::vector<VehiclePreview> previews;

	fgeal::Image* imgBackground, *imgArrow1, *imgArrow2;
	bool isSelectionTransitioning;
	int previousIndex;
	float selectionTransitionProgress;

	public:
	virtual int getId();

	VehicleSelectionShowroomLayoutState(CarseGame* game);
	~VehicleSelectionShowroomLayoutState();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void render();
	virtual void update(float delta);

	virtual void onKeyPressed(fgeal::Keyboard::Key);
	virtual void onMouseButtonPressed(fgeal::Mouse::Button button, int x, int y);

	void drawVehiclePreview(float x, float y, float scale=1.0f, int index=-1, int angleType=0);
	void drawVehicleSpec(float x, float y, float index=-1);
	void changeSprite(bool forward=true);

	private:
	void handleInput();
	void menuSelectionAction();
};

#endif /* VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_HPP_ */
