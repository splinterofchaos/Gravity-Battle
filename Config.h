
#include <string>
#include <map>
#include <fstream>

class Config
{
    //static std::ofstream out;

    void set_defaults();

  public:
    typedef std::map< std::string, std::string > Vars;
    Vars vars;

    Config( const std::string& filename );
    Config();

    bool reload( const std::string& filename );

    template< typename T >
    bool get( const std::string& handle, T* x ) const;

    std::string& operator [] ( const std::string& handle );
    const std::string& operator [] ( const std::string& handle ) const;
};

template< typename T >
bool Config::get( const std::string& handle, T* x ) const
{
    Vars::const_iterator it = vars.find( handle );
    if( it != vars.end() ) {
        std::stringstream ss( it->second );
        ss >> (*x);
        return true;
    }

    return false;
}
