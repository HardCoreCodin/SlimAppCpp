#include "../slim/app.h"
#include "../slim/math/vec2.h"
#include "../slim/draw/text.h"
#include "../slim/draw/number.h"
#include "../slim/draw/rectangle.h"
// Or using the single-header file:
// #include "../slim.h"

#define MOUSE_WHEEL__MAX 2000


struct MouseApp : SlimApp {
    void OnRender() override {
        Canvas &canvas = window::canvas;

        static vec2i raw_pos{};
        if (mouse::moved) {
            mouse::moved = false;
            raw_pos.x += mouse::pos_raw_diff_x;
            raw_pos.y += mouse::pos_raw_diff_y;
            mouse::pos_raw_diff_x = 0;
            mouse::pos_raw_diff_y = 0;
        }

        RectI rect;

        // Draw the mouse
        {
            // Draw the mouse contour:
            rect.left = 8;
            rect.right = 132;
            rect.top = 38;
            rect.bottom = 290;
            draw(rect, canvas);

            // Draw the left mouse button:
            rect.left = 10;
            rect.right = 50;
            rect.top = 40;
            rect.bottom = 120;
            fill(rect, canvas, mouse::left_button.is_pressed ? Blue : Cyan);

            // Draw the middle mouse button:
            rect.left += 40;
            rect.right += 40;
            fill(rect, canvas, mouse::middle_button.is_pressed ? Green : Yellow);

            // Draw the right mouse button:
            rect.left += 40;
            rect.right += 40;
            fill(rect, canvas, mouse::right_button.is_pressed ? Red : Magenta);

            // Draw the mouse wheel:
            rect.left = 60;
            rect.right = 80;
            rect.top = 60;
            rect.bottom = 100;
            fill(rect, canvas, Grey);

            // Draw a marker representing the state of the mouse wheel:
            rect.left += 2;
            rect.right -= 2;
            rect.top += 18;
            rect.bottom -= 18;

            static f32 accumulated_mouse_wheel_scroll_amount = 0;
            static f32 mouse_wheel_delta_y = 0;
            if (mouse::wheel_scrolled) {
                accumulated_mouse_wheel_scroll_amount += mouse::wheel_scroll_amount;
                if (accumulated_mouse_wheel_scroll_amount > MOUSE_WHEEL__MAX)
                    accumulated_mouse_wheel_scroll_amount = -MOUSE_WHEEL__MAX;
                if (accumulated_mouse_wheel_scroll_amount < -MOUSE_WHEEL__MAX)
                    accumulated_mouse_wheel_scroll_amount = MOUSE_WHEEL__MAX;

                mouse_wheel_delta_y = accumulated_mouse_wheel_scroll_amount;
                mouse_wheel_delta_y += MOUSE_WHEEL__MAX;
                mouse_wheel_delta_y /= MOUSE_WHEEL__MAX;
                mouse_wheel_delta_y -= 1;
                mouse_wheel_delta_y *= 38;
                mouse_wheel_delta_y /= -2;

                mouse::wheel_scroll_handled = true;
            }
            rect.top += (i32)mouse_wheel_delta_y;
            rect.bottom += (i32)mouse_wheel_delta_y;
            fill(rect, canvas);
        }

        // Draw mouse coords
        {
            i32 x1 = 150;
            i32 x2 = x1 + FONT_WIDTH * 13;
            i32 x3 = x2 + FONT_WIDTH * 5;

            i32 y = 8;
            draw((char*)"X", x2 - FONT_WIDTH * 2, y, canvas);
            draw((char*)"Y", x3 - FONT_WIDTH * 2, y, canvas);
            y += 2;

            // Draw Cursor position coordinates:
            y += FONT_HEIGHT + 4;
            draw((char*)"Cursor  :", x1, y, canvas);
            draw(mouse::pos_x, x2, y, canvas);
            draw(mouse::pos_y, x3, y, canvas);

            // Draw Raw position coordinates:
            y += FONT_HEIGHT + 2;
            draw((char*)"RawInput:", x1, y, canvas, Grey);
            draw(raw_pos.x, x2, y, canvas, Grey);
            draw(raw_pos.y, x3, y, canvas, Grey);

            // Draw LMB position coordinates:
            y += FONT_HEIGHT + 4;
            draw((char*)"LMB Down:", x1, y, canvas, Blue);
            draw(mouse::left_button.down_pos_x, x2, y, canvas, Blue);
            draw(mouse::left_button.down_pos_y, x3, y, canvas, Blue);
            y += FONT_HEIGHT + 2;
            draw((char*)"LMB   Up:", x1, y, canvas, Cyan);
            draw(mouse::left_button.up_pos_x, x2, y, canvas, Cyan);
            draw(mouse::left_button.up_pos_y, x3, y, canvas, Cyan);

            // Draw MMB position coordinates:
            y += FONT_HEIGHT + 4;
            draw((char*)"MMB Down:", x1, y, canvas, Green);
            draw(mouse::middle_button.down_pos_x, x2, y, canvas, Green);
            draw(mouse::middle_button.down_pos_y, x3, y, canvas, Green);
            y += FONT_HEIGHT + 2;
            draw((char*)"MMB   Up:", x1, y, canvas, Yellow);
            draw(mouse::middle_button.up_pos_x, x2, y, canvas, Yellow);
            draw(mouse::middle_button.up_pos_y, x3, y, canvas, Yellow);

            // Draw MMB position coordinates:
            y += FONT_HEIGHT + 4;
            draw((char*)"RMB Down:", x1, y, canvas, Red);
            draw(mouse::right_button.down_pos_x, x2, y, canvas, Red);
            draw(mouse::right_button.down_pos_y, x3, y, canvas, Red);
            y += FONT_HEIGHT + 2;
            draw((char*)"RMB   Up:", x1, y, canvas, Magenta);
            draw(mouse::right_button.up_pos_x, x2, y, canvas, Magenta);
            draw(mouse::right_button.up_pos_y, x3, y, canvas, Magenta);
        }

        // Draw mouse double-click area:
        {
            // Draw double-click area:
            rect.left = 140;
            rect.right = 460;
            rect.top = 258;
            rect.bottom = 290;
            draw(rect, canvas);

            rect.left += 5;
            rect.top += 5;
            if (mouse::is_captured) {
                draw((char*)"Captured!",
                     rect.left, rect.top, canvas, Red);
                draw((char*)"Double-click to release...",
                     8, rect.bottom + 30, canvas, Cyan);
            } else
                draw((char*)"Double-click me!",
                     rect.left, rect.top, canvas, Green);

            if (mouse::double_clicked) {
                mouse::double_clicked = false;
                if (mouse::is_captured) {
                    mouse::is_captured = false;
                    os::setWindowCapture(false);
                    os::setCursorVisibility(true);
                } else if (rect.x_range[mouse::pos_x] &&
                           rect.y_range[mouse::pos_y]) {
                    mouse::is_captured = true;
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