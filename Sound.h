
#pragma once

#include <SDL/SDL_mixer.h>

class Sound
{
    Mix_Chunk* data;

  public:
    Sound();
    Sound( const char* filename );

    void load( const char* filename );
    void play();

    void reset();

    ~Sound();
};

class Music
{
    Mix_Music* song;

  public:
    Music();
    Music( const char* file );

    // Observers
    bool playing();

    bool load( const char* file );
    bool loop();
    bool fade_in( int time );

    void reset();

    ~Music();
};
