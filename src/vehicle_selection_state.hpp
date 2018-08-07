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
class CarseGame;
class CarseSharedResources;
class CarseGameLogic;

class VehicleSelectionState extends public fgeal::Game::State
{
	CarseSharedResources& shared;
	CarseGameLogic& gameLogic;

	fgeal::Font* fontMain, *fontInfo, *fontSub;
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
		virtual void draw();
		virtual void update(float delta);
		virtual void onCursorUp();
		virtual void onCursorDown();
		virtual void onCursorLeft();
		virtual void onCursorRight();
		virtual void onCursorAccept();
	};

	struct ShowroomLayout extends GenericMenuStateLayout<VehicleSelectionState>
	{
		fgeal::Image imgBackground;
		bool isSelectionTransitioning;
		int previousIndex;
		float selectionTransitionProgress;

		ShowroomLayout(VehicleSelectionState& state);
		virtual void draw();
		virtual void update(float delta);
		virtual void onCursorUp();
		virtual void onCursorDown();
		virtual void onCursorLeft();
		virtual void onCursorRight();
		virtual void onCursorAccept();
	};

	public:
	virtual int getId();

	VehicleSelectionState(CarseGame* game);
	~VehicleSelectionState();

	virtual void initialize();
	virtual void onEnter();
	virtual void onLeave();

	virtual void render();
	virtual void update(float delta);

	virtual void onKeyPressed(fgeal::Keyboard::Key);

	void drawVehiclePreview(float x, float y, float scale=1.0f, int index=-1, int angleType=0);
	void drawVehicleSpec(float x, float y, float index=-1);
	void changeSprite(bool forward=true);

	private:
	void handleInput();
	void menuSelectionAction();
};

#endif /* PSEUDO3D_VEHICLE_SELECTION_STATE_HPP_ */
