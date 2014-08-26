/*
 * sdl1.2_game_engine.cpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#include "select.h"

#ifdef USE_SDL_12

#include "../game_engine.hpp"
#include "../util.hpp"

/** GameEngine code based on SDL 1.2 */

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>

//aux variables
const Uint32 sdlInitFlags = (SDL_INIT_VIDEO | SDL_INIT_AUDIO);
SDL_Rect srcrect;
SDL_Rect dstrect;
SDL_Surface* rotozoom_surface;
const double toDegree = (180.0/Math::PI);

SDL_Color create_SDL_Color(Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	return c;
}

bool checkInit()
{
	return (SDL_WasInit(sdlInitFlags) == sdlInitFlags) and TTF_WasInit();
}

namespace GameEngine
{
	/** * Definition of the "implementation" struct's (experimental) * */
	struct Display::Implementation
	{
		SDL_Surface* sdlDisplaySurface;
	};

	struct Image::Implementation
	{
		SDL_Surface* sdlSurface;
	};

	struct Event::Implementation
	{
		SDL_Event* sdlEvent;
	};

	struct EventQueue::Implementation
	{
	};

	struct Font::Implementation
	{
		TTF_Font* sdlttfFont;
		bool isAntialiased;
	};

	// initialize all SDL stuff
	void initialize()
	{
		if( SDL_Init( sdlInitFlags ) < 0 )
		{
			string message = string("SDL could not be initialized: ") + SDL_GetError();
			cout << message << endl;
			throw Exception(message);
		}

		/*
		if( Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096)== -1)
		{
			string message = "Erro ao abrir o audio. Mix_error: " << Mix_GetError();
			cout << message << endl;
			throw Exception(Mix_GetError());
		}
		*/

		//inicializar o sistema de TTF
		if( TTF_Init()== -1)
		{
			std::cout << "Nao foi possivel inicializar a SDL TTF: " << TTF_GetError() << std::endl;
			throw Exception(TTF_GetError());
		}


//		al_init();
//		al_init_image_addon();
//		al_init_font_addon();
//		al_init_ttf_addon();
//		al_init_primitives_addon();
//
//		if(!al_install_keyboard())
//		{
//			throw(Exception("Could not install keyboard"));
//		}
//
//		if(!al_install_mouse())
//		{
//			throw(Exception("Could not install mouse"));
//		}
	}

	void finalize()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		// while(Mix_Init(0)) Mix_Quit();
		SDL_Quit();
	}

	void rest(double seconds)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		int ms = (seconds*1000.0);
		SDL_Delay(ms>0? ms : 1);
	}

	//FIXME add implementation, possibly using PhysicsFS, or dirent.h
	list<string> getFilenamesWithinDirectory(const string& directoryPath)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
