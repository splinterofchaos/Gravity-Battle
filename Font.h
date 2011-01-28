
#include <string>

#if defined( _WIN32 )
    #include <windows.h>
#else
    #include <GL/glx.h>
#endif


class BitmapFont
{
    int      base;

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

    void writeln( const std::string& text="" );
};
