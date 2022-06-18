#define SLIMMER

#include "../slim/app.h"
#include "../slim/core/string.h"
// Or using the single-header file:
//#include "../slim.h"

// To provide custom behaviour, create a sub-struct of slim and override methods:
struct TimerApp : SlimApp {
    void OnWindowRedraw() override {
        // Track the time difference since last time this was called:
        update_timer.beginFrame();
        static f32 before = 0;
        f32 now = before + update_timer.delta_time;

        if ((u32)now > (u32)before) { // If on a seconds border:
            // Update the window title with the current time:
            static NumberString number;
            number = (i32)now;
            os::setWindowTitle(number.string.char_ptr);
        }
        before = now;
        update_timer.endFrame();
    }
};

// Return an instance of your sub-struct:
SlimApp* createApp() {
    window::title = (char*)"0";
    return new TimerApp();
}