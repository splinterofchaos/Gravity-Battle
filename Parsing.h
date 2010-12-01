
#include <string>

// All these functions take references and modify their arguments. This is for
// internal use only.

// Remove prefixed or postfixed whitespace; return true if there was any.
bool rm_whitespace_pref( std::string& str );
bool rm_whitespace_post( std::string& str );

// rm_whitespace(s) is equivalent to rm_whitespace_pref(rm_whitespace_post(s))
void rm_whitespace( std::string& str );

// True if first non-whitespace char is #.
bool is_comment( std::string& str );

struct Variable
{
    std::string handle, value;
};

const Variable VARIABLE_ERROR = { "ERROR", "ERROR" };

// Reads an expression (e.g. var = 5), and returns a variable ({"var","5"}).
// Returns VARIABLE_ERROR if not in expression form.
Variable evaluate_expression( const std::string& str );
