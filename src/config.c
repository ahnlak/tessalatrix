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
#include <stdlib.h>
#include <string.h>
#include "SDL.h"


/* Local headers. */

#include "tessalatrix.h"


/* Special wrangling to get optparse in the form we desire. */

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"


/* Module variables. */

static trix_config_st  m_config[CONF_MAX];


/*
 * Static functions; a collection of things only built for use locally.
 */

/*
 * config_set_* - a set of static convenience functions for filling in the given
 *                configuration item definition.
 */
static void config_set_int( trix_config_t p_item, int32_t p_intnum, bool p_persistent )
{
  /* Make sure the entry is blank. */
  memset( &m_config[p_item], 0, sizeof( trix_config_st ) );

  /* Set the persistent flag. */
  m_config[p_item].persistent = p_persistent;

  /* Set the type flag. */
  m_config[p_item].type_int = true;

  /* And copy the data value. */
  m_config[p_item].value.intnum = p_intnum;
  
  /* All done. */
  return;
}
static void config_set_float( trix_config_t p_item, double p_floatnum, bool p_persistent )
{
  /* Make sure the entry is blank. */
  memset( &m_config[p_item], 0, sizeof( trix_config_st ) );

  /* Set the persistent flag. */
  m_config[p_item].persistent = p_persistent;

  /* Set the type flag. */
  m_config[p_item].type_float = true;

  /* And copy the data value. */
  m_config[p_item].value.floatnum = p_floatnum;
  
  /* All done. */
  return;
}
static void config_set_string( trix_config_t p_item, const char *p_string, bool p_persistent )
{
  /* Make sure the entry is blank. */
  memset( &m_config[p_item], 0, sizeof( trix_config_st ) );

  /* Set the persistent flag. */
  m_config[p_item].persistent = p_persistent;

  /* Set the type flag. */
  m_config[p_item].type_string = true;

  /* And copy the data value. */
  strncpy( m_config[p_item].value.string, p_string, TRIX_PATH_MAX );
  m_config[p_item].value.string[TRIX_PATH_MAX] = '\0';

  /* All done. */
  return;
}


/*
 * config_save - writes out the entire configuration file, for all items which
 *               are flagged as persistent.
 */

static bool config_save( void )
{
  uint_fast8_t    l_index;
  FILE           *l_fileptr;

  /* This is a relatively simple operation, then! */
  l_fileptr = fopen( TRIX_CONFIG_FILENAME, "w" );
  if ( l_fileptr == NULL )
  {
    return false;
  }

  /* Work through the config array; only write out persistent ones. */
  for ( l_index = 0; l_index < CONF_MAX; l_index++ )
  {
    if ( m_config[l_index].persistent )
    {
      if ( m_config[l_index].type_int )
      {
        fprintf( l_fileptr, "%d:int:%d\n", l_index, m_config[l_index].value.intnum );
      }
      if ( m_config[l_index].type_float )
      {
        fprintf( l_fileptr, "%d:float:%f\n", l_index, m_config[l_index].value.floatnum );
      }
      if ( m_config[l_index].type_string )
      {
        fprintf( l_fileptr, "%d:string:%s\n", l_index, m_config[l_index].value.string );
      }      
    }
  }

  /* All done! */
  fclose( l_fileptr );
  return true;
}


/*
 * config_fetch - fetches any configuration stored in our config file.
 *                entries are skipped, and do not interrupt parsing.
 */

static void config_fetch( void )
{
  char            l_buffer[TRIX_PATH_MAX+1];
  int_fast8_t     l_item;
  FILE           *l_fileptr;
  char           *l_charptr;

  /* This is a relatively simple operation, then! */
  l_fileptr = fopen( TRIX_CONFIG_FILENAME, "r" );
  if ( l_fileptr == NULL )
  {
    return;
  }

  /* Work through the file one line at a time */
  while ( !feof( l_fileptr ) )
  {
    /* Fetch the next line. */
    if ( fgets( l_buffer, TRIX_PATH_MAX, l_fileptr ) == NULL )
    {
      break;
    }

    /* Try to extract the item number, from the start of the line. */
    l_charptr = strtok( l_buffer, ":" );
    if ( l_charptr == NULL )
    {
      continue;
    }
    l_item = atoi( l_charptr );
    if ( ( l_item == 0 ) || ( l_item >= CONF_MAX ) )
    {
      continue;
    }

    /* Good good. Next token is the type. */
    l_charptr = strtok( NULL, ":" );
    if ( l_charptr == NULL )
    {
      continue;
    }

    /* The rest of the processing depends, a bit, on the type. */
    if ( strcmp( l_charptr, "int" ) == 0 )
    {
      l_charptr = strtok( NULL, "\n" );
      m_config[l_item].persistent = true;
      m_config[l_item].type_int = true;
      m_config[l_item].type_float = m_config[l_item].type_string = false;
      m_config[l_item].value.intnum = atoi( l_charptr );
    }
    if ( strcmp( l_charptr, "float" ) == 0 )
    {
      l_charptr = strtok( NULL, "\n" );
      m_config[l_item].persistent = true;
      m_config[l_item].type_float = true;
      m_config[l_item].type_int = m_config[l_item].type_string = false;
      m_config[l_item].value.floatnum = atof( l_charptr );
    }
    if ( strcmp( l_charptr, "string" ) == 0 )
    {
      l_charptr = strtok( NULL, "\n" );
      m_config[l_item].persistent = true;
      m_config[l_item].type_string = true;
      m_config[l_item].type_int = m_config[l_item].type_float = false;
      strncpy( m_config[l_item].value.string, l_charptr, TRIX_PATH_MAX );
      m_config[l_item].value.string[TRIX_PATH_MAX] = '\0';
    }
  }

  /* All done! */
  fclose( l_fileptr );
  return;
}


