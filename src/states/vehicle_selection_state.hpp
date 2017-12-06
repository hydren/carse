/*
 * vehicle_selection_state.hpp
 *
 *  Created on: 7 de abr de 2017
 *      Author: carlosfaruolo
 */

#ifndef PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_
#define PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_
#include <ciso646>

#include "vehicle.hpp"

#include "fgeal/fgeal.hpp"
#include "fgeal/extra/game.hpp"
#include "fgeal/extra/menu.hpp"

#include "futil/language.hpp"

#include <vector>

#include "util.hpp"

// fwd decl.
class Pseudo3DCarseGame;
class CarseSharedResources;
class CarseGameLogic;

class VehicleSelectionState extends public fgeal::Game::State
{
	CarseSharedResources& shared;
	CarseGameLogic& gameLogic;

	fgeal::Font* fontMain, *fontInfo;
	fgeal::Menu* menu;

	unsigned lastEnterSelectedVehicleIndex, lastEnterSelectedVehicleAltIndex;

	struct VehiclePreview
	{
		fgeal::Image* sprite;
		std::vector<fgeal::Image*> altSprites;
		int altIndex;
	};

	std::vector<VehiclePreview> previews;

	GenericMenuStateLayout<VehicleSelectionState>* layout;
	friend class GenericMenuStateLayout<VehicleSelectionState>;

	struct ListLayout extends GenericMenuStateLayout<VehicleSelectionState>
	{
		ListLayout(VehicleSelectionState& state);
		void draw();
		void update(float delta);
		void navigate(NavigationDirection navDir);
		void onCursorChange();
		void onCursorAccept();
	};

	struct ShowroomLayout extends GenericMenuStateLayout<VehicleSelectionState>
	{
		ShowroomLayout(VehicleSelectionState& state);
		void draw();
		void update(float delta);
		void navigate(NavigationDirection navDir);
		void onCursorChange();
		void onCursorAccept();
	};

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
	void drawVehicleSpec(float x, float y, float index=-1);
	void changeSprite(bool forward=true);

	private:
	void handleInput();
	void menuSelectionAction();

	void renderMenuPrototypeList();
	void renderMenuPrototypeSlideStand();
};

#endif /* PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_ */
