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
      { {0,0}, {0,1}, {1,0}, {1,1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { {0,0}, {0,1}, {1,0}, {1,1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { {0,0}, {0,1}, {1,0}, {1,1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} },
      { {0,0}, {0,1}, {1,0}, {1,1}, {UINT_FAST8_MAX,UINT_FAST8_MAX} }
    }
  }
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
  /* For texting, do this easily. */
  return &m_pieces[0];
}

/* End of file piece.c */
