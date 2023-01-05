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

  /* All done, return success to the commandline. */
  return 0;
}

/* End of file tessalatrix.c */
