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


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static SDL_Texture *m_splash_texture;


/* Functions. */

/*
 * init - called when the engine is activated, to do any one-time initialising.
 */

void splash_init( void )
{
  /* Load up the splash image (hopefully!) */
  m_splash_texture = IMG_LoadTexture( display_get_renderer(), TRIX_ASSET_SPLASH );

  /* All done. */
  return;
}


/*
 * update - update the internal state of the engine; this is also where we
 *          handle and process any user input, but no drawing is done here.
 */

trix_engine_t splash_update( void )
{
  /* By default, ask to stay in our current engine. */
  return ENGINE_SPLASH;
}


/*
 * render - draws the internal state of the engine onto the screen; no logic
 *          should be here, it's all the presentational stuff.
 */

void splash_render( void )
{
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
