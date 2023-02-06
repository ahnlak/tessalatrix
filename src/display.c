/*
 * display.c - part of Tessalatrix
 *
 * Display-oriented functions; mostly just wrapping up SDL stuff for 
 * convenience. We'll also try to keep any platform-specific fiddling (although
 * sadly not Empscripten!) here in one place so the game logic doesn't have to
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
#include <unistd.h>
#include "SDL.h"
#include "SDL_image.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

const trix_resolution_st m_resolutions[] = {
  {.x=  0, .y=20, .w= 640, .h= 480, .scale=4},
  {.x=  0, .y=25, .w= 800, .h= 600, .scale=5},
  {.x= 32, .y=54, .w=1024, .h= 768, .scale=6},
  {.x= 80, .y=15, .w=1280, .h= 800, .scale=7},
  {.x= 80, .y=10, .w=1440, .h= 900, .scale=8},
  {.x=120, .y=30, .w=1680, .h=1050, .scale=9},
  {.x=160, .y=50, .w=1920, .h=1200, .scale=10}
};
static int            m_current_resolution;
static SDL_Window    *m_window;
static SDL_Renderer  *m_renderer;


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
  SDL_Rect    l_display_bounds;

  /* First step, ask SDL to wake up. */
  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
  {
    log_write( ERROR, "SDL_Init() failed  - %s", SDL_GetError() );
    return false;
  }

  /* And SDL_Image, too. */
  if ( IMG_Init( IMG_INIT_PNG ) == 0 )
  {
    log_write( ERROR, "IMG_Init() failed" );
    display_fini();
    return false;
  }

  /* Pull the window size from our configuration. */
  m_current_resolution = config_get_int( CONF_RESOLUTION );

  /* Fetch the desktop bounds; make sure our initial window size makes sense. */
  if ( SDL_GetDisplayBounds( 0, &l_display_bounds ) < 0 )
  {
    log_write( ERROR, "SDL_GetDisplayBounds() failed  - %s", SDL_GetError() );
    display_fini();
    return false;    
  }

  if ( ( m_resolutions[m_current_resolution].w > l_display_bounds.w ) ||
       ( m_resolutions[m_current_resolution].h > l_display_bounds.h ) )
  {
    /* Work through our resolutions until we find one that fits. */
    for ( l_index = ( sizeof(m_resolutions) / sizeof(trix_resolution_st) ) - 1; l_index >= 0; l_index-- )
    {
      if ( ( m_resolutions[l_index].w <= l_display_bounds.w ) &&
           ( m_resolutions[l_index].h <= l_display_bounds.h ) )
      {
        m_current_resolution = l_index;
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
                               m_resolutions[m_current_resolution].w,
                               m_resolutions[m_current_resolution].h,
                               SDL_WINDOW_SHOWN );
  if ( m_window == NULL )
  {
    /* Not opening the window is a definite fail! */
    display_fini();
    return false;
  }

  /* Try to build a renderer for that window. */
  m_renderer = SDL_CreateRenderer( m_window, -1, 0 );
  if ( m_renderer == NULL )
  {
    /* We can't work without a renderer. */
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
  /* Destroy the renderer, if we have one. */
  if ( m_renderer != NULL )
  {
    SDL_DestroyRenderer( m_renderer );
    m_renderer = NULL;
  }

  /* Close the window, if it's open. */
  if ( m_window != NULL )
  {
    SDL_DestroyWindow( m_window );
    m_window = NULL;
  }

  /* Try to shut down SDL_Image. */
  IMG_Quit();

  /* And shut down the rest of the library. */
  SDL_Quit();

  /* As we're shutting down there's no real point in returning failures! */
  return;
}


/*
 * display_get_renderer - exposes the renderer on which we want to do all our
 *                        drawing. Slight overhead, vs using a global but I
 *                        feel marginally less dirty this way.
 */

SDL_Renderer *display_get_renderer( void )
{
  /* Very simple, for now. */
  return m_renderer;
}


/*
 * display_get_scale - exposes the current screen scale; not ideal, but some
 *                     minor use cases make more sense to do it this way.
 */

uint_fast8_t display_get_scale( void )
{
  return m_resolutions[m_current_resolution].scale;
}


/*
 * display_scale_point - given a logical x, y co-ordinate, return an SDL_Point
 *                       that is scaled to the current screen resolution.
 *                       Note that this Point is a static, so should be copied
 *                       for long-term use.
 */

SDL_Point *display_scale_point( uint_fast8_t p_x, uint_fast8_t p_y )
{
  static SDL_Point l_point;

  /* Apply the scaling factor for the current display, plus offset if required */
  l_point.x = p_x * m_resolutions[m_current_resolution].scale + m_resolutions[m_current_resolution].x;
  l_point.y = p_y * m_resolutions[m_current_resolution].scale + m_resolutions[m_current_resolution].y;


  /* And return a pointer to our static structure. */
  return &l_point;
}


/*
 * display_scale_rect_to_screen - given a logical x, y co-ordinates and width, return an
 *                      SDL_Rect that is scaled to the current screen resolution.
 *                      Note that this Rect is a static, so should be copied
 *                      for long-term use.
 */

SDL_Rect *display_scale_rect_to_screen( uint_fast8_t p_x, uint_fast8_t p_y, 
                                        uint_fast8_t p_w, uint_fast8_t p_h )
{
  static SDL_Rect l_rect;

  /* Apply the scaling factor for the current display, plus offset if required */
  l_rect.x = p_x * m_resolutions[m_current_resolution].scale + m_resolutions[m_current_resolution].x;
  l_rect.y = p_y * m_resolutions[m_current_resolution].scale + m_resolutions[m_current_resolution].y;

  /* Scaling only for the width and height, no offsets. */
  l_rect.w = p_w * m_resolutions[m_current_resolution].scale;
  l_rect.h = p_h * m_resolutions[m_current_resolution].scale;

  /* And return a pointer to our static structure. */
  return &l_rect;  
}


/*
 * display_scale_rect_to_scale - does a similar job to _to_screen, but to a user
 *                               provided scale; useful for indexing different
 *                               scale spritesheets depending on resolution.
 */

SDL_Rect *display_scale_rect_to_scale( uint_fast8_t p_x, uint_fast8_t p_y, 
                                       uint_fast8_t p_w, uint_fast8_t p_h, uint_fast8_t p_scale )
{
  static SDL_Rect l_rect;

  /* Apply the scaling factor for the required scale. */
  l_rect.x = p_x * p_scale;
  l_rect.y = p_y * p_scale;

  /* Scaling only for the width and height, no offsets. */
  l_rect.w = p_w * p_scale;
  l_rect.h = p_h * p_scale;

  /* And return a pointer to our static structure. */
  return &l_rect;  
}


/* 
 * display_find_asset - given a bare asset name, determines the appropriate
 *                      PNG file to load for the current resolution - falling
 *                      back to the next highest resolutions as required.
 *                      Filenames are <asset-name>-<scale-factor>.png
 *                      This function returns the scale factor used, or 0 if
 *                      no suitable file could be identified.
 */

uint_fast8_t display_find_asset( const char *p_asset_name, char *p_file_buffer )
{
  int_fast8_t   l_index;
  uint_fast8_t  l_scale;
  char          l_asset_prefix[TRIX_PATH_MAX+1];

  /* All assets are in the asset path - work out the prefix. */
  snprintf( l_asset_prefix, sizeof( l_asset_prefix ), "%.100s/%.100s",
            TRIX_ASSET_PATH, p_asset_name );

  /* And now, work through scales, starting with the present resolution. */
  for ( l_index = m_current_resolution; l_index >= 0; l_index-- )
  {
    /* Form up the full asset name. */
    snprintf( p_file_buffer, TRIX_PATH_MAX, "%.200s-%d.png",
              l_asset_prefix, m_resolutions[l_index].scale );

    /* If it's readable, we have a hit. */
    if ( access( p_file_buffer, R_OK ) == 0 )
    {
      l_scale = m_resolutions[l_index].scale;
      break;
    }
  }

  /* If we got no match, last effort is the naked asset - fail to blank. */
  if ( l_index < 0 )
  {
    snprintf( p_file_buffer, TRIX_PATH_MAX, "%.200s.png", l_asset_prefix );
    if ( access( p_file_buffer, R_OK ) == 0 )
    {
      /* Naked assets are assumed to be the lowest resolution. */
      l_scale = m_resolutions[0].scale;
    }
    else
    {
      p_file_buffer[0] = '\0';
      l_scale = 0;
    }
  }

  /* And return the scale factor. */
  return l_scale;
}

/* End of file display.c */
