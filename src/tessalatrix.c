/*
 * tessalatrix.c - part of Tessalatrix
 *
 * This is the main entry point of the game; you'll find the main() function
 * here. Everything else should be neatly tucked away in their own files!
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include "SDL.h"


/* Local headers. */

#include "tessalatrix.h"
#include "version.h"


/* Functions. */

/*
 * main - the standard entry to the program.
 */

int main( int argc, char **argv )
{
  /* Initialise our configuration. */
  if ( !config_load( argc, argv ) )
  {
    /* False return here just means we want to exit; not really an error. */
    return 0;
  }

  /* Good; we can now safely initialise the log subsystem. */
  if ( !log_init() )
  {
    /* Not great, but no reason to completely give up. Just log and continue. */
    fprintf( stderr, "ALERT! Tessalatrix unable to intialise log subsystem.\n" );
  }
  log_write( ALWAYS, "Tessalatrix V%d.%d.%03d started.\n", 
             TRIX_VERSION_MAJOR, TRIX_VERSION_MINOR, TRIX_VERSION_PATCH );

  /* All done, return success to the commandline. */
  log_write( ALWAYS, "Tessalatrix V%d.%d.%03d terminated.\n", 
             TRIX_VERSION_MAJOR, TRIX_VERSION_MINOR, TRIX_VERSION_PATCH );
  return 0;
}

/* End of file tessalatrix.c */
