
#include <string> // Used for LOCAL_FILE.

#if defined( __APPLE__ )
#   define WINDOWS 0
#   define LINUX   0
#   define MAC     1

    // Define whether to use the Mac .app file system, or classic Unix.
#   define MAC_FS 0
#elif defined( __WIN32__ ) || defined( __WIN64__ )
#   define WINDOWS 1
#   define MAC     0
#   define LINUX   0
#elif defined( __linux__ )
#   define LINUX   1
#   define WINDOWS 0
#   define MAC     0
#endif


// Define LOCAL_FILE.
// On Mac, your .app is your application, its data, but its parent is your
// running directory, The art files, on Mac, should be IN this folder, not 
// around.
#if (MAC && MAC_FS)
#   define FILES std::string("Orbital Chaos.app/Contents/Resources/")
#else
#   define FILES std::string("./")
#endif

#define LOCAL_FILE(addr) ( std::string(FILES) + addr ).c_str()

