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
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "SDL.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

const SDL_Rect      m_resolutions[] = {
  {.x=0, .y=0, .w= 640, .h= 480},
  {.x=0, .y=0, .w= 800, .h= 600},
  {.x=0, .y=0, .w=1024, .h= 768},
  {.x=0, .y=0, .w=1280, .h= 800},
  {.x=0, .y=0, .w=1440, .h= 900},
  {.x=0, .y=0, .w=1680, .h=1050},
  {.x=0, .y=0, .w=1920, .h=1200}
};
static SDL_Window  *m_window;


/*
 * Static functions; a collection of things only built for use locally.
 */

/*
 * display_fits_inside - given two rectangles, returns true if rectangle A fits
 *                       inside rectangle B and false if not.
 */

static bool display_fits_inside( const SDL_Rect *p_rect_a, const SDL_Rect *p_rect_b )
{
  /* Check the top left corner. */
  if ( ( p_rect_a->x < p_rect_b->x ) || ( p_rect_a->y < p_rect_b->y ) )
  {
    return false;
  }

  /* And the bottom right, taking into account any x/y offset. */
  if ( ( ( p_rect_a->x + p_rect_a->w ) > ( p_rect_b->x + p_rect_b->w ) ) || 
       ( ( p_rect_a->y + p_rect_a->h ) > ( p_rect_b->y + p_rect_b->h ) ) )
  {
    return false;
  }

  /* Good, then he fits! */
  return true;
}


/* Functions. */

/*
 * display_init - sets up the SDL library and opens up our window. A boolean
 *                flag indicates if we had any catastrophic failures.
 */

bool display_init( void )
{
  int_fast8_t l_index;
  SDL_Rect    l_display_bounds, l_window_size;

  /* First step, ask SDL to wake up. */
  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
  {
    log_write( ERROR, "SDL_Init() failed  - %s", SDL_GetError() );
    return false;
  }

  /* Pull the window size from our configuration. */
  l_window_size.x = l_window_size.y = 0;
  l_window_size.w = config_get_int( CONF_WINDOW_WIDTH );
  l_window_size.h = config_get_int( CONF_WINDOW_HEIGHT );

  /* Fetch the desktop bounds; make sure our initial window size makes sense. */
  if ( SDL_GetDisplayBounds( 0, &l_display_bounds ) < 0 )
  {
    log_write( ERROR, "SDL_GetDisplayBounds() failed  - %s", SDL_GetError() );
    display_fini();
    return false;    
  }

  if ( !display_fits_inside( &l_window_size, &l_display_bounds ) )
  {
    /* Work through our resolutions until we find one that fits. */
    for ( l_index = ( sizeof(m_resolutions) / sizeof(SDL_Rect) ) - 1; l_index >=0; l_index-- )
    {
      if ( display_fits_inside( &m_resolutions[l_index], &l_display_bounds ) )
      {
        memcpy( &l_window_size, &m_resolutions[l_index], sizeof(SDL_Rect) );
        break;
      }
    }

    /* If we didn't find anything viable, then, well, bugger. */
    if ( l_index < 0 )
    {
      log_write( ERROR, "Unable to find any valid resolutions!" );
      display_fini();
      return false;
    }
  }

  /* Now, open up the window we'll use. */
  m_window = SDL_CreateWindow( util_app_name(), 
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                               l_window_size.w, l_window_size.h,
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
