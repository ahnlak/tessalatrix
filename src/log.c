/*
 * log.c - part of Tessalatrix
 *
 * Logging functions; anything that needs to be recorded passes through here;
 * command line and configuration options determine what gets saved, and where.
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include <stdbool.h>
#include "SDL.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static FILE        *m_log_fptr;


/* Functions. */

bool log_init( void )
{
  /* All fine. */
  return true;
}


bool log_write( trix_loglevel_t p_level, const char * p_message, ... )
{
  /* All fine. */
  return true;
}


/* End of file log.c */
