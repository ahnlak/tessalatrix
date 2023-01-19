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
#include "SDL.h"


/* Constants. */

#ifdef    PATH_MAX
#define   TRIX_PATH_MAX           PATH_MAX
#else
#define   TRIX_PATH_MAX           256
#endif /* PATH_MAX */

#define   TRIX_FPS_MS             16

/* Asset locations. */
#define   TRIX_ASSET_PATH         "assets"
#define   TRIX_ASSET_SPLASH       "logo-ahnlak-larger"
#define   TRIX_ASSET_MENU_SPRITES "menu-sprites"


/* Enums. */

typedef enum 
{
  ALWAYS, ERROR, WARN, LOG, TRACE
} trix_loglevel_t;

typedef enum
{
  CONF_LOG_LEVEL, CONF_LOG_FILENAME,
  CONF_RESOLUTION,
  CONF_MAX
} trix_config_t;

typedef enum
{
  ENGINE_SPLASH, ENGINE_MENU, ENGINE_EXIT
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
  trix_engine_t type;
  bool          running;
  void          (*init)(void);
  void          (*event)(const SDL_Event *);
  trix_engine_t (*update)(void);
  void          (*render)(void);
  void          (*fini)(void);
} trix_engine_st;

typedef struct {
  int_fast16_t  x;
  int_fast16_t  y;
  int_fast16_t  w;
  int_fast16_t  h;
  uint_fast8_t  scale;
} trix_resolution_st;


/* Prototypes. */

bool          config_load( int, char ** );
bool          config_save( void );
int32_t       config_get_int( trix_config_t );
double        config_get_float( trix_config_t );
const char   *config_get_string( trix_config_t );

bool          display_init( void );
void          display_fini( void );
SDL_Renderer *display_get_renderer( void );
SDL_Point    *display_scale_point( uint_fast8_t, uint_fast8_t );
SDL_Rect     *display_scale_rect_to_screen( uint_fast8_t, uint_fast8_t, uint_fast8_t, uint_fast8_t );
SDL_Rect     *display_scale_rect_to_scale( uint_fast8_t, uint_fast8_t, uint_fast8_t, uint_fast8_t, uint_fast8_t );
uint_fast8_t  display_find_asset( const char *, char * );

bool          log_init( void );
bool          log_write( trix_loglevel_t, const char *, ... );

void          menu_init( void );
void          menu_event(const SDL_Event *);
trix_engine_t menu_update( void );
void          menu_render( void );
void          menu_fini( void );

void          metrics_update( void );
void          metrics_render( void );

void          splash_init( void );
void          splash_event(const SDL_Event *);
trix_engine_t splash_update( void );
void          splash_render( void );
void          splash_fini( void );

const char   *util_app_name( void );
const char   *util_app_namever( void );


#endif /* TRIX_TESSALATRIX_H */


/* End of file tessalatrix.h */
