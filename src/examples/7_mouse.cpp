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
            canvas.fillRect(rect, left_button.is_pressed ? BrightBlue : BrightCyan);

            // Draw the middle mouse button:
            rect.left += 40;
            rect.right += 40;
            canvas.fillRect(rect, middle_button.is_pressed ? BrightGreen : BrightYellow);

            // Draw the right mouse button:
            rect.left += 40;
            rect.right += 40;
            canvas.fillRect(rect, right_button.is_pressed ? BrightRed : BrightMagenta);

            // Draw the mouse wheel:
            rect.left = 60;
            rect.right = 80;
            rect.top = 60;
            rect.bottom = 100;
            canvas.fillRect(rect, DarkGrey);

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
            i32 x1 = 200;
            i32 x2 = x1 + FONT_WIDTH * 13;
            i32 x3 = x2 + FONT_WIDTH * 5;

            i32 y = 30;
            canvas.drawText((char*)"X", x2 - FONT_WIDTH * 2, y);
            canvas.drawText((char*)"Y", x3 - FONT_WIDTH * 2, y);
            y += 6;

            // Draw Cursor position coordinates:
            y += FONT_HEIGHT + 14;
            canvas.drawText((char*)"Cursor  :", x1, y);
            canvas.drawNumber(pos_x, x2, y);
            canvas.drawNumber(pos_y, x3, y);

            // Draw Raw position coordinates:
            y += FONT_HEIGHT + 6;
            canvas.drawText((char*)"RawInput:", x1, y, BrightGrey);
            canvas.drawNumber(raw_pos.x, x2, y, BrightGrey);
            canvas.drawNumber(raw_pos.y, x3, y, BrightGrey);

            // Draw LMB position coordinates:
            y += FONT_HEIGHT + 14;
            canvas.drawText((char*)"LMB Down:", x1, y, BrightBlue);
            canvas.drawNumber(left_button.down_pos_x, x2, y, BrightBlue);
            canvas.drawNumber(left_button.down_pos_y, x3, y, BrightBlue);
            y += FONT_HEIGHT + 6;
            canvas.drawText((char*)"LMB   Up:", x1, y, BrightCyan);
            canvas.drawNumber(left_button.up_pos_x, x2, y, BrightCyan);
            canvas.drawNumber(left_button.up_pos_y, x3, y, BrightCyan);

            // Draw MMB position coordinates:
            y += FONT_HEIGHT + 14;
            canvas.drawText((char*)"MMB Down:", x1, y, BrightGreen);
            canvas.drawNumber(middle_button.down_pos_x, x2, y, BrightGreen);
            canvas.drawNumber(middle_button.down_pos_y, x3, y, BrightGreen);
            y += FONT_HEIGHT + 6;
            canvas.drawText((char*)"MMB   Up:", x1, y, BrightYellow);
            canvas.drawNumber(middle_button.up_pos_x, x2, y, BrightYellow);
            canvas.drawNumber(middle_button.up_pos_y, x3, y, BrightYellow);

            // Draw MMB position coordinates:
            y += FONT_HEIGHT + 14;
            canvas.drawText((char*)"RMB Down:", x1, y, BrightRed);
            canvas.drawNumber(right_button.down_pos_x, x2, y, BrightRed);
            canvas.drawNumber(right_button.down_pos_y, x3, y, BrightRed);
            y += FONT_HEIGHT + 6;
            canvas.drawText((char*)"RMB   Up:", x1, y, BrightMagenta);
            canvas.drawNumber(right_button.up_pos_x, x2, y, BrightMagenta);
            canvas.drawNumber(right_button.up_pos_y, x3, y, BrightMagenta);
        }

        // Draw mouse double-click area:
        {
            // Draw double-click area:
            rect.left = 200 - 5;
            rect.right = 350 - 5;
            rect.top = 260;
            rect.bottom = 282;
            canvas.drawRect(rect);

            rect.left += 5;
            rect.top += 5;
            if (is_captured) {
                canvas.drawText(
                        (char*)"Captured!",
                     rect.left, rect.top, BrightRed);
                canvas.drawText(
                        (char*)"Double-click anywhere to release...",
                     20, rect.bottom + 30, BrightCyan);
            } else
                canvas.drawText(
                        (char*)"Double-click me!",
                     rect.left, rect.top, BrightGreen);

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