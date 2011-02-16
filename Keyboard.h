
#include <SDL/SDL.h>

struct Keyboard
{
    enum KeyStatus { NOT_PRESSED=0, PRESSED, RELEASED };

    enum Keys {
        ESQ = SDLK_ESCAPE,

        LEFT  = SDLK_LEFT,
        RIGHT = SDLK_RIGHT,
        UP    = SDLK_UP,
        DOWN  = SDLK_DOWN
    };

  private:
    static Uint8* keys;
    static KeyStatus states[ SDLK_LAST ];

  public:
    static void update();
    static void add_key_status( unsigned int key, KeyStatus status );
    
    static bool key_down( unsigned int key );
    static KeyStatus key_state( unsigned int key );
};
