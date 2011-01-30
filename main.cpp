
// Local includes.
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
#include "Parsing.h" // For high score highScoreTable generation.

#include "Draw.h"

#include "Sound.h"

// 3rd party includes.
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

// std::includes.
#include <algorithm>  // For for_each.
#include <functional> // For mem_fun_ref   in main.
#include <cstdio>     // For file renaming in update_high_score.
#include <iomanip>    // For setw          in arcade_mode.

#include <sstream> // For int -> string conversions.

int spawnDelay;
int spawnWait;
int gameTime;

// How many particles to create proportionate to CircleActor::mass.
int particleRatio = 200; 

// Used for high score consistency. Increment when a change may effect scoring
// in any way--pretty much any game rule change at all.
const int VERSION = 9;

// SDL uses milliseconds so i will too.
const int SECOND = 1000;
int timePlayerDied = -1000;
bool showFrameTime = false;

std::ofstream loggit( "log" );
#define PANDE( cmd ) log << #cmd" ==> " << (cmd) << '\n'

bool motionBlur = false;

// Every frame, all actors are drawn and moved. This points to what decides how
// they are created, how the player scores, and highscore generation. 
typedef void(*GameLogic)(int dt );
GameLogic gameLogic;

typedef std::tr1::shared_ptr< CircleActor > SharedCActorPtr;
typedef std::tr1::weak_ptr< CircleActor >   WeakCActorPtr;
typedef std::tr1::weak_ptr<Actor>           ActorPtr;

typedef std::vector< SharedCActorPtr > CActors;
typedef std::vector< Particle > Particles;
typedef std::vector<ActorPtr>      Actors;

CActors   cActors;
Particles particles;

// Used everywhere to write text on the screen.
std::shared_ptr<BitmapFont> font;

// True if the player has moved since the game started.
bool playerHasMoved = false; 
// True if the player has pressed spacebar.
bool playerIncreasedGravity = false;

const Config defaultConfig; // Used when unsure about any config option.
Config fileConfig( "config.txt" ), config = fileConfig;

int packageLevel = 0;

const int SCREEN_WIDTH  = 900;
const int SCREEN_HEIGHT = 700;

typedef std::map< float, std::string > HighScoreTable;
unsigned int   nHighScores;
const size_t   HANDLE_SIZE  = 4;
bool           newHighScore = false;
std::string    highScoreHandle( HANDLE_SIZE, 'x' );
HighScoreTable highScoreTable; 

enum WasPressed { Z, X, C, V, T, N_WAS_PRESSED };
bool wasPressed[N_WAS_PRESSED] = { 0, 0, 0, 0, 0 };

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
    cfg.get( "particleRatio",       &particleRatio );
    cfg.get( "predictionLength",    &Orbital::predictionLength );
    cfg.get( "predictionPrecision", &Orbital::predictionPrecision );
    cfg.get( "gravityLine",         &Orbital::gravityLine );
    cfg.get( "velocityArrow",       &Orbital::velocityArrow );
    cfg.get( "accelerationArrow",   &Orbital::accelerationArrow );
    cfg.get( "motionBlur",          &motionBlur );
    cfg.get( "scale",               &Arena::scale );
    cfg.get( "nHighScores",         &nHighScores );
    cfg.get( "fps",                 &showFrameTime );



    int volume;
    cfg.get( "music-volume", &volume );
    if( volume > 128 )
        volume = 128;
    else if( volume < 0 )
        volume = 0;
    Mix_VolumeMusic( volume );

    cfg.get( "sfx-volume", &volume );
    if( volume > 128 )
        volume = 128;
    else if( volume < 0 )
        volume = 0;
    Mix_Volume( -1, volume );
    
    std::string particleBehaviour;
    cfg.get( "particle-behaviour", &particleBehaviour );

    if( particleBehaviour == "gravity-field" )
        Particle::gravityField = true;
    else
        Particle::gravityField = false;
}


GLenum init_gl( int w, int h )
{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, w, h, 0, -10, 10 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glEnable( GL_BLEND );
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return glGetError();
}

#ifdef __WIN32
// Borrowed and modified from http://www.devmaster.net/forums/showthread.php?t=443
void set_vsync( int interval = 1 )
{
    typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
    PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

    const char *extensions = (const char*)glGetString( GL_EXTENSIONS );

    if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 ) {
        return; // Error: WGL_EXT_swap_control extension not supported on your computer.\n");
    } else {
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

        if( wglSwapIntervalEXT )
            wglSwapIntervalEXT( interval );
    }
}
#endif

