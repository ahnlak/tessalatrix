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


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static time_t       m_current_second;
static uint_fast8_t m_current_frames;
static uint_fast8_t m_last_fps;


/* Functions. */

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
  /* All done. */
  return;
}

/* End of file metrics.c */
