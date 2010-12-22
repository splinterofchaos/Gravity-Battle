
#pragma once

// Define macros for using C++0x features if a proper version of g++ is used.
#if defined(__GNUC__) && __GXX_EXPERIMENTAL_CXX0X__
    // If GXX_EXPERIMENT is true, GNUC is 4 or greater.
    #if __GNUC__ > 4 || __GNUC_MINOR__ >= 3
        #define USE_RVAL_REFS
        #define USE_VARIADIC_TEMPLATES
    #endif
    #if __GNUC__ > 4 || __GNUC_MINOR__ >= 4
        #define USE_INIT_LISTS
    #endif
#endif

#if defined( USE_INIT_LISTS )
    #include <initializer_list>
#endif

#include <algorithm>  // For transform, copy, etc.
#include <functional> // For plus<>, etc.
#include <cmath> // For sqrt.

// Based on std::array, but with more mathematical intents.
template< class T, unsigned int S >
class Vector 
{
public:
    // STD typedefs.
    typedef       T&  reference;
    typedef const T&  const_reference;
    typedef       T*  iterator;       
    typedef const T*  const_iterator;
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    typedef T         value_type;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    
    enum { static_size = S };

protected: 
    value_type values[ static_size ];

    template< typename Iterator >
    void initialize( Iterator begin, Iterator end )
    {
#if defined( SOC_DEBUG )
        if( std::distance(begin, c.end) != static_size )
            throw "Improper initialization.";
#endif

        std::copy( begin, end, values );
    }

public:
    // Ctors.
    Vector()
    {
    }

    template< typename U >
    Vector( const Vector<U,S>& v )
    {
        initialize( v.begin(), v.end() );
    }

#if defined( USE_INIT_LISTS )
    Vector( const std::initializer_list<T>& l )
    {
        initialize( l.begin(), l.end() );
    }
#endif

#if defined( USE_VARIADIC_TEMPLATES )
private:
    template< typename U >
    void variadic_construct( const U x )
    {
        values[ static_size-1 ] = x;
    }

    template< typename U, typename... W >
    void variadic_construct( const U x, const W... args )
    {
        values[ static_size - (sizeof...(W)+1) ] = x;
        variadic_construct( args... );
    }

public:
    template< typename... U >
    Vector( const U... args )
    {
        #ifdef SOC_DEBUG
            if( sizeof...(U) != static_size )
                throw "Wrong number of variadic arguments in Vector ctor."
        #endif

        variadic_construct( args... );
    }

#else
    template< typename U >
    Vector( U x, U y )
    {
        this->x( x );
        this->y( y );
    }

    template< typename U >
    Vector( U x, U y, U z )
    {
        this->x( x );
        this->y( y );
        this->z( z );
    }

#endif

    template< class U >
    Vector& operator = ( const Vector<U,S>& v )
    {
        initialize( v.begin(), v.end() );
        return *this;
    }

    // Capacity:
    size_type size()     const { return static_size; }
    size_type max_size() const { return size();      }
    bool      empty()    const { return false;       }

    // Access:
    // TODO: at().
    // front and back not supplied because it makes little sense for vectors.
          reference operator[] ( size_type s )       { return values[s]; }
    const_reference operator[] ( size_type s ) const { return values[s]; }

    void x( value_type newVal ) { values[0] = newVal; }
    void y( value_type newVal ) { values[1] = newVal; }
    void z( value_type newVal ) { values[2] = newVal; }
    reference x() { return values[0]; }
    reference y() { return values[1]; }
    reference z() { return values[2]; }
    const_reference x() const { return values[0]; }
    const_reference y() const { return values[1]; }
    const_reference z() const { return values[2]; }
    
    // Iterators:
          iterator begin()       { return values; }
    const_iterator begin() const { return values; }
          iterator end()         { return values + static_size; }
    const_iterator end()   const { return values + static_size; }

    // OMG, so ugly! 
    // This is a function, data, returning a reference to the array, values.
    // Helps for functions with signatures like f( T array[SPECIFIC_SIZE] )
    // std::array.data() returns just val_tp*, but this doesn't break compat.
          value_type ( &data()       )[ static_size ] { return values; }
    const value_type ( &data() const )[ static_size ] { return values; }

    

    // Operators:
#define OP_TEMPLATE template< typename U >
#define OTHER_VEC   Vector<U,S>
    OP_TEMPLATE
    Vector operator += ( const OTHER_VEC& v )
    {
        std::transform (
            begin(), end(), v.begin(), begin(),
            std::plus<value_type>() 
        );
        return *this;
    }

