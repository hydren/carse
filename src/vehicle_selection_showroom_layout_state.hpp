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
#include "fgeal/extra/gui.hpp"

#include "futil/language.hpp"

#include <vector>

#include "util.hpp"

// fwd decl.
class CarseGame;

class VehicleSelectionShowroomLayoutState extends public fgeal::Game::State
{
	CarseGame& game;

	fgeal::Vector2D lastDisplaySize;

	fgeal::Font* fontTitle, *fontSubtitle, *fontInfo, *fontGui;
	fgeal::Menu menu;

	fgeal::Rectangle nextVehicleButtonBounds, previousVehicleButtonBounds, nextAppearanceButtonBounds, previousApperanceButtonBounds;

	fgeal::Sound* sndCursorMove, *sndCursorIn, *sndCursorOut;

	fgeal::Button selectButton, backButton;

	unsigned lastEnterSelectedVehicleIndex, lastEnterSelectedVehicleAltIndex;

	fgeal::Image* previewCurrentSprite, *previewPreviousSprite, *previewNextSprite;
	std::string previewCurrentSpriteFilename, previewPreviousSpriteFilename, previewNextSpriteFilename;
	std::vector<int> previewAltIndex;

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
	virtual void onJoystickAxisMoved(unsigned, unsigned, float, float);
	virtual void onJoystickButtonPressed(unsigned, unsigned);

	void drawVehiclePreview(fgeal::Image* sprite, const Pseudo3DVehicleAnimationSpec& spriteSpec, float x, float y, float scale=1.0f, int angleType=0);
	void drawVehicleSpec(float x, float y, float index=-1);
	void changeSprite(bool forward=true);

	private:
	void handleInput();
	void menuSelectionAction();
	void reloadSpriteIfMiss(unsigned menuIndex, fgeal::Image*& previewSprite, std::string& previewSpriteFilename);
};

#endif /* VEHICLE_SELECTION_SHOWROOM_LAYOUT_STATE_HPP_ */
