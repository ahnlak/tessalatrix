/*
 * splash.c - part of Tessalatrix
 *
 * Engine for rendering the 'ahnlak' splash branding; about as simple as
 * engines get, we just fade in and then out the logo, while allowing users
 * to anykey out of it.
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include "SDL.h"
#include "SDL_image.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static SDL_Texture   *m_splash_texture;
static uint_fast32_t  m_start_tick;
static bool           m_abort = false;
static SDL_Rect       m_target_rect;


/* Functions. */

/*
 * init - called when the engine is activated, to do any one-time initialising.
 */

void splash_init( void )
{
  char          l_splash_filename[TRIX_PATH_MAX+1];
  uint_fast8_t  l_scale;

  /* Load up the splash image (hopefully!) */
  l_scale = display_find_asset( TRIX_ASSET_SPLASH, l_splash_filename );
  m_splash_texture = IMG_LoadTexture( display_get_renderer(), l_splash_filename );
  if ( m_splash_texture == NULL )
  {
    log_write( ERROR, "IMG_LoadTexture of %s failed - %s", TRIX_ASSET_SPLASH, SDL_GetError() );
  }

  /* Work out the appropriately scaled target rectangle for this. */
  memcpy( &m_target_rect, 
          display_scale_rect_to_screen( 0, 0, 160, 110 ),
          sizeof( SDL_Rect ) );

  /* Remember what tick we were initialised at. */
  m_start_tick = SDL_GetTicks();

  /* Make sure the abort flag isn't set. */
  m_abort = false;

  /* All done. */
  return;
}


/*
 * event - called for every SDL event received; it's up to the engine what
 *         to do with them, but effects should be queued and handled within
 *         the update.
 */

void splash_event( const SDL_Event *p_event )
{
  /* Pressing any key is enough to skip the boring splash. */
  if ( ( p_event->type == SDL_KEYDOWN ) || ( p_event->type == SDL_MOUSEBUTTONDOWN ) )
  {
    m_abort = true;
  }

  /* All done. */
  return;
}


/*
 * update - update the internal state of the engine; this is also where we
 *          handle and process any user input, but no drawing is done here.
 */

trix_engine_t splash_update( void )
{
  static uint_fast8_t l_alpha = 0;
  const uint_fast16_t l_speed = 1000;

  /* Before anything, if we want to abort, do so. */
  if ( m_abort )
  {
    return ENGINE_MENU;
  }

  /* Work out how much time has passed... */
  const uint_fast32_t l_ticks_passed = SDL_GetTicks() - m_start_tick;

  /* We fade up / down at a pace determined by l_speed. */
  if ( l_ticks_passed < l_speed )
  {
    /* In the first phase, we fade up the alpha. */
    l_alpha = ( l_ticks_passed * 255 ) / l_speed;

  }
  else if ( l_ticks_passed < ( l_speed * 2 ) )
  {
    /* In the second phase, just hold the splash. */
    l_alpha = 255;
  }
  else if ( l_ticks_passed < ( l_speed * 3 ) )
  {
    /* In the third phase, we fade it back down again. */
    l_alpha = 255 - ( ( l_ticks_passed-l_speed-l_speed ) * 255 ) / l_speed;
  }
  else
  {
    /* And the end of it, we jump to the next engine. */
    return ENGINE_MENU;
  }

  /* Set the alpha on the splash image to an appropriate value. */
  SDL_SetTextureAlphaMod( m_splash_texture, l_alpha );

  /* By default, ask to stay in our current engine. */
  return ENGINE_SPLASH;
}


/*
 * render - draws the internal state of the engine onto the screen; no logic
 *          should be here, it's all the presentational stuff.
 */

void splash_render( void )
{
  /* For now, just clear down the screen, and draw the splash texture. */

  /* Clear to black. */
  SDL_SetRenderDrawColor( display_get_renderer(), 0, 0, 0, 255 );
  SDL_RenderClear( display_get_renderer() );

  /* Render the splash image, stretched if we need to. */
  SDL_RenderCopy( display_get_renderer(), m_splash_texture, NULL, &m_target_rect );

  /* Finally, render the metrics count. */
  metrics_render();

  /* Last thing to do, ask the renderer to present to the window. */
  SDL_RenderPresent( display_get_renderer() );

  /* All done. */
  return;
}


/*
 * fini - called when the engine is being stopped, to do any tear down. 
 */

void splash_fini( void )
{
  /* Release any loaded textures. */
  if ( m_splash_texture != NULL )
  {
    SDL_DestroyTexture( m_splash_texture );
    m_splash_texture = NULL;
  }
  
  /* All done. */
  return;
}


/* End of file splash.c */