/*
 * Public functions; declared in tessalatrix.h and used throughout the game.
 */

/*
 * config_load - creates our internal configuration structures, based on
 *               both the configuration file (if present) and the command line.
 */

bool config_load( int p_argc, char **p_argv )
{
  bool    l_retval = true;
  int     l_opt_active;
  struct  optparse      l_opt_struct;
  struct  optparse_long l_opt_opts[] = {
    {"version",  'v', OPTPARSE_NONE},
    {"help",     'h', OPTPARSE_NONE},
    {"loglevel", 'l', OPTPARSE_REQUIRED},
    {0}
  };

  /* First step, set up all our default values. */
  config_set_int( CONF_LOG_LEVEL, ERROR, false );
  config_set_string( CONF_LOG_FILENAME, "tessalatrix.log", false );
  config_set_int( CONF_RESOLUTION, 0, true );
  config_set_string( CONF_PLAYERNAME, "Player1", true );

  /* Load up any configuration file we can find. */
  config_fetch();

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
        printf( "%s\n", util_app_namever() );
        l_retval = false;
        break;
      /* Set the desired log level. */
      case 'l':
        /* need to parse l_opt_struct.optarg! */
        if ( strcmp( l_opt_struct.optarg, "ALWAYS" ) == 0 )
        {
          config_set_int( CONF_LOG_LEVEL, ALWAYS, false );
        }
        else if ( strcmp( l_opt_struct.optarg, "ERROR" ) == 0 )
        {
          config_set_int( CONF_LOG_LEVEL, ERROR, false );
        }
        else if ( strcmp( l_opt_struct.optarg, "WARN" ) == 0 )
        {
          config_set_int( CONF_LOG_LEVEL, WARN, false );
        }
        else if ( strcmp( l_opt_struct.optarg, "LOG" ) == 0 )
        {
          config_set_int( CONF_LOG_LEVEL, LOG, false );
        }
        else if ( strcmp( l_opt_struct.optarg, "TRACE" ) == 0 )
        {
          config_set_int( CONF_LOG_LEVEL, TRACE, false );
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
        printf( "%s\n", util_app_namever() );
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


/*
 * config_get_* - fetches the specific configuration entry; if the entry does
 *                not have the correct type set then zero (or null) is returned.
 */

int32_t config_get_int( trix_config_t p_item )
{
  /* If the type matches, return the value. */
  if ( m_config[p_item].type_int )
  {
    return m_config[p_item].value.intnum;
  }

  /* Default to zero on failure. */
  return 0;
}
double config_get_float( trix_config_t p_item )
{
  /* If the type matches, return the value. */
  if ( m_config[p_item].type_float )
  {
    return m_config[p_item].value.floatnum;
  }

  /* Default to zero on failure. */
  return 0;
}
const char *config_get_string( trix_config_t p_item )
{
  /* If the type matches, return the value. */
  if ( m_config[p_item].type_string )
  {
    return m_config[p_item].value.string;
  }

  /* Default to null on failure. */
  return NULL;
}


/*
 * config_save_* - saves the specific configuration entry; these are used for
 *                 user-configurable settings, and will trigger the re-writing
 *                 of the configuration file every time.
 */

bool config_save_string( trix_config_t p_item, const char *p_value )
{
  /* Copy the new value in then. */
  m_config[p_item].type_int = m_config[p_item].type_float = false;
  m_config[p_item].type_string = true;
  m_config[p_item].persistent = true;

  strncpy( m_config[p_item].value.string, p_value, TRIX_PATH_MAX );
  m_config[p_item].value.string[TRIX_PATH_MAX] = '\0';

  /* Then just write it out. */
  return config_save();
}


/* End of file config.c */
