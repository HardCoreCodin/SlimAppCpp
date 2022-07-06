#define SLIMMER

#include "../slim/math/vec2.h"
#include "../slim/draw/circle.h"
#include "../slim/app.h"

// Or using the single-header file:
//#include "../slim.h"

struct App : SlimApp {
    Canvas window_canvas, painting_canvas;
    Color color;
    int gradient_size = 100;
    int center_x;
    int center_y;
    int S = 10;
    int R = 30;
    bool user_is_painting;

    void OnWindowResize(u16 width, u16 height) override {
        window_canvas.dimensions.update(width, height);
    }

    void OnUpdate(float delta_time) override {
        if (mouse::wheel_scrolled)
        {
            if (controls::is_pressed::ctrl)
            {
                gradient_size += mouse::wheel_scroll_amount / 10;
                gradient_size = max(10, gradient_size);
            } else if (controls::is_pressed::alt)
            {
                S += mouse::wheel_scroll_amount / 10;
                S = max(10, S);
            } else {
                R += mouse::wheel_scroll_amount / 10;
                R = max(10, R);
            }
        }
        center_x = mouse::pos_x;
        center_y = mouse::pos_y;
        user_is_painting = mouse::left_button.is_pressed;
    }

    void OnRender() override {
        if (user_is_painting)
            fillCircle(center_x, center_y, R, painting_canvas, 0.2f);

        color.green = 0;
        for (int y = 0; y < window_canvas.dimensions.height; y++)
        {
            for (int x = 0; x < window_canvas.dimensions.width; x++)
            {
                color.red = (float)(y % gradient_size) / gradient_size;
                color.blue = (float)(x % gradient_size) / gradient_size;
                window_canvas.setPixel(x, y, color);
            }
        }
        color = Green;

        window_canvas.drawFrom(painting_canvas);
        fillCircle(center_x, center_y, R, window_canvas);
        window_canvas.drawToWindow();
    }

    void fillCircle(int Cx, int Cy, int R, Canvas &canvas, float opacity = 1.0f)
    {
        int x_start = Cx - R;
        int x_end = Cx + R;
        int y_start = Cy - R;
        int y_end = Cy + R;

        int F = R - S;
        for (int y = y_start; y <= y_end; y++)
        {
            for (int x = x_start; x <= x_end; x++)
            {
                float X = x - Cx;
                float Y = y - Cy;
                float r = sqrtf(X * X + Y * Y);
                if (r <= F)
                    canvas.setPixel(x, y, color, opacity);
                else if (r <= R)
                {
                    canvas.setPixel(x, y, color, smoothStep(0, 1, opacity * (R - r) / S) );
                }
            }
        }
    }
};

SlimApp* createApp() {
    return new App();
}