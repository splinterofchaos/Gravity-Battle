
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

#include "Keyboard.h"
#include "Timer.h"

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
Timer gameTimer;

// How many particles to create proportionate to CircleActor::mass.
int particleRatio = 200; 

// Used for high score consistency. Increment when a change may effect scoring
// in any way--pretty much any game rule change at all.
const int VERSION = 10;

// SDL uses milliseconds so i will too.
const int SECOND = 1000;
int timePlayerDied = -1000;
bool showFrameTime = false;

std::ofstream loggit( "log" );
#define PANDE( cmd ) log << #cmd" ==> " << (cmd) << '\n'

bool motionBlur = false;

typedef std::tr1::shared_ptr< CircleActor > SharedCActorPtr;
typedef std::tr1::weak_ptr< CircleActor >   WeakCActorPtr;
typedef std::tr1::weak_ptr< Actor >         ActorPtr;

typedef std::vector< SharedCActorPtr > CActors;
typedef std::vector< ActorPtr > Actors;
typedef std::vector< Particle > Particles;

CActors   cActors;
Particles particles;

bool simpleParts;
bool showExtraText = true;

// Used everywhere to write text on the screen.
std::shared_ptr<TrueTypeFont> font;

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

struct Mode
{
    static void null_update(int) { }
    static void null_event()     { }

    typedef std::shared_ptr< TextLine > LinePtr;
    typedef std::vector< LinePtr >      LineList;
    LineList  lines;

    std::string name;

    typedef std::function<void(int)> UpdateFunc;
    typedef std::function<void()   > EventFunc;

    EventFunc  init;
    UpdateFunc update;
    UpdateFunc onDeath;
    UpdateFunc score;
    UpdateFunc spawn;

    bool chaos;

    Mode( const std::string& name )
        : name( name )
    {
        chaos = false;

        init    = null_event;
        update  = null_update;
        onDeath = null_update;
        score   = null_update;
        spawn   = null_update;
    }
};

struct PackageMode : public Mode
{
    std::string tip;
    std::tr1::weak_ptr<Package> weakPackage;
    bool started;    // Has the round started?
    bool scoreSaved; // (At end:) Was the score saved?
    bool playerWon;
    int time;

    PackageMode()
        : Mode( "Package" )
    {
        started = scoreSaved = false;
        tip = "";
    }
};

Mode* mode = 0;

// FUNCTIONS //
void arcade_init();
void menu_init();
void package_init();
void training_init();

void arcade_mode( int dt );
void training_mode( int dt );
void menu( int dt );
void package_delivery( int dt );
void chaos_mode( int dt );

void on_death( int dt );
void score( int dt );

void arcade_spawn( int dt );
void chaos_spawn( int dt );
void training_spawn( int dt );

Mode arcadeMode( "Arcade" );
Mode trainingMode( "Training" );
Mode menuMode( "Menu" );
Mode chaosMode( "Chaos" );
PackageMode packageMode;

void initialize_modes()
{
    arcadeMode.init    = arcade_init;
    arcadeMode.update  = arcade_mode;
    arcadeMode.onDeath = on_death;
    arcadeMode.score   = score;
    arcadeMode.spawn   = arcade_spawn;

    chaosMode.init    = arcade_init;
    chaosMode.update  = chaos_mode;
    chaosMode.onDeath = on_death;
    chaosMode.score   = score;
    chaosMode.spawn   = chaos_spawn;
    chaosMode.chaos   = true;

    trainingMode.init   = training_init;
    trainingMode.update = training_mode;
    trainingMode.spawn  = training_spawn;

    menuMode.init   = menu_init;
    menuMode.update = menu;

    packageMode.init   = package_init;
    packageMode.update = package_delivery;
}

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
    cfg.get( "particle-ratio",       &particleRatio );
    cfg.get( "prediction-length",    &Orbital::predictionLength );
    cfg.get( "prediction-precision", &Orbital::predictionPrecision );
    cfg.get( "gravity-line",         &Orbital::gravityLine );
    cfg.get( "velocity-arrow",       &Orbital::velocityArrow );
    cfg.get( "acceleration-arrow",   &Orbital::accelerationArrow );
    cfg.get( "motion-blur",          &motionBlur );
    cfg.get( "nHighScores",          &nHighScores );
    cfg.get( "fps",                  &showFrameTime );
    cfg.get( "particle-size",        &Particle::sizeMult );

    float opacityTmp;
    cfg.get( "particle-opacity", &opacityTmp );
    Particle::opacity = opacityTmp / 100.f;

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

    if( particleBehaviour == "gravity-field" ) {
        Particle::gravityField = true;
        simpleParts = false;
    } else if( particleBehaviour == "simple" ) {
        Particle::gravityField = false;
        simpleParts = true;
    } else {
        Particle::gravityField = false;
        simpleParts = false;
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

bool resize_window( float w_, float h_ )
{
    float w = w_, h = h_;
    float ratio = (float)SCREEN_HEIGHT / SCREEN_WIDTH;

    if( !SDL_SetVideoMode( w, h, 32, SDL_OPENGL|SDL_RESIZABLE ) )
        return false;

    if( w*ratio > h ) 
        // h is the limiting factor.
        w = h / ratio;
    else
        h = w * ratio;

    float wOff = ( w_ - w ) / 2;
    float hOff = ( h_ - h );

    glViewport( wOff, hOff, w, h );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -10, 10 );
    glMatrixMode(GL_MODELVIEW);

    return true;
}

