/*
 * tessalatrix.c - part of Tessalatrix
 *
 * This is the main entry point of the game; you'll find the main() function
 * here. Everything else should be neatly tucked away in their own files!
 *
 * Copyright (c) 2023 Pete Favelle <ahnlak@ahnlak.com>
 *
 * This file is released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include "SDL.h"


/* Local headers. */

#include "tessalatrix.h"


/* Functions. */

/*
 * main - the standard entry to the program.
 */

int main( int argc, char **argv )
{
  bool            l_running = true;
  SDL_Event       l_event;
  trix_engine_t   l_target_engine;
  trix_engine_st  l_current_engine;

  /* Initialise our configuration. */
  if ( !config_load( argc, argv ) )
  {
    /* False return here just means we want to exit; not really an error. */
    return 0;
  }

  /* Good; we can now safely initialise the log subsystem. */
  if ( !log_init() )
  {
    /* Not great, but no reason to completely give up. Just log and continue. */
    fprintf( stderr, "ALERT! Tessalatrix unable to intialise log subsystem.\n" );
  }
  log_write( ALWAYS, "%s started.", util_app_namever() );

  /* Set up the initial engine. */
  l_current_engine.type = ENGINE_SPLASH;
  l_current_engine.init = splash_init;
  l_current_engine.update = splash_update;
  l_current_engine.render = splash_render;
  l_current_engine.fini = splash_fini;
  l_current_engine.init();

  /* Set up the display. */
  if ( display_init() )
  {
    /* Dive into the main logic loop, until it exists. */
    while( l_running )
    {
      /* So, work through any queued up events. */
      while( SDL_PollEvent( &l_event ) != 0 )
      {
        /* Handle the system-level events. */
        if ( l_event.type == SDL_QUIT )
        {
          l_running = false;
          break;
        }
      }

      /* If we've quit, we probably don't need to do any more! */
      if ( !l_running )
      {
        break;
      }

      /* Ask the current engine to update. */
      l_target_engine = l_current_engine.update();

      /* If the engine has requested a switch, do so and move straight on. */
      if ( l_target_engine != l_current_engine.type )
      {
        /* Tell the current engine to shut down. */
        l_current_engine.fini();

        /* Set up the current engine structure to point to the target. */
        switch( l_target_engine )
        {
          case ENGINE_EXIT:       /* We want to exit the game now. */
          default:
            l_running = false;
            break;
        }

        /* And move straight on with the next loop. */
        continue;
      }

      /* And then to render itself (if we have time) */
      l_current_engine.render();

      /* Wait for the next tick, if we need to. */
    }

    /* Lastly, tear down the display. */
    display_fini();
  }
  else /* display_init() failed */
  {
    /* Log it and never bother entering the main game loop. */
    log_write( ERROR, "Failed to initialise display!" );
  }

  /* All done, return success to the commandline. */
  log_write( ALWAYS, "%s terminated.", util_app_namever() );
  return 0;
}

/* End of file tessalatrix.c */
