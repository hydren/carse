/*
 * allegro_game_engine.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include "select.h"

#ifdef USE_ALLEGRO_50

#include "../game_engine.hpp"

/** GameEngine code based on Allegro 5.0 */
const char* GameEngine::BACKEND_NAME = "Allegro 5.0";

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

namespace GameEngine
{
	struct Display::Implementation
	{
		ALLEGRO_DISPLAY* allegroDisplay;
	};

	struct Image::Implementation
	{
		ALLEGRO_BITMAP* bitmap;
	};

	struct Event::Implementation
	{
		ALLEGRO_EVENT* allegroEvent;
	};

	struct EventQueue::Implementation
	{
		ALLEGRO_EVENT_QUEUE* allegroEventQueue;
		ALLEGRO_EVENT* allegroEvent;
	};

	struct Font::Implementation
	{
		ALLEGRO_FONT* allegroFont;
	};

	// initialize all allegro stuff
	void initialize()
	{
		al_init();
		al_init_image_addon();
		al_init_font_addon();
		al_init_ttf_addon();
		al_init_primitives_addon();

		if(!al_install_keyboard())
		{
			throw(Exception("Could not install keyboard"));
		}

		if(!al_install_mouse())
		{
			throw(Exception("Could not install mouse"));
		}
	}

