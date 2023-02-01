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
static uint_fast8_t   m_sprite_scale;

static uint_fast32_t  m_start_tick;

static uint_fast8_t   m_board_width;
static trix_piece_t   m_board[TRIX_BOARD_WIDTH][TRIX_BOARD_HEIGHT];
static trix_piece_st  m_current_piece;
static SDL_Point      m_current_location;
static uint_fast8_t   m_current_rotation;
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

  /* Load up the sprite image (hopefully!) */
  m_sprite_scale = display_find_asset( TRIX_ASSET_GAME_SPRITES, l_sprite_filename );
  m_sprite_texture = IMG_LoadTexture( display_get_renderer(), l_sprite_filename );
  if ( m_sprite_texture == NULL )
  {
    log_write( ERROR, "IMG_LoadTexture of %s failed - %s", TRIX_ASSET_GAME_SPRITES, SDL_GetError() );
    return false;
  }

  /* Now calculate the source rects we'll use for this sheet. */
  memcpy( &m_border_bl_src_rect, 
          display_scale_rect_to_scale( 0, 5, 5, 5, m_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_base_src_rect, 
          display_scale_rect_to_scale( 5, 5, 5, 5, m_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_br_src_rect, 
          display_scale_rect_to_scale( 10, 5, 5, 5, m_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_left_src_rect, 
          display_scale_rect_to_scale( 15, 5, 5, 5, m_sprite_scale ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_right_src_rect, 
          display_scale_rect_to_scale( 20, 5, 5, 5, m_sprite_scale ), 
          sizeof( SDL_Rect ) );

  /* And work out the target corners of the game board, too. */
  memcpy( &m_border_bl_target_rect,
          display_scale_rect_to_screen( 0, 105, 5, 5 ), 
          sizeof( SDL_Rect ) );  
  memcpy( &m_border_br_target_rect,
          display_scale_rect_to_screen( 5 * ( m_board_width+1 ), 105, 5, 5 ), 
          sizeof( SDL_Rect ) );  

  /* All done! */
  return true;
}


/*
 * init_board - called at the start of a game, to initialise the board ready
 *              to be played. This involves selecting the correct board width
 *              for our game type, and making sure it's empty.
 */

static void game_init_board( void )
{
  uint_fast8_t l_row, l_column;

  /* For now, we only understand four-block pieces. */
  m_board_width = 10;

  for ( l_row = 0; l_row < TRIX_BOARD_HEIGHT; l_row++ )
  {
    for ( l_column = 0; l_column < m_board_width; l_column++ )
    {
      m_board[l_column][l_row] = PIECE_NONE;
    }
  }
}


/* Functions. */

/*
 * init - called when the engine is activated, to do any one-time initialising.
 */

void game_init( void )
{
  /* Initialise the board, suitable for the current game mode. */
  game_init_board();

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
  /* If we don't have a current piece, we should probably pick one. */
  if ( m_current_piece.piece == PIECE_NONE )
  {
    memcpy( &m_current_piece, piece_select( GAME_MODE_STANDARD ), sizeof( trix_piece_st ) );
    m_current_location.x = ( m_board_width / 2 ) - 1;
    m_current_location.y = 0;
  }

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
  uint_fast8_t  l_row, l_column;
  SDL_Rect      l_source_block, l_target_block;

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
  for( l_index = 1; l_index <= TRIX_BOARD_HEIGHT; l_index++ )
  {
    SDL_RenderCopy( display_get_renderer(), m_sprite_texture, &m_border_left_src_rect,
                    display_scale_rect_to_screen( 0, 105 - ( 5 * l_index ), 5, 5 ) );
    SDL_RenderCopy( display_get_renderer(), m_sprite_texture, &m_border_right_src_rect,
                    display_scale_rect_to_screen( 5 * ( m_board_width+1 ), 105 - ( 5 * l_index ), 5, 5 ) );
  }

  /* Now, run through the board and render any blocks. */
  for ( l_row = 0; l_row < TRIX_BOARD_HEIGHT; l_row++ )
  {
    for ( l_column = 0; l_column < m_board_width; l_column++ )
    {
      /* Precalculate the destination rectangle. */
      memcpy( &l_target_block, 
              display_scale_rect_to_screen( 5 + ( 5 * l_column ), 5 + ( 5 * l_row ), 5, 5 ),
              sizeof( SDL_Rect ) );

      /* All the four-piece blocks are in a simply addressable row. */
      if ( ( m_board[l_column][l_row] > PIECE_4_MIN ) && 
           ( m_board[l_column][l_row] < PIECE_4_MAX ) )
      {
        SDL_RenderCopy( display_get_renderer(), m_sprite_texture, 
                        display_scale_rect_to_scale( 5 * ( m_board[l_column][l_row] - PIECE_4_MIN - 1 ), 0, 5, 5, m_sprite_scale ), 
                        &l_target_block );
      }
    }
  }

  /* Draw the current piece into it's board location. */
  if ( m_current_piece.piece != PIECE_NONE )
  {
    /* We'll use the same rect for all the blocks of the same piece. */

    /* All the four-piece blocks are in a simply addressable row. */
    if ( ( m_current_piece.piece > PIECE_4_MIN ) && 
         ( m_current_piece.piece < PIECE_4_MAX ) )
    {
      memcpy( &l_source_block,
              display_scale_rect_to_scale( 5 * ( m_current_piece.piece - PIECE_4_MIN - 1 ), 
                                           0, 5, 5, m_sprite_scale ),
              sizeof( SDL_Rect ) );
    }

    /* Work through the defined blocks on the current rotation. */
    for( l_index = 0; l_index < m_current_piece.block_count; l_index++ )
    {
      memcpy( &l_target_block, 
              display_scale_rect_to_screen( 5 + ( 5 * (m_current_location.x+m_current_piece.blocks[m_current_rotation][l_index].x) ), 
                                            5 + ( 5 * (m_current_location.y+m_current_piece.blocks[m_current_rotation][l_index].y) ),
                                            5, 5 ),
              sizeof( SDL_Rect ) );

      SDL_RenderCopy( display_get_renderer(), m_sprite_texture, 
                      &l_source_block, &l_target_block );
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

void game_fini( void )
{
  /* All done. */
  return;
}


/* End of file game.c */
