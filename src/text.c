/*
 * text.c - part of Tessalatrix
 *
 * Deals with rendering text, from all different engines. Fonts are stored in
 * a dedicated spritesheet, with kernings defined here. This does make it all
 * a bit hand-coded but should mean we get the best results (and don't need to
 * pull in a heavyweight OTF-wrangling library).
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdarg.h>
#include <stdio.h>
#include "SDL.h"
#include "SDL_image.h"


/* Local headers. */

#include "tessalatrix.h"


/* Module variables. */

static uint_fast8_t   m_char_widths[TRIX_TEXT_FONT_LENGTH] = {
  2, 1, 2, 4, 3, 5, 4, 1, 2, 2, 3, 3, 1, 3, 1, 2,          //  !"#$%&'()*+,-./
  3, 2, 3, 3, 4, 3, 3, 3, 3, 3, 1, 1, 4, 3, 4, 3,          // 0123456789:;<=>?
  4, 4, 3, 4, 4, 3, 3, 4, 4, 1, 3, 4, 3, 4, 3, 4,          // @ABCDEFGHIJKLMNO
  3, 4, 4, 3, 4, 4, 4, 5, 3, 3, 4, 2, 2, 2, 3, 3,          // PQRSTUVWXYZ[\]^_
  1, 3, 3, 3, 3, 3, 2, 3, 3, 1, 2, 3, 1, 4, 3, 3,          // `abcdefghijklmno
  3, 3, 3, 3, 3, 4, 3, 4, 3, 3, 3, 2, 1, 2, 3              // pqrstuvwxyz{|}~
};
static SDL_Rect       m_char_src_rect[TRIX_TEXT_FONT_LENGTH];
static SDL_Texture   *m_sprite_texture;
static uint_fast8_t   m_sprite_scale;


/*
 * Static functions; a collection of things only built for use locally.
 */

/*
 * load_sprites - called to (re) load the sprite sheet and calculate the
 *                various source rectangles from it. Always called in init,
 *                but can be recalled any time to deal with a change in
 *                resolution.
 */

static bool text_load_sprites( void )
{
  uint_fast8_t  l_index;
  char          l_sprite_filename[TRIX_PATH_MAX+1];
  SDL_Rect     *l_src_rect;

  /* Load up the sprite image (hopefully!) */
  m_sprite_scale = display_find_asset( TRIX_ASSET_TEXT_SPRITES, l_sprite_filename );
  m_sprite_texture = IMG_LoadTexture( display_get_renderer(), l_sprite_filename );
  if ( m_sprite_texture == NULL )
  {
    log_write( ERROR, "IMG_LoadTexture of %s failed - %s", TRIX_ASSET_TEXT_SPRITES, SDL_GetError() );
    return false;
  }

  /* Now calculate the source rects for our letters. */
  for( l_index = 0; l_index < TRIX_TEXT_FONT_LENGTH; l_index++ )
  {
    /* Our letters are in rows of 32, in 5x5 blocks. */
    l_src_rect = display_scale_rect_to_scale( (l_index%32) * 5, (l_index/32) * 5, 5, 5, m_sprite_scale );

    /* So save this. */
    memcpy( &m_char_src_rect[l_index], l_src_rect, sizeof( SDL_Rect ) );
  }

  /* All done! */
  return true;
}


/* Functions. */

/*
 * init - load up the appropriate fontsheets and generally prepare ourselves.
 *        Can be called repeatedly, for things like resolution switches.
 */

void text_init( void )
{
  /* Load up the sprite image (hopefully!) */
  if ( !text_load_sprites() )
  {
    log_write( ERROR, "Failed to load text sprites" );
  }

  /* All done. */
  return;
}


/*
 * draw - draws the provided printf-style string to the screen, starting at
 *        the provided (logical, not physical) co-ordinates.
 */

void text_draw( uint_fast8_t p_x, uint_fast8_t p_y, const char *p_format, ... )
{
  uint_fast8_t  l_index;
  int           l_msglen;
  char          l_buffer[64];
  va_list       l_args;
  SDL_Rect      l_target_rect;

  /* Attempt to assemble the message into our buffer. */
  va_start( l_args, p_format );
  l_msglen = vsnprintf( l_buffer, 64, p_format, l_args );
  va_end( l_args );

  /* Now work out our starting point in screen co-ordinates. */
  memcpy( &l_target_rect, display_scale_rect_to_screen( p_x, p_y, 5, 5 ), sizeof( SDL_Rect ) );

  /* And work through the buffer blitting one letter at a time. */
  for ( l_index = 0; l_index < l_msglen; l_index++ )
  {
    /* Write out the letter. */
    SDL_RenderCopy( display_get_renderer(), m_sprite_texture,
                    &m_char_src_rect[l_buffer[l_index]-TRIX_TEXT_FONT_START],
                    &l_target_rect );

    /* Move forward an appropriate amount - note we do no wrapping! */
    l_target_rect.x += ( m_char_widths[l_buffer[l_index]-TRIX_TEXT_FONT_START] * display_get_scale() );
  }

  /* All done. */
  return;
}


/*
 * fini - release any allocated resources.
 */

void text_fini( void )
{
  /* Release any loaded textures. */
  if ( m_sprite_texture != NULL )
  {
    SDL_DestroyTexture( m_sprite_texture );
    m_sprite_texture = NULL;
  }

  /* All done. */
  return;
}


/* End of file text.c */
