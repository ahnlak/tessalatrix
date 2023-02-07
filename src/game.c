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
#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static SDL_Texture   *m_sprite_texture;
static uint_fast8_t   m_sprite_scale;

static uint_fast32_t  m_last_move_tick;
static uint_fast32_t  m_last_drop_tick;

static uint_fast32_t  m_drop_speed;

static bool           m_dropping;
static SDL_Keycode    m_current_cmd;
static uint_fast16_t  m_score;
static uint_fast16_t  m_lines;

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

  /* Reset our game parameters. */
  m_drop_speed = TRIX_BASE_DROP_MS;
  m_dropping = false;
  m_score = m_lines = 0;
}


/*
 * check_space - a simple boolean flag to show if a given piece / rotation /
 *               location can fit onto the game board.
 */

static bool game_check_space( const trix_piece_st *p_piece, uint_fast8_t p_rotation, 
                              SDL_Point p_location )
{
  uint_fast8_t  l_index;
  SDL_Point     l_block_loc;

  /* Work through all the blocks of the piece. */
  for( l_index = 0; l_index < p_piece->block_count; l_index++ )
  {
    /* Work out the block address. */
    l_block_loc.x = p_location.x + p_piece->blocks[p_rotation][l_index].x;
    l_block_loc.y = p_location.y + p_piece->blocks[p_rotation][l_index].y;

    /* Check that we're not off the board. */
    if ( ( l_block_loc.y >= TRIX_BOARD_HEIGHT ) ||
         ( l_block_loc.x < 0 ) || ( l_block_loc.x >= m_board_width ) )
    {
      return false;
    }

    /* Check to see if the board is occupied, but ignore space above. */
    if ( ( l_block_loc.y >= 0 ) && ( m_board[l_block_loc.x][l_block_loc.y] != PIECE_NONE ) )
    {
      return false;
    }
  }

  /* No clashes, so it's a valid move. */
  return true;
}


/*
 * copy_to_board - adds the specified piece to the game board, if possible.
 */

static bool game_copy_to_board( const trix_piece_st *p_piece, uint_fast8_t p_rotation, 
                                SDL_Point p_location )
{
  uint_fast8_t  l_index;
  SDL_Point     l_block_loc;

  /* Sanity check that we can do this. */
  if ( !game_check_space( p_piece, p_rotation, p_location ) )
  {
    return false;
  }

  /* Good; should be a simple process then. */
  for( l_index = 0; l_index < p_piece->block_count; l_index++ )
  {
    /* Work out the block address. */
    l_block_loc.x = p_location.x + p_piece->blocks[p_rotation][l_index].x;
    l_block_loc.y = p_location.y + p_piece->blocks[p_rotation][l_index].y;

    /* And add it to the board. */
    m_board[l_block_loc.x][l_block_loc.y] = p_piece->piece;
  }

  /* All good then! */
  return true;
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
    log_write( ERROR, "Failed to load game sprites" );
  }

  /* Clear any current command. */
  m_current_cmd = SDLK_UNKNOWN;

  /* Remember what tick we were initialised at. */
  m_last_drop_tick = m_last_move_tick = SDL_GetTicks();

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

