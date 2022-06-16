#define SLIM_ENABLE_CANVAS_TEXT_DRAWING
#define SLIM_ENABLE_CANVAS_NUMBER_DRAWING
#define SLIM_ENABLE_CANVAS_RECTANGLE_DRAWING

#include "../slim/math/vec2.h"
#include "../slim/app.h"
#include "../slim/draw/text.h"
#include "../slim/draw/number.h"
#include "../slim/draw/rectangle.h"
// Or using the single-header file:
//#include "../slim.h"

static constexpr int MAX_MOUSE_WHEEL = 2000;

struct MouseApp : SlimApp {
    void OnRender() override {
        using namespace window;
        using namespace mouse;

        static vec2i raw_pos{};
        if (moved) {
            moved = false;
            raw_pos.x += pos_raw_diff_x;
            raw_pos.y += pos_raw_diff_y;
            pos_raw_diff_x = 0;
            pos_raw_diff_y = 0;
        }

        RectI rect;

        // Draw the mouse
        {
            // Draw the mouse contour:
            rect.left = 8;
            rect.right = 132;
            rect.top = 38;
            rect.bottom = 290;
            canvas.drawRect(rect);

            // Draw the left mouse button:
            rect.left = 10;
            rect.right = 50;
            rect.top = 40;
            rect.bottom = 120;
            canvas.fillRect(rect, left_button.is_pressed ? Blue : Cyan);

            // Draw the middle mouse button:
            rect.left += 40;
            rect.right += 40;
            canvas.fillRect(rect, middle_button.is_pressed ? Green : Yellow);

            // Draw the right mouse button:
            rect.left += 40;
            rect.right += 40;
            canvas.fillRect(rect, right_button.is_pressed ? Red : Magenta);

            // Draw the mouse wheel:
            rect.left = 60;
            rect.right = 80;
            rect.top = 60;
            rect.bottom = 100;
            canvas.fillRect(rect, Grey);

            // Draw a marker representing the state of the mouse wheel:
            rect.left += 2;
            rect.right -= 2;
            rect.top += 18;
            rect.bottom -= 18;

            static f32 accumulated_mouse_wheel_scroll_amount = 0;
            static f32 mouse_wheel_delta_y = 0;
            if (wheel_scrolled) {
                accumulated_mouse_wheel_scroll_amount += wheel_scroll_amount;
                if (accumulated_mouse_wheel_scroll_amount > MAX_MOUSE_WHEEL)
                    accumulated_mouse_wheel_scroll_amount = -MAX_MOUSE_WHEEL;
                if (accumulated_mouse_wheel_scroll_amount < -MAX_MOUSE_WHEEL)
                    accumulated_mouse_wheel_scroll_amount = MAX_MOUSE_WHEEL;

                mouse_wheel_delta_y = accumulated_mouse_wheel_scroll_amount;
                mouse_wheel_delta_y += MAX_MOUSE_WHEEL;
                mouse_wheel_delta_y /= MAX_MOUSE_WHEEL;
                mouse_wheel_delta_y -= 1;
                mouse_wheel_delta_y *= 38;
                mouse_wheel_delta_y /= -2;

                wheel_scroll_handled = true;
            }
            rect.top += (i32)mouse_wheel_delta_y;
            rect.bottom += (i32)mouse_wheel_delta_y;
            canvas.fillRect(rect);
        }

        // Draw mouse coords
        {
            i32 x1 = 150;
            i32 x2 = x1 + FONT_WIDTH * 13;
            i32 x3 = x2 + FONT_WIDTH * 5;

            i32 y = 8;
            canvas.drawText((char*)"X", x2 - FONT_WIDTH * 2, y);
            canvas.drawText((char*)"Y", x3 - FONT_WIDTH * 2, y);
            y += 2;

            // Draw Cursor position coordinates:
            y += FONT_HEIGHT + 4;
            canvas.drawText((char*)"Cursor  :", x1, y);
            canvas.drawNumber(pos_x, x2, y);
            canvas.drawNumber(pos_y, x3, y);

            // Draw Raw position coordinates:
            y += FONT_HEIGHT + 2;
            canvas.drawText((char*)"RawInput:", x1, y, Grey);
            canvas.drawNumber(raw_pos.x, x2, y, Grey);
            canvas.drawNumber(raw_pos.y, x3, y, Grey);

            // Draw LMB position coordinates:
            y += FONT_HEIGHT + 4;
            canvas.drawText((char*)"LMB Down:", x1, y, Blue);
            canvas.drawNumber(left_button.down_pos_x, x2, y, Blue);
            canvas.drawNumber(left_button.down_pos_y, x3, y, Blue);
            y += FONT_HEIGHT + 2;
            canvas.drawText((char*)"LMB   Up:", x1, y, Cyan);
            canvas.drawNumber(left_button.up_pos_x, x2, y, Cyan);
            canvas.drawNumber(left_button.up_pos_y, x3, y, Cyan);

            // Draw MMB position coordinates:
            y += FONT_HEIGHT + 4;
            canvas.drawText((char*)"MMB Down:", x1, y, Green);
            canvas.drawNumber(middle_button.down_pos_x, x2, y, Green);
            canvas.drawNumber(middle_button.down_pos_y, x3, y, Green);
            y += FONT_HEIGHT + 2;
            canvas.drawText((char*)"MMB   Up:", x1, y, Yellow);
            canvas.drawNumber(middle_button.up_pos_x, x2, y, Yellow);
            canvas.drawNumber(middle_button.up_pos_y, x3, y, Yellow);

            // Draw MMB position coordinates:
            y += FONT_HEIGHT + 4;
            canvas.drawText((char*)"RMB Down:", x1, y, Red);
            canvas.drawNumber(right_button.down_pos_x, x2, y, Red);
            canvas.drawNumber(right_button.down_pos_y, x3, y, Red);
            y += FONT_HEIGHT + 2;
            canvas.drawText((char*)"RMB   Up:", x1, y, Magenta);
            canvas.drawNumber(right_button.up_pos_x, x2, y, Magenta);
            canvas.drawNumber(right_button.up_pos_y, x3, y, Magenta);
        }

        // Draw mouse double-click area:
        {
            // Draw double-click area:
            rect.left = 140;
            rect.right = 460;
            rect.top = 258;
            rect.bottom = 290;
            canvas.drawRect(rect);

            rect.left += 5;
            rect.top += 5;
            if (is_captured) {
                canvas.drawText((char*)"Captured!",
                     rect.left, rect.top, Red);
                canvas.drawText((char*)"Double-click to release...",
                     8, rect.bottom + 30, Cyan);
            } else
                canvas.drawText((char*)"Double-click me!",
                     rect.left, rect.top, Green);

            if (double_clicked) {
                double_clicked = false;
                if (is_captured) {
                    is_captured = false;
                    os::setWindowCapture(false);
                    os::setCursorVisibility(true);
                } else if (rect.x_range[pos_x] &&
                           rect.y_range[pos_y]) {
                    is_captured = true;
                    os::setWindowCapture(true);
                    os::setCursorVisibility(false);
                }
            }
        }
    }
};

SlimApp* createApp() {
    return new MouseApp();
}