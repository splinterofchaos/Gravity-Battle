
#include "Color.h"

Color::Color()
{
    r( 0 ); g( 0 ); b( 0 ); a( 1 );
}

Color::Color( Color::value_type r, Color::value_type g, Color::value_type b, Color::value_type a )
{
    this->r( r ); this->g( g ); this-> b( b ); this->a( a );
}

Color::Color( const Color::vector_type& v )
{
    for( int i=0; i < 4; i++ )
        values[i] = v[i];
}

void Color::r( Color::value_type val ) { values[0] = val; }
void Color::g( Color::value_type val ) { values[1] = val; }
void Color::b( Color::value_type val ) { values[2] = val; }
void Color::a( Color::value_type val ) { values[3] = val; }

Color::value_type Color::r() { return values[0]; }
Color::value_type Color::g() { return values[1]; }
Color::value_type Color::b() { return values[2]; }
Color::value_type Color::a() { return values[3]; }
