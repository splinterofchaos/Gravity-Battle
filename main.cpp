
#include <iostream>
#include <fstream>

#include "Vector.h"
#include "functional_plus.h"

#include "Actor.h"
#include "Player.h"
#include "Orbitals.h"

#include "Collision.h"

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <algorithm> // For for_each().
#include <functional> // For mem_fun_ptr.
#include <fstream>    // For debugging.
#include <ctime>      // For randomization.

int SCREEN_WIDTH;
int SCREEN_HEIGT;

GLenum init_gl( int w, int h )
{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, w, h, 0, -10, 10 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    return glGetError();
}

bool make_sdl_gl_window( int w, int h )
{
    if( ! SDL_SetVideoMode(w, h, 32, SDL_OPENGL) )
        return false;
    init_gl( w, h );

    SCREEN_WIDTH = w;
    SCREEN_HEIGT = h;

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

    Orbital::target = player;
}

void spawn_orbital( Actor::value_type x, Actor::value_type y )
{
    Actor::vector_type v(0,0);
    Orbital* orbital = new Orbital( Actor::vector_type(x,y), v );
    cActors.push_back( CActorPtr( orbital ) );
}

void spawn_orbital()
{
    Actor::vector_type pos(0,0);

    int range = SCREEN_WIDTH - 2 * Orbital::RADIUS;
    pos.x( rand() % range + Orbital::RADIUS );
    
    range = SCREEN_HEIGT - 2 * Orbital::RADIUS;
    pos.y( rand() % range + Orbital::RADIUS );

    spawn_orbital( pos.x(), pos.y() );
}

bool delete_me( CActorPtr& actor )
{
    if( actor->deleteMe && actor.get() == Orbital::target )
        Orbital::target = 0;
    return actor->deleteMe;
}

int main( int argc, char** argv )
{
    const int MAX_FRAME_TIME = 10;

    bool quit = false;

    srand( time(0) );

    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
        return 1;
    make_sdl_gl_window( 700, 600 );

    Arena::minX = 0;
    Arena::maxX = 700;
    Arena::minY = 0;
    Arena::maxY = 600;

    Player::body.load(   "art/Orbital.bmp" );
    Player::shield.load( "art/Sheild2.bmp" );
    Orbital::image.load( "art/Orbital.bmp" );

    std::ofstream log( "log" );

#define PANDE( cmd ) log << #cmd" ==> " << (cmd) << '\n'

    spawn_player( 350, 300 );
    spawn_orbital();
    spawn_orbital();

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

        if( cActors.size() )
            for( int i=0; i < cActors.size()-1; i++ )
                for( int j=i+1; j < cActors.size(); j++ )
                    if( collision( *cActors[i], *cActors[j] ) ) {
                        cActors[i]->collide_with( *cActors[j] );
                        cActors[j]->collide_with( *cActors[i] );
                    }
                    

        for_each ( 
            cActors.begin(), cActors.end(), 
            std::mem_fn( &Actor::draw ) 
        );

        cActors.erase ( 
            remove_if (
                cActors.begin(), cActors.end(), delete_me
            ), 
            cActors.end() 
        );

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


