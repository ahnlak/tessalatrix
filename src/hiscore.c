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
#include <string.h>
#include <time.h>


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static bool             m_initialised = false;
static trix_hiscore_st  m_hiscores[GAME_MODE_MAX][TRIX_HISCORE_COUNT];


/*
 * Static functions; a collection of things only built for use locally.
 */


/* Functions. */

/*
 * read - returns the array of high scores for the requested game mode.
 */

const trix_hiscore_st *hiscore_read( trix_gamemode_t p_mode )
{
  FILE             *l_table_fptr;
  uint_fast8_t      l_index;
  trix_gamemode_t   l_mode;
  trix_hiscore_st   l_hiscore_record;
  char              l_format[32];
  char              l_buffer[TRIX_PATH_MAX+1];


  /* If we haven't already, we need to read our saved file data. */
  if ( !m_initialised )
  {
    /* If we can't open up the file then... we're doomed. */
    l_table_fptr = fopen( TRIX_HISCORE_FILENAME, "r" );
    if ( l_table_fptr != NULL )
    {
      /* Work through it one line at a time. */
      snprintf( l_format, 32, "%%d-%%d:%%ld,%%ld,%%ld,%%%ds", TRIX_NAMELEN_MAX );
      while( !feof( l_table_fptr ) )
      {
        /* Fetch the next line from the table. */
        if ( fgets( l_buffer, TRIX_PATH_MAX, l_table_fptr ) == NULL )
        {
          break;
        }

        /* See if we can find all the data. */
        memset( &l_hiscore_record, 0, sizeof( trix_hiscore_st ) );
        if ( sscanf( l_buffer, l_format, &l_mode, &l_index, 
                     &l_hiscore_record.score,
                     &l_hiscore_record.lines,
                     &l_hiscore_record.datestamp,
                     l_hiscore_record.name  ) != 6 )
        {
          log_write( ERROR, "Bad record in high score table - skipping" );
          continue;
        }

        /* We did, so squirrel it away in the right place. */
        memcpy( &m_hiscores[l_mode][l_index], &l_hiscore_record, sizeof( trix_hiscore_st ) );
      }

      /* All done. */
      fclose( l_table_fptr );
    }
  }

  /* Return the appropriate hiscore table. */
  return m_hiscores[p_mode];
}


/*
 * save - adds the provided entry to the score table, if it's high enough to
 *        qualify. Returns true if the score was added, false if it was too low,
 *        or if there was an error saving it.
 */

bool hiscore_save( trix_gamemode_t p_mode, uint_fast16_t p_score,
                   uint_fast16_t p_lines, const char *p_name )
{
  int_fast8_t   l_index, l_entry, l_mode;
  FILE         *l_table_fptr;

  /* Ensure that we have a high score table loaded! */
  hiscore_read( p_mode );

  /* First step, work out if it's a new entry or not. */
  for ( l_entry = 0; l_entry < TRIX_HISCORE_COUNT; l_entry++ )
  {
    if ( p_score > m_hiscores[p_mode][l_entry].score )
    {
      break;
    }
  }

  /* If we dropped out the bottom, the score is too low. */
  if ( l_entry == TRIX_HISCORE_COUNT )
  {
    return false;
  }

  /* Good stuff; so, shuffle the table down and add our entry. */
  for ( l_index = TRIX_HISCORE_COUNT - 1; l_index > l_entry; l_index-- )
  {
    /* Copy all entry down a slot. */
    memcpy( &m_hiscores[p_mode][l_index], &m_hiscores[p_mode][l_index-1], sizeof( trix_hiscore_st ) );
  }
  m_hiscores[p_mode][l_entry].score = p_score;
  m_hiscores[p_mode][l_entry].lines = p_lines;
  m_hiscores[p_mode][l_entry].datestamp = time(NULL);
  strncpy( m_hiscores[p_mode][l_entry].name, p_name, TRIX_NAMELEN_MAX );
  m_hiscores[p_mode][l_entry].name[TRIX_NAMELEN_MAX] = '\0';

  /* Lastly, write out the whole high score table. */
  l_table_fptr = fopen( TRIX_HISCORE_FILENAME, "w" );
  if ( l_table_fptr == NULL )
  {
    return false;
  }

  /* Work through each mode. */
  for ( l_mode = 0; l_mode < GAME_MODE_MAX; l_mode++ )
  {
    /* Work through each row. */
    for ( l_entry = 0; l_entry < TRIX_HISCORE_COUNT; l_entry++ )
    {
      /* Only bother writing out actual scores, though. */
      if ( m_hiscores[l_mode][l_entry].score > 0 )
      {
        fprintf( l_table_fptr, "%d-%d:%ld,%ld,%ld,%s\n", l_mode, l_entry, 
                 m_hiscores[l_mode][l_entry].score,
                 m_hiscores[l_mode][l_entry].lines,
                 m_hiscores[l_mode][l_entry].datestamp,
                 m_hiscores[l_mode][l_entry].name );
      }
    }
  }
  fclose( l_table_fptr );

  /* All good then! */
  return true;
}

/* End of file hiscore.c */
