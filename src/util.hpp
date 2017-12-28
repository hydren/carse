/*
 * util.h
 *
 *  Created on: 6 de dez de 2017
 *      Author: carlosfaruolo
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_

#define scaledToSize(imgPtr, size) (size).getWidth()/(float)((imgPtr)->getWidth()), (size).getHeight()/(float)((imgPtr)->getHeight())
#define scaledToRect(imgPtr, rect) (rect).w/(float)((imgPtr)->getWidth()), (rect).h/(float)((imgPtr)->getHeight())

// device-independent pixel (size based on a 480px tall display); can only be used if there is a 'display' instance in the scope
#define dip(px) (px*(display.getHeight()/480.0))

template<class GameStateClass>
struct GenericMenuStateLayout
{
	GameStateClass& state;

	GenericMenuStateLayout(GameStateClass& state)
	: state(state)
	{}

	virtual ~GenericMenuStateLayout() {}

	// draws the layout
	virtual void draw() abstract;

	// performs any logic-related updates, if needed
	virtual void update(float delta) abstract;

	enum NavigationDirection { NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT };

	// action when user navigates
	virtual void navigate(NavigationDirection navDir) abstract;

	// action when user accept or selects and confirm a item of the menu
	virtual void onCursorAccept() { state.menuSelectionAction(); }

	// stuff to be done when exiting the menu
	virtual void onQuit() { state.game.running = false; }
};

#endif /* UTIL_HPP_ */