//		ALLEGRO_FS_ENTRY* directory = al_create_fs_entry(directoryPath.c_str());
//		al_open_directory(directory);
//
//		list<string> filenames;
//		for(ALLEGRO_FS_ENTRY* entry = al_read_directory(directory); entry != null; entry = al_read_directory(directory))
//		{
//			filenames.push_back(al_get_fs_entry_name(entry));
//		}
//		return filenames;
		return list<string>();
	}

	//******************* DISPLAY

	Display::Display(int width, int height, const string& title, Image* icon)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;
		this->implementation->sdlDisplaySurface = SDL_SetVideoMode(width, height, 32, SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWACCEL);

		if(this->implementation->sdlDisplaySurface == null)
		{
			string message = string("Could not create display! Error ") + SDL_GetError();
			throw Exception(message);
		}

		SDL_WM_SetCaption(title.c_str(), title.c_str());

		if(icon != null)
			SDL_WM_SetIcon(icon->implementation->sdlSurface, null);
	}

	Display::~Display()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		SDL_FreeSurface(this->implementation->sdlDisplaySurface);
		delete this->implementation;
	}

	int Display::getWidth()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return implementation->sdlDisplaySurface->w;
	}

	int Display::getHeight()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return implementation->sdlDisplaySurface->h;
	}

	void Display::setTitle(const string& title)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		SDL_WM_SetCaption(title.c_str(), title.c_str());
	}

	void Display::setIcon(Image* image)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		SDL_WM_SetIcon(image->implementation->sdlSurface,  null);
	}

	void Display::refresh()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		//flip surface if double-buffered, update rect if single-buffered instead
		if( SDL_Flip(this->implementation->sdlDisplaySurface) == -1 )
		{
			string message = string("Failed to swap the buffers/refresh the display: ") + SDL_GetError();
			cout << message << endl;
			throw Exception(message);
		}
	}

	void Display::clear()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		if ( SDL_FillRect(this->implementation->sdlDisplaySurface, null, 0) == -1 )
		{
			string msg = string("SDL_FillRect error! ") + SDL_GetError();
			cout << msg << endl;
			throw Exception(msg);
		}
	}

	//******************* IMAGE

	Image::Image(string filename)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;
		this->implementation->sdlSurface = IMG_Load(filename.c_str() );
		if ( this->implementation->sdlSurface == null)
			throw Exception("Could not load image \"" + filename + "\"" + IMG_GetError());
	}

	/**
	 * Creates a colored shape. Depending on the shape, differents arguments are used.
	 *
	 * RECTANGLE				width, height, thickness
	 * FILLED_RECTANGLE			width, height
	 *
	 * All of them must be int.
	 * */
	Image::Image(Shape shape, Color color, float arg1, float arg2, float arg3)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;

		switch(shape)
		{
			case Image::RECTANGLE:
			{
				float width = arg1;
				float height = arg2;
//				float thickness = arg3; //XXX needed to be used
				this->implementation->sdlSurface = SDL_CreateRGBSurface(SDL_SRCALPHA, width, height, 32, 0, 0, 0, 0);
				rectangleRGBA(implementation->sdlSurface, 0, 0, width, height, color.r, color.g, color.b, 0);
			}
			case Image::FILLED_RECTANGLE:
			{
				float width = arg1;
				float height = arg2;
				this->implementation->sdlSurface = SDL_CreateRGBSurface(SDL_SRCALPHA, width, height, 32, 0, 0, 0, 0);
				boxRGBA(implementation->sdlSurface, 0, 0, width, height, color.r, color.g, color.b, 0);
			}
			//TODO finish other cases with other shapes

			default: break;
		}
	}

	Image::~Image()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		SDL_FreeSurface(this->implementation->sdlSurface);
		delete implementation;
	}

	void Image::draw(float x, float y, float from_x, float from_y, float w, float h)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		//COUT << x << " " << y << " " << from_x << " " << from_y << " " << w << " " << h << " " << ENDL;

		dstrect.x = x; dstrect.y = y;

		 //draws all source region
		if(w == -1 and h == -1)
			SDL_BlitSurface(this->implementation->sdlSurface, null, GameEngine::display->implementation->sdlDisplaySurface, &dstrect);

		//draws selected region
		else
		{
			srcrect.x = from_x; srcrect.y = from_y;
			srcrect.w = w; srcrect.h = h;
			SDL_BlitSurface(this->implementation->sdlSurface, &srcrect, GameEngine::display->implementation->sdlDisplaySurface, &dstrect);
		}
	}

	void Image::draw_rotated(float x, float y, float ax, float ay, float angle, float from_x, float from_y, float w, float h)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		rotozoom_surface = rotozoomSurface(implementation->sdlSurface, angle*toDegree, 1, 0);

		int w1 = implementation->sdlSurface->w,
			h1 = implementation->sdlSurface->h,
			w2 = rotozoom_surface->w,
			h2 = rotozoom_surface->h;

		int ax2 = w2/2 + (ax - w1/2)*cos(angle) - (ay - h1/2)*sin(angle);
		int ay2 = h2/2 + (ay - h1/2)*cos(angle) - (ax - w1/2)*sin(angle);

		dstrect.x = x-ax2;
		dstrect.y = y-ay2;

		 //draws all source region
		if(w == -1 and h == -1)
			SDL_BlitSurface(rotozoom_surface, null, GameEngine::display->implementation->sdlDisplaySurface, &dstrect);

		//draws selected region
		else
		{
			srcrect.x = from_x; srcrect.y = from_y;
			srcrect.w = w; srcrect.h = h;
			SDL_BlitSurface(rotozoom_surface, &srcrect, GameEngine::display->implementation->sdlDisplaySurface, &dstrect);
		}
		SDL_FreeSurface(rotozoom_surface);
	}


	void Image::blit(Image& img2, float x, float y, float from_x, float from_y, float h, float w)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		dstrect.x = x; dstrect.y = y;

		 //draws all source region
		if(w == -1 and h == -1)
			SDL_BlitSurface(this->implementation->sdlSurface, null, img2.implementation->sdlSurface, &dstrect);

		//draws selected region
		else
		{
			srcrect.x = from_x; srcrect.y = from_y;
			srcrect.w = w; srcrect.h = h;
			SDL_BlitSurface(this->implementation->sdlSurface, &srcrect, img2.implementation->sdlSurface, &dstrect);
		}
	}

	//XXX Does this work correctly?
	float Image::getWidth()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return this->implementation->sdlSurface->w;
	}
	float Image::getHeight()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return this->implementation->sdlSurface->h;
	}

	//******************* EVENT

	Event::Event()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;
		this->implementation->sdlEvent = new SDL_Event;
	}

	Event::~Event()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		delete implementation->sdlEvent;
		delete implementation;
	}

	Event::Type::value Event::getEventType()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		switch(this->implementation->sdlEvent->type)
		{
			case SDL_QUIT:				return Event::Type::DISPLAY_CLOSURE;
			case SDL_KEYDOWN: 			return Event::Type::KEY_PRESS;
			case SDL_KEYUP:				return Event::Type::KEY_RELEASE;
			case SDL_MOUSEBUTTONDOWN: 	return Event::Type::MOUSE_BUTTON_PRESS;
			case SDL_MOUSEBUTTONUP:		return Event::Type::MOUSE_BUTTON_RELEASE;

			//TODO map more events...

			default:					return Event::Type::NOTHING;
		}
	}

	Event::Key::value Event::getEventKeyCode()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		switch(this->implementation->sdlEvent->key.keysym.sym)
		{
			case SDLK_UP:	return Event::Key::ARROW_UP;
			case SDLK_DOWN:	return Event::Key::ARROW_DOWN;
			case SDLK_LEFT:	return Event::Key::ARROW_LEFT;
			case SDLK_RIGHT:	return Event::Key::ARROW_RIGHT;

			case SDLK_RETURN: return Event::Key::ENTER;
			case SDLK_SPACE: return Event::Key::SPACE;
			case SDLK_ESCAPE: return Event::Key::ESCAPE;

			//TODO map more buttons...

			default: 				return Event::Key::UNKNOWN;
		}
	}

	Event::MouseButton::value Event::getEventMouseButton()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		switch(this->implementation->sdlEvent->button.button)
		{
			case SDL_BUTTON_LEFT:		return Event::MouseButton::LEFT;
			case SDL_BUTTON_MIDDLE:		return Event::MouseButton::MIDDLE;
			case SDL_BUTTON_RIGHT:		return Event::MouseButton::RIGHT;

			default:	return Event::MouseButton::UNKNOWN;
		}
	}

	int Event::getEventMouseX()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return this->implementation->sdlEvent->button.x;
	}

	int Event::getEventMouseY()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return this->implementation->sdlEvent->button.y;
	}

	//******************* EVENTQUEUE

	EventQueue::EventQueue()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;
	}

	EventQueue::~EventQueue()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		delete implementation;
	}

	bool EventQueue::isEmpty()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return (SDL_PollEvent(null) == 0);
	}

	Event* EventQueue::waitForEvent()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		Event* ev = new Event;
		SDL_WaitEvent(ev->implementation->sdlEvent);
		return ev;
	}

	void EventQueue::waitForEvent(Event* container)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		SDL_WaitEvent(container->implementation->sdlEvent);
	}

	//FIXME
	void EventQueue::ignoreEvents()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
