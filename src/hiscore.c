/*
 * hiscore.c - part of Tessalatrix
 *
 * Routines for managing the high score tables; this file handles all the work
 * or persisting the scores to a local file.
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

static trix_hiscore_st  m_standard_hiscores[GAME_MODE_MAX][TRIX_HISCORE_COUNT];


/*
 * Static functions; a collection of things only built for use locally.
 */


/* Functions. */

/*
 * read - returns the array of high scores for the requested game mode.
 */

const trix_hiscore_st *hiscore_read( trix_gamemode_t p_mode )
{
  m_standard_hiscores[p_mode][0].score = 1000;
  m_standard_hiscores[p_mode][0].lines = 10;
  m_standard_hiscores[p_mode][0].datestamp = time(NULL);
  strcpy( m_standard_hiscores[p_mode][0].name, "Debug1" );

  /* Return the appropriate hiscore table. */
  return m_standard_hiscores[p_mode];
}


/*
 * save - adds the provided entry to the score table, if it's high enough to
 *        qualify. Returns true if the score was added, false if it was too low,
 *        or if there was an error saving it.
 */

bool hiscore_save( trix_gamemode_t p_mode, uint_fast16_t p_score,
                   uint_fast16_t p_lines, const char *p_name )
{
  return false;
}

/* End of file hiscore.c */
