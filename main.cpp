
#include <iostream>
#include <fstream>

#include "Vector.h"
#include "functional_plus.h"

#include "Actor.h"
#include "Player.h"

#include "Collision.h"

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <algorithm> // For for_each().
#include <functional> // For mem_fun_ptr.
#include <fstream>    // For debugging.

GLenum init_gl( int w, int h )
{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

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

typedef std::shared_ptr< CircleActor > CActorPtr;
typedef std::vector< CActorPtr > CActors;
CActors cActors;

void spawn_player( Actor::value_type x, Actor::value_type y )
{
    Player* player = new Player( Actor::vector_type(x,y) );
    cActors.push_back( CActorPtr( player ) );
}

int main( int argc, char** argv )
{
    const int MAX_FRAME_TIME = 10;

    bool quit = false;

    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
        return 1;
    make_sdl_gl_window( 700, 600 );

    Player::body.load( "art/Orbital.bmp" );
    Player::shield.load( "art/Sheild2.bmp" );

    std::ofstream log( "log" );

#define PANDE( cmd ) log << #cmd" ==> " << (cmd) << '\n'

    PANDE( Player::body.handle() );
    PANDE( Player::shield.handle() );

    spawn_player( 350, 300 );

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
            cActors.begin(), cActors.end(), 
            std::bind2nd( std::mem_fun_ref(&Actor::move), frameTime )
        );

//        for( Actor::ActorList::iterator it1 = Actor::cActors.begin();
//             it1 != Actor::cActors.end();
//             it1++ )
//        {
//            for( Actor::ActorList::iterator it2 = it1+1;
//                 it2 != Actor::cActors.end();
//                 it2++ )
//            {
//                if( collision((**it1).collision_data(), (**it2).collision_data()) )
//                {
//                    (**it1).collide( **it2 );
//                    (**it2).collide( **it1 );
//                }
//            }
//        }

        for_each ( 
            cActors.begin(), cActors.end(), 
            std::mem_fn( &Actor::draw ) 
        );

        cActors[0]->draw();
 
        //cActors.erase ( 
        //    remove_if (
        //        cActors.begin(), cActors.end(), destroy_me
        //    ), 
        //    cActors.end() 
        //);

        update_screen();

        frameStart = frameEnd;
        frameEnd = SDL_GetTicks();
        frameTime = frameEnd - frameStart;
        if( frameTime > MAX_FRAME_TIME )
            frameTime = MAX_FRAME_TIME;
    }

    SDL_Quit();
    glFlush();

    return 0;
}


