#define SLIM_ENABLE_CANVAS_TEXT_DRAWING
#define SLIM_ENABLE_CANVAS_NUMBER_DRAWING

#include "../slim/math/vec2.h"
#include "../slim/app.h"
#include "../slim/draw/number.h"
// Or using the single-header file:
//#include "../slim.h"

struct TextApp : SlimApp {
    void OnRender() override {
        using namespace window;

        // Draw a multi colored line of text:
        vec2i pos{20, 50};
        String text{(char*)"The answer is... :    (!)"};
        canvas.drawText(text.char_ptr, pos, Green);

        pos.x += ((i32)text.length - 4) * FONT_WIDTH;
        canvas.drawNumber(42, pos, Red);
    }
};

SlimApp* createApp() {
    return new TextApp();
}