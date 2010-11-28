
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

const int VERSION = 8;

// SDL used milliseconds.
const int SECOND = 1000;

const int SCORE_DELAY = SECOND;
int scoreIncWait;

std::ofstream loggit( "log" );
#define PANDE( cmd ) log << #cmd" ==> " << (cmd) << '\n'

bool motionBlur = false;

typedef void(*GameLogic)(int dt );
GameLogic gameLogic;

typedef std::tr1::shared_ptr< CircleActor > CActorPtr;
typedef std::vector< CActorPtr > CActors;
CActors cActors;

typedef std::shared_ptr<Particle> ParticlePtr;
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

int packageLevel = 0;

const int SCREEN_WIDTH  = 900;
const int SCREEN_HEIGHT = 700;

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
        CActorPtr player(  new Player( Actor::vector_type(x,y) ) );
        cActors.push_back( player );

        Orbital::target = std::tr1::static_pointer_cast<Player>( player );
        Player::original = Orbital::target;
    } else {
        CActorPtr player(  new Player( Actor::vector_type(x-50,y) ) );
        cActors.push_back( player );
        Orbital::target = std::tr1::static_pointer_cast<Player>( player );

        CActorPtr player2(  new Player2( Actor::vector_type(x+50,y) ) );
        cActors.push_back( CActorPtr( player2 ) );
        Orbital::target2 = std::tr1::static_pointer_cast<Player>( player2 );
 

        Player::copy = Orbital::target2;
        Player2::original = Orbital::target;
    }
}

template< typename T >
std::tr1::weak_ptr<T> spawn( Actor::value_type x, Actor::value_type y )
{
    float speed = random( .15f, .40f );
    float angle = random_angle();

    float vx = std::cos(angle) * speed;
    float vy = std::sin(angle) * speed;

    std::tr1::shared_ptr<T> newActor ( 
        new T( Actor::vector_type(x,y), Actor::vector_type(vx,vy) )
    );

    cActors.push_back( newActor );

    return newActor;
}

template< typename T >
std::tr1::weak_ptr<T> spawn()
{
    int x, y;

    x = 
        random (
            int(Arena::minX + T::RADIUS*Arena::scale), 
            int(Arena::maxX - T::RADIUS*Arena::scale) 
        );

    y =  
        random (
            int(Arena::minY + T::RADIUS*Arena::scale),
            int(Arena::maxY - T::RADIUS*Arena::scale) 
        );

    // On Linux, sometimes x or y is negative. Fix that.
    if( x < 0 )
        x = -x;
    if( y < 0 )
        y = -y;

    return spawn<T>( x, y );
}