    OP_TEMPLATE
    Vector operator -= ( const OTHER_VEC& v )
    {
        std::transform (
            begin(), end(), v.begin(), begin(),
            std::minus<value_type>() 
        );
        return *this;
    }

    template< typename U >
    Vector operator *= ( const U& x )
    {
        std::transform (
            begin(), end(), begin(), 
            std::bind1st( std::multiplies<value_type>(), x )
        );
        return *this;
    }

    template< typename U >
    Vector operator /= ( const U& x )
    {
        std::transform (
            begin(), end(), begin(), 
            std::bind2nd( std::divides<value_type>(), x )
        );
        return *this;
    }
#undef OP_TEMPLATE
#undef OTHER_VEC

    // Unary operators.
    Vector operator - () const
    {
        Vector v;
        std::transform( begin(), end(), v.begin(), std::negate<value_type>() );
        return v;
    }
};

#define BIN_OP_TEMPLATE template< typename T, typename U, size_t S >
#define BIN_OP_RET_TYPE Vector<T,S> /* TODO: This should use type promotion. */
#define VEC1        Vector<T,S>
#define VEC2        Vector<U,S>

BIN_OP_TEMPLATE
bool operator == ( const VEC1& a, const VEC2& b )
{
    return std::equal( a.begin(), a.end(), b.begin() );
}

BIN_OP_TEMPLATE
bool operator != ( const VEC1& a, const VEC2& b )
{
    return !( a == b );
}

BIN_OP_TEMPLATE
BIN_OP_RET_TYPE operator + ( const VEC1& a, const VEC2& b )
{
    typedef typename BIN_OP_RET_TYPE::value_type value_type;

    BIN_OP_RET_TYPE c;
    std::transform (
        a.begin(), a.end(), b.begin(), c.begin(), std::plus<value_type>() 
    );
    
    return c;
}

BIN_OP_TEMPLATE
BIN_OP_RET_TYPE operator - ( const VEC1& a, const VEC2& b )
{
    typedef typename BIN_OP_RET_TYPE::value_type value_type;

    BIN_OP_RET_TYPE c;
    std::transform (
        a.begin(), a.end(), b.begin(), c.begin(), std::minus<value_type>() 
    );

    return c;
}

#if defined( USE_RVAL_REFS_ )
BIN_OP_TEMPLATE
VEC1&& operator + ( VEC1&& a, const VEC2& b )
{
    typedef typename VEC1::value_type value_type;

    std::transform (
        a.begin(), a.end(), b.begin(), a.begin(), std::plus<value_type>() 
    );

    return std::move( a );
}

BIN_OP_TEMPLATE
VEC2&& operator + ( const VEC1& a, VEC2&& b )
{
    typedef typename VEC2::value_type value_type;

    std::transform (
        a.begin(), a.end(), b.begin(), b.begin(), std::plus<value_type>()
    );

    return std::move( b );
}

BIN_OP_TEMPLATE
VEC1&& operator - ( VEC1&& a, const VEC2& b )
{
    typedef typename VEC1::value_type value_type;

    std::transform (
        a.begin(), a.end(), b.begin(), a.begin(), std::minus<value_type>() 
    );
    
    return std::move( a );
}

BIN_OP_TEMPLATE
VEC2&& operator - ( const VEC1& a, VEC2&& b )
{
    typedef typename VEC2::value_type value_type;

    std::transform (
        a.begin(), a.end(), b.begin(), b.begin(), std::minus<value_type>() 
    );

    return std::move( a );
}
#endif

BIN_OP_TEMPLATE
BIN_OP_RET_TYPE operator * ( const VEC1& a, const U b )
{
    typedef typename BIN_OP_RET_TYPE::value_type value_type;

    BIN_OP_RET_TYPE c;
    std::transform ( 
        a.begin(), a.end(), c.begin(), 
        std::bind1st( std::multiplies<value_type>(), b )
    );
    
    return c;
}

BIN_OP_TEMPLATE
BIN_OP_RET_TYPE operator * ( const U a, const VEC1& b )
{
    return b * a;
}

// Dot product.
BIN_OP_TEMPLATE
typename BIN_OP_RET_TYPE::value_type operator * ( const VEC1& a, const VEC2& b )
{
    typename BIN_OP_RET_TYPE::value_type sum = 0;

    typedef typename VEC1::const_iterator I1;
    typedef typename VEC2::const_iterator I2;

    I1 i1 = a.begin(), end = a.end();
    I2 i2 = b.begin();

    for( ; i1 != end; i1++, i2++ )
        sum += (*i1) * (*i2);

    return sum;
}

