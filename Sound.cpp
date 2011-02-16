
#include "Sound.h"

Sound::Sound()
{
    data = 0;
}

Sound::Sound( const char* filename )
{
    load( filename );
}

void Sound::load( const char* filename )
{
    data = Mix_LoadWAV( filename );
}

void Sound::play()
{
    Mix_PlayChannel( -1, data, 0 );
}

void Sound::reset()
{
    Mix_FreeChunk( data );
    data = 0;
}

Sound::~Sound()
{
    if( data )
        reset();
}

Music::Music()
{
    song = 0;
}

Music::Music( const char* file )
{
    load( file );
}

bool Music::playing()
{
    return Mix_PlayingMusic();
}

bool Music::load( const char* file )
{
    song = Mix_LoadMUS( file );
    return song;
}

bool Music::loop()
{
    return Mix_PlayMusic( song, -1 ) != -1;
}

bool Music::fade_in( int time )
{
    return Mix_FadeInMusic( song, -1, time );
}

void Music::reset()
{
    Mix_FreeMusic( song );
    song = 0;
}

Music::~Music()
{
    if( song )
        reset();
}
