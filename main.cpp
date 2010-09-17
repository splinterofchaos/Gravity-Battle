
#ifdef _WIN32
    #include <Windows.h>
    #include <ctime>
#endif

#include "Actor.h"
#include "Player.h"
#include "Orbitals.h"
#include "Particle.h"
#include "MenuOrbital.h"

#include "Vector.h"
#include "functional_plus.h"
#include "Random.h"

#include "Collision.h"

#include "Font.h"

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <algorithm> // For for_each().
#include <functional> // For mem_fun_ptr.
#include <fstream>    // For debugging.

#include <sstream> // For int -> string conversions.

// GLOBALS //
int spawnDelay;
int spawnWait;
int gameTime;

int particleRatio = 200; // How many particles to create compared to CircleActor::mass.

const int VERSION = 4;

// SDL used milliseconds.
const int SECOND = 1000;

const int SCORE_DELAY = SECOND;
int scoreIncWait;

std::ofstream loggit( "log" );
#define PANDE( cmd ) log << #cmd" ==> " << (cmd) << '\n'

bool motionBlur = false;

typedef void(*GameLogic)(int dt );
GameLogic gameLogic;

typedef std::shared_ptr< CircleActor > CActorPtr;
typedef std::vector< CActorPtr > CActors;
CActors cActors;

typedef Particle* ParticlePtr;
typedef std::vector< ParticlePtr > Particles;
Particles particles;

std::shared_ptr<BitmapFont> font;

int timePlayerDied = -1000;

// FUNCTIONS //
std::string to_string( int x )
{
    std::stringstream ss;
    ss << x;
    return ss.str();
}

int to_int( std::string str )
{
    std::stringstream ss;
    ss << str;
    int x;
    ss >> x;
    return x;
}

void configure()
{
    std::ifstream cfg( "config.txt" );

    std::string line;
    while( std::getline( cfg, line ) ) 
    {
        if( line.size() == 0 || line[0] == '#' )
            continue;

        std::string::iterator it = std::find( line.begin(), line.end(), ' ' );
        std::string valName( line.begin(), it );
        it += 3;
        int value;
        if( it < line.end() )
            value = to_int( std::string( it, line.end() ) );
        else
            continue;

        if( valName == "particleRatio" )
            particleRatio = value;
        else if( valName == "predictionLength" )
            Orbital::predictionLength = value;
        else if( valName == "predictionPrecision" )
            Orbital::predictionPrecision = value;
        else if( valName == "gravityLine" )
            Orbital::gravityLine = value;
        else if( valName == "velocityArrow" )
            Orbital::velocityArrow = value;
        else if( valName == "motionBlur" )
            motionBlur = value;
    }
}


GLenum init_gl( int w, int h )
{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, w, h, 0, -10, 10 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return glGetError();
}

bool make_sdl_gl_window( int w, int h )
{
    if( ! SDL_SetVideoMode(w, h, 32, SDL_OPENGL) )
        return false;
    init_gl( w, h );

    font.reset( new BitmapFont );

    return true;
}

