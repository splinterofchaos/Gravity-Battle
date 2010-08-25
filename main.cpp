
#include <iostream>
#include <fstream>

#include "math/Vector.h"
#include "ScopeGuard.h"
#include "functional_plus.h"

#include "Actor.h"
#include "Playfield.h"
#include "Gunman.h"

#include "Collision.h"

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <algorithm> // For for_each().
#include <functional> // For mem_fun_ptr.

GLenum init_gl( int w, int h )
{
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, w, h, 0, -1, 1 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    return glGetError();
}

bool make_sdl_gl_window( int w, int h )
{
    if( ! SDL_SetVideoMode(w, h, 32, SDL_OPENGL) )
        return false;
    init_gl( w, h );
    return true;
}

void update_screen()
{
    SDL_GL_SwapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

int main( int argc, char** argv )
{
    // Portably suppresses unused variable compiler warnings.
    #define NOT_USED(var) ((void)(var))

    const int MAX_FRAME_TIME = 10;

    bool quit = false;

    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
        return 1;
    make_sdl_gl_window( 700, 600 );

#ifdef POSIX
    // This works best on Linux, but seems to fail on Windows. TODO: WHY!?
    ScopeGuard quitSdl = scope_guard( SDL_Quit ); NOT_USED( quitSdl ); 
    ScopeGuard flushGl = scope_guard( glFlush ); NOT_USED( flushGl );
#endif

    Actor::vector_type pos( 350, 300 );
    Playfield& playfield = *(new Playfield( pos, 200, 40 ));

    pos.x( 500 );
    Gunman* gunny = new Gunman (
        pos, playfield, 
        new HumanGunmanController
    );

    int frameStart=SDL_GetTicks(), frameEnd=frameStart, frameTime=0;
    while( quit == false )
    {
        static SDL_Event event;
		while( SDL_PollEvent(&event) )
		{
			if( event.type == SDL_QUIT )
                quit = true;
		}

        Uint8* keyState = SDL_GetKeyState( 0 );
        if( keyState[SDLK_ESCAPE] )
            quit = true;

        // TODO: Fixed time-step. 
        // Note that for_each_ptr is needed here. std::mem_fn should work, but
        // for whatever reason it's not. TODO: fix it?
        for_each_ptr ( 
            Actor::actors.begin(), Actor::actors.end(), 
            std::bind2nd( std::mem_fun_ref(&Actor::move), frameTime )
        );

        for( Actor::ActorList::iterator it1 = Actor::actors.begin();
             it1 != Actor::actors.end();
             it1++ )
        {
            for( Actor::ActorList::iterator it2 = it1+1;
                 it2 != Actor::actors.end();
                 it2++ )
            {
                if( collision((**it1).collision_data(), (**it2).collision_data()) )
                {
                    (**it1).collide( **it2 );
                    (**it2).collide( **it1 );
                }
            }
        }

        for_each ( 
            Actor::actors.begin(), Actor::actors.end(), 
            std::mem_fn( &Actor::draw ) 
        );
 
        Actor::actors.erase ( 
            remove_if (
                Actor::actors.begin(), Actor::actors.end(), destroy_me
            ), 
            Actor::actors.end() 
        );

        update_screen();

        frameStart = frameEnd;
        frameEnd = SDL_GetTicks();
        frameTime = frameEnd - frameStart;
        if( frameTime > MAX_FRAME_TIME )
            frameTime = MAX_FRAME_TIME;
    }

#ifdef _WIN32
    SDL_Quit();
#endif

    return 0;
}


