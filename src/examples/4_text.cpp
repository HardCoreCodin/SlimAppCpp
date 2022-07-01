#define SLIMMER

#include "../slim/math/vec2.h"
#include "../slim/draw/number.h"
#include "../slim/app.h"
// Or using the single-header file:
//#include "../slim.h"

struct TextApp : SlimApp {
    Canvas canvas;

    void OnRender() override {
        canvas.clear();

        // Draw a multi colored line of text:
        vec2i pos{20, 50};
        String text{(char*)"The answer is... :    (!)"};
        canvas.drawText(text.char_ptr, pos, Green);

        pos.x += ((i32)text.length - 4) * FONT_WIDTH;
        canvas.drawNumber(42, pos, Red);

        canvas.drawToWindow();
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }
};

SlimApp* createApp() {
    return new TextApp();
}