
#include "Config.h"

#include <sstream>
#include <fstream>
#include <algorithm>

void Config::set_defaults()
{
    particleRatio = 200;
    predictionLength = 100;
    predictionPrecision = 10;

    gravityLine = true;
    velocityArrow = false;

    motionBlur = false;

    scale = 1;
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

    std::ifstream cfg( "config.txt" );

    std::string line;
    while( std::getline( cfg, line ) ) 
    {
        if( line.size() == 0 || line[0] == '#' )
            continue;

        std::string::iterator it = std::find( line.begin(), line.end(), ' ' );
        std::string valName( line.begin(), it );
        it += 3;

        float value;

        if( it < line.end() ) {
            std::stringstream ss;
            ss << std::string( it, line.end() );
            ss >> value;
        } else {
            continue;
        }

        if( valName == "particleRatio" )
            particleRatio = value;
        else if( valName == "predictionLength" )
            predictionLength = value;
        else if( valName == "predictionPrecision" )
            predictionPrecision = value;
        else if( valName == "gravityLine" )
            gravityLine = value;
        else if( valName == "velocityArrow" )
            velocityArrow = value;
        else if( valName == "motionBlur" )
            motionBlur = value;
        else if( valName == "scale" )
            scale = value;
    }

    return cfg;
}
