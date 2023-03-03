/*
 * piece.c - part of Tessalatrix
 *
 * Definitions of all the actual pieces, along with support functions to select
 * them easily.
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdint.h>
#include <stdio.h>


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static trix_piece_st  m_pieces[] = {
  { .piece = PIECE_4_SQUARE, .value = 4, .block_count = 4,
    .blocks = { 
      { { 0, 0}, { 0, 1}, { 1, 0}, { 1, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 0, 1}, { 1, 0}, { 1, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 0, 1}, { 1, 0}, { 1, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 0, 1}, { 1, 0}, { 1, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} }
    }
  },
  { .piece = PIECE_4_LONG, .value = 2, .block_count = 4,
    .blocks = { 
      { { 0, 0}, { 1, 0}, { 2, 0}, { 3, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 1,-1}, { 1, 0}, { 1, 1}, { 1, 2}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 1, 0}, { 2, 0}, { 3, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 1,-1}, { 1, 0}, { 1, 1}, { 1, 2}, {UINT_FAST8_MAX,UINT_FAST8_MAX} }
    }
  },
  { .piece = PIECE_4_ELL, .value = 3, .block_count = 4,
    .blocks = { 
      { { 0, 0}, { 1, 0}, { 2, 0}, { 2, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 1}, { 1,-1}, { 1, 0}, { 1, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0,-1}, { 0, 0}, { 1, 0}, { 2, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 1,-1}, { 1, 0}, { 1, 1}, { 2,-1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} }
    }
  },
  { .piece = PIECE_4_BELL, .value = 3, .block_count = 4,
    .blocks = { 
      { { 0, 0}, { 1, 0}, { 2,-1}, { 2, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 1,-1}, { 1, 0}, { 1, 1}, { 2, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 0, 1}, { 1, 0}, { 2, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0,-1}, { 1,-1}, { 1, 0}, { 1, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} }
    }
  },
  { .piece = PIECE_4_TEE, .value = 1, .block_count = 4,
    .blocks = { 
      { { 1,-1}, { 1, 0}, { 1, 1}, { 2, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 1, 0}, { 1, 1}, { 2, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 1,-1}, { 1, 0}, { 1, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 1,-1}, { 1, 0}, { 2, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} }
    }
  },
  { .piece = PIECE_4_ESS, .value = 5, .block_count = 4,
    .blocks = { 
      { { 0, 0}, { 0, 1}, { 1,-1}, { 1, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0,-1}, { 1,-1}, { 1, 0}, { 2, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 0, 1}, { 1,-1}, { 1, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0,-1}, { 1,-1}, { 1, 0}, { 2, 0}, {UINT_FAST8_MAX,UINT_FAST8_MAX} }
    }
  },
  { .piece = PIECE_4_BESS, .value = 5, .block_count = 4,
    .blocks = { 
      { { 0,-1}, { 0, 0}, { 1, 0}, { 1, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 1,-1}, { 1, 0}, { 2,-1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0,-1}, { 0, 0}, { 1, 0}, { 1, 1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { { 0, 0}, { 1,-1}, { 1, 0}, { 2,-1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} }
    }
  },
};


/*
 * Static functions; a collection of things only built for use locally.
 */


/* Functions. */

/*
 * select - picks a suitable next piece, based on the specified game mode.
 *          Note that this returns a const pointer to our internal piece list.
 */

const trix_piece_st *piece_select( trix_gamemode_t p_mode )
{
  uint_fast8_t  l_chosen_piece;
  bool          l_good_pick = false;

  /* Keep picking random pieces until one aligns with the game mode. Yes,   */
  /* this is potentially inefficient but it's a rare event so roll with it. */
  do
  {
    /* Pick piece. */
    l_chosen_piece = rand() % ( sizeof( m_pieces ) / sizeof( m_pieces[0] ) );

    /* If it's valid for the game mode, stick with it. */
    switch( p_mode )
    {
      case GAME_MODE_STANDARD:
        l_good_pick = true;
        break;
      default:
        log_write( ERROR, "Invalid game mode in piece_select()" );
        break;
    }
  }
  while( !l_good_pick );

  /* All done! */
  return &m_pieces[l_chosen_piece];
}

/* End of file piece.c */
