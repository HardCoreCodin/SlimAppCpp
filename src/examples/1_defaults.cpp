#define SLIMMER

#include "../slim/app.h"
// Or using the single-header file:
//#include "../slim.h"

SlimApp* createApp() {
    // Some initial defaults can be overridden before the window is displayed:
    window::title  = (char*)"My Title";
    window::width  = 300;
    window::height = 60;
    return new SlimApp();
}

