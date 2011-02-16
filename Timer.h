
class Timer
{
    static int ticks;

    int startTime;

    // Keep a last time not to cache this value, but to modify it if needed.
    int lastTime;

  public:
    static const int SECOND;

    Timer();

    void zero();             // Zero time.
    void clamp_ms( int ms ); // Clamp  lastTime.
    int  reset();            // Update startTime.
    int  update();           // Update lastTime.

    int   time_ms();
    float time_sec();
};
