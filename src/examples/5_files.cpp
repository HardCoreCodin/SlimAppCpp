#define SLIMMER

#include "../slim/draw/text.h"
#include "../slim/app.h"
// Or using the single-header file:
//#include "../slim.h"

static constexpr u32 file_buffer_size = 4096;
static char file_str[file_buffer_size];

struct FilesApp : SlimApp {
    Canvas canvas;

    FilesApp() {
        void* f = os::openFileForReading(__FILE__);
        os::readFromFile(file_str,
                         file_buffer_size, f);
        os::closeFile(f);
    }

    void OnRender() override {
        canvas.clear();
        canvas.drawText(file_str, 5, 5);
        canvas.drawToWindow();
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }
};

SlimApp* createApp() {
    return new FilesApp();
}