
#include "Actor.h"
#include "Player.h"
#include "Orbitals.h"
#include "Particle.h"

#include "Vector.h"
#include "functional_plus.h"
#include "Random.h"

#include "Collision.h"

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <algorithm> // For for_each().
#include <functional> // For mem_fun_ptr.
#include <fstream>    // For debugging.

#include <iostream>

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

typedef Particle* ParticlePtr;
typedef std::vector< ParticlePtr > Particles;
Particles particles;

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

    pos.x( random( Orbital::RADIUS, SCREEN_WIDTH - Orbital::RADIUS ) );
    pos.y( random( Orbital::RADIUS, SCREEN_HEIGT - Orbital::RADIUS ) );

    spawn_orbital( pos.x(), pos.y() );
}

void spawn_particle( const Actor::vector_type& pos, const Actor::vector_type& v )
{
    typedef Actor::vector_type V;

    ParticlePtr particle = new Particle ( 
        pos, v, 0, 1, {1,1,1,1}
    );

    particles.push_back( particle );
}

bool delete_me( CActorPtr& actor )
{
    if( actor->deleteMe )
    {
        if( actor.get() == Orbital::target )
            Orbital::target = 0;

        for( int i=0; i < 300; i++ )
            spawn_particle( actor->s, actor->v );
    }
    return actor->deleteMe;
}

int main( int argc, char** argv )
{
    const int MAX_FRAME_TIME = 10;

    bool quit = false;

    Arena::minX = 0;
    Arena::maxX = 1000;
    Arena::minY = 0;
    Arena::maxY = 800;

    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
        return 1;
    make_sdl_gl_window( Arena::maxX, Arena::maxY );

    Player::body.load(   "art/Orbital.bmp" );
    Player::shield.load( "art/Sheild2.bmp" );
    Orbital::image.load( "art/Orbital.bmp" );

    std::ofstream log( "log" );

#define PANDE( cmd ) log << #cmd" ==> " << (cmd) << '\n'

    spawn_player( 350, 300 );
    spawn_orbital();
    spawn_orbital();
    spawn_particle( Actor::vector_type(300,300), Actor::vector_type(0,0) );

    PANDE( cActors[1]->s.x() );
    PANDE( cActors[1]->s.y() );
    PANDE( cActors[2]->s.x() );
    PANDE( cActors[2]->s.y() );

    int frameStart=SDL_GetTicks(), frameEnd=frameStart, frameTime=0, gameTime=0;
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

        static int spawnDelay = 3000;
        static int spawnWait = gameTime + spawnDelay;
        if( spawnWait < gameTime ) {
            spawn_orbital();
            spawnWait = gameTime + spawnDelay;
        }

        // TODO: Fixed time-step. 
        // Note that for_each_ptr is needed here. std::mem_fn should work, but
        // for whatever reason it's not. TODO: fix it?
        for_each_ptr ( 
            cActors.begin(), cActors.end(), 
            std::bind2nd( std::mem_fun_ref(&Actor::move), frameTime )
        );

        for_each_ptr ( 
            particles.begin(), particles.end(), 
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

        for_each ( 
            particles.begin(), particles.end(), 
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
        gameTime += frameTime;
        if( frameTime > MAX_FRAME_TIME )
            frameTime = MAX_FRAME_TIME;
    }

    SDL_Quit();
    glFlush();

    return 0;
}


