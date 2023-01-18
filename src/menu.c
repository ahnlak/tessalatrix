/*
 * menu.c - part of Tessalatrix
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

static SDL_Texture   *m_sprite_texture;
static uint_fast8_t   m_sprite_scale;
static uint_fast32_t  m_start_tick;


/* Functions. */

/*
 * init - called when the engine is activated, to do any one-time initialising.
 */

void menu_init( void )
{
  char          l_sprite_filename[TRIX_PATH_MAX+1];

  /* Load up the sprite image (hopefully!) */
  m_sprite_scale = display_find_asset( TRIX_ASSET_MENU_SPRITES, l_sprite_filename );
  m_sprite_texture = IMG_LoadTexture( display_get_renderer(), l_sprite_filename );
  if ( m_sprite_texture == NULL )
  {
    log_write( ERROR, "IMG_LoadTexture of %s failed - %s", TRIX_ASSET_MENU_SPRITES, SDL_GetError() );
  }

  /* Remember what tick we were initialised at. */
  m_start_tick = SDL_GetTicks();

  /* All done. */
  return;
}


/*
 * event - called for every SDL event received; it's up to the engine what
 *         to do with them, but effects should be queued and handled within
 *         the update.
 */

void menu_event( const SDL_Event *p_event )
{
  /* All done. */
  return;
}


/*
 * update - update the internal state of the engine; this is also where we
 *          handle and process any user input, but no drawing is done here.
 */

trix_engine_t menu_update( void )
{
  /* By default, ask to stay in our current engine. */
  return ENGINE_MENU;
}


/*
 * render - draws the internal state of the engine onto the screen; no logic
 *          should be here, it's all the presentational stuff.
 */

void menu_render( void )
{
  SDL_Rect l_title_src, l_title_dest;

  /* Clear to black. */
  SDL_SetRenderDrawColor( display_get_renderer(), 0, 0, 0, 255 );
  SDL_RenderClear( display_get_renderer() );

  /* Draw the title, centered, top of the screen. */
  memcpy( &l_title_src, display_scale_rect_to_scale( 0, 0, 142, 17, m_sprite_scale ), sizeof( SDL_Rect ) );
  memcpy( &l_title_dest, display_scale_rect_to_screen( 9, 1, 142, 17 ), sizeof( SDL_Rect ) );

  /* Do the blit. */
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, &l_title_src, &l_title_dest );

  /* Last thing to do, ask the renderer to present to the window. */
  SDL_RenderPresent( display_get_renderer() );

  /* All done. */
  return;
}


/*
 * fini - called when the engine is being stopped, to do any tear down. 
 */

void menu_fini( void )
{
  /* All done. */
  return;
}


/* End of file menu.c */
