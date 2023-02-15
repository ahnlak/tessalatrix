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
#define   TRIX_PATH_MAX               PATH_MAX
#else
#define   TRIX_PATH_MAX               256
#endif /* PATH_MAX */

#define   TRIX_FPS_MS                 16
#define   TRIX_BOARD_HEIGHT           20
#define   TRIX_BOARD_WIDTH            15

#define   TRIX_MOVE_MS                75
#define   TRIX_FALL_MS                25
#define   TRIX_BASE_DROP_MS           250

#define   TRIX_MENU_ENTRIES           5

#define   TRIX_TEXT_FONT_START        32
#define   TRIX_TEXT_FONT_LENGTH       95
#define   TRIX_NAMELEN_MAX            32
#define   TRIX_HISCORE_COUNT          10


/* Asset locations. */

#define   TRIX_ASSET_PATH             "assets"
#define   TRIX_ASSET_SPLASH           "logo-ahnlak-larger"
#define   TRIX_ASSET_METRICS_SPRITES  "metrics-sprites"
#define   TRIX_ASSET_MENU_SPRITES     "menu-sprites"
#define   TRIX_ASSET_GAME_SPRITES     "game-sprites"
#define   TRIX_ASSET_TEXT_SPRITES     "text-sprites"
#define   TRIX_ASSET_HST_SPRITES      "hst-sprites"


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
  ENGINE_SPLASH, ENGINE_MENU, ENGINE_HSTABLE, ENGINE_GAME, ENGINE_OVER, ENGINE_EXIT
} trix_engine_t;

typedef enum
{
  GAME_MODE_STANDARD, GAME_MODE_MAX
} trix_gamemode_t;

typedef enum
{
  PIECE_NONE, 
  PIECE_4_MIN, PIECE_4_SQUARE, PIECE_4_LONG, PIECE_4_ELL, 
  PIECE_4_BELL, PIECE_4_TEE, PIECE_4_ESS, PIECE_4_BESS, PIECE_4_MAX,
  PIECE_MAX
} trix_piece_t;


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

typedef struct {
  trix_piece_t  piece;
  uint_fast8_t  value;
  uint_fast8_t  block_count;
  SDL_Point     blocks[4][5];
} trix_piece_st;

typedef struct {
  uint_fast16_t score;
  uint_fast16_t lines;
  time_t        datestamp;
  char          name[TRIX_NAMELEN_MAX+1];
} trix_hiscore_st;


/* Prototypes. */

bool          config_load( int, char ** );
bool          config_save( void );
int32_t       config_get_int( trix_config_t );
double        config_get_float( trix_config_t );
const char   *config_get_string( trix_config_t );

bool          display_init( void );
void          display_fini( void );
SDL_Renderer *display_get_renderer( void );
uint_fast8_t  display_get_scale( void );
SDL_Point    *display_scale_point( uint_fast8_t, uint_fast8_t );
SDL_Rect     *display_scale_rect_to_screen( uint_fast8_t, uint_fast8_t, uint_fast8_t, uint_fast8_t );
SDL_Rect     *display_scale_rect_to_scale( uint_fast8_t, uint_fast8_t, uint_fast8_t, uint_fast8_t, uint_fast8_t );
uint_fast8_t  display_find_asset( const char *, char * );

void          game_init( void );
void          game_event( const SDL_Event * );
trix_engine_t game_update( void );
void          game_render( void );
void          game_fini( void );

const trix_hiscore_st *hiscore_read( trix_gamemode_t );
bool                   hiscore_save( trix_gamemode_t, uint_fast16_t, uint_fast16_t, const char * );

void          hstable_init( void );
void          hstable_event( const SDL_Event * );
trix_engine_t hstable_update( void );
void          hstable_render( void );
void          hstable_fini( void );

bool          log_init( void );
bool          log_write( trix_loglevel_t, const char *, ... );

void          menu_init( void );
void          menu_event( const SDL_Event * );
trix_engine_t menu_update( void );
void          menu_render( void );
void          menu_fini( void );

void          metrics_enable( void );
void          metrics_disable( void );
void          metrics_toggle( void );
void          metrics_update( void );
void          metrics_render( void );

const trix_piece_st *piece_select( trix_gamemode_t );

void          splash_init( void );
void          splash_event( const SDL_Event * );
trix_engine_t splash_update( void );
void          splash_render( void );
void          splash_fini( void );

void          text_init( void );
void          text_draw( uint_fast8_t, uint_fast8_t, const char *, ... );
void          text_fini( void );

const char   *util_app_name( void );
const char   *util_app_namever( void );


#endif /* TRIX_TESSALATRIX_H */


/* End of file tessalatrix.h */
