
#include <string>

#include <SDL/SDL_ttf.h>


class TrueTypeFont
{
  public:
    TTF_Font* ttf;

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

#include "Vector.h"
#include "Color.h"

#include <vector>
#include <memory>

struct TextLine
{
    TrueTypeFont* ttf;
    std::string text;
    Vector< float, 2 > pos;
    Vector< float, 2 > dims; // < width, height >
    Color              color;
    unsigned int tex;

    TextLine( TrueTypeFont* f, const std::string& t, const Vector<float,2>& p );
    virtual ~TextLine();

    virtual void update( int dt ) { };

    void str( const std::string& s );
    void draw();
};

struct LinePrinter
{
    TrueTypeFont* ttf;
    std::vector< std::shared_ptr<TextLine> > lines;
    Vector< float, 2 > pos;
    Color color;

    float space; // The distance between lines.

    LinePrinter( TrueTypeFont* f, const Vector< float, 2 >& p );

    void add_line( const std::string& str );
};