trix_engine_t game_update( void )
{
  uint_fast8_t  l_index, l_column, l_row;
  bool          l_line_complete;
  uint_fast32_t l_current_tick = SDL_GetTicks();
  uint_fast8_t  l_new_rotation;
  SDL_Point     l_new_location;

  /* Process any queued key command. */
  switch( m_current_cmd )
  {
    case SDLK_COMMA:                                      /* Move left. */
    case SDLK_LEFT:
      /* Only attempt the move every TRIX_MOVE_MS milliseconds. */
      if ( l_current_tick > ( m_last_move_tick + TRIX_MOVE_MS ) )
      {
        /* Work out the new position. */
        l_new_location.x = m_current_location.x - 1;
        l_new_location.y = m_current_location.y;

        /* Check that we'll fit. */
        if ( game_check_space( &m_current_piece, m_current_rotation, l_new_location ) )
        {
          m_current_location.x = l_new_location.x;
          m_last_move_tick = l_current_tick;
        }
      }
      break;
    case SDLK_SLASH:                                     /* Move right. */
    case SDLK_RIGHT:
      /* Only attempt the move every TRIX_MOVE_MS milliseconds. */
      if ( l_current_tick > ( m_last_move_tick + TRIX_MOVE_MS ) )
      {
        /* Work out the new position. */
        l_new_location.x = m_current_location.x + 1;
        l_new_location.y = m_current_location.y;

        /* Check that we'll fit. */
        if ( game_check_space( &m_current_piece, m_current_rotation, l_new_location ) )
        {
          m_current_location.x = l_new_location.x;
          m_last_move_tick = l_current_tick;
        }
      }
      break;
    case SDLK_PERIOD:                                        /* Rotate. */
    case SDLK_UP:
      /* Only attempt the move every TRIX_MOVE_MS milliseconds. */
      if ( l_current_tick > ( m_last_move_tick + TRIX_MOVE_MS ) )
      {
        /* Work out the new position. */
        l_new_rotation = m_current_rotation >= 3 ? 0 : m_current_rotation+1;

        /* Check that we'll fit. */
        if ( game_check_space( &m_current_piece, l_new_rotation, m_current_location ) )
        {
          m_current_rotation = l_new_rotation;
          m_last_move_tick = l_current_tick;
        }
      }
      break;

    case SDLK_SPACE:                                           /* Drop. */
      m_dropping = true;
      break;
  }

  /* Clear any current command, for the next input. */
  m_current_cmd = SDLK_UNKNOWN;

  /* If it's time to drop the current piece another line, do so. */
  if ( ( l_current_tick >= ( m_last_drop_tick + m_drop_speed ) ) ||
       ( ( m_dropping ) && ( l_current_tick >= ( m_last_drop_tick + TRIX_FALL_MS ) ) ) )
  {
    /* Fairly simple, increment the Y axis and check it worked. */
    l_new_location.x = m_current_location.x;
    l_new_location.y = m_current_location.y + 1;

    /* Check that we'll fit. */
    if ( game_check_space( &m_current_piece, m_current_rotation, l_new_location ) )
    {
      m_current_location.y = l_new_location.y;
      m_last_drop_tick = l_current_tick;
    }
    else
    {
      /* It doesn't fit, so transfer it to the board, and spawn a fresh piece. */
      game_copy_to_board( &m_current_piece, m_current_rotation, m_current_location );
      m_score += m_current_piece.value;
      m_current_piece.piece = PIECE_NONE;

      /* This is probably a good time to check for any completed lines. */
      for ( l_row = TRIX_BOARD_HEIGHT - 1; l_row > 0; l_row-- )
      {
        /* Look for any empty blocks in the line. */
        l_line_complete = true;
        for ( l_column = 0; l_column < m_board_width; l_column++ )
        {
          if ( m_board[l_column][l_row] == PIECE_NONE )
          {
            l_line_complete = false;
            break;
          }
        }

        /* If that line is complete, blank it and drop everything above it down. */
        if ( l_line_complete )
        {
          /* Go over every line. */
          for ( l_index = l_row; l_index > 0; l_index-- )
          {
            for ( l_column = 0; l_column < m_board_width; l_column++ )
            {
              m_board[l_column][l_index] = m_board[l_column][l_index-1];
            }
          }

          /* Blank the top row. */
          for ( l_column = 0; l_column < m_board_width; l_column++ )
          {
            m_board[l_column][0] = PIECE_NONE;
          }

          /* And check the newly dropped line. */
          m_lines++;
          m_score += 10;
          l_row++;
        }
      }
    }
  }

  /* If we don't have a current piece, we should probably pick one. */
  if ( m_current_piece.piece == PIECE_NONE )
  {
    memcpy( &m_current_piece, piece_select( GAME_MODE_STANDARD ), sizeof( trix_piece_st ) );
    m_current_location.x = ( m_board_width / 2 ) - 1;
    m_current_location.y = -1;
    m_current_rotation = rand() % 4;
    m_dropping = false;

    /* Now check to see if that fit; if it didn't, the game is over. */
    if ( !game_check_space( &m_current_piece, m_current_rotation, m_current_location ) )
    {
      return ENGINE_MENU;
    }
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

  /* Scores next; shown to the right of the board. */
  text_draw(  90, 10, "Score:" );
  text_draw( 120, 10, "%05d", m_score );
  text_draw(  90, 17, "Lines:" );
  text_draw( 120, 17, "%d", m_lines );

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
  /* Release any loaded textures. */
  if ( m_sprite_texture != NULL )
  {
    SDL_DestroyTexture( m_sprite_texture );
    m_sprite_texture = NULL;
  }

  /* All done. */
  return;
}


/* End of file game.c */
