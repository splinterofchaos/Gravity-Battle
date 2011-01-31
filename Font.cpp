
#include "Font.h"

#include <GL/gl.h>

BitmapFont::BitmapFont()
#if defined( _WIN32 )
    : hdc( wglGetCurrentDC() )
#endif
{
#if defined( _WIN32 )
    SelectObject( hdc, GetStockObject(SYSTEM_FONT) );
    wglUseFontBitmaps( hdc, 0, 255, 1000 );
    base = 1000;
#else
    base = glGenLists( 256 );
    Display* display = XOpenDisplay(0);
    XFontStruct* fontInfo = 
        XLoadQueryFont ( 
            display, 
            "-adobe-helvsetica-medium-r-normal--18-*-*-*-p-*-iso8859-1" 
        );

    if( ! fontInfo ) {
        fontInfo = XLoadQueryFont( display, "fixed" );
        if( ! fontInfo ) {
            XCloseDisplay( display );
            throw;
        }
    }

    int first = fontInfo->min_char_or_byte2;
    int last  = fontInfo->max_char_or_byte2;
    glXUseXFont( fontInfo->fid, first, last-first+1, base+first );

    XFreeFont( display, fontInfo );

    XCloseDisplay( display );
#endif
}

BitmapFont::~BitmapFont()
{
    glDeleteLists( base, 255 );
}

void BitmapFont::draw( const std::string& text, float x, float y )
{
    glDisable( GL_TEXTURE_2D );

    // Alert that we're about to offset the display lists with glListBase
    glPushAttrib(GL_LIST_BIT);                  

    glRasterPos2f( x, y );
    glListBase( base );
    glCallLists( text.size(), GL_UNSIGNED_BYTE, text.c_str() );

    glEnable( GL_TEXTURE_2D );

    glPopAttrib();
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
