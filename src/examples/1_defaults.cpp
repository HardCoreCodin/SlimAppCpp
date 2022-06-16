#include "../slim/app.h"
// Or using the single-header file:
//#include "../slim.h"

SlimApp* createApp() {
    // Some initial defaults can be overridden before the window is displayed:
    using namespace window;
    title  = (char*)"My Title";
    width  = 300;
    height = 60;
    return new SlimApp();
}

