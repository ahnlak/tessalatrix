/*
 * hstable.c - part of Tessalatrix
 *
 * Engine for rendering the high score tables; fairly simple UI here, showing
 * any high scores we have stored for each of the game types.
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

static uint_fast32_t  m_blink_tick;
static bool           m_button_blink;
static SDL_Texture   *m_sprite_texture;
static uint_fast32_t  m_start_tick;
static SDL_Rect       m_target_rect;
static SDL_Rect       m_button_deco_rect;
static bool           m_button_active;
static SDL_Keycode    m_current_cmd;
static SDL_Point      m_mouse_location;
static bool           m_mouse_moved;
static bool           m_mouse_clicked;

static const trix_hiscore_st *m_table;

/* Functions. */

/*
 * init - called when the engine is activated, to do any one-time initialising.
 */

void hstable_init( void )
{
  char          l_sprite_filename[TRIX_PATH_MAX+1];
  uint_fast8_t  l_scale;

  /* Load up the our spritesheet */
  l_scale = display_find_asset( TRIX_ASSET_HST_SPRITES, l_sprite_filename );
  m_sprite_texture = IMG_LoadTexture( display_get_renderer(), l_sprite_filename );
  if ( m_sprite_texture == NULL )
  {
    log_write( ERROR, "IMG_LoadTexture of %s failed - %s", TRIX_ASSET_HST_SPRITES, SDL_GetError() );
  }

  /* Work out the appropriately scaled target rectangle for this. */
  memcpy( &m_target_rect, 
          display_scale_rect_to_screen( 0, 0, 160, 110 ),
          sizeof( SDL_Rect ) );

  /* And the button deco. */
  memcpy( &m_button_deco_rect,
          display_scale_rect_to_screen( 50, 91, 60, 11 ), sizeof( SDL_Rect ) );
  m_button_active = true;

  /* Clear any current command. */
  m_current_cmd = SDLK_UNKNOWN;
  m_mouse_moved = false;

  /* Remember what tick we were initialised at. */
  m_blink_tick = m_start_tick = SDL_GetTicks();

  /* Load up the high score table for our current game mode, by default. */
  m_table = hiscore_read( GAME_MODE_STANDARD );

  /* All done. */
  return;
}


/*
 * event - called for every SDL event received; it's up to the engine what
 *         to do with them, but effects should be queued and handled within
 *         the update.
 */

void hstable_event( const SDL_Event *p_event )
{
  /* If it's a keypress, set the current command to this; yes, if the user */
  /* presses multiple keys in the same frame, we may drop some. So, maybe  */
  /* don't do that?!                                                       */
  if ( p_event->type == SDL_KEYDOWN )
  {
    m_current_cmd = p_event->key.keysym.sym;
  }

  /* If the mouse has moved, flag the movement and remember the location. */
  if ( p_event->type == SDL_MOUSEMOTION )
  {
    m_mouse_moved = true;
    m_mouse_location.x = p_event->motion.x;
    m_mouse_location.y = p_event->motion.y;
  }
  if ( p_event->type == SDL_MOUSEBUTTONDOWN )
  {
    m_mouse_clicked = true;
  }

  /* All done. */
  return;
}


/*
 * update - update the internal state of the engine; this is also where we
 *          handle and process any user input, but no drawing is done here.
 */

trix_engine_t hstable_update( void )
{
  uint_fast32_t l_current_tick = SDL_GetTicks();

  /* Blink the cursor on the menu. */
  if ( l_current_tick >= ( m_blink_tick + TRIX_MOVE_MS ) )
  {
    m_button_blink = !m_button_blink;
    m_blink_tick = l_current_tick;
  }

  /* Handle any mouse movements. */
  if ( m_mouse_moved || m_mouse_clicked )
  {
    /* See if it's over the button */
    if ( SDL_PointInRect( &m_mouse_location, &m_button_deco_rect ) )
    {
      /* Select the option. */
      m_button_active = true;

      /* And if we've clicked, select that option too. */
      if ( m_mouse_clicked )
      {
        m_current_cmd = SDLK_RETURN;
      }
    }
    else
    {
      m_button_active = false;
    }

    /* And clear the mouse flags. */
    m_mouse_moved = m_mouse_clicked = false;
  }

  /* Process the current command. */
  switch( m_current_cmd )
  {
    case SDLK_UP:                                             /* Move up. */
    case SDLK_DOWN:                                         /* Move down. */
      /* This simply activates the main menu buttton if it's not. */
      m_button_active = true;
      break;
    case SDLK_RETURN:            /* If the button is active, activate it! */
      if ( m_button_active )
      {
        return ENGINE_MENU;
      }
      break;
  }

  /* Clear any current command, for the next input. */
  m_current_cmd = SDLK_UNKNOWN;

  /* Stay in our present engine. */
  return ENGINE_HSTABLE;
}


/*
 * render - draws the internal state of the engine onto the screen; no logic
 *          should be here, it's all the presentational stuff.
 */

void hstable_render( void )
{
  uint_fast8_t  l_index;

  /* Clear to black. */
  SDL_SetRenderDrawColor( display_get_renderer(), 0, 0, 0, 255 );
  SDL_RenderClear( display_get_renderer() );

  /* Fill in the button halo, if required. */
  if ( m_button_active )
  {
    if ( m_button_blink )
    {
      SDL_SetRenderDrawColor( display_get_renderer(), 255, 213, 65, 255 );
    }
    else
    {
      SDL_SetRenderDrawColor( display_get_renderer(), 255, 252, 64, 255 );    
    }
    SDL_RenderFillRect( display_get_renderer(), &m_button_deco_rect );  
  }

  /* Render the frame in which we'll draw the table. */
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, NULL, &m_target_rect );

  /* Now build the currency displaying table. */
  for ( l_index = 0; l_index < TRIX_HISCORE_COUNT; l_index++ )
  {
    text_draw(  20, 20 + ( l_index * 7 ), m_table[l_index].name );
    text_draw(  90, 20 + ( l_index * 7 ), "%5d", m_table[l_index].score );
    text_draw( 130, 20 + ( l_index * 7 ), "%4d", m_table[l_index].lines );
  }

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

void hstable_fini( void )
{
  /* Release any loaded textures. */
  if ( m_sprite_texture != NULL )
  {
    SDL_DestroyTexture( m_sprite_texture );
    m_sprite_texture = NULL;
  }
  
  /* All done. */
  return;
}


/* End of file hstable.c */
