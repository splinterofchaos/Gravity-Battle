
#pragma once

#ifdef _WIN32
    #include <Windows.h>
#endif

#include "CircleActor.h"
#include "Texture.h"

#include "Player.h"

class Orbital : public CircleActor
{
  protected:
    GLfloat colorIntensity;

  public:
    static const value_type RADIUS = 18;
    static const int ACTIVATION_DELAY = 2000;
    static const float BOUNCINESS = 0.8;

    static Texture  image;
    static Player*  target;
    static Player2* target2;

    static unsigned int predictionLength;
    static unsigned int predictionPrecision;
    static unsigned int gravityLine;
    static bool         velocityArrow;
    static bool         accelerationArrow;

    int activationDelay;

    Orbital( const vector_type& position, const vector_type& v );

    State on_off_screen( State state );

    virtual vector_type acceleration( const vector_type& r );
    State integrate( State state, int dt, value_type maxSpeed=0 );
    void move( int dt );

    void draw_impl( float* verts, float zRotation=0, bool extra=true );

    void draw();

    int score_value();

    value_type radius() const;
    value_type mass()   const;

    void collide_with( CircleActor& collider );

    Color color();
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

    void move( int dt );
    void draw();

    int score_value();

    Color color();
};

class Stopper : public Orbital
{
    static const unsigned int N_COLLISIONS_PER_SEC = 5;
    unsigned int timesOfCollisions[N_COLLISIONS_PER_SEC]; // In ascending order.

    static const unsigned int N_LAST_COLLIDERS = 8;
    CircleActor* lastColiders[N_LAST_COLLIDERS];
    int lastCollisionTime;

  public:
    static const value_type RADIUS = 34;
    static const value_type STOPPED_RADIUS = 29;

    Stopper( const vector_type& pos, const vector_type& v );

    vector_type acceleration( const vector_type& r );

    void move( int dt );
    void draw();

    int score_value();

    value_type radius() const;
    value_type mass()   const;

    void collide_with( CircleActor& collider );

    Color color();
};

struct Sticker : public Orbital
{
	static const float RADIUS = 10; 

	Sticker( const vector_type& pos, const vector_type& v );

	vector_type acceleration( const vector_type& r );

	value_type radius() const;

	Color color();
};
