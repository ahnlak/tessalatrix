/*
 * over.c - part of Tessalatrix
 *
 * Engine for handling the 'game over' phase; either asking for high score
 * details or (if not a good enough score) just allowing a quick entry into
 * a new game.
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "SDL.h"
#include "SDL_image.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static uint_fast32_t  m_blink_tick;
static bool           m_button_blink;
static uint_fast32_t  m_cursor_tick;
static bool           m_cursor_blink;
static SDL_Texture   *m_sprite_texture;
static uint_fast32_t  m_start_tick;

static SDL_Rect       m_title_src_rect;
static SDL_Rect       m_main_button_src_rect;
static SDL_Rect       m_again_button_src_rect;

static SDL_Rect       m_title_target_rect;
static SDL_Rect       m_main_button_target_rect;
static SDL_Rect       m_main_button_deco_rect;
static SDL_Rect       m_again_button_target_rect;
static SDL_Rect       m_again_button_deco_rect;

static uint_fast8_t   m_active_button;

static bool           m_high_score;

static SDL_Keysym     m_current_cmd;
static SDL_Point      m_mouse_location;
static bool           m_mouse_moved;
static bool           m_mouse_clicked;

static char           m_player_name[TRIX_NAMELEN_MAX+1];

static const trix_gamestate_st  *m_gamestate;


/* Functions. */

/*
 * init - called when the engine is activated, to do any one-time initialising.
 */

