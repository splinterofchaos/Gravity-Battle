
#ifdef _WIN32
    #include <Windows.h>
    #include <ctime>
#endif

#include "Actor.h"
#include "Player.h"
#include "Orbitals.h"
#include "Particle.h"
#include "MenuOrbital.h"
#include "Challenge.h"

#include "Vector.h"
#include "functional_plus.h"
#include "Random.h"

#include "Collision.h"

#include "Font.h"

#include "Config.h"

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

const int VERSION = 5;

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

// True if the player has moved since the game started.
bool playerHasMoved = false; 
// True if the player has pressed spacebar.
bool playerIncreasedGravity = false;

const Config defaultConfig; // Used when no unsure about any config option.
Config fileConfig( "config.txt" ), config = fileConfig;

// FUNCTIONS //
void arcade_mode( int dt );
void dual_mode( int dt );
void menu( int dt );

std::string to_string( int x )
{
    std::stringstream ss;
    ss << x;
    return ss.str();
}

float to_float( std::string str )
{
    std::stringstream ss;
    ss << str;
    float x;
    ss >> x;
    return x;
}

void configure( const Config& cfg )
{
    particleRatio                = cfg.particleRatio;
    Orbital::predictionLength    = cfg.predictionLength;
    Orbital::predictionPrecision = cfg.predictionPrecision;
    Orbital::gravityLine         = cfg.gravityLine;
    Orbital::velocityArrow       = cfg.velocityArrow;
    Orbital::accelerationArrow   = cfg.accelerationArrow;
    motionBlur                   = cfg.motionBlur;
    Arena::scale                 = cfg.scale;
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
    if( gameLogic != dual_mode ) {
        Player* player = new Player( Actor::vector_type(x,y) );
        cActors.push_back( CActorPtr( player ) );

        Orbital::target = player;
        Player::original = player;
        Player::copy = 0;
    } else {
        Player* player = new Player( Actor::vector_type(x-50,y) );
        cActors.push_back( CActorPtr( player ) );
        Orbital::target = player;

        Player2* player2 = new Player2( Actor::vector_type(x+50,y) );
        cActors.push_back( CActorPtr( player2 ) );
        Orbital::target2 = player2;

        Player::copy = player2;
        Player2::original = player;
    }
}

template< typename T >
T* spawn( Actor::value_type x, Actor::value_type y )
{
    float speed = random( .11f, .40f );
    float angle = random_angle();

    float vx = std::cos(angle) * speed;
    float vy = std::sin(angle) * speed;

    T* newActor = new T(
        Actor::vector_type(x,y), Actor::vector_type(vx,vy) 
    );

    cActors.push_back( CActorPtr( newActor ) );

    return newActor;
}

