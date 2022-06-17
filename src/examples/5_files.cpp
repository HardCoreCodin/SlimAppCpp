#define SLIM_ENABLE_CANVAS_TEXT_DRAWING

#include "../slim/app.h"
#include "../slim/draw/text.h"
// Or using the single-header file:
//#include "../slim.h"

static constexpr u32 file_buffer_size = 4096;
static char file_str[file_buffer_size];

struct FilesApp : SlimApp {
    FilesApp() {
        void* f = os::openFileForReading(__FILE__);
        os::readFromFile(file_str,
                         file_buffer_size, f);
        os::closeFile(f);
    }

    void OnRender() override {
        window::canvas.drawText(file_str, 5, 5);
    }
};

SlimApp* createApp() {
    return new FilesApp();
}