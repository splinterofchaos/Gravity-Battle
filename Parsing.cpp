
#include "Parsing.h"

#include <cctype>
#include <sstream> // for sstream.

bool rm_whitespace_pref( std::string* str )
{
    std::string::size_type beginPos = str->find_first_not_of( " " );
    if( beginPos )
        str->erase( 0, beginPos );
    return beginPos;
}

bool rm_whitespace_post( std::string* str )
{
    std::string::size_type lastAlpha = str->find_last_not_of( " " );
    if( lastAlpha != std::string::npos ) {
        str->erase( lastAlpha+1, str->size() );
        return true;
    }
    return false;
}

void rm_whitespace( std::string* str )
{
    rm_whitespace_pref( str );
    rm_whitespace_post( str );
}

void rm_comments( std::string* str )
{
    std::string::size_type commentStart = str->find('#');
    if( commentStart != std::string::npos )
        str->erase( commentStart, str->size() );
}

Variable evaluate_expression( const std::string& str )
{
    Variable ret;
    
    // Lazy, but easy solution.
    std::stringstream ss( str );
    char c;

    ss >> ret.handle;
    ss >> c;
    ss >> ret.value;

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
