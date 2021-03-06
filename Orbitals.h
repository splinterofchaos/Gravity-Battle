
#pragma once

#include "CircleActor.h"
#include "Texture.h"

#include "Player.h"

#include "Sound.h"

// include shared ptr.
#if defined( __GNUC__ )
    #include <memory> 
#elif defined( __MSVC__ )
    #error "Insert whatever you have to to use shared_ptr here!"
#endif

class Orbital : public CircleActor
{
  protected:
    GLfloat colorIntensity;

  public:
    typedef std::tr1::weak_ptr<CircleActor>   WeakCActorPtr;
    typedef std::tr1::shared_ptr<CircleActor> SharedCActorPtr;
    typedef std::vector< WeakCActorPtr > Attractors;
    static Attractors attractors;

    static const value_type RADIUS;
    static const int ACTIVATION_DELAY;
    static const float BOUNCINESS;

    typedef std::tr1::weak_ptr<Player>   WeakPlayerPtr;
    typedef std::tr1::shared_ptr<Player> SharedPlayerPtr;

    static Texture  image;
    static WeakPlayerPtr target;
    static WeakPlayerPtr target2;

    static unsigned int predictionLength;
    static unsigned int predictionPrecision;
    static unsigned int gravityLine;
    static bool         velocityArrow;
    static bool         accelerationArrow;

    static const int N_WALL_SFX = 3;
    static Sound wallSfx[N_WALL_SFX];

    static const int N_BIRTH_SFX = 3;
    static Sound birthSfx[ N_BIRTH_SFX ];

    // Member data
    int activationDelay;
    vector_type g; // Gravity accumulator.
    bool hitWall;

    Orbital( const vector_type& position, const vector_type& v, 
             bool playSound=true );

    State on_off_screen( State state );

    State integrate( State state, int dt, value_type maxSpeed=0 );
    void move( float dt );

    void draw_impl( float* verts, float zRotation=0, bool extra=true );

    void draw();

    int score_value();

    value_type radius() const;
    value_type mass()   const;

    void collide_with( CircleActor& collider );

    Color color();

    virtual value_type g_multiplier();
    virtual value_type g_dist( const vector_type& r );
};

class Twister : public Orbital
{
    float angleAcc;
    float angleVel;
    float angle;

  public:
    Twister( const vector_type& pos, const vector_type& v );

    State on_off_screen( State );

    vector_type acceleration( const vector_type& r );

    void move( float dt );
    void draw();

    int score_value();

    Color color();

    value_type g_dist( const vector_type& r );
    value_type g_multiplier();

    value_type mass() const;
};

class Stopper : public Orbital
{
    static const unsigned int N_COLLISIONS_PER_SEC = 5;
    unsigned int timesOfCollisions[N_COLLISIONS_PER_SEC]; // In ascending order.

    int lastCollisionTime;

  public:
    static const int N_SWITCHS = 4;
    static Sound switchSfx[N_SWITCHS];

    static const value_type RADIUS;
    static const value_type STOPPED_RADIUS;

    Stopper( const vector_type& pos, const vector_type& v );

    vector_type acceleration( const vector_type& r );

    void move( float dt );
    void draw();

    int score_value();

    value_type radius() const;
    value_type mass()   const;

    void collide_with( CircleActor& collider );

    Color color();

    value_type g_multiplier();
};

struct Negative : public Orbital
{
	static const float RADIUS; 

	Negative( const vector_type& pos, const vector_type& v );

	vector_type acceleration( const vector_type& r );

	value_type radius() const;
    value_type mass()   const;

	Color color();

    value_type g_multiplier();
};

class Greedy : public Orbital
{
  public:
    Greedy( const vector_type& pos, const vector_type& v );

    Color color();

    value_type mass() const;

    void draw();

    State integrate( State state, int dt, value_type maxSpeed=0 );
};
