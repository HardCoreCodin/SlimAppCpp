#define SLIM_ENABLE_CANVAS_TEXT_DRAWING
#define SLIM_ENABLE_CANVAS_RECTANGLE_DRAWING

#include "../slim/app.h"
#include "../slim/draw/text.h"
#include "../slim/draw/rectangle.h"
// Or using the single-header file:
//#include "../slim.h"

struct KeyboardApp : public SlimApp {
    Move move{};

    void OnKeyChanged(u8 key, bool is_pressed) override {
        if (key == 'R') move.up       = is_pressed;
        if (key == 'F') move.down     = is_pressed;
        if (key == 'W') move.forward  = is_pressed;
        if (key == 'S') move.backward = is_pressed;
        if (key == 'A') move.left     = is_pressed;
        if (key == 'D') move.right    = is_pressed;
    }
    void OnRender() override {
        using namespace window;
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
        canvas.fillRect(rect, move.left ? White : Grey);
        canvas.drawText((char*)"A", rect.left + 2, rect.top - 1, move.left ? Grey : White);

        // Draw the 'S' key:
        rect.left += 22;
        rect.right += 22;
        canvas.fillRect(rect, move.backward ? White : Grey);
        canvas.drawText((char*)"S", rect.left + 2, rect.top - 1, move.backward ? Grey : White);

        // Draw the 'D' key:
        rect.left += 22;
        rect.right += 22;
        canvas.fillRect(rect, move.right ? White : Grey);
        canvas.drawText((char*)"D", rect.left + 2, rect.top - 1, move.right ? Grey : White);

        // Draw the 'D' key:
        rect.left += 22;
        rect.right += 22;
        canvas.fillRect(rect, move.down ? White : Grey);
        canvas.drawText((char*)"F", rect.left + 2, rect.top - 1, move.down ? Grey : White);

        // Draw the 'Q' key:
        rect.left -= 28 * 3;
        rect.right -= 28 * 3;
        rect.top -= 22;
        rect.bottom -= 22;

        // Draw the 'W' key:
        rect.left += 22;
        rect.right += 22;
        canvas.fillRect(rect, move.forward ? White : Grey);
        canvas.drawText((char*)"W", rect.left + 2, rect.top - 1, move.forward ? Grey : White);

        // Draw the 'E' key:
        rect.left += 22;
        rect.right += 22;

        // Draw the 'R' key:
        rect.left += 22;
        rect.right += 22;
        canvas.fillRect(rect, move.up ? White : Grey);
        canvas.drawText((char*)"R", rect.left + 2, rect.top - 1, move.up ? Grey : White);


        // Draw the left Ctrl key:
        rect.right = right - 200;
        rect.left = rect.right - 26;
        rect.bottom = bottom - 6;
        rect.top = rect.bottom - 16;
        canvas.fillRect(rect, is_pressed::ctrl ? Blue : Cyan);

        // Draw the left Alt key:
        rect.left += 30;
        rect.right += 30;
        canvas.fillRect(rect, is_pressed::alt ? Red : Magenta);

        // Draw the left Shift key:
        rect.left -= 30;
        rect.right -= 15;
        rect.top -= 20;
        rect.bottom -= 20;
        canvas.fillRect(rect, is_pressed::shift ? Green : Yellow);

        // Draw the right Ctrl key:
        rect.right = right - 4;
        rect.left = rect.right - 26;
        rect.bottom = bottom - 6;
        rect.top = rect.bottom - 16;
        canvas.fillRect(rect, is_pressed::ctrl ? Blue : Cyan);

        // Draw the right Alt key:
        rect.left -= 30;
        rect.right -= 30;
        canvas.fillRect(rect, is_pressed::alt ? Red : Magenta);

        // Draw the right Shift key:
        rect.left += 15;
        rect.right += 30;
        rect.top -= 20;
        rect.bottom -= 20;
        canvas.fillRect(rect, is_pressed::shift ? Green : Yellow);

        // Draw the Space key:
        rect.right = right - 64;
        rect.left = rect.right - 102;
        rect.bottom = bottom - 6;
        rect.top = rect.bottom - 16;
        canvas.fillRect(rect, is_pressed::space ? White : Grey);
    }
};

SlimApp* createApp() {
    return new KeyboardApp();
}