	void finalize()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		//TODO delete other stuff
		//delete display;
	}

	void rest(double seconds)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_rest(seconds);
	}

	list<string> getFilenamesWithinDirectory(const string& directoryPath)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		ALLEGRO_FS_ENTRY* directory = al_create_fs_entry(directoryPath.c_str());
		al_open_directory(directory);

		list<string> filenames;
		for(ALLEGRO_FS_ENTRY* entry = al_read_directory(directory); entry != null; entry = al_read_directory(directory))
		{
			filenames.push_back(al_get_fs_entry_name(entry));
		}
		return filenames;
	}


	Display::Display(int width, int height, const string& title, Image* icon)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;
		this->implementation->allegroDisplay = al_create_display(width, height);

		if(this->implementation->allegroDisplay == NULL)
			throw Exception("Could not create display! Error "+al_get_errno());

		al_set_window_title(this->implementation->allegroDisplay, title.c_str());

		if(icon != NULL)
			al_set_display_icon(this->implementation->allegroDisplay, icon->implementation->bitmap);
	}

	Display::~Display()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_destroy_display(this->implementation->allegroDisplay);
	}

	int Display::getWidth()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return al_get_display_width(implementation->allegroDisplay);
	}

	int Display::getHeight()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return al_get_display_height(implementation->allegroDisplay);
	}

	void Display::setTitle(const string& title)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_set_window_title(this->implementation->allegroDisplay, title.c_str());
	}

	void Display::setIcon(Image* image)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_set_display_icon(this->implementation->allegroDisplay, image->implementation->bitmap);
	}

	void Display::refresh()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_flip_display();
	}

	void Display::clear()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_clear_to_color(al_map_rgb(0,0,0));
	}

	Image::Image(string filename)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;
		this->implementation->bitmap = al_load_bitmap(filename.c_str());
		if ( this->implementation->bitmap == null)
			throw Exception("AllegroAPI Constructor - Could not load image: " + filename);
	}

	Image::~Image()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_destroy_bitmap(this->implementation->bitmap);
	}

	void Image::draw(float x, float y)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		//COUT << x << " " << y << " " << from_x << " " << from_y << " " << w << " " << h << " " << ENDL;
		//draw all source region
		al_draw_bitmap(this->implementation->bitmap, x, y, 0);
	}

	void Image::draw(float x, float y, float from_x, float from_y, float w, float h)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		//COUT << x << " " << y << " " << from_x << " " << from_y << " " << w << " " << h << " " << ENDL;
		al_draw_bitmap_region(this->implementation->bitmap, from_x, from_y, w, h, x, y, 0);
	}

	void Image::draw_rotated(float x, float y, float ax, float ay, float angle)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		//draw all source region
		al_draw_rotated_bitmap(this->implementation->bitmap, ax, ay, x, y, 2*ALLEGRO_PI - angle, 0);
	}
	void Image::draw_rotated(float x, float y, float ax, float ay, float angle, float from_x, float from_y, float w, float h)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_draw_tinted_scaled_rotated_bitmap_region(this->implementation->bitmap, from_x, from_y, w, h, al_map_rgba_f(1, 1, 1, 1), ax, ay, x, y, 1, 1, angle, 0);
	}

	void Image::blit(Image& img2, float x, float y, float from_x, float from_y, float h, float w)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_set_target_bitmap( img2.implementation->bitmap );

		if(w == -1 && h == -1) //draw all source region
			al_draw_bitmap(this->implementation->bitmap, x, y, 0);
		else
			al_draw_bitmap_region(this->implementation->bitmap, from_x, from_y, w, h, x, y, 0);

		al_set_target_backbuffer(al_get_current_display());
	}

	//XXX possibly incorrect
	float Image::getWidth()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return al_get_bitmap_width(this->implementation->bitmap);
	}

	float Image::getHeight()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return al_get_bitmap_height(this->implementation->bitmap);
	}

	void Image::draw_rectangle(Color c, float x1, float y1, float x2, float y2, bool filled)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		ALLEGRO_COLOR ac =  al_map_rgb(c.r, c.g, c.b);
		if(filled)
			al_draw_filled_rectangle(x1, y1, x2, y2, ac);
		else
			al_draw_rectangle(x1, y1, x2, y2, ac, 1);
	}

	Event::Event()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;
		this->implementation->allegroEvent = new ALLEGRO_EVENT;
	}

	Event::~Event()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		delete this->implementation->allegroEvent;
		delete this->implementation;
	}

	Event::Type::value Event::getEventType()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		switch(this->implementation->allegroEvent->type)
		{
			case ALLEGRO_EVENT_DISPLAY_CLOSE:		return Event::Type::DISPLAY_CLOSURE;
			case ALLEGRO_EVENT_KEY_DOWN: 			return Event::Type::KEY_PRESS;
			case ALLEGRO_EVENT_KEY_UP:				return Event::Type::KEY_RELEASE;
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: 	return Event::Type::MOUSE_BUTTON_PRESS;
			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:		return Event::Type::MOUSE_BUTTON_RELEASE;

			//TODO map more events...

			default:								return Event::Type::NOTHING;
		}
	}

	Event::Key::value Event::getEventKeyCode()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		switch(this->implementation->allegroEvent->keyboard.keycode)
		{
			case ALLEGRO_KEY_UP:	return Event::Key::ARROW_UP;
			case ALLEGRO_KEY_DOWN:	return Event::Key::ARROW_DOWN;
			case ALLEGRO_KEY_LEFT:	return Event::Key::ARROW_LEFT;
			case ALLEGRO_KEY_RIGHT:	return Event::Key::ARROW_RIGHT;

			case ALLEGRO_KEY_ENTER: return Event::Key::ENTER;
			case ALLEGRO_KEY_SPACE: return Event::Key::SPACE;
			case ALLEGRO_KEY_ESCAPE: return Event::Key::ESCAPE;

			//TODO map more buttons...

			default: 				return Event::Key::UNKNOWN;
		}
	}

	Event::MouseButton::value Event::getEventMouseButton()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		switch(this->implementation->allegroEvent->mouse.button)
		{
			case 1:		return Event::MouseButton::LEFT;
			case 2:		return Event::MouseButton::RIGHT;
			case 3:		return Event::MouseButton::MIDDLE;

			default:	return Event::MouseButton::UNKNOWN;
		}
	}

	int Event::getEventMouseX()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return this->implementation->allegroEvent->mouse.x;
	}

	int Event::getEventMouseY()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return this->implementation->allegroEvent->mouse.y;
	}

	EventQueue::EventQueue()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;

		this->implementation->allegroEvent = new ALLEGRO_EVENT;
		this->implementation->allegroEventQueue = al_create_event_queue();

		if(this->implementation->allegroEventQueue == NULL)
			throw Exception("Could not create event queue");

		al_register_event_source(this->implementation->allegroEventQueue, al_get_display_event_source(GameEngine::display->implementation->allegroDisplay));
		al_register_event_source(this->implementation->allegroEventQueue, al_get_keyboard_event_source());
		al_register_event_source(this->implementation->allegroEventQueue, al_get_mouse_event_source());
	}

	EventQueue::~EventQueue()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_destroy_event_queue(implementation->allegroEventQueue);
		delete implementation->allegroEvent;
		delete implementation;
	}

	bool EventQueue::isEmpty()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return al_is_event_queue_empty(this->implementation->allegroEventQueue);
	}

	Event* EventQueue::waitForEvent()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_wait_for_event(this->implementation->allegroEventQueue, this->implementation->allegroEvent);
		Event* ev = new Event;
		ev->implementation->allegroEvent = this->implementation->allegroEvent;
		return ev;
	}

	void EventQueue::waitForEvent(Event* container)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_wait_for_event(this->implementation->allegroEventQueue, container->implementation->allegroEvent);
	}

	void EventQueue::ignoreEvents()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_unregister_event_source(this->implementation->allegroEventQueue, al_get_display_event_source(GameEngine::display->implementation->allegroDisplay));
		al_unregister_event_source(this->implementation->allegroEventQueue, al_get_keyboard_event_source());
		al_unregister_event_source(this->implementation->allegroEventQueue, al_get_mouse_event_source());
		al_flush_event_queue(this->implementation->allegroEventQueue);
	}

	void EventQueue::listenEvents()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_flush_event_queue(this->implementation->allegroEventQueue);
		al_register_event_source(this->implementation->allegroEventQueue, al_get_display_event_source(GameEngine::display->implementation->allegroDisplay));
		al_register_event_source(this->implementation->allegroEventQueue, al_get_keyboard_event_source());
		al_register_event_source(this->implementation->allegroEventQueue, al_get_mouse_event_source());
	}

	void EventQueue::flushEvents()
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_flush_event_queue(this->implementation->allegroEventQueue);
	}

	Font::Font(string filename, int size, bool antialiasing, bool hinting, bool kerning)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;

		//I know, pretty odd...
		int flags = (antialiasing? 0 : ALLEGRO_TTF_MONOCHROME)	| (hinting? 0 : ALLEGRO_TTF_NO_AUTOHINT) | (kerning? 0 : ALLEGRO_TTF_NO_KERNING);
		this->implementation->allegroFont = al_load_ttf_font(filename.c_str(), size, flags);

		if(this->implementation->allegroFont == null)
			throw Exception("Font"+filename+" could not be loaded!");
	}

	void Font::draw_text(string text, float x, float y, Color color)
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		al_draw_text(this->implementation->allegroFont, al_map_rgb(color.r, color.g, color.b), x, y, ALLEGRO_ALIGN_LEFT, text.c_str());
	}

	int Font::getSize() const
	{
		if(not al_is_system_installed()) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return al_get_font_line_height(this->implementation->allegroFont);
	}
}

#endif