bool make_sdl_gl_window( int w, int h )
{
    if( ! SDL_SetVideoMode(w, h, 32, SDL_OPENGL) )
        return false;
    init_gl( w, h );

    Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 2048 );

    font.reset( new BitmapFont );

#ifdef __WIN32
    set_vsync( 0 );
#endif

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
    SharedCActorPtr player(  new Player( Actor::vector_type(x,y) ) );
    cActors.push_back( player );

    Orbital::target = std::tr1::static_pointer_cast<Player>( player );
    Player::original = Orbital::target;

    Orbital::attractors.push_back( Orbital::target );
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

    x = random (
        int(Arena::minX + T::RADIUS*Arena::scale), 
        int(Arena::maxX - T::RADIUS*Arena::scale) 
    );

    y =  random (
        int(Arena::minY + T::RADIUS*Arena::scale),
        int(Arena::maxY - T::RADIUS*Arena::scale) 
    );

    return spawn<T>( x, y );
}

#include <fstream>
void spawn_particle( const Actor::vector_type& pos, const Actor::vector_type& v,
                     float scale, const Color& c )
{
    typedef Actor::vector_type V;

    scale = random( 0.5f, scale );

    Particle particle ( 
        pos, v, 0, 1, scale, c
    );

    particles.push_back( particle );
}

float scoreVal = 0;

std::ofstream& operator << ( std::ofstream& of, HighScoreTable& highScoreTable )
{
    typedef HighScoreTable::reverse_iterator Rit;
    for( Rit it=highScoreTable.rbegin(); it!=highScoreTable.rend(); it++ )
        of << it->second << " = " << it->first << '\n';

    return of;
}

void update_high_score()
{
    std::string tableFile = 
        gameLogic==arcade_mode? "Arcade Scores.txt" : "Chaos Scores.txt";

    // Get the old scores first.
    std::string oldVersion = "1";
    {
        std::ifstream scoresIn( tableFile );
        highScoreTable.clear();
        if( scoresIn ) 
        {
            std::string line;
            while( std::getline( scoresIn, line ) )
            {
                // Comments and whitespace shouldn't appear in this
                // file, but they shouldn't break this code either.
                rm_comments(   &line );
                rm_whitespace( &line );
                if( line.size() == 0 )
                    continue;

                Variable var = evaluate_expression( line );

                if( var.handle == "version" ) {
                    oldVersion = var.value;
                    continue;
                }

                float key;
                sstream_convert( var.value, &key );
                highScoreTable[ key ] = var.handle;
            }
        }

    }

    // Don't clobber out-dated high scores, back them up.
    int oldVersionInt;
    sstream_convert( oldVersion, &oldVersionInt );
    if( highScoreTable.size() && oldVersionInt != VERSION ) {
        std::stringstream filename;
        std::string base( tableFile.begin(), tableFile.end()-sizeof(".txt") );

        filename << base << " (" << oldVersion << ").txt";
        std::rename( tableFile.c_str(), filename.str().c_str() );
        highScoreTable.clear();
    }

    // Add the new score to the highScoreTable.
    for( size_t i=0; i < HANDLE_SIZE; i++ )
        highScoreHandle[i] = std::rand()%('z'-'a') + 'a';

    highScoreTable[ scoreVal ] = highScoreHandle;

    newHighScore = true;

    // The highScoreTable is sorted by score, so highScoreTable.begin() must be the
    // lowest. But only remove if the highScoreTable's too large.
    while( highScoreTable.size() > nHighScores ) {
        if( highScoreTable.begin()->second == highScoreHandle )
            newHighScore = false;

        highScoreTable.erase( highScoreTable.begin() );
    }

    std::ofstream out( tableFile );
    out << "version = " << VERSION << '\n';
    out << highScoreTable;
}

bool delete_me( SharedCActorPtr& actor )
{
    static Sound explosions[] = { 
        Sound("art/sfx/Explode1.wav"),
        Sound("art/sfx/Explode2.wav"),
        Sound("art/sfx/Explode3.wav"),
        Sound("art/sfx/Explode4.wav"),
    };
    static const int N_EXPLOSIONS = sizeof( explosions ) / sizeof( Sound );

    if( actor->deleteMe )
    {
        size_t newParticles = particles.size();

        // Explode.
        for( int i=0; i < std::abs(actor->mass()*particleRatio); i++ )
            spawn_particle( actor->s, actor->v/6, actor->radius()/9,
                            actor->color() );

        // Give the new particles a chance to spread out before they collide
        // with surrounding cActors.
        for( ; newParticles < particles.size(); newParticles++ )
            particles[ newParticles++ ].move( 30 );

        // Add to score if player is alive.
        if( !Orbital::target.expired() )
            scoreVal += actor->score_value();

        // If the player's what just died...
        if( actor.get() == Orbital::target.lock().get() ) 
        {
            timePlayerDied = gameTime;
            update_high_score();
        }

        explosions[ random(0, N_EXPLOSIONS) ].play();
    }
    return actor->deleteMe;
}

