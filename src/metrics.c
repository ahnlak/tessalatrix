/*
 * metrics.c - part of Tessalatrix
 *
 * Engine for rendering the 'ahnlak' splash branding; about as simple as
 * engines get, we just fade in and then out the logo, while allowing users
 * to anykey out of it.
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include <time.h>
#include "SDL.h"
#include "SDL_image.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static time_t         m_current_second;
static uint_fast8_t   m_current_frames;
static uint_fast8_t   m_last_fps;
static SDL_Texture   *m_sprite_texture = NULL;
static SDL_Rect       m_fps_frame_src_rect;
static SDL_Rect       m_fps_frame_target_rect;


/* Functions. */

/*
 * enable - turn on metric gathering; we'll load up a suitable spritesheet
 *          to allow us to render the FPS, and start counting. 
 */

void metrics_enable( void )
{
  char          l_sprite_filename[TRIX_PATH_MAX+1];
  uint_fast8_t  l_sprite_scale;

  /* Load up the sprite image if we don't already have it. */
  if ( m_sprite_texture == NULL )
  {
    l_sprite_scale = display_find_asset( TRIX_ASSET_METRICS_SPRITES, l_sprite_filename );
    m_sprite_texture = IMG_LoadTexture( display_get_renderer(), l_sprite_filename );
    if ( m_sprite_texture == NULL )
    {
      log_write( ERROR, "IMG_LoadTexture of %s failed - %s", TRIX_ASSET_METRICS_SPRITES, SDL_GetError() );
      return;
    }

    /* Now calculate the source rects we'll use for this sheet. */
    memcpy( &m_fps_frame_src_rect, 
            display_scale_rect_to_scale( 0, 0, 12, 8, l_sprite_scale ), 
            sizeof( SDL_Rect ) );
  }

  /* And lastly the destination rects. */
  memcpy( &m_fps_frame_target_rect, 
          display_scale_rect_to_screen( 0, 102, 12, 8 ),
          sizeof( SDL_Rect ) );

  /* All done. */
  return;
}


/*
 * disaale - turn off metric gathering; stop counting and rendering the FPS
 *           counter.
 */

void metrics_disable( void )
{
  /* All done. */
  return;
}


/*
 * update - called every time we render a frame, to count the number of frames
 *          we manage to render every second.
 */

void metrics_update( void )
{
  time_t  l_this_second = time( NULL );

  /* If a new second has begun, start counting again. */
  if ( l_this_second != m_current_second )
  {
    m_current_second = l_this_second;
    m_last_fps = m_current_frames;
    m_current_frames = 0;
  }

  /* And then just increment our fps counter. */
  m_current_frames++;

  /* All done. */
  return;
}


/*
 * render - draws the current fps in the corner of the screen; should be the
 *          last thing called by an engine's renderer, ideally.
 */

void metrics_render( void )
{
  /* Render the splash image, stretched if we need to. */
  SDL_RenderCopy( display_get_renderer(), m_sprite_texture, 
                  &m_fps_frame_src_rect, &m_fps_frame_target_rect );

  /* All done. */
  return;
}

/* End of file metrics.c */