#include <fstream>
void spawn_particle( const Actor::vector_type& pos, const Actor::vector_type& v, float scale, const Color& c )
{
    typedef Actor::vector_type V;

    scale = random( 0.75f, scale );

    Particle* particle = new Particle ( 
		pos, v, 0, 1, scale, c
    );

    particles.push_back( ParticlePtr(particle) );
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
            spawn_particle( actor->s, actor->v/6, actor->radius()/4.5, actor->color() );

        // Add to score if player is alive.
        if( !Orbital::target.expired() )
            scoreVal += actor->score_value();

        // If this player's what just died...
        if( actor.get() == Orbital::target.lock().get() ) 
        {
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
    Arena::minX = Arena::minY = 3;
    Arena::maxX = SCREEN_WIDTH-3;
    Arena::maxY = SCREEN_HEIGHT-3;

    Player::SharedPlayerPtr target = Orbital::target.lock();

    if( logic && logic != gameLogic ) {
        gameLogic = logic;

        packageLevel = 0;

        // Before clearing the actors, make them explode.
        for( size_t i=0; i < cActors.size(); i++ )
            cActors[i]->deleteMe = true;
        if( target )
            target->deleteMe = false;
        for_each( cActors.begin(), cActors.end(), delete_me );
    } else {
        // Normal resets clear the screen.
        particles.clear();
    }

    Actor::vector_type playerPos( 350, 300 );
    if( target )
        playerPos = target->s;

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

void random_spawn( int difficulty )
{
    static int recentSpawns[2] = { 0, 1 };

    int newSpawn = 0;

    newSpawn = spawnSlots[ random(0, difficulty) ];

    /* Commented out because setting newSpawn to N_SPAWN_SLOTS just makes more
     * bugs. */
    //if( recentSpawns[0] == recentSpawns[1] &&
    //    recentSpawns[0] == newSpawn )
    //    // Try just once more. Trying until the new spawn is truly new seems to
    //    // cause an infinite loop.
    //    newSpawn = N_SPAWN_SLOTS;
    
    recentSpawns[1] = recentSpawns[0];
    if( newSpawn != N_SPAWN_SLOTS )
        recentSpawns[0] = newSpawn;

    switch( newSpawn )
    {
      case ORBITAL: spawn<Orbital>(); break;
      case STOPPER: spawn<Stopper>(); break;
      case TWISTER: spawn<Twister>(); break;
      case STICKER: spawn<Sticker>(); break;
      default: random_spawn( difficulty ); // This line should never be reached.
    }
}

void standard_spawn()
{
    static int difficulty = 1;
    if( spawnDelay > 5500 )
        difficulty = 1;
    else if( spawnDelay > 5300 )
        difficulty = 3;
    else if( spawnDelay > 4000 )
        difficulty = 6;
    else if( spawnDelay > 3000 )
        difficulty = N_SPAWN_SLOTS;

    random_spawn( difficulty );
}

void arcade_mode( int dt )
{
    font->draw( "Score: " + to_string((int)scoreVal), 100, 100 );

    if( timePlayerDied && gameTime < timePlayerDied + 7*SECOND )
        font->draw( "Press r to reset, m for menu", 600, 200 );

    // If the player is alive...
    if( Orbital::target.lock() ) 
    {
        scoreIncWait = gameTime + SCORE_DELAY;

        float sum = 0;
        unsigned int nEnemies = 0;
        for( size_t i=1; i < cActors.size(); i++ )
            if( cActors[i]->isActive && cActors[i]->isMovable ) {
                sum += cActors[i]->score_value();
                nEnemies++;
            }

        scoreVal += sum / 4.0 * nEnemies*nEnemies * (float(dt)/SECOND);
    }

    if( spawnWait <= gameTime ) {
        spawnWait = gameTime + spawnDelay;

        spawnDelay -= 300;
        if( spawnDelay <= 3000 )
            spawnDelay -= -500;
        if( spawnDelay < 500 )
            spawnDelay = 500;

        standard_spawn();
    }
}

void dual_mode( int dt )
{
    font->draw( "Score: " + to_string((int)scoreVal), 100, 100 );

    if( timePlayerDied && gameTime < timePlayerDied + 7*SECOND )
        font->draw( "Press r to reset, m for menu", 600, 200 );

    // If the player is alive and SCORE_DELAY seconds have passed...
    if( !Orbital::target.expired() ) 
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

        standard_spawn();
    }
}

void package_delivery( int dt )
{
    static std::tr1::weak_ptr<Package> weakPackage;
    static bool started = false;
    static std::string tip = "";
    static int time;

    Orbital::SharedPlayerPtr target = Orbital::target.lock();

    // Will get reinitialized if loading new level.
    std::tr1::shared_ptr<Package> package = weakPackage.lock();

    if( cActors.size() == 1 && target ) 
    {
        time = 0;

        std::stringstream filename;
        filename << "challenge/package/level" << packageLevel;

        std::ifstream level( filename.str() );

        std::string val;

        if( level ) 
        {
            started = false;
            tip = "";
            // TODO: Delete me. But may be needed still.
            //Package::goal = 0;

            while( level >> val ) 
            {
                if( val == "tip" )
                {
                    std::getline( level, tip );
                }
                else
                {
                    float x,y;
                    level >> x >> y;

                    if( val == "player" ) 
                        target->s = Actor::vector_type( x, y );
                    else if( val == "package" )
                        package = spawn<Package>( x, y ).lock();
                    else if( val == "obsticle" )
                        spawn<Obsticle>( x, y );
                    else if( val == "goal" )
                        Package::goal = spawn<Goal>( x, y ).lock();
                    else if( val == "arenaX" ) {
                        Arena::minX = x; Arena::maxX = y;
                    } else if( val == "arenaY" ) {
                        Arena::minY = x; Arena::maxY = y;
                    }

                }
            }
        }
        else 
        {
            packageLevel--;
            package_delivery( dt );
        }
    }

    if( !started && package && package->started ) {
        started = true;
    }

    if( started )
        time += dt;

    // Draw hint.
    if( tip.size() )
        font->draw( tip, Arena::minX + (Arena::maxX-Arena::minX)/4, 100 );

    // If player won the level...
    if( target && package && package->reachedGoal ) {
        std::stringstream filename;
        filename << "challenge/package-times/level" << packageLevel << ".txt";

        int previousTime;
        {
            float input;
            std::ifstream scoreIn( filename.str() );
            if( scoreIn >> input )
                previousTime = input * SECOND;
            else
                previousTime = 0;
        }

        if( previousTime < time ) {
            std::ofstream score( filename.str() );
            score << float(time) / SECOND;
        }

        time = 0;

        packageLevel++;
        reset();
    }

    // Keep the static pointer up-to-date.
    weakPackage = package;
}

void challenge( int dt )
{
    package_delivery( dt );
}

void menu( int dt )
{
    if( cActors.size() == 1 )
        for( int i=0; i < 3; i++ )
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
    for( size_t i=0; i < cActors.size(); i++ ) {
        if( !cActors[i]->isActive )
            continue;

        if( cActors[i]->s.y() < Arena::minX + cActors[i]->radius()  )
            reset( arcade_mode );
        else if( Arena::maxY < cActors[i]->s.y() + cActors[i]->radius() )
            reset( dual_mode );
        else if( Arena::maxX < cActors[i]->s.x() + cActors[i]->radius() )
            reset( challenge );
    }
}

int main( int argc, char** argv )
{
    const int IDEAL_FRAME_TIME = SECOND / 60;
    const int MAX_FRAME_TIME = 3 * IDEAL_FRAME_TIME;

    bool quit = false;
    bool paused = false;

    Arena::minX = 0;
    Arena::maxX = SCREEN_WIDTH;
    Arena::minY = 0;
    Arena::maxY = SCREEN_HEIGHT;

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
                  case 'p': paused = !paused;    break;
                  case 'z': spawn<Stopper>();    break;

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

        float boarder[] = {
            Arena::minX, Arena::minY,
            Arena::maxX, Arena::minY,
            Arena::maxX, Arena::maxY,
            Arena::minX, Arena::maxY
        };

        glColor3f( 1, 1, 1 );
        glDisable( GL_TEXTURE_2D );
        glEnableClientState( GL_VERTEX_ARRAY );
        {
            glVertexPointer( 2, GL_FLOAT, 0, boarder );
            glDrawArrays( GL_LINE_LOOP, 0, 4 );
        }
        glDisableClientState( GL_VERTEX_ARRAY );
        glEnable( GL_TEXTURE_2D );
        
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

        if( paused )
            frameTime = 0;

        if( frameTime > MAX_FRAME_TIME )
            frameTime = MAX_FRAME_TIME;

        gameTime += frameTime;
    }

    SDL_Quit();
    glFlush();

    return 0;
}


