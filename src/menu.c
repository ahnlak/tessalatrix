/*
 * menu.c - part of Tessalatrix
 *
 * Handles the main menu, from which users can start/resume the game, change
 * game and display options, and of course exit.
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
static uint_fast32_t  m_start_tick;
static SDL_Rect       m_sprite_rect_title;
static SDL_Keycode    m_current_cmd;


/*
 * Static functions; a collection of things only built for use locally.
 */

/*
 * load_sprites - called to (re) load the sprite sheet and calculate the
 *                various source rectangles from it. Always called in init,
 *                but can be recalled any time to deal with a change in
 *                resolution.
 */

static bool menu_load_sprites( void )
{
  char          l_sprite_filename[TRIX_PATH_MAX+1];
  uint_fast8_t  l_sprite_scale;

  /* Load up the sprite image (hopefully!) */
  l_sprite_scale = display_find_asset( TRIX_ASSET_MENU_SPRITES, l_sprite_filename );
  m_sprite_texture = IMG_LoadTexture( display_get_renderer(), l_sprite_filename );
  if ( m_sprite_texture == NULL )
  {
    log_write( ERROR, "IMG_LoadTexture of %s failed - %s", TRIX_ASSET_MENU_SPRITES, SDL_GetError() );
    return false;
  }

  /* Now calculate the source rects we'll use for this sheet. */
  memcpy( &m_sprite_rect_title, 
          display_scale_rect_to_scale( 0, 0, 152, 18, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  

  /* All done! */
  return true;
}


/* Functions. */

/*
 * init - called when the engine is activated, to do any one-time initialising.
 */

void menu_init( void )
{
  /* Load up the sprite image (hopefully!) */
  if ( !menu_load_sprites() )
  {
    log_write( ERROR, "Failed to load menu sprites" );
  }

  /* Clear any current command. */
  m_current_cmd = SDLK_UNKNOWN;

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
  /* If it's a keypress, set the current command to this; yes, if the user */
  /* presses multiple keys in the same frame, we may drop some. So, maybe  */
  /* don't do that?!                                                       */
  if ( p_event->type == SDL_KEYDOWN )
  {
    m_current_cmd = p_event->key.keysym.sym;
  }

  /* All done. */
  return;
}


/*
 * update - update the internal state of the engine; this is also where we
 *          handle and process any user input, but no drawing is done here.
 */

trix_engine_t menu_update( void )
{
  /* Process the current command. */
  switch( m_current_cmd )
  {
    case SDLK_RETURN:     /* For now, bounce onto the game. */
      return ENGINE_GAME;
      break;
  }

  /* Clear any current command, for the next input. */
  m_current_cmd = SDLK_UNKNOWN;

  /* By default, ask to stay in our current engine. */
  return ENGINE_MENU;
}


/*
 * render - draws the internal state of the engine onto the screen; no logic
 *          should be here, it's all the presentational stuff.
 */

void menu_render( void )
{
  SDL_Rect l_title_dest;

  /* Clear to black. */
  SDL_SetRenderDrawColor( display_get_renderer(), 0, 0, 0, 255 );
  SDL_RenderClear( display_get_renderer() );

  /* Draw the title, centered, top of the screen. */
  memcpy( &l_title_dest, display_scale_rect_to_screen( 4, 1, 152, 18 ), sizeof( SDL_Rect ) );

  /* Do the blit. */
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, &m_sprite_rect_title, &l_title_dest );

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

void menu_fini( void )
{
  /* All done. */
  return;
}


/* End of file menu.c */
