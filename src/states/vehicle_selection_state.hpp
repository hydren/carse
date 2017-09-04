/*
 * vehicle_selection_state.hpp
 *
 *  Created on: 7 de abr de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_
#define PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_
#include <ciso646>

#include "carse_game.hpp"

#include "futil/language.hpp"
#include "fgeal/fgeal.hpp"
#include "fgeal/extra/menu.hpp"
#include "vehicle.hpp"

#include <vector>

class VehicleSelectionState extends public fgeal::Game::State
{
	fgeal::Font* fontMain, *fontInfo;
	fgeal::Menu* menu;
	fgeal::Sound* sndCursorMove, *sndCursorAccept, *sndCursorOut;

	unsigned lastEnterSelectedVehicleIndex, lastEnterSelectedVehicleAltIndex;

	struct VehiclePreview
	{
		fgeal::Image* sprite;
		std::vector<fgeal::Image*> altSprites;
		int altIndex;
	};

	std::vector<Vehicle> vehicles;
	std::vector<VehiclePreview> previews;

	enum Layout {
		LAYOUT_PROTOTYPE_LIST, LAYOUT_PROTOTYPE_SLIDE_STAND
	} layout;

	public:
	int getId();

	VehicleSelectionState(Pseudo3DCarseGame* game);
	~VehicleSelectionState();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	void drawVehiclePreview(float x, float y, float scale=1.0f, int index=-1, int angleType=0);

	private:
	void handleInput();
	void onMenuSelect();

	void renderMenuPrototypeList();
	void renderMenuPrototypeSlideStand();
};

#endif /* PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_ */
