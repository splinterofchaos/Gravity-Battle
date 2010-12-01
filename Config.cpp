
#include "Config.h"
#include "Parsing.h"

#include <fstream>
#include <algorithm>

void Config::set_defaults()
{
    particleRatio = 200;
    predictionLength = 100;
    predictionPrecision = 10;

    gravityLine = true;
    velocityArrow = false;
	accelerationArrow = false;

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
        rm_comments(   &line );
        rm_whitespace( &line );

        if( line.size() == 0 )
            continue;

        // Check for errors in var after checking it against the possible
        // choices. That way, bad expression and unknown handle errors 
        // can be handled together.
        Variable var = evaluate_expression( line );

        float value;
        sstream_convert( var.value, &value );

        if( var.handle == "particleRatio" )
            particleRatio = value;
        else if( var.handle == "predictionLength" )
            predictionLength = value;
        else if( var.handle == "predictionPrecision" )
            predictionPrecision = value;
        else if( var.handle == "gravityLine" )
            gravityLine = value;
        else if( var.handle == "velocityArrow" )
            velocityArrow = value;
        else if( var.handle == "motionBlur" )
            motionBlur = value;
        else if( var.handle == "scale" )
            scale = value;
		else if( var.handle == "accelerationArrow" )
			accelerationArrow = value;
        else {
            static std::ofstream out( "config.error" );
            out << "Unknown valName (" << var.handle << "}\n";
            out << "Line: \"" << line << "\"\n\n";
        }
    }

    return cfg;
}
