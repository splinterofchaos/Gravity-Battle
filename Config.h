
#include <string>

class Config
{
    void set_defaults();

  public:
    int particleRatio;
    
    int predictionLength;
    int predictionPrecision;
    
    bool gravityLine;
    bool velocityArrow;
	bool accelerationArrow;

    bool motionBlur;

    float scale;

    Config( const std::string& filename );
    Config();

    bool reload( const std::string& filename );
};
