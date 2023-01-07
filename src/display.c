/*
 * display.c - part of Tessalatrix
 *
 * Display-oriented functions; mostly just wrapping up SDL stuff for 
 * convenience. We'll also keep any platform-specific fiddling (I'm looking
 * at you, Empscripten!) here in one place so the game logic doesn't have to
 * think about anything.
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "SDL.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static SDL_Window  *m_window;


/* Functions. */

/*
 * display_init - sets up the SDL library and opens up our window. A boolean
 *                flag indicates if we had any catastrophic failures.
 */

bool display_init( void )
{
  SDL_Rect  l_display_bounds;

  /* First step, ask SDL to wake up. */
  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
  {
    log_write( ERROR, "SDL_Init() failed  - %s", SDL_GetError() );
    return false;
  }

  /* Fetch the desktop bounds; make sure our initial window size makes sense. */
  if ( SDL_GetDisplayBounds( 0, &l_display_bounds ) < 0 )
  {
    log_write( ERROR, "SDL_GetDisplayBounds() failed  - %s", SDL_GetError() );
    display_fini();
    return false;    
  }

  /* Now, open up the window we'll use. */
  m_window = SDL_CreateWindow( util_app_name(), 
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                               640, 480,
                               SDL_WINDOW_SHOWN );
  if ( m_window == NULL )
  {
    /* Not opening the window is a definite fail! */
    display_fini();
    return false;
  }

  /* All fine. */
  return true;
}


/*
 * display_fini - closes up the window and shuts down SDL.
 */

void display_fini( void )
{
  /* Close the window, if it's open. */
  if ( m_window != NULL )
  {
    SDL_DestroyWindow( m_window );
    m_window = NULL;
  }

  /* And shut down the rest of the library. */
  SDL_Quit();

  /* As we're shutting down there's no real point in returning failures! */
  return;
}


/* End of file display.c */