void update_screen()
{
    if( motionBlur ) {
        float q = 0.97;
        glAccum( GL_MULT, q );
        glAccum( GL_ACCUM, 1-q );
        glAccum( GL_RETURN, 1 );
    }

    SDL_GL_SwapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void spawn_player( Actor::value_type x, Actor::value_type y )
{
    Player* player = new Player( Actor::vector_type(x,y) );
    cActors.push_back( CActorPtr( player ) );

    Orbital::target = player;
}

template< typename T >
void spawn( Actor::value_type x, Actor::value_type y )
{
    float speed = random( .11f, .40f );
    float angle = random_angle();

    float vx = std::cos(angle) * speed;
    float vy = std::sin(angle) * speed;

    T* newActor = new T(
        Actor::vector_type(x,y), Actor::vector_type(vx,vy) 
    );

    cActors.push_back( CActorPtr( newActor ) );
}

template< typename T >
void spawn()
{
    Actor::vector_type pos(0.0f,0.0f);

    pos.x( random( int(Orbital::RADIUS), int(Arena::maxX - Orbital::RADIUS) ) );
    pos.y( random( int(Orbital::RADIUS), int(Arena::maxY - Orbital::RADIUS) ) );

    spawn<T>( pos.x(), pos.y() );
}

void spawn_particle( const Actor::vector_type& pos, const Actor::vector_type& v, float scale, const Color& c )
{
    typedef Actor::vector_type V;

    scale = random( 0.75f, scale );

    ParticlePtr particle = new Particle ( 
		pos, v, 0, 1, scale, c
    );

    particles.push_back( particle );
}

bool is_off_screen( ParticlePtr p )
{
    return p->s.x() < Arena::minX-p->scale || p->s.x() > Arena::maxX || 
        p->s.y() < Arena::minY-p->scale || p->s.y() > Arena::maxY;
}

int scoreVal = 0;

void reset( GameLogic logic = 0 )
{
    if( logic )
        gameLogic = logic;

    particles.clear();

    if( cActors.size() ) {
        if ( Orbital::target ) {
            cActors.erase( cActors.begin() + 1, cActors.end() );
        } else {
            cActors.clear();
            spawn_player( 350, 300 );
        }
    } else {
        spawn_player( 350, 300 );
    }

    gameTime   = 0;
    spawnDelay = 6000;
    spawnWait  = 30;
    timePlayerDied = 0;

    scoreVal = 0;

    scoreIncWait = gameTime + SCORE_DELAY;

    configure();
}

bool delete_me( CActorPtr& actor )
{
    if( actor->deleteMe )
    {
        // Explode.
        for( int i=0; i < actor->mass()*particleRatio; i++ )
            spawn_particle( actor->s, actor->v/4, actor->radius()/8, actor->color() );

        // Add to score if player is alive.
        if( Orbital::target )
            scoreVal += actor->score_value();

        // If this player's what just died...
        if( actor.get() == Orbital::target ) 
        {
            Orbital::target = 0;

            timePlayerDied = gameTime;

            int version;
            int score;

            {
                std::ifstream scoresIn( "Highscore.txt" );
                scoresIn >> version >> score;
            }

            if( version != VERSION ) {
                // Preserve the old high scores in another file.
                std::string filename = "Highscore (" + to_string(version) + ").txt";
                std::ofstream out( filename );
                out << version << ' ' << score;

                // Make sure the new score, no matter how low, is put into
                // Highscore.txt.
                score = -100; 
            }

            if( scoreVal > score ) {
                std::ofstream scoresOut( "Highscore.txt" );
                scoresOut << VERSION << ' ' << scoreVal;
            }
        }
    }
    return actor->deleteMe;
}

void arcade_mode( int dt )
{

    font->draw( "Score: " + to_string(scoreVal), 100, 100 );

    if( timePlayerDied && gameTime < timePlayerDied + 7*SECOND )
        font->draw( "Press r to reset, m for menu", 600, 200 );

    // If the player is alive and SCORE_DELAY seconds have passed...
    if( Orbital::target && scoreIncWait < gameTime ) {
        scoreIncWait = gameTime + SCORE_DELAY;

        int sum = 0;
        unsigned int nEnemies = 0;
        for( size_t i=1; i < cActors.size(); i++ )
            if( cActors[i]->isActive ) {
                sum += cActors[i]->score_value();
                nEnemies++;
            }

        scoreVal += sum / 4 * nEnemies*nEnemies;
    }

    if( spawnWait < gameTime ) {
        spawnWait = gameTime + spawnDelay;

        spawnDelay -= 300;
        if( spawnDelay <= 3000 )
            spawnDelay -= -500;
        if( spawnDelay < 1000 )
            spawnDelay = 1000;

        static int difficulty = 1;
        if( spawnDelay > 5000 )
            difficulty = 1;
        else if( spawnDelay > 4000 )
            difficulty = 9;
        else if( spawnDelay > 3000 )
            difficulty = 13;

        if( random( 0, difficulty ) <= 7 )
            spawn<Orbital>();
        else
            spawn<Twister>();
    }
}

void menu( int dt )
{
    if( cActors.size() == 1 )
        spawn<MenuOrbital>();

    font->draw( "WASD to move.", 400, 300 );
    font->draw( "^^ Up here for arcade mode! ^^", 350, 50 );

    if( cActors[1]->s.y() < cActors[1]->radius() ) {
        reset( arcade_mode );
    }
}

int main( int argc, char** argv )
{
    const int IDEAL_FRAME_TIME = SECOND / 60;
    const int MAX_FRAME_TIME = 3 * IDEAL_FRAME_TIME;

    bool quit = false;

    Arena::minX = 0;
    Arena::maxX = 900;
    Arena::minY = 0;
    Arena::maxY = 700;

    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
        return 1;
    make_sdl_gl_window( (int)Arena::maxX, (int)Arena::maxY );

    Player::body.load(   "art/Orbital.bmp" );
    Player::shield.load( "art/Sheild2.bmp" );
    Orbital::image.load( "art/Orbital.bmp" );

    reset( menu ); 

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
        if( keyState[ SDLK_r ] )
            reset();
        if( keyState[ SDLK_m ] )
            reset( menu );

        gameLogic( frameTime );

        const int DT = IDEAL_FRAME_TIME / 4;
        static int time = 0;
        for( time += frameTime; time >= DT; time -= DT )
            for_each_ptr ( 
                cActors.begin(), cActors.end(), 
                std::bind2nd( std::mem_fun_ref(&Actor::move), DT )
            );

        for_each_ptr ( 
            particles.begin(), particles.end(), 
            std::bind2nd( std::mem_fun_ref(&Actor::move), frameTime )
        );

        if( cActors.size() )
            for( size_t i=0; i < cActors.size()-1; i++ )
                for( size_t j=i+1; j < cActors.size(); j++ )
                    if( collision( *cActors[i], *cActors[j] ) ) {
                        cActors[i]->collide_with( *cActors[j] );
                        cActors[j]->collide_with( *cActors[i] );
                    }

        for_each ( 
            particles.begin(), particles.end(), 
            std::mem_fn( &Actor::draw ) 
        );

        for_each ( 
            cActors.begin(), cActors.end(), 
            std::mem_fn( &Actor::draw ) 
        );

        glLoadIdentity();

        cActors.erase ( 
            remove_if (
                cActors.begin(), cActors.end(), delete_me
            ), 
            cActors.end() 
        );

        particles.erase ( 
            remove_if (
                particles.begin(), particles.end(), is_off_screen
            ), 
            particles.end() 
        );

        static int lastUpdate = gameTime;
        if( lastUpdate + IDEAL_FRAME_TIME/2 <= gameTime )
            update_screen();
        
        frameStart = frameEnd;
        frameEnd = SDL_GetTicks();
        frameTime = frameEnd - frameStart;

        if( frameTime > MAX_FRAME_TIME )
            frameTime = MAX_FRAME_TIME;

        gameTime += frameTime;
    }

    SDL_Quit();
    glFlush();

    return 0;
}


