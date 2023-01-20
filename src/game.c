/*
 * game.c - part of Tessalatrix
 *
 * The actual game; if blocks are falling, we're in here :-)
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

static uint_fast8_t   m_board_width;

static SDL_Rect       m_sprite_rect_title;
static SDL_Rect       m_border_bl_src_rect;
static SDL_Rect       m_border_base_src_rect;
static SDL_Rect       m_border_br_src_rect;
static SDL_Rect       m_border_left_src_rect;
static SDL_Rect       m_border_right_src_rect;

static SDL_Rect       m_border_bl_target_rect;
static SDL_Rect       m_border_br_target_rect;


/*
 * Static functions; a collection of things only built for use locally.
 */

/*
 * load_sprites - called to (re) load the sprite sheet and calculate the
 *                various source rectangles from it. Always called in init,
 *                but can be recalled any time to deal with a change in
 *                resolution.
 */

static bool game_load_sprites( void )
{
  char          l_sprite_filename[TRIX_PATH_MAX+1];
  uint_fast8_t  l_sprite_scale;

  /* Load up the sprite image (hopefully!) */
  l_sprite_scale = display_find_asset( TRIX_ASSET_GAME_SPRITES, l_sprite_filename );
  m_sprite_texture = IMG_LoadTexture( display_get_renderer(), l_sprite_filename );
  if ( m_sprite_texture == NULL )
  {
    log_write( ERROR, "IMG_LoadTexture of %s failed - %s", TRIX_ASSET_GAME_SPRITES, SDL_GetError() );
    return false;
  }

  /* Now calculate the source rects we'll use for this sheet. */
  memcpy( &m_border_bl_src_rect, 
          display_scale_rect_to_scale( 0, 5, 5, 5, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_base_src_rect, 
          display_scale_rect_to_scale( 5, 5, 5, 5, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_br_src_rect, 
          display_scale_rect_to_scale( 10, 5, 5, 5, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_left_src_rect, 
          display_scale_rect_to_scale( 15, 5, 5, 5, l_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_right_src_rect, 
          display_scale_rect_to_scale( 20, 5, 5, 5, l_sprite_scale ), 
          sizeof( SDL_Rect ) );

  /* And work out the target corners of the game board, too. */
  memcpy( &m_border_bl_target_rect,
          display_scale_rect_to_screen( 0, 105, 5, 5 ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_br_target_rect,
          display_scale_rect_to_screen( 5 * ( m_board_width+1 ), 105, 5, 5 ), 
          sizeof( SDL_Rect ) );  

printf( "left is %d, %d, %dx%d\n", m_border_left_src_rect.x, m_border_left_src_rect.y, m_border_left_src_rect.w, m_border_left_src_rect.h );
printf( "right is %d, %d, %dx%d\n", m_border_right_src_rect.x, m_border_right_src_rect.y, m_border_right_src_rect.w, m_border_right_src_rect.h );
  /* All done! */
  return true;
}


/* Functions. */

/*
 * init - called when the engine is activated, to do any one-time initialising.
 */

void game_init( void )
{
  /* Initialise the board, suitable for the current game mode. */
  m_board_width = 10;

  /* Load up the sprite image (hopefully!) */
  if ( !game_load_sprites() )
  {
    log_write( ERROR, "Failed to load menu sprites" );
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

void game_event( const SDL_Event *p_event )
{
  /* All done. */
  return;
}


/*
 * update - update the internal state of the engine; this is also where we
 *          handle and process any user input, but no drawing is done here.
 */

trix_engine_t game_update( void )
{
  /* By default, ask to stay in our current engine. */
  return ENGINE_GAME;
}


/*
 * render - draws the internal state of the engine onto the screen; no logic
 *          should be here, it's all the presentational stuff.
 */

void game_render( void )
{
  uint_fast8_t  l_index;

  /* Clear to black. */
  SDL_SetRenderDrawColor( display_get_renderer(), 0, 0, 0, 255 );
  SDL_RenderClear( display_get_renderer() );

  /* Draw the board frame - corners first. */
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, &m_border_bl_src_rect, &m_border_bl_target_rect );
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, &m_border_br_src_rect, &m_border_br_target_rect );

  /* And the bottom line next. */
  for( l_index = 1; l_index <= m_board_width; l_index++ )
  {
    SDL_RenderCopy( display_get_renderer(), m_sprite_texture, &m_border_base_src_rect,
                    display_scale_rect_to_screen( 5 * l_index, 105, 5, 5 ) );
  }

  /* Lastly the walls. */
  for( l_index = 1; l_index < TRIX_BOARD_HEIGHT; l_index++ )
  {
    SDL_RenderCopy( display_get_renderer(), m_sprite_texture, &m_border_left_src_rect,
                    display_scale_rect_to_screen( 0, 105 - ( 5 * l_index ), 5, 5 ) );
    SDL_RenderCopy( display_get_renderer(), m_sprite_texture, &m_border_right_src_rect,
                    display_scale_rect_to_screen( 5 * ( m_board_width+1 ), 105 - ( 5 * l_index ), 5, 5 ) );
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

void game_fini( void )
{
  /* All done. */
  return;
}


/* End of file game.c */
