/*
 * menu.cpp
 *
 *  Created on: 3 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "menu.hpp"

using std::string;

Menu::Menu(Rectangle bounds, fgeal::Font* font, fgeal::Color color, string title)
 : entries(),
   title(title),
   selectedIndex(-1),
   font(font),
   bounds(bounds),
   bgColor(fgeal::Color::BLACK),
   fontColor(color),
   selectedColor(fgeal::Color(255-color.r, 255-color.g, 255-color.b)),
   layoutMode(PACK_ENTRIES), entrySpacing(0.1), entrySpacingIsRelative(true),
   manageFontDeletion(false)
{}

Menu::~Menu()
{
	if(manageFontDeletion)
		delete font;
}

void Menu::addEntry(string label, int index)
{
	if(index < 0)
		entries.push_back(Entry(label));
	else
		entries.insert(entries.begin()+index, Entry(label));

	if(entries.size() == 1)
	{
		selectedIndex = 0;
	}
}

void Menu::removeEntry(unsigned index)
{
	if(index < 0 || index > entries.size()-1)
		return;

	if(index == selectedIndex)
	{
		if(entries.size() == 1)
		{
			selectedIndex = -1;
		}

		else if(index == entries.size()-1)
		{
			selectedIndex = index-1;
		}

		else
		{
			selectedIndex = index+1;
		}
	}
	entries.erase(entries.begin()+index);
}

Menu::Entry& Menu::operator [] (int index)
{
	return entries[index];
}

Menu::Entry& Menu::getSelectedEntry()
{
	return entries[selectedIndex];
}

unsigned Menu::getSelectedIndex()
{
	return selectedIndex;
}

unsigned Menu::getNumberOfEntries()
{
	return entries.size();
}

void Menu::setSelectedEntry(const Entry& entry)
{
	for(unsigned i = 0; i < entries.size(); i++)
		if(&entries[i] == &entry)
		{
			selectedIndex = i;
			break;
		}
}

void Menu::setSelectedIndex(unsigned index)
{
	if(index + 1 > entries.size())
		return;

	selectedIndex = index;
}

Menu& Menu::cursorUp()
{
	if(selectedIndex > 0)
		selectedIndex--;
	return *this;
}

Menu& Menu::cursorDown()
{
	if(selectedIndex + 1 < entries.size())
		selectedIndex++;
	return *this;
}

Menu& Menu::operator --()
{
	return this->cursorUp();
}

Menu& Menu::operator ++()
{
	return this->cursorDown();
}

/** Draw the menu according the menu bounds and number of entries */
void Menu::draw()
{
//	if(bg != null)
//		bg->draw(bounds.x, bounds.y);

	fgeal::Image::drawRectangle(selectedColor, bounds.x, bounds.y, bounds.w, bounds.h);
	fgeal::Image::drawRectangle(bgColor, bounds.x+2, bounds.y+2, bounds.w-4, bounds.h-4);

	float distanceBetween = entrySpacingIsRelative? font->getFontHeight() * (1+entrySpacing) : entrySpacing;

	if(layoutMode == STRETCH_SPACING)
		distanceBetween = (bounds.h-font->getFontHeight()) / ((float) entries.size() + (title.empty()?0:1));

	float offset = (title.empty()?0:font->getFontHeight());

	if(not title.empty())
		font->drawText(title, bounds.x, bounds.y, fontColor);

	for(unsigned i = 0; i < entries.size(); i++)
	{
		//quick dirty fix TODO remove this and do a better structure
		string str = entries[i].label;
		if(str.length() > 30) do str = "..."+str.substr(4); while(str.length() > 30);

		if(i == selectedIndex)
			font->drawText(str, bounds.x+2, bounds.y + offset, selectedColor);
		else
			font->drawText(str, bounds.x+2, bounds.y + offset, fontColor);

		offset += distanceBetween;
	}
}
