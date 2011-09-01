
#include "Font.h"
#include "Draw.h"

#include <GL/gl.h>

#include <fstream>

TrueTypeFont::TrueTypeFont()
{
    TTF_Init();

    ttf = TTF_OpenFont( "art/font/xlmonoalt.ttf", 20 );

    if( ! ttf )
        throw "Could not load font.";
}

TrueTypeFont::~TrueTypeFont()
{
}

void TrueTypeFont::draw( const std::string& text, float x, float y )
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, Message->w, Message->h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, Message->pixels);

    int texCoords[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    int verts[] = {
        0,          0, 
        Message->w, 0,
        Message->w, Message->h,
        0,          Message->h
    };

    glTranslatef( x, y, 0 );
    draw::draw( verts, 4, tex, texCoords );
    glLoadIdentity();

    glDeleteTextures( 1, &tex );
    SDL_FreeSurface( Message );
}

TextBox::TextBox( TrueTypeFont& font, float x, float y )
    : font(font), x(x), y(y), line(0)
{
}

void TextBox::write( const std::string& text )
{
    // This algorithm REQUIRES that end initialize to 0 - 1.
    std::string::size_type i, end = 0 - 1;

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
     y += TrueTypeFont::LINE_HEIGHT;
}

TextLine::TextLine( TrueTypeFont* f, const std::string& t, 
                    const Vector<float,2>& p )
    : ttf(f), text(t), pos(p), color(1,1,1,1)
{
    SDL_Color c = { 1, 1, 1, 1 };
    SDL_Color v = { 0, 0, 0, 0 };
    SDL_Surface* sdlTexture = TTF_RenderText_Shaded( ttf->ttf, text.c_str(), c, v );

    dims.x( sdlTexture->w );
    dims.y( sdlTexture->h );

    if( !sdlTexture )
        throw;

    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_2D, tex );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, dims.x(), dims.y(), 0, 
                  GL_ALPHA, GL_UNSIGNED_BYTE, sdlTexture->pixels );

    SDL_FreeSurface( sdlTexture );
}

void TextLine::str( const std::string& s )
{
    text = s;
    glDeleteTextures( 1, &tex );

    SDL_Color c = { 1, 1, 1, 1 };
    SDL_Color v = { 0, 0, 0, 0 };
    SDL_Surface* sdlTexture = TTF_RenderText_Shaded( ttf->ttf, text.c_str(), c, v );

    dims.x( sdlTexture->w );
    dims.y( sdlTexture->h );

    if( !sdlTexture )
        throw;

    glBindTexture( GL_TEXTURE_2D, tex );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, dims.x(), dims.y(), 0, 
                  GL_ALPHA, GL_UNSIGNED_BYTE, sdlTexture->pixels );

    SDL_FreeSurface( sdlTexture );
}

void TextLine::draw()
{
    static int texCoords[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    float verts[] = {
        0,        0, 
        dims.x(), 0,
        dims.x(), dims.y(),
        0,        dims.y()
    };

    glTranslatef( pos.x(), pos.y(), 0 );
    glColor4f( color.r(), color.g(), color.b(), color.a() );
    draw::draw( verts, 4, tex, texCoords );
    glLoadIdentity();

}

TextLine::~TextLine()
{
    glDeleteTextures( 1, &tex );
}

LinePrinter::LinePrinter( TrueTypeFont* f, const Vector<float,2>& p )
    : ttf(f), pos( p ), color(1,1,1,1), space( 0.0f ) 
{
}

void LinePrinter::add_line( const std::string& str )
{
    Vector<float, 2> p = pos;
    p.y( p.y() + space );

    lines.push_back( 
        std::shared_ptr<TextLine>( new TextLine(ttf, str, p) )
    );
    lines.back()->color = color;

    space += lines.back()->dims.y();
}
