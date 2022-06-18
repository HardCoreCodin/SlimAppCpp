#define SLIMMER

#include "../slim/app.h"
// Or using the single-header file:
//#include "../slim.h"

// This is the entry point and is the only required function:
SlimApp* createApp() {
    // This still yields a window that can be resized, minimized and closed
    return new SlimApp();
}