//		al_unregister_event_source(this->implementation->allegroEventQueue, al_get_display_event_source(GameEngine::display->implementation->allegroDisplay));
//		al_unregister_event_source(this->implementation->allegroEventQueue, al_get_keyboard_event_source());
//		al_unregister_event_source(this->implementation->allegroEventQueue, al_get_mouse_event_source());
//		al_flush_event_queue(this->implementation->allegroEventQueue);
	}

	//FIXME
	void EventQueue::listenEvents()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
//		al_flush_event_queue(this->implementation->allegroEventQueue);
//		al_register_event_source(this->implementation->allegroEventQueue, al_get_display_event_source(GameEngine::display->implementation->allegroDisplay));
//		al_register_event_source(this->implementation->allegroEventQueue, al_get_keyboard_event_source());
//		al_register_event_source(this->implementation->allegroEventQueue, al_get_mouse_event_source());
	}

	//FIXME
	void EventQueue::flushEvents()
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
//		al_flush_event_queue(this->implementation->allegroEventQueue);
	}

	//******************* FONT

	Font::Font(string filename, int size, bool antialiasing, bool hinting, bool kerning)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		this->implementation = new Implementation;

		if(not TTF_WasInit())
			throw Exception("FATAL ERROR: attempt to use a SDL_ttf function before initializing TTF module!");

		this->implementation->sdlttfFont = TTF_OpenFont(filename.c_str(), size);

		if(this->implementation->sdlttfFont == null)
			throw Exception("TTF Font \""+filename+"\" could not be loaded!");

		this->implementation->isAntialiased = antialiasing;
		//XXX Ignoring hinting and kerning parameters...
	}

	void Font::draw_text(string text, float x, float y, Color color)
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		SDL_Surface* renderedText = null;
		if(this->implementation->isAntialiased)
			renderedText = TTF_RenderText_Blended(this->implementation->sdlttfFont, text.c_str(), create_SDL_Color(color.r, color.g, color.b));
		else
			renderedText = TTF_RenderText_Solid(this->implementation->sdlttfFont, text.c_str(), create_SDL_Color(color.r, color.g, color.b));

		if(renderedText == null)
			throw Exception(string("Error: rendered text creation failed! ") + TTF_GetError() + "\n\""+text+"\"");

		dstrect.x = x; dstrect.y = y;
		SDL_BlitSurface(renderedText, null, GameEngine::display->implementation->sdlDisplaySurface, &dstrect);
	}

	int Font::getSize() const
	{
		if(checkInit()==false) throw Exception("Fatal error: attempt to use GameEngine library without initialization!");
		return TTF_FontHeight(this->implementation->sdlttfFont);
	}
}

#endif
