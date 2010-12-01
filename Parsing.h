
#include <sstream>
#include <string>

// All these functions take references and modify their arguments. This is for
// internal use only.

// Remove prefixed or postfixed whitespace; return true if there was any.
bool rm_whitespace_pref( std::string& str );
bool rm_whitespace_post( std::string& str );

// rm_whitespace(s) is equivalent to rm_whitespace_pref(rm_whitespace_post(s))
void rm_whitespace( std::string& str ); 

// Removes everything after a # character.
void rm_comments( std::string& str );

struct Variable
{
    std::string handle, value;

    bool operator == ( const Variable& other )
    {
        return handle==other.handle && value==other.value;
    }
};

const Variable VARIABLE_ERROR = { "ERROR", "ERROR" };

// Reads an expression (e.g. var = 5), and returns a variable ({"var","5"}).
// Returns VARIABLE_ERROR if not in expression form.
Variable evaluate_expression( const std::string& str );

template< typename T >
bool sstream_convert( const std::string& str, T* t )
{
    // Would be more efficient if ss were static. But then, it would have to be
    // revived from fail states. Be warned.
    std::stringstream ss;
    ss.str( str );
    ss >> (*t);
    return ss;
}
