
#define MAC 0
#define WINDOWS 0
#define LINUX 0

#if defined( __APPLE_CC__ )
    #undef MAC
    #define MAC 1

    // This defines whether we will use Apple's .app structure or Unix's.
    #define MAC_FS 1
#elif defined( __WIN32__ ) || defined( __WIN64__ )
    #undef WINDOWS 
    #define WINDOWS 1
#elif defined( __linux__ )
    #undef LINUX
    #define LINUX 1
#endif

// Define LOCAL_FILE.
// On Mac, your .app is your application, its data, but its parent is your
// running directory, The art files, on Mac, should be IN this folder, not 
// around.
#if MAC && MAC_FS
    #define FILES std::string("Orbital Chaos.app/Contents/Resources/")
    #define LOCAL_FILE(addr) \
        ( FILES + (addr) ).c_str()
#else
    #define FILES std::string("./")
    #define LOCAL_FILE(addr) (addr)
#endif