void over_init( void )
{
  char                    l_sprite_filename[TRIX_PATH_MAX+1];
  uint_fast8_t            l_scale;
  const trix_hiscore_st  *l_hiscore_table;

  /* Load up the our spritesheet */
  l_scale = display_find_asset( TRIX_ASSET_OVER_SPRITES, l_sprite_filename );
  m_sprite_texture = IMG_LoadTexture( display_get_renderer(), l_sprite_filename );
  if ( m_sprite_texture == NULL )
  {
    log_write( ERROR, "IMG_LoadTexture of %s failed - %s", TRIX_ASSET_OVER_SPRITES, SDL_GetError() );
  }

  /* Work out the appropriately scaled source and target rectangles for this. */
  memcpy( &m_title_src_rect, 
          display_scale_rect_to_scale( 0, 0, 125, 18, l_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_main_button_src_rect,
          display_scale_rect_to_scale( 0, 20, 58, 10, l_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_again_button_src_rect,
          display_scale_rect_to_scale( 58, 20, 58, 10, l_scale ), 
          sizeof( SDL_Rect ) );  

  memcpy( &m_title_target_rect,
          display_scale_rect_to_screen( 17, 1, 125, 18 ), sizeof( SDL_Rect ) );
  memcpy( &m_main_button_target_rect,
          display_scale_rect_to_screen( 51, 75, 58, 10 ), sizeof( SDL_Rect ) );
  memcpy( &m_main_button_deco_rect,
          display_scale_rect_to_screen( 50, 74, 60, 12 ), sizeof( SDL_Rect ) );
  memcpy( &m_again_button_target_rect,
          display_scale_rect_to_screen( 51, 90, 58, 10 ), sizeof( SDL_Rect ) );
  memcpy( &m_again_button_deco_rect,
          display_scale_rect_to_screen( 50, 89, 60, 12 ), sizeof( SDL_Rect ) );

  /* Clear any current command. */
  m_current_cmd.sym = SDLK_UNKNOWN;
  m_mouse_moved = false;
  m_active_button = 2;

  /* Check to see if it's a new high score. */
  m_gamestate = game_state();
  l_hiscore_table = hiscore_read( m_gamestate->mode );

  /* We just need to have exceeded the last entry in the table! */
  if ( m_gamestate->score > l_hiscore_table[TRIX_HISCORE_COUNT-1].score )
  {
    strncpy( m_player_name, config_get_string( CONF_PLAYERNAME ), TRIX_NAMELEN_MAX );
    m_player_name[TRIX_NAMELEN_MAX] = '\0';
    m_high_score = true;
  }
  else
  {
    m_high_score = false;
  }

  /* Remember what tick we were initialised at. */
  m_blink_tick = m_start_tick = m_cursor_tick = SDL_GetTicks();

  /* All done. */
  return;
}


/*
 * event - called for every SDL event received; it's up to the engine what
 *         to do with them, but effects should be queued and handled within
 *         the update.
 */

void over_event( const SDL_Event *p_event )
{
  /* If it's a keypress, set the current command to this; yes, if the user */
  /* presses multiple keys in the same frame, we may drop some. So, maybe  */
  /* don't do that?!                                                       */
  if ( p_event->type == SDL_KEYDOWN )
  {
    m_current_cmd = p_event->key.keysym;
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

trix_engine_t over_update( void )
{
  uint_fast32_t l_current_tick = SDL_GetTicks();
  size_t        l_name_length;
  uint_fast8_t  l_index;

  /* Blink the cursor on the menu. */
  if ( l_current_tick >= ( m_blink_tick + TRIX_MOVE_MS ) )
  {
    m_button_blink = !m_button_blink;
    m_blink_tick = l_current_tick;
  }

  /* And the text entry cursor. */
  if ( l_current_tick >= ( m_cursor_tick + 300 ) )
  {
    m_cursor_blink = !m_cursor_blink;
    m_cursor_tick = l_current_tick;
  }

  /* Handle any mouse movements. */
  if ( m_mouse_moved || m_mouse_clicked )
  {
    /* See if it's over a button */
    if ( SDL_PointInRect( &m_mouse_location, &m_main_button_deco_rect ) )
    {
      /* Select the option. */
      m_active_button = 1;

      /* And if we've clicked, select that option too. */
      if ( m_mouse_clicked )
      {
        m_current_cmd.sym = SDLK_RETURN;
      }
    }
    else if ( SDL_PointInRect( &m_mouse_location, &m_again_button_deco_rect ) )
    {
      /* Select the option. */
      m_active_button = 2;

      /* And if we've clicked, select that option too. */
      if ( m_mouse_clicked )
      {
        m_current_cmd.sym = SDLK_RETURN;
      }
    }
    else
    {
      m_active_button = 0;
    }

    /* And clear the mouse flags. */
    m_mouse_moved = m_mouse_clicked = false;
  }

  /* Handle typing; these commands map rather kindly into lowercase ASCII */
  /* and although I don't like relying on that, it's a lot tidier.        */
  if ( ( m_current_cmd.sym >= SDLK_a && m_current_cmd.sym <= SDLK_z ) ||
       ( m_current_cmd.sym >= SDLK_SPACE && m_current_cmd.sym <= SDLK_AT ) )
  {
    l_name_length = strlen( m_player_name );
    if ( l_name_length < ( TRIX_NAMELEN_MAX-1 ) )
    {
      if ( m_current_cmd.mod & (KMOD_SHIFT|KMOD_CAPS) )
      {
        m_player_name[l_name_length] = toupper( m_current_cmd.sym );
      }
      else
      {
        m_player_name[l_name_length] = m_current_cmd.sym;
      }
      m_player_name[l_name_length+1] = '\0';
    }
  }
  if ( m_current_cmd.sym == SDLK_BACKSPACE )
  {
    l_name_length = strlen( m_player_name );
    if ( l_name_length > 0 )
    {
      m_player_name[l_name_length-1] = '\0';
    }    
  }

  /* Process the current command. */
  switch( m_current_cmd.sym )
  {
    case SDLK_UP:                                             /* Move up. */
      /* Effectively just select the top button. */
      m_active_button = 1;
      break;
    case SDLK_DOWN:                                         /* Move down. */
      /* Effectively just select the bottom button. */
      m_active_button = 2;
      break;
    case SDLK_RETURN:            /* If the button is active, activate it! */
      /* If there is no active button, do nothing. */
      if ( m_active_button == 0 )
      {
        break;
      }

      /* Save the high score, if there is one. */
      if ( m_high_score )
      {
        /* If the player name is blanked out, default to unknown. */
        for ( l_index = strlen( m_player_name ); l_index > 0; l_index-- )
        {
          if ( !isspace( m_player_name[l_index-1] ) )
          {
            break;
          }
        }
        if ( l_index == 0 )
        {
          strcpy( m_player_name, "unknown" );
        }
        hiscore_save( m_gamestate->mode, m_gamestate->score,
                      m_gamestate->lines, m_player_name );
      }

      /* Lastly, return the appropriate engine mode. */
      if ( m_active_button == 1 )
      {
        return ENGINE_MENU;
      }
      return ENGINE_GAME;
      break;
  }

  /* Clear any current command, for the next input. */
  m_current_cmd.sym = SDLK_UNKNOWN;

  /* Stay in our present engine. */
  return ENGINE_OVER;
}


/*
 * render - draws the internal state of the engine onto the screen; no logic
 *          should be here, it's all the presentational stuff.
 */

void over_render( void )
{
  SDL_Rect   l_name_size;

  /* Clear to black. */
  SDL_SetRenderDrawColor( display_get_renderer(), 0, 0, 0, 255 );
  SDL_RenderClear( display_get_renderer() );

  /* Fill in the button halo, if required. */
  if ( m_active_button > 0 )
  {
    if ( m_button_blink )
    {
      SDL_SetRenderDrawColor( display_get_renderer(), 255, 213, 65, 255 );
    }
    else
    {
      SDL_SetRenderDrawColor( display_get_renderer(), 255, 252, 64, 255 );    
    }
  }
  switch( m_active_button )
  {
    case 1:
      SDL_RenderFillRect( display_get_renderer(), &m_main_button_deco_rect );  
      break;
    case 2:
      SDL_RenderFillRect( display_get_renderer(), &m_again_button_deco_rect );  
      break;
  }

  /* Render the title, and buttons. */
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, 
                  &m_title_src_rect, &m_title_target_rect );
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, 
                  &m_main_button_src_rect, &m_main_button_target_rect );
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, 
                  &m_again_button_src_rect, &m_again_button_target_rect );

  /* Drop in the score and lines of the completed game. */
  text_draw_around( 80, 30, "You scored %05d with %d lines", 
                    m_gamestate->score, m_gamestate->lines );

  /* And then, if required, render the name entry. */
  if ( m_high_score )
  {
    /* Work out the size of the player name. */
    l_name_size = text_measure( "%s_", m_player_name );

    /* And draw that in the middle of the screen, suitably centered. */
    if ( m_cursor_blink )
    {
      text_draw( 80 - ( l_name_size.w / 2 ), 45, "%s", m_player_name );
    }
    else
    {
      text_draw( 80 - ( l_name_size.w / 2 ), 45, "%s_", m_player_name );
    }
    text_draw_around( 80, 60, "This is a new high score!" );
  }
  else
  {
    text_draw_around( 80, 60, "This is not a new high score, %s", m_player_name );
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

void over_fini( void )
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


/* End of file over.c */
