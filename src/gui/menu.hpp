/*
 * menu.hpp
 *
 *  Created on: 3 de abr de 2017
 *      Author: carlosfaruolo
 */

#ifndef GUI_MENU_HPP_
#define GUI_MENU_HPP_
#include <ciso646>

#include <vector>

#include <string>

#include "fgeal/fgeal.hpp"
using fgeal::Rectangle;

class Menu
{
	struct Entry
	{
		std::string label;
		bool enabled;

		Entry(std::string str)
		: label(str),
		  enabled(true)
		{}
	};

	std::vector<Entry> entries;
	std::string title;

	unsigned selectedIndex;
	fgeal::Font* font;

	public:

	Rectangle bounds;
	fgeal::Color bgColor, fontColor, selectedColor;

	/// Set to true to delete the passed font when deleting this menu
	bool manageFontDeletion;

	Menu(const Rectangle bounds, fgeal::Font* font, const fgeal::Color color, const std::string title="");
	~Menu();

	void addEntry(std::string label, int index=-1);
	void removeEntry(unsigned index);

	Entry& operator [] (int index);

	Entry& getSelectedEntry();
	unsigned getNumberOfEntries();
	void setSelectedEntry(const Entry& entry);

	/** Safe way to set the selected index */
	void setSelectedIndex(unsigned index);

	/** Decrement the selected index in a safe way */
	Menu& cursorUp();

	/** Increment the selected index in a safe way */
	Menu& cursorDown();

	/** alias to cursorUp() */
	Menu& operator --();

	/** alias to cursorDown() */
	Menu& operator ++();

	/** Draw the menu according the menu bounds and number of entries */
	void draw();
};

#endif /* GUI_MENU_HPP_ */