template< typename T >
T* spawn()
{
    Actor::vector_type pos(0.0f,0.0f);

    pos.x( random( int(Orbital::RADIUS), int(Arena::maxX - Orbital::RADIUS) ) );
    pos.y( random( int(Orbital::RADIUS), int(Arena::maxY - Orbital::RADIUS) ) );

    return spawn<T>( pos.x(), pos.y() );
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

float scoreVal = 0;

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

void reset( GameLogic logic = 0 )
{
    if( logic )
        gameLogic = logic;
    else
        particles.clear();

    Actor::vector_type playerPos( 350, 300 );
    if( Orbital::target )
        playerPos = Orbital::target->s;

    Orbital::target2 = 0;
    cActors.clear();

    spawn_player( playerPos.x(), playerPos.y() );

    gameTime   = 0;
    spawnDelay = 6000;
    spawnWait  = 30;
    timePlayerDied = 0;

    scoreVal = 0;

    scoreIncWait = gameTime + SCORE_DELAY;
}

enum Spawns { ORBITAL, STOPPER, TWISTER, STICKER, N_SPAWN_SLOTS=9 };
Spawns spawnSlots[14] = {
    STOPPER, ORBITAL,
    ORBITAL, ORBITAL, ORBITAL, TWISTER,
    TWISTER, ORBITAL, TWISTER 
};

void randome_spawn( int difficulty )
{
    switch( spawnSlots[ random(0, difficulty) ] )
    {
      case ORBITAL: spawn<Orbital>(); break;
      case STOPPER: spawn<Stopper>(); break;
      case TWISTER: spawn<Twister>(); break;
      case STICKER: spawn<Sticker>(); break;
      default: break;// ...
    }
}

void arcade_mode( int dt )
{
    font->draw( "Score: " + to_string((int)scoreVal), 100, 100 );

    if( timePlayerDied && gameTime < timePlayerDied + 7*SECOND )
        font->draw( "Press r to reset, m for menu", 600, 200 );

    // If the player is alive and SCORE_DELAY seconds have passed...
    if( Orbital::target ) 
    {
        scoreIncWait = gameTime + SCORE_DELAY;

        float sum = 0;
        unsigned int nEnemies = 0;
        for( size_t i=1; i < cActors.size(); i++ )
            if( cActors[i]->isActive && cActors[i]->isMovable ) {
                sum += cActors[i]->score_value();
                nEnemies++;
            }

        scoreVal += sum / 4 * nEnemies*nEnemies * float(dt)/SECOND;
    }

    if( spawnWait <= gameTime ) {
        spawnWait = gameTime + spawnDelay;

        spawnDelay -= 300;
        if( spawnDelay <= 3000 )
            spawnDelay -= -500;
        if( spawnDelay < 1000 )
            spawnDelay = 1000;

        static int difficulty = 1;
        if( spawnDelay > 5000 )
            difficulty = 2;
        else if( spawnDelay > 4000 )
            difficulty = 6;
        else if( spawnDelay > 3000 )
            difficulty = N_SPAWN_SLOTS;

        randome_spawn( difficulty );
    }
}

void dual_mode( int dt )
{
    font->draw( "Score: " + to_string((int)scoreVal), 100, 100 );

    if( timePlayerDied && gameTime < timePlayerDied + 7*SECOND )
        font->draw( "Press r to reset, m for menu", 600, 200 );

    // If the player is alive and SCORE_DELAY seconds have passed...
    if( Orbital::target ) 
    {
        scoreIncWait = gameTime + SCORE_DELAY;

        float sum = 0;
        unsigned int nEnemies = 0;
        for( size_t i=1; i < cActors.size(); i++ )
            if( cActors[i]->isActive && cActors[i]->isMovable ) {
                sum += cActors[i]->score_value();
                nEnemies++;
            }

        scoreVal += sum / 4 * nEnemies*nEnemies * float(dt)/SECOND;
    }

    if( spawnWait <= gameTime ) {
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
            difficulty = 6;
        else if( spawnDelay > 3000 )
            difficulty = 9;

        randome_spawn( difficulty );
    }
}

void challenge( int dt )
{
    static Package* package = 0;
    static bool started = false;

    if( cActors.size() == 1 ) {
        package = spawn<Package>( 100, 100 );
        package->isMovable = false;
        started = false;
    }

    if( !started && magnitude(cActors[0]->s - package->s) < 300 ) {
        package->isMovable = true;
        started = true;
    }

    if( started && !package->isMovable )
        font->draw( "You won!!!", 300, 300 );
}

void menu( int dt )
{
    if( cActors.size() == 1 )
        spawn<MenuOrbital>();

    const int LINE_HEIGHT = 20;

    int y = 350;
    if( ! playerHasMoved ) {
        font->draw( "WASD to move.", 270, 350 );
    } else {
        y = 350; 
        font->draw( "Press 1 to switch on/off prediction lines.", 500, y );
        font->draw( "Press 2 to switch on/off gravity lines.", 500, y += LINE_HEIGHT );
        font->draw( "Press 3 to switch on/off velocity arrows.", 500, y += LINE_HEIGHT );
        font->draw( "Press 4 to switch on/off acceleration arrows.", 500, y += LINE_HEIGHT );
        font->draw( "Press 5 to switch on/off motion blur.", 500, y += LINE_HEIGHT );
        font->draw( "To permanently change, edit config.txt", 500, y += 2*LINE_HEIGHT );

        font->draw( "^^ Move up here for arcade mode! ^^", 350, 50 );
        font->draw( "Move down here for dual mode! (use WASD and arrow keys)", 350, Arena::maxY - 50 );
        font->draw( "To the left for challenge mode. >>>", 650, 300 );
    }

    if( ! playerIncreasedGravity )
        font->draw( "SPACEBAR to increase gravity.", 270, 350 + LINE_HEIGHT );


    // Enter arcade mode when the orbital reaches the top of the screen.
    if( cActors[1]->s.y() < cActors[1]->radius()  ) {
            // Before being deleted, explode--EYE CANDY!
            // Since delete_me is what spawns the particles, and reset clears
            // cActors without calling delete_me, do it here.
            cActors[1]->deleteMe = true;
            delete_me( cActors[1] );

            reset( arcade_mode );
    } else if( Arena::maxY < cActors[1]->s.y() + cActors[1]->radius() ) {
            cActors[1]->deleteMe = true;
            delete_me( cActors[1] );

            reset( dual_mode );
    } else if( Arena::maxX < cActors[1]->s.x() + cActors[1]->radius() ) {
            cActors[1]->deleteMe = true;
            delete_me( cActors[1] );

            reset( challenge );
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
            switch( event.type ) 
            {
              case SDL_QUIT: quit = true; break;

              case SDL_KEYDOWN:
                switch( event.key.keysym.sym )
                {
                  case 'r': reset();             break;
                  case 'm': reset( menu );       break;
                  case SDLK_ESCAPE: quit = true; break;

                  case '1': 
                    // Do this in case the user updated prediction- Length or Precision.
                    fileConfig.reload( "config.txt" );

                    if( ! config.predictionLength )
                        if( fileConfig.predictionLength )
                            config.predictionLength = fileConfig.predictionLength;
                        else
                            config.predictionLength = defaultConfig.predictionLength;
                    else
                        config.predictionLength = 0;

                    config.predictionPrecision = fileConfig.predictionPrecision;

                    break;

                  case '2': config.gravityLine   = ! config.gravityLine;   break;
                  case '3': config.velocityArrow = ! config.velocityArrow; break;
                  case '4': config.accelerationArrow = ! config.accelerationArrow; break;

                  case '5': 
                            config.motionBlur = ! config.motionBlur;                         
                            if( config.motionBlur )
                                glAccum( GL_LOAD, 1 );
                  break;

                  case 'w': case 'a': case 's': case 'd': playerHasMoved = true; break;
                  case SDLK_SPACE: playerIncreasedGravity = true;

                  default:                       break;
                }
                break;

              default: break;
            }
		}

        gameLogic( frameTime );

        const int DT = IDEAL_FRAME_TIME / 4;
        static int time = 0;
        for( time += frameTime; time >= DT; time -= DT ) {
            for_each_ptr ( 
                cActors.begin(), cActors.end(), 
                std::bind2nd( std::mem_fun_ref(&Actor::move), DT )
            );

            if( cActors.size() )
                for( size_t i=0; i < cActors.size()-1; i++ )
                    for( size_t j=i+1; j < cActors.size(); j++ )
                        if( collision( *cActors[i], *cActors[j] ) ) {
                            cActors[i]->collide_with( *cActors[j] );
                            cActors[j]->collide_with( *cActors[i] );
                        }
        }

        for_each_ptr ( 
            particles.begin(), particles.end(), 
            std::bind2nd( std::mem_fun_ref(&Actor::move), frameTime )
        );

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
        if( lastUpdate + IDEAL_FRAME_TIME/2 <= gameTime ) {
            configure( config );
            update_screen();
        }
        
        frameStart = frameEnd;
        frameEnd = SDL_GetTicks();
        frameTime = frameEnd - frameStart;

        if( frameTime > MAX_FRAME_TIME )
            frameTime = MAX_FRAME_TIME;

        gameTime += frameTime;
    }

    particles.clear();
    cActors.clear();

    SDL_Quit();
    glFlush();

    return 0;
}


