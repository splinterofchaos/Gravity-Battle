
#include "Vector.h"

#include <string>

#if defined( _WIN32 )
    #include <windows.h>
#else
    #include <GL/glx.h>
#endif


class BitmapFont
{
  private:
    int      base;

#if defined( _WIN32 )
    HDC hdc;
#else
#endif

  public:
    BitmapFont();
    ~BitmapFont();

    void draw( const std::string& text, float x, float y );
};
