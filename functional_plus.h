
// Like for_each, but acknowledging containers using pointers that need to be
// dereferenced before calling f. I thought of making a Dereference functor
// object to be super generic...but this is MUCH less work.
template< typename Iterator, typename F >
void for_each_ptr( Iterator begin, Iterator end, F f )
{
    for( ; begin != end; begin++ )
        f( **begin );
}

// Since std has no non-c++0x way of binding an argument to a unary function...
template< typename F, typename Arg >
struct BinderArg
{
    F f;
    Arg a;

    BinderArg( F f, Arg a )
        : f( f ), a( a )
    {
    }

    void operator () ()
    {
        f( a );
    }
};

template< typename F, typename Arg >
BinderArg<F,Arg> bind_arg( F f, Arg a )
{
    return BinderArg<F,Arg>( f, a );
}
