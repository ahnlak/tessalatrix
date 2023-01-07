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
#include <stdint.h>


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
  CONF_LOG_LEVEL, CONF_LOG_FILENAME,
  CONF_WINDOW_WIDTH, CONF_WINDOW_HEIGHT,
  CONF_MAX
} trix_config_t;

typedef enum
{
  ENGINE_SPLASH 
} trix_engine_t;


/* Structs. */

typedef struct
{
  bool  persistent;
  bool  type_int;
  bool  type_float;
  bool  type_string;
  union {
    int32_t intnum;
    double  floatnum;
    char    string[TRIX_PATH_MAX+1];
  }     value;
} trix_config_st;

typedef struct {
    uint_fast8_t (*update)(void);
    void         (*render)(void);
} trix_engine_st;


/* Prototypes. */

bool        config_load( int, char ** );
bool        config_save( void );
int32_t     config_get_int( trix_config_t );
double      config_get_float( trix_config_t );
const char *config_get_string( trix_config_t );

bool        display_init( void );
void        display_fini( void );

bool        log_init( void );
bool        log_write( trix_loglevel_t, const char *, ... );

const char *util_app_name( void );
const char *util_app_namever( void );


#endif /* TRIX_TESSALATRIX_H */


/* End of file tessalatrix.h */
