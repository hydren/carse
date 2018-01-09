/*
 * util.h
 *
 *  Created on: 6 de dez de 2017
 *      Author: carlosfaruolo
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_

#include "fgeal/filesystem.hpp"

#include <string>

#define scaledToSize(imgPtr, size) (size).getWidth()/(float)((imgPtr)->getWidth()), (size).getHeight()/(float)((imgPtr)->getHeight())
#define scaledToRect(imgPtr, rect) (rect).w/(float)((imgPtr)->getWidth()), (rect).h/(float)((imgPtr)->getHeight())

// device-independent pixel (size based on a 480px tall display); can only be used if there is a 'display' instance in the scope
#define dip(px) (px*(display.getHeight()/480.0))

/** Attempt to get a contextualized filename.
 *  First it attempts to check if "baseDir1 + specifiedFilename" is a valid file and returns it if true.
 *  If not, then it tries the same with "baseDir2 + specifiedFilename".
 *  If not, then it tries the same with "baseDir3 + specifiedFilename".
 *  If not, then it tries the same with "current working dir. + specifiedFilename".
 *  If not, then it tries the same with "specifiedFilename" alone by itself.
 *  If not, then we could not come up with a valid filename and an empty string is returned. */
inline std::string getContextualizedFilename(const std::string& specifiedFilename, const std::string& baseDir1, const std::string& baseDir2, const std::string& baseDir3)
{
	if(fgeal::filesystem::isFilenameArchive(baseDir1 + specifiedFilename))
		return baseDir1 + specifiedFilename;

	if(fgeal::filesystem::isFilenameArchive(baseDir2 + specifiedFilename))
		return baseDir2 + specifiedFilename;

	if(fgeal::filesystem::isFilenameArchive(baseDir3 + specifiedFilename))
		return baseDir3 + specifiedFilename;

	if(fgeal::filesystem::isFilenameArchive(fgeal::filesystem::getCurrentWorkingDirectory() + specifiedFilename))
		return fgeal::filesystem::getCurrentWorkingDirectory() + specifiedFilename;

	if(fgeal::filesystem::isFilenameArchive(specifiedFilename))
		return specifiedFilename;

	return std::string();
}

/// Same as the 4-argument version, but with only two "baseDir" option.
inline std::string getContextualizedFilename(const std::string& specifiedFilename, const std::string& baseDir1, const std::string& baseDir2)
{
	return getContextualizedFilename(specifiedFilename, baseDir1, baseDir2, baseDir2);
}

/// Same as the 3-argument version, but with only one "baseDir" option.
inline std::string getContextualizedFilename(const std::string& specifiedFilename, const std::string& baseDir)
{
	return getContextualizedFilename(specifiedFilename, baseDir, baseDir, baseDir);
}

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
