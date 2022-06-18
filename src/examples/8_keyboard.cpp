#define SLIMMER
#define SLIM_DISABLE_ALL_CANVAS_DRAWING
#define SLIM_ENABLE_CANVAS_TEXT_DRAWING
#define SLIM_ENABLE_CANVAS_RECTANGLE_DRAWING

#include "../slim/app.h"
#include "../slim/draw/text.h"
#include "../slim/draw/rectangle.h"
// Or using the single-header file:
//#include "../slim.h"

struct KeyboardApp : public SlimApp {
    Canvas canvas;
    Move move{};

    void OnKeyChanged(u8 key, bool is_pressed) override {
        if (key == 'W') move.up       = is_pressed;
        if (key == 'A') move.left     = is_pressed;
        if (key == 'S') move.down     = is_pressed;
        if (key == 'D') move.right    = is_pressed;
    }
    void OnRender() override {
        canvas.clear();

        using namespace controls;

        i32 right  = canvas.dimensions.width / 2 + 114;
        i32 bottom = canvas.dimensions.height / 2 + 54;
        RectI rect;

        // Draw the keyboard contour:
        rect.right = right - 2;
        rect.left = rect.right - 228;
        rect.bottom = bottom - 2;
        rect.top = rect.bottom - 98;
        canvas.drawRect(rect);

        // Draw the 'A' key:
        rect.right = right - 170;
        rect.left = rect.right - 18;
        rect.bottom = bottom - 46;
        rect.top = rect.bottom - 18;
        canvas.fillRect(rect, move.left ? BrightGrey : DarkGrey);
        canvas.drawText((char*)"A", rect.left + 6, rect.top + 4,
                        move.left ? DarkGrey : BrightGrey);

        // Draw the 'S' key:
        rect.left += 22;
        rect.right += 22;
        canvas.fillRect(rect, move.down ? BrightGrey : DarkGrey);
        canvas.drawText((char*)"S", rect.left + 6, rect.top + 4,
                        move.backward ? DarkGrey : BrightGrey);

        // Draw the 'D' key:
        rect.left += 22;
        rect.right += 22;
        canvas.fillRect(rect, move.right ? BrightGrey : DarkGrey);
        canvas.drawText((char*)"D", rect.left + 6, rect.top + 4,
                        move.right ? DarkGrey : BrightGrey);

        // Draw the 'W' key:
        rect.left -= 28;
        rect.right -= 28;
        rect.top -= 22;
        rect.bottom -= 22;
        canvas.fillRect(rect, move.up ? BrightGrey : DarkGrey);
        canvas.drawText((char*)"W", rect.left + 6, rect.top + 4,
                        move.forward ? DarkGrey : BrightGrey);

        // Draw the left Ctrl key:
        rect.right = right - 200;
        rect.left = rect.right - 26;
        rect.bottom = bottom - 6;
        rect.top = rect.bottom - 16;
        canvas.fillRect(rect, is_pressed::ctrl ? BrightBlue : BrightCyan);

        // Draw the left Alt key:
        rect.left += 30;
        rect.right += 30;
        canvas.fillRect(rect, is_pressed::alt ? BrightRed : BrightMagenta);

        // Draw the left Shift key:
        rect.left -= 30;
        rect.right -= 15;
        rect.top -= 20;
        rect.bottom -= 20;
        canvas.fillRect(rect, is_pressed::shift ? BrightGreen : BrightYellow);

        // Draw the right Ctrl key:
        rect.right = right - 4;
        rect.left = rect.right - 26;
        rect.bottom = bottom - 6;
        rect.top = rect.bottom - 16;
        canvas.fillRect(rect, is_pressed::ctrl ? BrightBlue : BrightCyan);

        // Draw the right Alt key:
        rect.left -= 30;
        rect.right -= 30;
        canvas.fillRect(rect, is_pressed::alt ? BrightRed : BrightMagenta);

        // Draw the right Shift key:
        rect.left += 15;
        rect.right += 30;
        rect.top -= 20;
        rect.bottom -= 20;
        canvas.fillRect(rect, is_pressed::shift ? BrightGreen : BrightYellow);

        // Draw the Space key:
        rect.right = right - 64;
        rect.left = rect.right - 102;
        rect.bottom = bottom - 6;
        rect.top = rect.bottom - 16;
        canvas.fillRect(rect, is_pressed::space ? BrightGrey : Grey);

        canvas.drawToWindow();
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }
};

SlimApp* createApp() {
    return new KeyboardApp();
}