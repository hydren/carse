/*
 * vehicle_selection_state.hpp
 *
 *  Created on: 7 de abr de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_
#define PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_
#include <ciso646>

#include <vector>

#include "carse_game.hpp"
#include "fgeal/extra/menu.hpp"
#include "vehicle.hpp"

#include "futil/language.hpp"
#include "fgeal/fgeal.hpp"

class VehicleSelectionState extends public fgeal::Game::State
{
	fgeal::Font* fontMain, *fontInfo;
	fgeal::Menu* menu;
	fgeal::Sound* sndCursorMove, *sndCursorAccept, *sndCursorOut;

	std::vector<Vehicle> vehicles;
	std::vector<fgeal::Image*> vehiclePreview;

	public:
	int getId();

	VehicleSelectionState(Pseudo3DCarseGame* game);
	~VehicleSelectionState();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	private:
	void handleInput();
	void onMenuSelect();
};

#endif /* PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_ */
