
#include "Parsing.h"

#include <cctype>
#include <iostream> // for sstream.

bool rm_whitespace_pref( std::string& str )
{
    std::string::size_type beginPos = str.find_first_not_of( " " );
    if( beginPos )
        str.erase( 0, beginPos );
    return beginPos;
}

bool rm_whitespace_post( std::string& str )
{
    std::string::size_type endPos = str.find_last_not_of( " " );
    if( endPos != std::string::npos )
        str.erase( endPos, str.size() );
    return endPos != std::string::npos;
}

void rm_whitespace( std::string& str )
{
    rm_whitespace_post( rm_whitespace_pref( str ) );
}

Variable evaluate_expression( std::string& str )
{
    Variable ret;
    
    rm_whitespace( str );

    // Lazy, but easy solution.
    std::ostringstream ss( str );
    char c;

    ss << ret.handle << c << ret.value;

    // I could check that handle and value are proper alphanum, or that nothing
    // is after the expression besides comments, i don't think it's currently
    // necessary.
    
    // Just make sure it's an actual assignment. This also makes sure two
    // handles aren't written before the assignment.
    if( c == '=' )
        return ret;
    else
        return VARIABLE_ERROR;
}   
