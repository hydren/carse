/*
 * choose_vehicle_state.hpp
 *
 *  Created on: 7 de abr de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_CHOOSE_VEHICLE_STATE_HPP_
#define PSEUDO3D_CHOOSE_VEHICLE_STATE_HPP_
#include <ciso646>

#include <vector>

#include "carse_game.hpp"
#include "gui/menu.hpp"
#include "vehicle.hpp"

#include "futil/general/language.hpp"
#include "fgeal/fgeal.hpp"

class ChooseVehicleState extends public fgeal::Game::State
{
	fgeal::Font* fontMain, *fontInfo;
	Menu* menu;

	std::vector<Vehicle> vehicles;
	std::vector<fgeal::Image*> vehiclePreview;

	public:
	int getId();

	ChooseVehicleState(CarseGame* game);
	~ChooseVehicleState();

	void initialize();
	void onEnter();
	void onLeave();

	void render();
	void update(float delta);

	private:
	void handleInput();
	void onMenuSelect();
};

#endif /* PSEUDO3D_CHOOSE_VEHICLE_STATE_HPP_ */