BIN_OP_TEMPLATE
BIN_OP_RET_TYPE operator / ( const VEC1& a, const U b )
{
    typedef typename BIN_OP_RET_TYPE::value_type value_type;

    BIN_OP_RET_TYPE c;
    std::transform ( 
        a.begin(), a.end(), c.begin(), 
        std::bind2nd( std::divides<value_type>(), b )
    );

    return c;
}

BIN_OP_TEMPLATE
BIN_OP_RET_TYPE operator / ( const U a, const VEC1& b )
{
    typedef typename BIN_OP_RET_TYPE::value_type value_type;

    BIN_OP_RET_TYPE c;
    std::transform ( 
        b.begin(), b.end(), c.begin(), 
        std::bind1st( std::divides<value_type>(), a )
    );

    return c;
}

BIN_OP_TEMPLATE
typename BIN_OP_RET_TYPE::value_type dot( const VEC1& a, const VEC2& b )
{
    return a * b;
}

BIN_OP_TEMPLATE
BIN_OP_RET_TYPE cross( const VEC1& a, const VEC2& b )
{
    BIN_OP_RET_TYPE c;
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
    return c;
}

template< typename T >
T cross( const Vector<T,2>& a, const Vector<T,2>& b )
{
    return a.x()*b.y() - a.y()*b.x();
}

BIN_OP_TEMPLATE
typename BIN_OP_RET_TYPE::value_type angle_between( const VEC1& a, const VEC2& b )
{
    return std::acos( (a*b) / (magnitude(a)*magnitude(b)) );
}

BIN_OP_TEMPLATE
BIN_OP_RET_TYPE projection( const VEC1& a, const VEC2& b )
{
    return ( a * unit(b) ) * unit(b);
}

#undef BIN_OP_RET_TYPE
#undef BIN_OP_TEMPLATE
#undef VEC1
#undef VEC2

#define UNARY_OP_TEMPLATE template< typename T, size_t S >
#define VEC Vector<T,S>

UNARY_OP_TEMPLATE
T magnitude_sqr( const VEC& v )
{
    return v * v;
}

UNARY_OP_TEMPLATE
T magnitude( const VEC& v )
{
    return std::sqrt( magnitude_sqr(v) );
}

UNARY_OP_TEMPLATE
VEC magnitude( const VEC& v, T newMag )
{
    T mag = magnitude( v );
    T k = newMag / mag; // Rate of change.
    return v * k;
}

UNARY_OP_TEMPLATE
VEC unit( const VEC& v )
{
    T x = magnitude_sqr( v );

    if( x ) {
        // x = ||v||
        x = std::sqrt( x );
        // normal(v) == v / ||v||
        return v / x;
    } else {
        return v;
    }
}

template< typename T >
Vector<T,2> clockwise_tangent( const Vector<T,2>& v )
{
    return Vector<T,2>( -v.y(), v.x() );
}

template< typename T >
Vector<T,2> counter_clockwise_tangent( const Vector<T,2>& v )
{
    return Vector<T,2>( v.y(), -v.x() );
}

// vector( x... ) -> Vector( x... )
//      A convenience for users not wanting to mess with the template
//      parameters of Vector's ctor.
#if defined( USE_VARIADIC_TEMPLATES )
    template< typename T, typename... VT >
    Vector< T, sizeof...(VT)+1 > vector( const T x, const VT... varg )
    {
        return Vector< T, sizeof...(VT)+1 >( x, varg... );
    }
#else
    template< typename T >
    Vector<T,2> vector( T x, T y )
    {
        Vector<T,2> v;
        v.x(x); v.y(y);
        return v;
    }

    template< typename T >
    Vector<T,3> vector( T x, T y, T z )
    {
        Vector<T,3> v;
        v.x(x); v.y(y); v.z(z);
        return v;
    }

    template< typename T >
    Vector<T,4> vector( T x, T y, T z, T h )
    {
        Vector<T,4> v;
        v.x(x); v.y(y); v.z(z);// v[3] = h;
        return v;
    }
#endif

#undef UNARY_OP_TEMPLATE
#undef VEC

#undef USE_INIT_LISTS
#undef USE_RVAL_REFS
#undef USE_VARIADIC_TEMPLATES