bool make_sdl_gl_window( int w, int h )
{
    if( ! resize_window(w,h) )
        return false;
    init_gl( w, h );

    Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 2048 );

    font.reset( new TrueTypeFont );

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
        mode->name + " Scores.txt";

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

    // Now construct the high scores for display.
    Vector<int,2> pos( 250, 370 );
    mode->lines.push_back( Mode::LinePtr (
            new TextLine( font.get(), "Scores stored in '" + mode->name + 
                          " Scores.txt'", pos )
        )
    );
    pos.y( pos.y() + 30 );

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

        // Let the player know his/hew awesome score.
        if( it->second == highScoreHandle )
            ss << " ** NEW HIGH SCORE";

        pos.y( pos.y() + 15 );
        mode->lines.push_back ( 
            Mode::LinePtr (
                new TextLine( font.get(), ss.str(), pos )
            )
        );

        mode->lines.back()->color = Color( color, color, 0 );
        color += dcolor;

        if( it->second == highScoreHandle ) 
            mode->lines.back()->color = Color( 1, 0, 0 );
    }

    // Update the file.
    std::ofstream out( tableFile );
    out << "version = " << VERSION << '\n';
    out << highScoreTable;
}

// This is called for each actor and returns true if it should be deleted.
// It may seem unclean, but it also handles all on-death events.
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
        particles.size();

        // Explode.
        for( int i=0; i < std::abs(actor->mass()*particleRatio); i++ )
            spawn_particle( actor->s, actor->v/6, actor->radius()/9,
                            actor->color() );


        // Add to score if player is alive.
        if( !Orbital::target.expired() )
            scoreVal += actor->score_value();

        // If the player's what just died...
        if( actor.get() == Orbital::target.lock().get() ) 
        {
            timePlayerDied = gameTimer.time_ms();
            update_high_score();
        }

        // Let it be heard.
        explosions[ random(0, N_EXPLOSIONS) ].play();
    }

    return actor->deleteMe;
}

// Resets whatever mode the game is in. 
// If no mode is given, it resets the current mode.
void reset( Mode* newMode = 0 )
{
    if( mode )
        mode->lines.clear();

    if( newMode )
        Mix_FadeOutMusic( 200 );

    Arena::minX = Arena::minY = 3;
    Arena::maxX = SCREEN_WIDTH-3;
    Arena::maxY = SCREEN_HEIGHT-3;

    Player::SharedPlayerPtr target = Orbital::target.lock();

    newHighScore = false;

    // If we're switching to a different mode...
    if( newMode && newMode != mode ) {
        mode = newMode;

        // Erase package mode progress, if any.
        packageLevel = 0;

        // Before clearing the actors, make them explode.
        for( size_t i=0; i < cActors.size(); i++ )
            cActors[i]->deleteMe = true;

        // But don't explode the player!
        if( target )
            target->deleteMe = false;

        for_each( cActors.begin(), cActors.end(), delete_me );
    } else {
        // Normal resets clear the screen.
        particles.clear();
    }

    // We need to clear cActors, but keep the player. We'll clear it
    // completely, then respawn the player in the same place so the user
    // doesn't notice..

    Actor::vector_type playerPos( 350, 300 );
    if( target )
        playerPos = target->s;

    cActors.clear();

    spawn_player( playerPos.x(), playerPos.y() );

    gameTimer.reset();

    spawnDelay = 6000;
    spawnWait  = 30;
    timePlayerDied = 0;

    scoreVal = 0;

    mode->init();
}

