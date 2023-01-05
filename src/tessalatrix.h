/*
 * tessalatrix.h - part of Tessalatrix
 *
 * This is the main header file for Tessalatrix; it declares all our structs,
 * consts and enums, as well as function prototypes.
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

#ifndef   TRIX_TESSALATRIX_H
#define   TRIX_TESSALATRIX_H

#include <stdbool.h>

/* Constants. */

#ifdef    PATH_MAX
#define   TRIX_PATH_MAX         PATH_MAX
#else
#define   TRIX_PATH_MAX         256
#endif /* PATH_MAX */


/* Enums. */

typedef enum 
{
  ALWAYS, ERROR, WARN, LOG, TRACE
} trix_loglevel_t;

typedef enum
{
  CONF_LOG_LEVEL
} trix_confitem_t;


/* Structs. */



/* Prototypes. */

bool        config_load( int, char ** );
bool        log_init( void );
bool        log_write( trix_loglevel_t, const char *, ... );


#endif /* TRIX_TESSALATRIX_H */


/* End of file tessalatrix.h */
