/*
 * config.c - part of Tessalatrix
 *
 * Configuration handling; this parses the command line and configuration
 * files, to set up all our optional features. Config data is held in module
 * variables, and accessed via appropriate getter functions.
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
#include "version.h"


/* Special wrangling to get optparse in the form we desire. */

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"


/* Module variables. */

static trix_loglevel_t m_log_level;
static char            m_log_fname[TRIX_PATH_MAX+1];


/* Functions. */

bool config_load( int p_argc, char **p_argv )
{
  bool    l_retval = true;
  int     l_opt_active;
  struct  optparse      l_opt_struct;
  struct  optparse_long l_opt_opts[] = {
    {"version", 'v', OPTPARSE_NONE},
    {"help", 'h', OPTPARSE_NONE},
    {"loglevel", 'l', OPTPARSE_REQUIRED},
    {0}
  };

  /* First step, set up all our default values. */
  m_log_level = ERROR;
  strncpy( m_log_fname, "tessalatrix.log", TRIX_PATH_MAX );

  /* Load up any configuration file we can find. */
  /* __RETURN__ */

  /* And lastly, check for command line overrides / commands. */
  optparse_init( &l_opt_struct, p_argv );

  /* And work through them all. */
  while( ( l_retval ) && 
         ( ( l_opt_active = optparse_long( &l_opt_struct, l_opt_opts, NULL ) ) != -1 ) )
  {
    /* Handle each option. */
    switch( l_opt_active )
    {
      /* Display version, and exit. */
      case 'v':
        printf( "Tessalatrix V%d.%d.%03d\n", 
                TRIX_VERSION_MAJOR, TRIX_VERSION_MINOR, TRIX_VERSION_PATCH );
        l_retval = false;
        break;
      /* Set the desired log level. */
      case 'l':
        /* need to parse l_opt_struct.optarg! */
        if ( strcmp( l_opt_struct.optarg, "ALWAYS" ) == 0 )
        {
          m_log_level = ALWAYS;
        }
        else if ( strcmp( l_opt_struct.optarg, "ERROR" ) == 0 )
        {
          m_log_level = ERROR;
        }
        else if ( strcmp( l_opt_struct.optarg, "WARN" ) == 0 )
        {
          m_log_level = WARN;
        }
        else if ( strcmp( l_opt_struct.optarg, "LOG" ) == 0 )
        {
          m_log_level = LOG;
        }
        else if ( strcmp( l_opt_struct.optarg, "TRACE" ) == 0 )
        {
          m_log_level = TRACE;
        }
        else
        {
          printf( "Invalid log level - must be one of ALWAYS, ERROR, WARN, LOG or TRACE\n" );
          l_retval = false;
        }
        break;
      /* Handle any errors. */
      case '?':
        printf( "Tessalatrix error: %s\n", l_opt_struct.errmsg );
        l_retval = false;
        break;
      /* Help, and if we can't figure anything else out. */
      case 'h':
      default:
        printf( "Tessalatrix V%d.%d.%03d\n", 
                TRIX_VERSION_MAJOR, TRIX_VERSION_MINOR, TRIX_VERSION_PATCH );
        printf( "\nUsage: %s [OPTIONS]\nwhere [OPTIONS] is one or more of:\n\n", p_argv[0] );
        printf( "-v, --version      display version number, and exit\n" );
        printf( "-h, --help         display this help text, and exit\n" );
        printf( "-l, --loglevel=LVL sets the desired logging level - must be one of ALWAYS, ERROR, WARN, LOG or TRACE\n\n" );
        l_retval = false;
        break;
    }
  }

  /* Return the calculated return value - false indicates program end. */
  return l_retval;
}


/* End of file config.c */
