
#include <string>

#include <SDL/SDL_ttf.h>


class BitmapFont
{
    TTF_Font* ttf;

#if defined( _WIN32 )
    HDC hdc;
#else
#endif

  public:
    BitmapFont();
    ~BitmapFont();

    void draw( const std::string& text, float x, float y );

    static const int LINE_HEIGHT = 20;
};

class TextBox
{
    BitmapFont& font;

    float x, y; // Base position.
    unsigned int line;

  public:
    TextBox( BitmapFont& font, float x, float y );

    void write( const std::string& text="" );
    void writeln( const std::string& text="" );
};
