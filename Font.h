
#include <string>

#include <SDL/SDL_ttf.h>


class TrueTypeFont
{
    TTF_Font* ttf;

#if defined( _WIN32 )
    HDC hdc;
#else
#endif

  public:
    TrueTypeFont();
    ~TrueTypeFont();

    void draw( const std::string& text, float x, float y );

    static const int LINE_HEIGHT = 20;
};

class TextBox
{
    TrueTypeFont& font;

    float x, y; // Base position.
    unsigned int line;

  public:
    TextBox( TrueTypeFont& font, float x, float y );

    void write( const std::string& text="" );
    void writeln( const std::string& text="" );
};
