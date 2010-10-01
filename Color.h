
#pragma once

#include "Vector.h"

struct Color : public Vector<float,4>
{
    typedef Vector<float,4> vector_type;

    Color();
    Color( value_type r, value_type g, value_type b, value_type a=1 );
    Color( const vector_type& v );

    value_type r();
    value_type g();
    value_type b();
    value_type a();

    void r( value_type val );
    void g( value_type val );
    void b( value_type val );
    void a( value_type val );
};
