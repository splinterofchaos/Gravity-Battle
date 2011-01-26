
#include "Config.h"
#include "Parsing.h"

#include <fstream>
#include <algorithm>

void Config::set_defaults()
{
    vars["particleRatio"] = "200";

    vars["predictionLength"]    = "100";
    vars["predictionPrecision"] =  "10";

    vars["gravityLine"]       = "1";
    vars["velocityArrow"]     = "0";
	vars["accelerationArrow"] = "0";

    vars["motionBlur"] = "0";

    vars["scale"] = "1";

    vars["nHighScores"] = "10";

    vars["fps"] = "0";

    vars["particle-behaviour"] = "orbit";

    // Values out of 128 for SDL_mixer.
    vars["music-volume"] = 100;
    vars["sfx-volume"]   = 128; 
}

Config::Config()
{
    set_defaults();
}

Config::Config( const std::string& filename )
{
    reload( filename );
}

bool Config::reload( const std::string& filename )
{
    // To ensure nothing is left uninitialized.
    set_defaults();

    std::ifstream cfg( filename.c_str() );

    std::string line;
    while( std::getline( cfg, line ) ) 
    {
        rm_comments(   &line );
        rm_whitespace( &line );

        if( line.size() == 0 )
            continue;

        // Check for errors in var after checking it against the possible
        // choices. That way, bad expression and unknown handle errors 
        // can be handled together.
        Variable var = evaluate_expression( line );

        if( vars.find(var.handle) != vars.end() ) {
            vars[ var.handle ] = var.value;
        } else {
            static std::ofstream out( "config.error" );
            out << "Unknown valName (" << var.handle << "}\n";
            out << "Line: \"" << line << "\"\n\n";
        }
    }

    return cfg;
}

std::string& Config::operator [] ( const std::string& handle )
{
    return vars[ handle ];
}

const std::string& Config::operator [] ( const std::string& handle ) const
{
    static const std::string ZILCH = "";
    Vars::const_iterator it = vars.find( handle );
    if( it != vars.end() )
        return it->second;
    else
        return ZILCH;
}
