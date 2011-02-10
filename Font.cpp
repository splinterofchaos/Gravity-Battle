
#include "Font.h"
#include "Draw.h"

#include <GL/gl.h>

#include <fstream>

BitmapFont::BitmapFont()
{
    TTF_Init();

    ttf = TTF_OpenFont( "art/font/Aapex.ttf", 54 );

    if( ! ttf )
        throw "Could not load font.";
}

BitmapFont::~BitmapFont()
{
}

void BitmapFont::draw( const std::string& text, float x, float y )
{
    SDL_Color c = { 1, 1, 1, 1 };
    SDL_Color v = { 0, 0, 0, 0 };
    SDL_Surface *Message = TTF_RenderText_Shaded(ttf, text.c_str(), c, v);

    if( ! Message )
        return;

    unsigned int tex = 0;

    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_2D, tex );

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Message->w, Message->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, Message->pixels);

    int texCoords[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    float verts[] = {
        x, y, 
        x + Message->w, y,
        x + Message->w, y + Message->h,
        x, y + Message->h
    };

    draw::draw( verts, 4, tex, texCoords );

    glDeleteTextures( 1, &tex );
    SDL_FreeSurface( Message );
}

TextBox::TextBox( BitmapFont& font, float x, float y )
    : font(font), x(x), y(y), line(0)
{
}

void TextBox::write( const std::string& text )
{
    // This algorithm REQUIRES that end initialize to 0 - 1.
    unsigned int i, end = 0 - 1;

    do {
        // This prevents i from referencing the \n char.
        i = end + 1;
        end = text.find( '\n', i );

        if( end == std::string::npos )
            end = text.size();

        writeln( text.substr(i,end) );
    } while( end < text.size() );
}

void TextBox::writeln( const std::string& text )
{
     font.draw( text, x, y );
     y += BitmapFont::LINE_HEIGHT;
}
