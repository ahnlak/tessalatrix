/*
 * util.c - part of Tessalatrix
 *
 * A collection of utility functions that don't really fit elsewhere.
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>


/* Local headers. */

#include "tessalatrix.h"
#include "version.h"


/* Functions. */

/*
 * util_app_name - returns a pointer to the display name of the game.
 */

const char *util_app_name( void )
{
  /* Trivial hard-coded string via CMake. */
  return TRIX_PROJECT_NAME;
}

/*
 * util_app_namever - returns a pointer to the game name, with version info
 *                    appended in a (hopefully!) consistent manner.
 */

const char *util_app_namever( void )
{
  static char l_buffer[64];

  /* Format it up nicely. */
  snprintf( l_buffer, sizeof( l_buffer ), "%s V%d.%d.%03d", TRIX_PROJECT_NAME, 
            TRIX_VERSION_MAJOR, TRIX_VERSION_MINOR, TRIX_VERSION_PATCH );

  /* And hand it back. */
  return l_buffer;
}


/* End of file util.c */