enum Spawns { ORBITAL, STOPPER, TWISTER, NEGATIVE, GREEDY, N_SPAWN_SLOTS=9 };
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
      case ORBITAL:  s = spawn<Orbital>(); break;
      case STOPPER:  s = spawn<Stopper>(); break;
      case TWISTER:  s = spawn<Twister>(); break;
      case NEGATIVE: s = spawn<Negative>(); break;
      case GREEDY :  s = spawn<Greedy>();  break;
      default: break; // Should never be reached.
    }

    return s;
}

WeakCActorPtr standard_spawn( const std::vector<Spawns>& slots, int maxTime )
{
    unsigned int rank = (float)slots.size() / maxTime * gameTimer.time_ms();
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

void chaos_spawn( int dt )
{
    static std::vector<Spawns> chaosSlots = {
        ORBITAL, TWISTER, 
        NEGATIVE, ORBITAL, STOPPER, GREEDY,
        NEGATIVE, TWISTER, ORBITAL, TWISTER 
    };

    // Time to spawn a new enemy?
    spawnWait -= dt;
    if( spawnWait < 0 ) 
    {
        // This is calibrated to be 3 seconds when gameTime=50 seconds.
        // PROOF:
        //  D = delay, T = gameTime
        //  If D = 5000 - a sqrt(T) and D(50,000) = 2000:
        //      3000 = 5000 - a sqrt(50000) 
        //      2000 = a sqrt(5*100*100)
        //      2000 = 100 * a * sqrt(5)
        //      a = 20 / sqrt(5)
        spawnDelay = 
            5*SECOND - std::sqrt(gameTimer.time_ms()) * (20.f/std::sqrt(5));
        if( spawnDelay < 1 * SECOND )
            spawnDelay = 1 * SECOND;

        spawnWait = spawnDelay;

        SharedCActorPtr spawn = standard_spawn( chaosSlots, 40*SECOND).lock();
        Orbital::attractors.push_back( spawn );
        spawn->isAttractor = true;
    }
}

void chaos_mode( int dt )
{ 
    static Music menuSong( "art/music/Stuck Zipper.ogg" );
    static int lastScore = 0;

    play_song( menuSong );

    if( (int)scoreVal != lastScore )
    {
        mode->lines[0].reset ( 
            new TextLine( font.get(), "Score: " + to_string((int)scoreVal),
                          vector(100,100) ) 
        );

        lastScore = scoreVal;
    }

    mode->lines[0]->color = Color( 0.8, 0.8, 0 );
}

void score( int dt )
{
    float sum = 0;
    unsigned int nEnemies = 0;
    unsigned int nActive = 0;
    for( size_t i=1; i < cActors.size(); i++ ) 
    {
        if( cActors[i]->isActive )
        {
            nActive++;
            if( cActors[i]->isMovable ) 
            {
                sum += cActors[i]->score_value();
                nEnemies++;
            }
        }
    }

    scoreVal += sum / 4.0 * nEnemies*nEnemies * (float(dt)/SECOND);
}

void arcade_spawn( int dt )
{
    int nMoving = 0;
    for( size_t i=1; i < cActors.size(); i++ ) 
        nMoving += cActors[i]->isActive && cActors[i]->isMovable;

    // If cActors has only active enemies + the player 
    // and there's one or zero enemies...
    if( nMoving <= 1 && cActors.back()->isActive )
    {
        enum SpawnPoints {
            ORBITAL = 2,
            STOPPER = 1,
            TWISTER = 3
        };

        // An arbitrary equation. Not very tested.
        int points = std::sqrt(scoreVal) / 5.f + 3.f;

        int orbitalChance = 1.3f * scoreVal + 1;
        int stopperChance = 1.0f * scoreVal + 3;
        int twisterChance = 2.0f * scoreVal - 250*2;

        if( twisterChance < 0 )
            twisterChance = 0;

        int sum = orbitalChance + stopperChance + twisterChance;

        while( points > 0 )
        {
            int pick = random( 0, sum );
            Spawns code = N_SPAWN_SLOTS;

            if( pick <= orbitalChance ) {
                code    =      Spawns::ORBITAL;
                points -= SpawnPoints::ORBITAL;
            } else if( pick <= stopperChance+orbitalChance ) {
                code    =      Spawns::STOPPER;
                points -= SpawnPoints::STOPPER;
            } else {
                code    =      Spawns::TWISTER;
                points -= SpawnPoints::TWISTER;
            }

            delegate_spawn( code );
        }

    }
}

void arcade_init()
{
    Mode::LinePtr line ( 
        new TextLine( font.get(), "Score: 0",
                      vector(100,100) )
    );
    mode->lines.push_back( line );
}

void arcade_mode( int dt )
{
    static Music menuSong( "art/music/Stuck Zipper.ogg" );
    static int lastScore = 0;

    play_song( menuSong );

    if( (int)scoreVal != lastScore )
    {
        mode->lines[0].reset ( 
            new TextLine( font.get(), "Score: " + to_string((int)scoreVal),
                          vector(100,100) ) 
        );

        lastScore = scoreVal;

        mode->lines[0]->color = Color( 0.8, 0.8, 0 );
    }
}


void on_death( int dt )
{
    if( gameTimer.time_ms() < timePlayerDied + 30*SECOND ) 
    {
        glColor3f( 1, 1, 1 );

        font->draw( "Press r to reset, m for menu", 600, 200 );
    }
    else
    {
        for_each ( 
            mode->lines.begin(), mode->lines.end(), 
            // Lower alpha value.
            [](Mode::LineList::value_type& linePtr){ linePtr->color[3] -= 0.001; } 
        );
    }
}

void training_init()
{
    
    if( showExtraText ) 
    {
        LinePrinter intro( font.get(), vector(50,50) );

        intro.add_line( "This is how-to-play mode." );
        intro.add_line( "Move around with WASD (like in an FPS) or the arrow keys." );
        intro.add_line( "Spawn enemies to practice dealing with them." );
        intro.add_line( "    Don't worry, you're invincible here.." );
        intro.add_line( " " );
        intro.add_line( "Press ENTER to switch between chaos/arcade physics. (Some enemies only available in chaos mode.)" );
        intro.add_line( " " );
        intro.add_line( "To return to the menu, press M." );

        LinePrinter spawnList( font.get(), vector(650,50) );

        spawnList.add_line( "Press Z to spawn an Orbital." );
        spawnList.lines.back()->color = Color( 0.4f, 0.4f, 1.0f );

        spawnList.add_line( "Press X to spawn a Stopper." );
        spawnList.lines.back()->color = Color( 0.7f, 0.7f, 0.7f );

        spawnList.add_line( "Press C to spawn a Twister." );
        spawnList.lines.back()->color = Color( 1.0f, 0.1f, 0.1f );

        spawnList.add_line( "Press V to spawn a Negative. (chaos mode)" );
        spawnList.lines.back()->color = Color( 0.3f, 1.0f, 1.0f );

        spawnList.add_line( "Press B to spawn a Greedy.   (chaos mode)" );
        spawnList.lines.back()->color = Color( 1.0f, 0.0f, 1.0f );

        mode->lines.insert( mode->lines.end(), intro.lines.begin(), intro.lines.end() );
        mode->lines.insert( mode->lines.end(), spawnList.lines.begin(), spawnList.lines.end() );
    }
}

void training_mode( int )
{
    // This is a sandbox mode where the player can't die and s/he can manually
    // spawn any enemy in the game.

    // These colors correlate, somewhat, to the colors of the enemies.
    static Color colors[ N_SPAWN_SLOTS ];
    colors[ ORBITAL  ] = Color( 0.4f, 0.4f, 1.0f );
    colors[ STOPPER  ] = Color( 0.7f, 0.7f, 0.7f );
    colors[ TWISTER  ] = Color( 1.0f, 0.1f, 0.1f );
    colors[ NEGATIVE ] = Color( 0.3f, 1.0f, 1.0f );
    colors[ GREEDY   ] = Color( 1.0f, 0.0f, 1.0f );

    // Swap the mode when enter is pressed.
    if( Keyboard::key_state('\r') ) {
        mode->chaos = ! mode->chaos;

        int offset = !Orbital::target.expired();
        for_each( cActors.begin()+offset, cActors.end(), []( CActors::value_type a ) { a->deleteMe = true; } );
    }

    Orbital::target.lock()->invinsible = true;

}

void training_spawn( int dt )
{
    // Tips will appear on the bottom of the screen after an enemy is spawned. 
    static const char* tips[ N_SPAWN_SLOTS ];
    tips[ ORBITAL  ] = "Orbitals are the most common spawns, and most enemies share at lease something in common with them.";
    tips[ TWISTER  ] = "While an orbital's motion is circular, the twister's motion is elliptical, like the earth around the sun.";
    tips[ NEGATIVE ] = "A negative is attracted to you, like an orbital, but repels all other enemies. It's mass is negative.";
    tips[ GREEDY   ] = "A greedy will only orbit the player, ignoring everything else.";
    tips[ STOPPER  ] = "Stoppers are slow, big, though light orbitals. When hit, one will stop; if it again, it will move again."
                       "\nThe only way to kill a stopper is to run into it while it's stopped.";

    // A newly spawned enemy.
    std::tr1::weak_ptr<CircleActor> p;

    int spawnCode = -1;
    static int recentSpawn = -1;

    if( Keyboard::key_state('z') )
        spawnCode = ORBITAL;
    else if( Keyboard::key_state('x') )
        spawnCode = STOPPER;
    else if( Keyboard::key_state('c') )
        spawnCode = TWISTER;
    else if( mode->chaos ) {
        if( Keyboard::key_state('v') )
            spawnCode = NEGATIVE;
        else if( Keyboard::key_state('b') )
            spawnCode = GREEDY;
    }

    if( spawnCode != -1 ) {
        p = delegate_spawn( spawnCode );
        recentSpawn = spawnCode;
    }

    if( recentSpawn != -1 ) {
        glColor3f( 1, 1, 1 );

        // Textbox::write offers newline detection. Font::draw does not.
        TextBox b( *font, 150, 650 );
        b.write( tips[recentSpawn] );
    }

    if( mode->chaos && !p.expired() ) {
        Orbital::attractors.push_back( p );
        p.lock()->isAttractor = true;
    }
}

void package_init()
{
    std::stringstream filename;
    filename << "challenge/package/level" << packageLevel;

    std::ifstream level( filename.str() );

    Orbital::SharedPlayerPtr target = Orbital::target.lock();

    if( level ) 
    {
        // Construct level.
        packageMode.started    = false;
        packageMode.playerWon  = false;
        packageMode.scoreSaved = false;
        packageMode.time = 0;
        packageMode.tip  = "";

        std::string val;

        // Parse file.
        while( level >> val ) 
        {
            if( val == "tip" )
            {
                // Only the text after "tip" will go into the string.
                std::getline( level, packageMode.tip );
            }
            else
            {
                float x,y;
                level >> x >> y;

                if( val == "player" ) 
                    target->s = Actor::vector_type( x, y );
                else if( val == "package" )
                    packageMode.weakPackage = spawn<Package>( x, y ).lock();
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
        } // End parse file.

        Vector<float,2> pos = vector( Arena::minX + (Arena::maxX-Arena::minX)/4, 
                                      100 );
        Mode::LinePtr tip ( 
            new TextLine( font.get(), packageMode.tip, pos )
        );
        tip->color = Color( 0.3f, 0.8f, 0.3f, 1.f );

        pos = vector( 270, 150 );
        Mode::LinePtr deathMessage (
            new TextLine( font.get(), 
                          "Press space to advance, r to try again", pos )
        );
        deathMessage->color = Color( 0.5f, 0.9f, 0.7f, 0.f );

        mode->lines.push_back( tip );
        mode->lines.push_back( deathMessage );
    }
    else 
    {
        // Under increment from finishing previous level.
        packageLevel--;
        package_init();
    }
}

void package_delivery( int dt )
{
    // In this mode, the player must guide the "package" to the "goal".

    static Music menuSong( "art/music/Magic.ogg" );

    if( ! menuSong.playing() )
    {
        menuSong.fade_in( 1 * SECOND );
        Mix_VolumeMusic( MIX_MAX_VOLUME / 2 );
    }

    Orbital::SharedPlayerPtr target = Orbital::target.lock();

    // Will get reinitialized if loading new level.
    std::tr1::shared_ptr<Package> package = packageMode.weakPackage.lock();

    if( !packageMode.started && package && package->started ) {
        packageMode.started = true;
    }

    if( packageMode.started )
        packageMode.time += dt;
        
    // The conditions for winning are: (1) Be alive, (2) the package is alive,
    // (3) package reached goal. Worst case scenario is the player wins, then
    // runs into the package meaning (1) and (2) are false. We must remember if
    // the player won, instead of just testing the conditions.
    if( target && package && package->reachedGoal )
        packageMode.playerWon = true;

    if( packageMode.playerWon )
    {
        mode->lines[1]->color.a( 1 );

        if( ! packageMode.scoreSaved ) 
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

            if( previousTime < packageMode.time ) {
                std::ofstream score( filename.str() );
                score << float(packageMode.time) / SECOND;
            }

            packageMode.scoreSaved = true;
        }

        // Advance when the player presses spacebar.
        Uint8* keyStates = SDL_GetKeyState( 0 );
        if( keyStates[ SDLK_SPACE ] ) {
            packageMode.time = 0;

            packageLevel++;
            reset();
        }
    }

    // Keep the static pointer up-to-date.
    packageMode.weakPackage = package;
}

void challenge( int dt )
{
    package_delivery( dt );
}

void menu_init()
{
    // menuMode.lines[1] - [3]: Into code.

    LinePrinter intro(   font.get(), vector(270,350) );
    LinePrinter options( font.get(), vector(500,350) );

    intro.color = Color( 1, 1, 0 );
    options.color = Color( 0.5f, 0.5f, 1.f );

    intro.add_line( "WASD or arrow keys to move." );
    intro.add_line( "SPACEBAR to dash." );
    intro.add_line( "Press P to pause." );

    options.add_line( "Press 1 to switch on/off prediction lines.");
    options.add_line( "Press 2 to switch on/off gravity lines." );
    options.add_line( "Press 3 to switch on/off velocity arrows." );
    options.add_line( "Press 4 to switch on/off acceleration arrows." );
    options.add_line( "Press 5 to switch on/off motion blur." );
    options.add_line( "To permanently change, edit config.txt" );
    options.add_line( " " );
    options.add_line( "Press E to hide this text." );

    mode->lines.insert( mode->lines.end(), intro.lines.begin(), intro.lines.end() );
    mode->lines.insert( mode->lines.end(), options.lines.begin(), options.lines.end() );


    Color c( 1.f, 0.5f, 0.5f );

    Mode::LinePtr ptr ( 
        new TextLine( font.get(), "^^ Move up here for arcade mode! ^^", vector(350,50) ) 
    );
    ptr->color = c;
    mode->lines.push_back( ptr);

    ptr.reset( 
        new TextLine( font.get(), "To the left for challenge mode. >>>", vector(650,300) )
    );
    ptr->color = c;
    mode->lines.push_back( ptr);

    ptr.reset( 
        new TextLine( font.get(), "<<< To the right for CHAOS mode.", vector(20,300) )
    );
    ptr->color = c;
    mode->lines.push_back( ptr);

    ptr.reset( 
        new TextLine( font.get(), "Press and hold S or the DOWN ARROW to enter training mode.", vector(350,650) )
    );
    ptr->color = c;
    mode->lines.push_back( ptr);
}

void menu( int )
{
    static Music menuSong( "art/music/The Creep Behind.ogg" );
    if( ! menuSong.playing() )
        menuSong.fade_in( 1 * SECOND );

    if( cActors.size() == 1 )
        for( int i=0; i < 3; i++ )
            spawn<MenuOrbital>();

    if( !playerHasMoved ) 
    {
        for( auto it=mode->lines.begin()+3; it < mode->lines.end(); it++ )
            (*it)->color.a( 0 );
    }
    else
    {
        // Darken the intro text.
        for( int i=0; i < 3; i++ )
            mode->lines[i]->color[3] -= 0.001;

        // Brighten the rest.
        for( auto it=mode->lines.begin()+3; it < mode->lines.end(); it++ )
            (*it)->color[3] += 0.001;

        if( showExtraText ) 
        {
            // Use a box for config prints.
        }
    }

    // Enter the next mode when the orbital reaches the edge of the screen.
    for( size_t i=0; i < cActors.size(); i++ ) {
        if( !cActors[i]->isActive )
            continue;

        if(      cActors[i]->s.y() < Arena::minX + cActors[i]->radius() )
            reset( &arcadeMode );
        else if( Arena::maxY < cActors[i]->s.y() + cActors[i]->radius() )
            reset( &trainingMode );
        else if( Arena::maxX < cActors[i]->s.x() + cActors[i]->radius() )
            reset( &packageMode );
        else if( Arena::minX > cActors[i]->s.x() - cActors[i]->radius() )
            reset( &chaosMode );
    }
}

void load_resources()
{
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
}

void destroy_resources()
{
    auto f = []( Sound& s ) { s.reset(); };
    std::for_each( Orbital::birthSfx,  Orbital::birthSfx+Orbital::N_BIRTH_SFX, f );
    std::for_each( Orbital::wallSfx,   Orbital::wallSfx+Orbital::N_WALL_SFX,   f );
    std::for_each( Stopper::switchSfx, Stopper::switchSfx+Stopper::N_SWITCHS,  f );

    Player::body.reset();
    Player::shield.reset();
    Orbital::image.reset();
}

void keyboard_events()
{
    if( Keyboard::key_state('r') )
        reset();
    if( Keyboard::key_state('m') )
        reset( &menuMode );

    if( Keyboard::key_state('1') ) 
    {
        // Do this in case the user updated prediction- length or
        // precision.
        fileConfig.reload( "config.txt" );

        bool tmp;
        if( config.get("prediction-length",&tmp), ! tmp )
            if( fileConfig.get("prediction-length",&tmp), tmp )
                config["prediction-length"] = 
                    fileConfig["prediction-length"];
            else
                config["prediction-length"] = 
                    defaultConfig["prediction-length"];
        else
            config["prediction-length"] = "0";

        config["prediction-precision"] = 
            fileConfig["prediction-precision"];
    }

#define FLIP_VALUE(handle) config[#handle] = (config[#handle]=="1")? "0" : "1"
    if( Keyboard::key_state('2') )
        FLIP_VALUE(gravity-line);
    if( Keyboard::key_state('3') )
        FLIP_VALUE(velocity-arrow);
    if( Keyboard::key_state('4') )
        FLIP_VALUE(acceleration-arrow);
    if( Keyboard::key_state('5') )
    {
        FLIP_VALUE(motion-blur);                         
        if( config["motion-blur"]=="1" )
            glAccum( GL_LOAD, 1 );
    }

    if( Keyboard::key_state('f') )
        FLIP_VALUE( fps );
#undef FLIP_VALUE

    int motionKeys[] = { 'w', 'a', 's', 'd', Keyboard::RIGHT, Keyboard::LEFT, Keyboard::UP, Keyboard::DOWN, 0 };
    for( int* it=motionKeys; *it != 0; it++ )
        if( Keyboard::key_down( *it ) )
            playerHasMoved = true;

    if( Keyboard::key_down(' ') )
        playerIncreasedGravity = true;

    if( Keyboard::key_state('e') )
        showExtraText = !showExtraText;
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

    load_resources();

    initialize_modes();

    reset( &menuMode ); 

    Timer frameTimer;
    while( quit == false )
    {
        Keyboard::update();

        static SDL_Event event;
		while( SDL_PollEvent(&event) )
		{
            switch( event.type ) 
            {
              case SDL_QUIT: quit = true; break;

              case SDL_KEYDOWN:
                Keyboard::add_key_status( event.key.keysym.sym, Keyboard::PRESSED ); break;

              case SDL_VIDEORESIZE:
                float w=event.resize.w, h=event.resize.h;
                resize_window( w, h );
                break;
            }
        }

        keyboard_events();

        // These lines use local vars thus couldn't be in keyboard_events.
        if( Keyboard::key_state('p') )
            paused = ! paused;
        if( Keyboard::key_state( Keyboard::ESQ ) )
            quit = true;

        float DT = IDEAL_FRAME_TIME * ( 1.f / 4.f );
        if( timePlayerDied && gameTimer.time_ms() < timePlayerDied + 6*SECOND )
            DT /= 2;

        mode->update( frameTimer.time_ms() );
        mode->spawn(  frameTimer.time_ms() );

        if( !timePlayerDied ) {
            mode->score( frameTimer.time_ms() );
        } else {
            mode->onDeath( frameTimer.time_ms() );
        }

        // For each time-step:
        static int time = 0;
        for( time += frameTimer.time_ms(); !paused && time >= DT; time -= DT ) 
        {
            for_each ( 
                cActors.begin(), cActors.end(), 
                [DT]( std::tr1::shared_ptr<Actor> ptr ) { ptr->move(DT); }
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
            if( frameTimer.time_ms() > IDEAL_FRAME_TIME * 2.f/3 ) 
                timeMult = (float)frameTimer.time_ms() / (IDEAL_FRAME_TIME*2/3.f);
            timeMult *= timeMult;

            float time = DT * timeMult;

            // ALGORITHM FOR PARTICLE PHYSICS:
            //     For each particle, p:
            //         For each attractor, a:
            //             If p is colliding with a:
            //                 Repel.
            //             else:
            //                 Attract
            //
            //         For each circle actor, c:
            //             Repel if p is close.
            //
            //         Move p.

            for( auto part=particles.begin(); part < particles.end(); part++ )
            {
                part->a *= 0;
                part->isVisible = true;

                auto repel = []( Particles::iterator p, const Vector<float,2>& r, float mult )
                {
                    p->a -= magnitude( r, mult * random(0.8f, 1.2f) );

                    if( p->v * r < 0 ) {
                        auto u = unit( r );
                        p->v = p->v - 2 * (p->v*u) * u;
                    }
                };

                // When parts aren't simple (default), do calculations.
                if( ! simpleParts )
                {
                    // Integration.
                    CActors::iterator attr = cActors.begin();
                    for( ; attr < cActors.end() && (*attr)->isAttractor; attr++ )
                    {
                        Vector<float,2> r = (*attr)->s - part->s;

                        float combRad = (*attr)->radius() + part->scale + 15;
                        if( ! (*attr)->isActive )
                            combRad *= 2;

                        if( magnitude(r) < combRad )
                        {
                            repel( part, r, 0.29f * 0.6f );
                        } else {
                            part->a += magnitude (
                                r, 
                                (*attr)->mass() * (1.f/31.f) / 
                                std::pow( magnitude( r ), 1.2f ) *
                                Arena::scale 
                            );
                        }
                    }

                    // Collision.
                    for( ; attr != cActors.end(); attr++ )
                    {
                        // This r is the negative of the one in the above loop.
                        Vector<float,2> r = part->s - (*attr)->s;

                        float combRad = (*attr)->radius() + part->scale + 4;
                        if( ! (*attr)->isActive )
                            combRad *= 2;

                        if( magnitude(r) < combRad )
                        {
                            repel( part, r, -0.01f * 0.6f );
                        }
                    }
                } // if not simple parts

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

        int partsDrawn = 0;

        static Timer realTimer;
        realTimer.update();
        static int lastUpdate = realTimer.time_ms();
        if( IDEAL_FRAME_TIME < realTimer.time_ms() ) 
        {
            realTimer.zero();

            if( ! paused )
                for( auto it=cActors.begin(); it < cActors.end() ; it++ ) 
                    (*it)->draw();

            static int texCoords[] = {
                0, 0,
                1, 0,
                1, 1, 
                0, 1
            };

            glEnable( GL_TEXTURE_2D );

            glEnableClientState( GL_TEXTURE_COORD_ARRAY );

            glBindTexture( GL_TEXTURE_2D, 1 );
            glTexCoordPointer( 2, GL_INT, 0, texCoords );
            for( auto it=particles.begin(); it < particles.end(); it++ )
            {

                if( it->s.x() > Arena::minX && 
                    it->s.x() < Arena::maxX && 
                    it->s.y() > Arena::minY && 
                    it->s.y() < Arena::maxY )
                {

                    it->draw();
                    partsDrawn++;
                }
            }

            for( auto it=mode->lines.begin(); it < mode->lines.end(); it++ )
                (*it)->draw();

            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            glDisable( GL_TEXTURE_2D );

            if( paused ) {
                glColor3f( 1, 1, 1 );
                TextBox b( *font, 200, 250 );
                b.writeln( "Thank you for playing ORBITAL CHAOS." );
                b.writeln( "I hope you enjoy. It is my first game." );
                b.writeln( "Please refer questions, comments, etc., to my email hakusa@gmail.com." );
                b.writeln( "The source code is available at https://github.com/splinterofchaos/Gravity-Battle" );
            }

            float boarder[] = {
                Arena::minX, Arena::minY,
                Arena::maxX, Arena::minY,
                Arena::maxX, Arena::maxY,
                Arena::minX, Arena::maxY
            };

            glColor3f( 1, 1, 1 );
            draw::draw( boarder, 4, GL_LINE_LOOP );

            glLoadIdentity();

            if( showFrameTime ) 
            {
                std::stringstream ss;
                TextBox b( *font, 10, 600 );

                float val = frameTimer.time_sec();
                if( !val )
                    val = 0.5;

                ss << "fps: " << ( 1.f / val );
                b.writeln( ss.str() );

                ss.str( "" );
                ss << "parts: " << particles.size();
                b.writeln( ss.str() );

                ss.str( "" );
                ss << "parts on screen: " << partsDrawn;
                b.writeln( ss.str() );

                ss.str( "" );
                ss << "spawn delay: " << spawnDelay;
                b.writeln( ss.str() );

                ss.str( "" );
                ss << "time: " << gameTimer.time_sec();
                b.writeln( ss.str() );
            }

            configure( config );
            update_screen();
        }
        else
        {
            SDL_Delay( 5 );
        }
        
        if( paused )
            frameTimer.zero();

        frameTimer.reset();
        frameTimer.clamp_ms( MAX_FRAME_TIME );

        float mult = 1.f; // Frametime multiplier.

        float timeAfter = float( gameTimer.time_ms() - timePlayerDied ) / (float)SECOND;
        if( timePlayerDied ) {
            if( timeAfter <= 4.f ) 
                mult = 0.1f;
            else if( timeAfter > 2 && timeAfter < 9 )
                // Smoothly return to normal time.
                mult = std::sqrt( (timeAfter-4) / (9.f-4.f) );
        }


        frameTimer.clamp_ms( frameTimer.time_ms() * mult );

        gameTimer.update();
    }

    destroy_resources();

    Mix_CloseAudio();

    while( Mix_Init(0) )
        Mix_Quit();

    glFinish();
    SDL_Quit();

    return 0;
}


