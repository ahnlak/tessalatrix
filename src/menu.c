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
static uint_fast32_t  m_blink_tick;
static uint_fast32_t  m_last_move_tick;
static SDL_Rect       m_sprite_rect_title;
static SDL_Rect       m_target_rect_title;
static SDL_Rect       m_sprite_menu_new;
static SDL_Rect       m_sprite_menu_resume;
static SDL_Rect       m_sprite_menu_highscore;
static SDL_Rect       m_sprite_menu_options;
static SDL_Rect       m_sprite_menu_rect[TRIX_MENU_ENTRIES];
static SDL_Rect       m_target_menu_rect[TRIX_MENU_ENTRIES];
static SDL_Rect       m_target_menu_deco_rect[TRIX_MENU_ENTRIES];
static bool           m_menu_blink;
static bool           m_option_enabled[TRIX_MENU_ENTRIES];

static SDL_Keycode    m_current_cmd;
static SDL_Point      m_mouse_location;
static bool           m_mouse_moved;
static bool           m_mouse_clicked;

static uint_fast8_t   m_current_option;


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

  memcpy( &m_sprite_menu_rect[0],
          display_scale_rect_to_scale( 0, 20, 58, 10, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_sprite_menu_rect[1],
          display_scale_rect_to_scale( 0, 30, 58, 10, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_sprite_menu_rect[2],
          display_scale_rect_to_scale( 0, 40, 58, 10, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_sprite_menu_rect[3],
          display_scale_rect_to_scale( 58, 20, 58, 10, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_sprite_menu_rect[4],
          display_scale_rect_to_scale( 58, 30, 58, 10, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  

  /* Also calculate some target rectangles, for the menu and decorations. */
  memcpy( &m_target_rect_title,
          display_scale_rect_to_screen( 4, 1, 152, 18 ), sizeof( SDL_Rect ) );

  memcpy( &m_target_menu_rect[0],
          display_scale_rect_to_screen( 51, 30, 58, 10 ), sizeof( SDL_Rect ) );
  memcpy( &m_target_menu_deco_rect[0],
          display_scale_rect_to_screen( 50, 29, 60, 12 ), sizeof( SDL_Rect ) );
  memcpy( &m_target_menu_rect[1],
          display_scale_rect_to_screen( 51, 45, 58, 10 ), sizeof( SDL_Rect ) );
  memcpy( &m_target_menu_deco_rect[1],
          display_scale_rect_to_screen( 50, 44, 60, 12 ), sizeof( SDL_Rect ) );
  memcpy( &m_target_menu_rect[2],
          display_scale_rect_to_screen( 51, 60, 58, 10 ), sizeof( SDL_Rect ) );
  memcpy( &m_target_menu_deco_rect[2],
          display_scale_rect_to_screen( 50, 59, 60, 12 ), sizeof( SDL_Rect ) );
  memcpy( &m_target_menu_rect[3],
          display_scale_rect_to_screen( 51, 75, 58, 10 ), sizeof( SDL_Rect ) );
  memcpy( &m_target_menu_deco_rect[3],
          display_scale_rect_to_screen( 50, 74, 60, 12 ), sizeof( SDL_Rect ) );
  memcpy( &m_target_menu_rect[4],
          display_scale_rect_to_screen( 51, 90, 58, 10 ), sizeof( SDL_Rect ) );
  memcpy( &m_target_menu_deco_rect[4],
          display_scale_rect_to_screen( 50, 89, 60, 12 ), sizeof( SDL_Rect ) );

  /* All done! */
  return true;
}


/* Functions. */

/*
 * init - called when the engine is activated, to do any one-time initialising.
 */

void menu_init( void )
{
  uint_fast8_t l_index;

  /* Load up the sprite image (hopefully!) */
  if ( !menu_load_sprites() )
  {
    log_write( ERROR, "Failed to load menu sprites" );
  }

  /* Clear any current command. */
  m_current_cmd = SDLK_UNKNOWN;
  m_current_option = 0;
  m_mouse_moved = false;

  for ( l_index = 0; l_index < TRIX_MENU_ENTRIES; l_index++ )
  {
    m_option_enabled[l_index] = true;
  }
  m_option_enabled[1] = false;
  m_option_enabled[3] = false;

  /* Remember what tick we were initialised at. */
  m_blink_tick = m_last_move_tick = SDL_GetTicks();

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

trix_engine_t menu_update( void )
{
  uint_fast8_t  l_index;
  int_fast8_t   l_new_option;
  uint_fast32_t l_current_tick = SDL_GetTicks();

  /* Blink the cursor on the menu. */
  if ( l_current_tick >= ( m_blink_tick + TRIX_MOVE_MS ) )
  {
    m_menu_blink = !m_menu_blink;
    m_blink_tick = l_current_tick;
  }

  /* Handle any mouse movements. */
  if ( m_mouse_moved || m_mouse_clicked )
  {
    /* Work out which menu it's in... */
    for( l_index = 0; l_index < TRIX_MENU_ENTRIES; l_index++ )
    {
      /* If the mouse location is in the menu button, select it! */
      if ( ( m_option_enabled[l_index] ) && 
           ( SDL_PointInRect( &m_mouse_location, &m_target_menu_rect[l_index] ) ) )
      {
        /* Select the option. */
        m_current_option = l_index;

        /* And if we've clicked, select that option too. */
        if ( m_mouse_clicked )
        {
          m_current_cmd = SDLK_RETURN;
        }
        break;
      }
    }

    /* And clear the mouse flags. */
    m_mouse_moved = m_mouse_clicked = false;
  }

  /* Process the current command. */
  switch( m_current_cmd )
  {
    case SDLK_UP:                                             /* Move up. */
      /* Only attempt the move every TRIX_MOVE_MS milliseconds. */
      if ( l_current_tick > ( m_last_move_tick + TRIX_MOVE_MS ) )
      {
        /* Move around the menu. */
        if ( m_current_option > 0 ) 
        {
          l_new_option = m_current_option - 1;
          while( l_new_option >= 0 )
          {
            if ( m_option_enabled[l_new_option] )
            {
              break;
            }
            l_new_option--;
          }
          if ( l_new_option >= 0 )
          {
            m_current_option = l_new_option;
          }
        }
        m_last_move_tick = l_current_tick;
      }
      break;
    case SDLK_DOWN:                                         /* Move down. */
      /* Only attempt the move every TRIX_MOVE_MS milliseconds. */
      if ( l_current_tick > ( m_last_move_tick + TRIX_MOVE_MS ) )
      {
        /* Move around the menu. */
        if ( m_current_option < TRIX_MENU_ENTRIES-1 ) 
        {
          l_new_option = m_current_option + 1;
          while( l_new_option < TRIX_MENU_ENTRIES )
          {
            if ( m_option_enabled[l_new_option] )
            {
              break;
            }
            l_new_option++;
          }
          if ( l_new_option < TRIX_MENU_ENTRIES )
          {
            m_current_option = l_new_option;
          }
        }
        m_last_move_tick = l_current_tick;
      }
      break;
    case SDLK_RETURN:         /* Activate the currently selection option. */
      switch( m_current_option )
      {
        case 0:   /* New Game. */
          return ENGINE_GAME;
          break;
        case 2:   /* New Game. */
          return ENGINE_HSTABLE;
          break;
        case 4:   /* Exit. */
          return ENGINE_EXIT;
          break;
      }
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
  uint_fast8_t  l_index;

  /* Clear to black. */
  SDL_SetRenderDrawColor( display_get_renderer(), 0, 0, 0, 255 );
  SDL_RenderClear( display_get_renderer() );

  /* Draw the title, centered, top of the screen. */
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, 
                  &m_sprite_rect_title, &m_target_rect_title );

  /* Now a nice glowy box around the currently selected menu item. */
  if ( m_menu_blink )
  {
    SDL_SetRenderDrawColor( display_get_renderer(), 255, 213, 65, 255 );
  }
  else
  {
    SDL_SetRenderDrawColor( display_get_renderer(), 255, 252, 64, 255 );    
  }
  SDL_RenderFillRect( display_get_renderer(), &m_target_menu_deco_rect[m_current_option] );

  /* Work through all the menu entries themselves. */
  for ( l_index = 0; l_index < TRIX_MENU_ENTRIES; l_index++ )
  {
    /* For disabled options, just drop the alpha. */
    if ( !m_option_enabled[l_index] )
    {
      SDL_SetTextureAlphaMod( m_sprite_texture, 150 );
    }
    SDL_RenderCopy( display_get_renderer(), m_sprite_texture,
                    &m_sprite_menu_rect[l_index], &m_target_menu_rect[l_index] );
    if ( !m_option_enabled[l_index] )
    {
      SDL_SetTextureAlphaMod( m_sprite_texture, 255 );
    }
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

void menu_fini( void )
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


/* End of file menu.c */
