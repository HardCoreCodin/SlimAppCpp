#include "../slim/app.h"
#include "../slim/math/vec2.h"
#include "../slim/draw/text.h"
#include "../slim/draw/number.h"
// Or using the single-header file:
// #include "../slim.h"

struct TextApp : SlimApp {
    void OnRender() override {
        Canvas &canvas = window::canvas;

        // Draw a multi colored line of text:
        vec2i pos{20, 50};
        String text{(char*)"The answer is... :    (!)"};
        draw(text.char_ptr, pos, canvas, Green);

        pos.x += ((i32)text.length - 4) * FONT_WIDTH;
        draw(42, pos, canvas, Red);
    }
};

SlimApp* createApp() {
    return new TextApp();
}