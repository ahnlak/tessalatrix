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

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "SDL.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static FILE        *m_log_fptr;


/* Functions. */

/*
 * init - initialises the logging subsystem, using the configured log file
 *        and falling back to stdout.
 */

bool log_init( void )
{
  /* If the log filename is set to 'stdout', just use, well, stdout. */
  if ( strcmp( config_get_string( CONF_LOG_FILENAME ), "stdout" ) == 0 )
  {
    m_log_fptr = stdout;
  }
  else
  {
    /* Otherwise, try to open the requested file up, in append mode. */
    m_log_fptr = fopen( config_get_string( CONF_LOG_FILENAME ), "a" );
    if ( m_log_fptr == NULL )
    {
      /* Couldn't open the file; fallback to stdout, but flag it as a fail. */
      m_log_fptr = stdout;
      return false;
    }
  }

  /* All fine. */
  return true;
}


/*
 * write - if the current log level is at or above the requested level, write
 *         the message to the logfile, date and timestamp prefixed.
 *         Returns false if an error is encountered.
 */

bool log_write( trix_loglevel_t p_level, const char * p_message, ... )
{
  char        l_buffer[32];
  time_t      l_time;
  struct tm  *l_localtime;
  va_list     l_args;

  /* If the message log level is *above* the system's level, do nothing. */
  if ( p_level > (trix_loglevel_t)config_get_int( CONF_LOG_LEVEL ) )
  {
    return true;
  }

  /* Sanity check that we have a valid fileptr; abort if not. */
  if ( m_log_fptr == NULL )
  {
    return false;
  }

  /* Try and timestamp entries (but failure shouldn't stop us) */
  l_time = time( NULL );
  l_localtime = localtime( &l_time );
  if ( l_localtime != NULL )
  {
    if ( strftime( l_buffer, 32, "%Y/%m/%d %H:%M:%S ", l_localtime ) > 0 )
    {
      fputs( l_buffer, m_log_fptr );
    }
  }
  
  /* Good; simply output the message then, wrangling the varargs stuff. */
  va_start( l_args, p_message );
  vfprintf( m_log_fptr, p_message, l_args );
  if ( p_message[strlen(p_message)-1] != '\n' )
  {
    /* Add a newline if we didn't get passed one. */
    fputc( '\n', m_log_fptr );
  }
  va_end( l_args );

  /* All fine. */
  return true;
}


/* End of file log.c */