void reset( GameLogic logic = 0 )
{
    if( logic )
        Mix_FadeOutMusic( 200 );

    Arena::minX = Arena::minY = 3;
    Arena::maxX = SCREEN_WIDTH-3;
    Arena::maxY = SCREEN_HEIGHT-3;

    Player::SharedPlayerPtr target = Orbital::target.lock();

    newHighScore = false;

    if( logic && logic != gameLogic ) {
        gameLogic = logic;

        packageLevel = 0;

        // Before clearing the actors, make them explode.
        for( size_t i=0; i < cActors.size(); i++ )
            cActors[i]->deleteMe = true;

        // But don't kill the player!
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
}

enum Spawns { ORBITAL, STOPPER, TWISTER, NEGATIVE, N_SPAWN_SLOTS };
std::vector<Spawns> spawnSlots = {
    STOPPER, ORBITAL,
    ORBITAL, ORBITAL, ORBITAL, TWISTER,
    TWISTER, ORBITAL, TWISTER 
};

WeakCActorPtr delegate_spawn( int spawnCode )
{
    WeakCActorPtr s;

    switch( spawnCode )
    {
      case ORBITAL: s = spawn<Orbital>(); break;
      case STOPPER: s = spawn<Stopper>(); break;
      case TWISTER: s = spawn<Twister>(); break;
      case NEGATIVE: s = spawn<Negative>(); break;
      default: break;
    }

    return s;
}

WeakCActorPtr standard_spawn( const std::vector<Spawns>& slots, int maxTime )
{
    unsigned int rank = (float)slots.size() / maxTime * gameTime;
    if( rank > slots.size() )
        rank = slots.size();

    int newSpawn = slots[ random(0, rank) ];

    return delegate_spawn( newSpawn );
}

void play_song( Music& song )
{
    if( ! song.playing() && !Orbital::target.expired() )
    {
        song.fade_in( 1 * SECOND );
    }

    if( song.playing() && Orbital::target.expired() )
        Mix_FadeOutMusic( 2500 );
}

void chaos_mode( int dt )
{ 
    static Music menuSong( "art/music/Stuck Zipper.ogg" );
    play_song( menuSong );

    static std::vector<Spawns> chaosSlots = {
        ORBITAL, ORBITAL,
        ORBITAL, TWISTER, STOPPER, TWISTER,
        NEGATIVE, ORBITAL, TWISTER 
    };

    glColor3f( 1, 1, 0 );
    font->draw( "Score: " + to_string((int)scoreVal), 100, 100 );

    if( timePlayerDied && gameTime < timePlayerDied + 30*SECOND ) {
        glColor3f( 1, 1, 1 );

        font->draw( "Press r to reset, m for menu", 600, 200 );

        // Draw high scores to screen.
        TextBox b( *font, 470, 250 );
        b.writeln( "Scores stored in Chaos Scores.txt:" );
        b.writeln( "" );

        // To draw text with a gradient, keep track of these:
        float color = 1;
        float dcolor = ( 0.01 - 1 ) / highScoreTable.size();

        // Find the largest handle and score for to make nice
        // columns for printing.
        size_t largestHandleSize = 0;
        size_t largestScoreSize = 0;
        for( HighScoreTable::iterator it=highScoreTable.begin();
             it != highScoreTable.end(); it++ )
        {
            if( it->second.size() > largestHandleSize )
                largestHandleSize = it->second.size();

            unsigned int nDigits = std::log10( it->first );
            
            if( nDigits > largestScoreSize )
                largestScoreSize = nDigits;
        }

        // Use this to format the text.
        std::stringstream ss;
        ss.precision( 3 );
        ss.fill( '.' );

        const int HANDLE_WIDTH = largestHandleSize + 2;
        const int SCORE_WIDTH  = largestScoreSize  + 5;

        // Assume highScoreTable is newly initialized by update_high_score. 
        for( HighScoreTable::reverse_iterator it = highScoreTable.rbegin(); 
             it!=highScoreTable.rend(); it++ ) 
        {
            ss.str( "" );

            ss << std::setw(HANDLE_WIDTH) << std::left << it->second;

            // internal is the only iostream object that seems to work here.
            // The obvious fixed << right did not work.
            ss << std::setw(SCORE_WIDTH) << std::fixed << std::internal <<
                it->first;

            glColor3f( color, color, 0 );
            color += dcolor;

            // Let the player know his/hew awesome score.
            if( it->second == highScoreHandle ) {
                glColor3f( 1, 0, 0 );
                ss << " ** NEW HIGH SCORE";
            }

            b.writeln( ss.str() );
        }
    }

    // If the player is alive...
    if( !Orbital::target.expired() ) 
    {
        float sum = 0;
        unsigned int nEnemies = 0;
        for( size_t i=1; i < cActors.size(); i++ )
            if( cActors[i]->isActive && cActors[i]->isMovable ) {
                sum += cActors[i]->score_value();
                nEnemies++;
            }

        scoreVal += sum / 4.0 * nEnemies*nEnemies * (float(dt)/SECOND);
    }

    if( spawnWait <= gameTime ) 
    {
        if( ! Orbital::target.expired() )
        {
            // This is calibrated to be 3 seconds when gameTime=50 seconds.
            // PROOF:
            //  D = delay, T = gameTime
            //  If D = 5000 - a sqrt(T) and D(50,000) = 2:
            //      3000 = 5000 - a sqrt(50000)
            //      2000 = a sqrt(5*100*100) 
            //      2000 = 100 * a * sqrt(5)
            //      a = 20 / sqrt(5)
            spawnDelay = 5*SECOND - std::sqrt(gameTime) * (20.f/std::sqrt(5));
        }

        if( spawnDelay < 1 * SECOND )
            spawnDelay = 1 * SECOND;

        spawnWait = gameTime + spawnDelay;

        SharedCActorPtr spawn = standard_spawn( chaosSlots, 10*SECOND).lock();
        Orbital::attractors.push_back( spawn );
        spawn->isAttractor = true;
    }
}

void arcade_mode( int dt )
{
    static Music menuSong( "art/music/Stuck Zipper.ogg" );
    play_song( menuSong );

    glColor3f( 1, 1, 0 );
    font->draw( "Score: " + to_string((int)scoreVal), 100, 100 );

    if( timePlayerDied && gameTime < timePlayerDied + 30*SECOND ) 
    {
        glColor3f( 1, 1, 1 );

        font->draw( "Press r to reset, m for menu", 600, 200 );

        // Draw high scores to screen.
        TextBox b( *font, 470, 250 );
        b.writeln( "Scores stored in 'Arcade Scores.txt:'" );
        b.writeln( "" );

        // The table should include two columns, handles and scores, both
        // determining their width by their longest member. A two pass
        // algorithm is used. Pass one finds the size of the longest members,
        // the second prints them.

        // Pass one:
        size_t largestHandleSize = 0;
        size_t largestScoreSize = 0;
        for( HighScoreTable::iterator it=highScoreTable.begin();
             it != highScoreTable.end(); it++ )
        {
            if( it->second.size() > largestHandleSize )
                largestHandleSize = it->second.size();

            unsigned int nDigits = std::log10( it->first );
            
            if( nDigits > largestScoreSize )
                largestScoreSize = nDigits;
        }

        // Use this to format the text.
        std::stringstream ss;
        ss.precision( 3 );
        ss.fill( '.' );

        const int HANDLE_WIDTH = largestHandleSize + 2;
        const int SCORE_WIDTH  = largestScoreSize  + 5;

        // To draw text with a gradient, keep track of these:
        float color = 1;
        float dcolor = ( 0.01 - 1 ) / highScoreTable.size();

        // Pass 2:
        // Assume highScoreTable is newly initialized by update_high_score. 
        for( HighScoreTable::reverse_iterator it = highScoreTable.rbegin(); 
             it!=highScoreTable.rend(); it++ ) 
        {
            ss.str( "" );

            ss << std::setw(HANDLE_WIDTH) << std::left << it->second;

            // internal is the only iostream object that seems to work here.
            // The obvious fixed << right did not work.
            ss << std::setw(SCORE_WIDTH) << std::fixed << std::internal <<
                it->first;

            glColor3f( color, color, 0 );
            color += dcolor;

            // Let the player know his/hew awesome score.
            if( it->second == highScoreHandle ) {
                glColor3f( 1, 0, 0 );
                ss << " ** NEW HIGH SCORE";
            }

            b.writeln( ss.str() );
        }
    }

    // If the player is alive, increase the score.
    if( !Orbital::target.expired() ) 
    {
        float sum = 0;
        unsigned int nEnemies = 0;
        for( size_t i=1; i < cActors.size(); i++ )
            if( cActors[i]->isActive && cActors[i]->isMovable ) {
                sum += cActors[i]->score_value();
                nEnemies++;
            }

        scoreVal += sum / 4.0 * nEnemies*nEnemies * (float(dt)/SECOND);
    }

    if( spawnWait <= gameTime ) 
    {
        if( ! Orbital::target.expired() )
        {
            // This is calibrated to be 3 seconds when gameTime=50 seconds.
            // PROOF:
            //  D = delay, T = gameTime
            //  If D = 5000 - a sqrt(T) and D(50,000) = 2:
            //      3000 = 5000 - a sqrt(50000)
            //      2000 = a sqrt(5*100*100) 
            //      2000 = 100 * a * sqrt(5)
            //      a = 20 / sqrt(5)
            spawnDelay = 5*SECOND - std::sqrt(gameTime) * (20.f/std::sqrt(5));
        }

        if( spawnDelay < 1 * SECOND )
            spawnDelay = 1 * SECOND;

        spawnWait = gameTime + spawnDelay;

        standard_spawn( spawnSlots, 10*SECOND );
    }
}

void dual_mode( int )
{
    static bool chaos = false;

    if( wasPressed[T] ) {
        chaos = ! chaos;

        int offset = !Orbital::target.expired();
        for_each( cActors.begin()+offset, cActors.end(), []( CActors::value_type a ) { a->deleteMe = true; } );
    }

    Orbital::target.lock()->invinsible = true;

    TextBox intro( *font, 50, 50 );

    intro.writeln( "This is how-to-play mode." );
    intro.writeln( "Move around with WASD (like in an FPS) or the arrow keys." );
    intro.writeln( "Spawn enemies to practice dealing with them." );
    intro.writeln( "    Don't worry, you're invincible here.." );
    intro.writeln();
    intro.writeln( "Press ENTER to switch between chaos/arcade physics. (Some enemies only available in chaos mode.)" );
    intro.writeln();
    intro.writeln( "To return to the menu, press M." );
    
    TextBox spawnList( *font, 650, 50 );
    glColor3f(.5,.5, 1); spawnList.writeln( "Press Z to spawn an Orbital." );
    glColor3f(.8,.8,.8); spawnList.writeln( "Press X to spawn a Stopper." );
    glColor3f( 1,.5,.5); spawnList.writeln( "Press C to spawn a Twister." );

    if( chaos ) {
        glColor3f(.3,.9,.9); 
        spawnList.writeln( "Press V to spawn a Negative." );
    }

    typedef std::tr1::weak_ptr<CircleActor> WP;
    WP p;

    int spawnCode = -1;

    if( wasPressed[Z] )
        spawnCode = ORBITAL;
    else if( wasPressed[X] )
        spawnCode = STOPPER;
    else if( wasPressed[C] )
        spawnCode = TWISTER;
    else if( chaos )
        if( wasPressed[V] )
            spawnCode = NEGATIVE;

    if( spawnCode != -1 )
        p = delegate_spawn( spawnCode );

    std::fill( wasPressed, wasPressed+N_WAS_PRESSED, false );

    if( chaos && !p.expired() ) {
        Orbital::attractors.push_back( p );
        p.lock()->isAttractor = true;
    }
}

void package_delivery( int dt )
{
    static Music menuSong( "art/music/Magic.ogg" );

    if( ! menuSong.playing() )
    {
        menuSong.fade_in( 1 * SECOND );
        Mix_VolumeMusic( MIX_MAX_VOLUME / 2 );
    }

    static std::tr1::weak_ptr<Package> weakPackage;
    static bool started    = false; // Has the round started?
    static bool scoreSaved = false; // (At end:) Was the score saved?
    static std::string tip = "";
    static int time;

    // The conditions for winning are: (1) Be alive, (2) the package is alive,
    // (3) package reached goal. Worst case scenario is the player wins, then
    // runs into the package meaning (1) and (2) are false. We must remember if
    // the player won, instead of just testing the conditions.
    static bool playerWon = false; 

    Orbital::SharedPlayerPtr target = Orbital::target.lock();

    // Will get reinitialized if loading new level.
    std::tr1::shared_ptr<Package> package = weakPackage.lock();

    if( cActors.size() == 1 && target ) 
    {
        // Initialize.

        std::stringstream filename;
        filename << "challenge/package/level" << packageLevel;

        std::ifstream level( filename.str() );

        if( level ) 
        {
            // Construct level.
            started    = false;
            playerWon  = false;
            scoreSaved = false;
            tip  = "";
            time = 0;

            std::string val;

            // Parse file.
            while( level >> val ) 
            {
                if( val == "tip" )
                {
                    // Only the text after "tip" will go into the string.
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
            // Under increment from finishing previous level.
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
    if( tip.size() ) {
        glColor3f( 0.3, 0.8, 0.3 );
        font->draw( tip, Arena::minX + (Arena::maxX-Arena::minX)/4, 100 );
    }
        
    if( target && package && package->reachedGoal )
        playerWon = true;

    if( playerWon )
    {
        glColor3f( 0.5, 0.9, 0.7 );
        font->draw( "Press space to advance, r to try again.", 270, 150 );

        if( ! scoreSaved ) 
        {
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

            scoreSaved = true;
        }

        // Advance when the player presses spacebar.
        Uint8* keyStates = SDL_GetKeyState( 0 );
        if( keyStates[ SDLK_SPACE ] ) {
            time = 0;

            packageLevel++;
            reset();
        }
    }

    // Keep the static pointer up-to-date.
    weakPackage = package;
}

void challenge( int dt )
{
    package_delivery( dt );
}

void menu( int )
{
    static Music menuSong( "art/music/The Creep Behind.ogg" );
    if( ! menuSong.playing() )
        menuSong.fade_in( 1 * SECOND );

    if( cActors.size() == 1 )
        for( int i=0; i < 3; i++ )
            spawn<MenuOrbital>();

    if( ! playerHasMoved ) {
        glColor3f( 1, 1, 0 );

        TextBox b( *font, 270, 350 );
        b.writeln( "WASD or arrow keys to move." );

        if( ! playerIncreasedGravity )
            b.writeln( "SPACEBAR to dash." );
    } else {
        glColor3f( 0.5, 0.5, 1 );

        // Use a box for config prints.
        TextBox b( *font, 500, 350 );
        b.writeln( "Press 1 to switch on/off prediction lines.");
        b.writeln( "Press 2 to switch on/off gravity lines." );
        b.writeln( "Press 3 to switch on/off velocity arrows." );
        b.writeln( "Press 4 to switch on/off acceleration arrows." );
        b.writeln( "Press 5 to switch on/off motion blur." );
        b.writeln( "To permanently change, edit config.txt" );

        glColor3f( 1, 0.5, 0.5 );

        // Misc prints.
        font->draw( "^^ Move up here for arcade mode! ^^", 350, 50 );
        font->draw( "To the left for challenge mode. >>>", 650, 300 );
        font->draw( "<<< To the right for CHAOS mode.", 20, 300 );
        font->draw ( 
            "Press and hold S or the down arrow to enter how-to-play mode.", 
            350, Arena::maxY - 50 
        );
    }

    // Enter arcade mode when the orbital reaches the top of the screen.
    for( size_t i=0; i < cActors.size(); i++ ) {
        if( !cActors[i]->isActive )
            continue;

        if(      cActors[i]->s.y() < Arena::minX + cActors[i]->radius()  )
            reset( arcade_mode );
        else if( Arena::maxY < cActors[i]->s.y() + cActors[i]->radius() )
            reset( dual_mode );
        else if( Arena::maxX < cActors[i]->s.x() + cActors[i]->radius() )
            reset( challenge );
        else if( Arena::minX > cActors[i]->s.x() - cActors[i]->radius() )
            reset( chaos_mode );
    }
}

int main( int, char** )
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

    Orbital::birthSfx[0].load( "art/sfx/Birth.wav" );
    Orbital::birthSfx[1].load( "art/sfx/Birth1.wav" );
    Orbital::birthSfx[2].load( "art/sfx/Birth2.wav" );

    Orbital::wallSfx[0].load( "art/sfx/Hit-wall.wav" );
    Orbital::wallSfx[1].load( "art/sfx/Hit-wall1.wav" );
    Orbital::wallSfx[2].load( "art/sfx/Hit-wall2.wav" );

    Stopper::switchSfx[0].load( "art/sfx/Stopper-change.wav" );
    Stopper::switchSfx[1].load( "art/sfx/Stopper-change1.wav" );
    Stopper::switchSfx[2].load( "art/sfx/Stopper-change2.wav" );
    Stopper::switchSfx[3].load( "art/sfx/Stopper-change3.wav" );

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

                  case 'r': reset();             break;
                  case 'm': reset( menu );       break;
                  case SDLK_ESCAPE: quit = true; break;

                  case '1': 
                    // Do this in case the user updated prediction- Length or
                    // Precision.
                    fileConfig.reload( "config.txt" );

                    bool tmp;
                    if( config.get("predictionLength",&tmp), ! tmp )
                        if( fileConfig.get("predictionLength",&tmp), tmp )
                            config["predictionLength"] = 
                                fileConfig["predictionLength"];
                        else
                            config["predictionLength"] = 
                                defaultConfig["predictionLength"];
                    else
                        config["predictionLength"] = "0";

                    config["predictionPrecision"] = 
                        fileConfig["predictionPrecision"];

                    break;

#define FLIP_VALUE(handle) config[#handle] = (config[#handle]=="1")? "0" : "1"
                  case '2': FLIP_VALUE(gravityLine);   break;
                  case '3': FLIP_VALUE(velocityArrow); break;
                  case '4': FLIP_VALUE(accelerationArrow); break;

                  case '5': FLIP_VALUE(motionBlur);                         
                            if( config["motionBlur"]=="1" )
                                glAccum( GL_LOAD, 1 );
                  break;

                  case 'f': FLIP_VALUE( fps ); break;
#undef FLIP_VALUE

                  case 'w': case 'a': case 's': case 'd': 
                  case SDLK_LEFT: case SDLK_RIGHT: case SDLK_UP: case SDLK_DOWN:
                            playerHasMoved = true; break;
                  case SDLK_SPACE: playerIncreasedGravity = true; break;

                  case 'z': wasPressed[Z] = true; break;
                  case 'x': wasPressed[X] = true; break;
                  case 'c': wasPressed[C] = true; break;
                  case 'v': wasPressed[V] = true; break;
                  case 't': wasPressed[T] = true; break;

                  default: break;
                }
                break;

              default: break;
            }
		}

        gameLogic( frameTime );

        // Update cActors.
        const int DT = IDEAL_FRAME_TIME / 4;
        static int time = 0;
        // For each time-step:
        for( time += frameTime; time >= DT; time -= DT ) 
        {
            for_each ( 
                cActors.begin(), cActors.end(), 
                []( std::tr1::shared_ptr<Actor> ptr ) { ptr->move(DT); }
            );

            for( auto act1=cActors.begin(); act1+1 < cActors.end(); act1++ )
                for( auto act2=act1+1;      act2   < cActors.end(); act2++ )
                    if( collision( **act1, **act2 ) ) {
                        (*act1)->collide_with( **act2 );
                        (*act2)->collide_with( **act1 );
                    }

            cActors.erase ( 
                remove_if (
                    cActors.begin(), cActors.end(), delete_me
                ), 
                cActors.end() 
            );

            // This may seem not obvious, but it works.
            // Particle physics is by far the most CPU intensive part of this
            // whole program. When a frame lasts too long, it is probably
            // because there are too many particles on screen. We want to
            // minimize the data set as quickly as possible. To do this,
            // destabilize the physics system by making it integrate more time.
            // But when the frame time is ideal, this has no effect.
            float timeMult = 1;
            if( frameTime > IDEAL_FRAME_TIME * 2.f/3 ) 
                timeMult = (float)frameTime / (IDEAL_FRAME_TIME*2/3.f);
            timeMult *= timeMult;

            int time = DT * timeMult;

            // ALGORITHM FOR PARTICLE PHYSICS:
            //     For each particle, p:
            //         For each attractor, a:
            //             If particle is colliding with a:
            //                 Repel.
            //             else:
            //                 Attract
            //
            //         For each circle actor, c:
            //             Repel if close.
            //
            //         Move particle.
            //
            // A particle is repelled when near a circle actor or attractor.
            // Since all attractors are circle actors, the repulsion is added
            // twice. Attractors need more repulsion than non-attractors
            // so adding it twice is fine.

            for( auto part=particles.begin(); part < particles.end(); part++ )
            {
                part->a *= 0;
                part->isVisible = true;

                CActors::iterator attr = cActors.begin();
                for( ; attr < cActors.end() && (*attr)->isAttractor; attr++ )
                {
                    Vector<float,2> r = (*attr)->s - part->s;

                    float combRad = (*attr)->radius() + part->scale + 15;
                    if( ! (*attr)->isActive )
                        combRad *= 2;

                    if( magnitude(r) < combRad )
                    {
                        part->a -= magnitude (
                            r,
                            0.29f * Arena::scale
                        );

                        if( part->v * r < 0 )
                        {
                            auto u = unit( r );
                            part->v = part->v - 2 * (part->v * u) * u;
                        }
                    } else {
                        part->a += magnitude (
                            r, 
                            (*attr)->mass() * (1.f/31.f) / 
                                std::pow( magnitude( r ), 1.2f ) *
                                Arena::scale 
                        );
                    }
                }

                for( ; attr != cActors.end(); attr++ )
                {
                    // This r is the negative of the one in the above loop.
                    Vector<float,2> r = part->s - (*attr)->s;

                    float combRad = (*attr)->radius() + part->scale + 4;
                    if( ! (*attr)->isActive )
                        combRad *= 2;

                    if( magnitude(r) < combRad )
                    {
                        part->a += magnitude (
                            r,
                            0.01f * Arena::scale
                        );

                        if( part->v * r < 0 )
                        {
                            auto u = unit( r );
                            part->v = part->v - 2 * (part->v * u) * u;
                        }

                        part->isVisible = false;
                    }
                }

                part->move( time );
            }

            // Rather than erasing the particles after this loop, durring worst
            // case scenarios, this helps reduce the excess particles quickly. 
            particles.erase ( 
                remove_if (
                    particles.begin(), particles.end(),
                    []( const Particle& p )
                    {
                        // Letting the particles go a little off-screen safely
                        // gives a better "endless space!" feeling.
                        return p.s.x() < Arena::minX-400 || 
                               p.s.x() > Arena::maxX+400 || 
                               p.s.y() < Arena::minY-400 || 
                               p.s.y() > Arena::maxY+400;
                    }
                ), 
                particles.end() 
            );

            Orbital::attractors.erase (
                remove_if (
                    Orbital::attractors.begin(), Orbital::attractors.end(),
                    std::mem_fun_ref( &std::tr1::weak_ptr<CircleActor>::expired )
                ),
                Orbital::attractors.end()
            );
        }

        for( auto it=cActors.begin(); it < cActors.end() ; it++ ) {
            (*it)->draw();
        }

        for( auto it=particles.begin(); it < particles.end(); it++ )
            if( it->s.x() > Arena::minX && 
                it->s.x() < Arena::maxX && 
                it->s.y() > Arena::minY && 
                it->s.y() < Arena::maxY )
                it->draw();


        float boarder[] = {
            Arena::minX, Arena::minY,
            Arena::maxX, Arena::minY,
            Arena::maxX, Arena::maxY,
            Arena::minX, Arena::maxY
        };

        glColor3f( 1, 1, 1 );
        draw::draw( boarder, 4, GL_LINE_LOOP );
        
        glLoadIdentity();

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

        if( showFrameTime ) {
            std::stringstream ss;
            TextBox b( *font, 10, 620 );

            float val = frameTime;
            if( !frameTime )
                val = 0.5;

            ss << "fps: " << ( (float)SECOND / val );
            b.writeln( ss.str() );

            ss.str( "" );
            ss << "parts: " << particles.size();
            b.writeln( ss.str() );

            ss.str( "" );
            ss << "spawn delay: " << spawnDelay;
            b.writeln( ss.str() );

            ss.str( "" );
            ss << "time: " << gameTime / SECOND;
            b.writeln( ss.str() );
        }

        if( frameTime > MAX_FRAME_TIME )
            frameTime = MAX_FRAME_TIME;

        gameTime += frameTime;
    }

    std::for_each( Orbital::birthSfx, Orbital::birthSfx+Orbital::N_BIRTH_SFX, [](Sound& s) { s.reset(); } );
    std::for_each( Orbital::wallSfx, Orbital::wallSfx+Orbital::N_WALL_SFX,    [](Sound& s) { s.reset(); } );
    std::for_each( Stopper::switchSfx, Stopper::switchSfx+Stopper::N_SWITCHS, [](Sound& s) { s.reset(); } );
    std::for_each( Stopper::switchSfx, Stopper::switchSfx+Stopper::N_SWITCHS, [](Sound& s) { s.reset(); } );

    Mix_CloseAudio();

    while( Mix_Init(0) )
        Mix_Quit();

    SDL_Quit();
    glFlush();

    return 0;
}


