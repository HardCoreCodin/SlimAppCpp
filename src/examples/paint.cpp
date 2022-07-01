#define SLIMMER
#define SLIM_DISABLE_ALL_CANVAS_DRAWING
#define SLIM_ENABLE_CANVAS_CIRCLE_DRAWING

#include "../slim/math/vec2.h"
#include "../slim/app.h"
#include "../slim/draw/circle.h"

// Or using the single-header file:
//#include "../slim.h"

struct PaintApp : SlimApp {
    Canvas canvas, painting;

    Color color;
    int center_x;
    int center_y;
    int gradient_size = 256;
    float gradient_factor = 1.0f / (float)gradient_size;
    float radius = 20;
    float falloff = 18;
    float full_radius = radius - falloff;
    float opacity = 0.1f;

    void OnUpdate(f32 delta_time) override {
        center_x = mouse::pos_x;
        center_y = mouse::pos_y;

        if (mouse::wheel_scrolled) {
            if (controls::is_pressed::alt) {
                if (controls::is_pressed::ctrl)
                    opacity += (f32)mouse::wheel_scroll_amount * 0.0001f;
                else {
                    gradient_size += mouse::wheel_scroll_amount > 0 ? 1 : -1;
                    gradient_factor = 1.0f / (float)gradient_size;
                }
            } else {
                const float diff = (f32)mouse::wheel_scroll_amount * 0.01f;
                if (controls::is_pressed::ctrl)
                    falloff += diff;
                else
                    radius += diff;
            }
            full_radius = radius - falloff;
        }
    }

    void OnRender() override {
        canvas.clear();

        color.green = 0;
        for (int y = 0; y < canvas.dimensions.height; y++)
            for (int x = 0; x < canvas.dimensions.width; x++) {
                color.red = (float)(x % gradient_size) * gradient_factor;
                color.blue = (float)(y % gradient_size) * gradient_factor;
                canvas.setPixel(x, y, color);
            }

        color = Green;
        drawBrush(canvas, 10.0f);

        if (mouse::left_button.is_pressed)
            drawBrush(painting);

        canvas.drawFrom(painting);
        canvas.drawToWindow();
    }

    void drawBrush(const Canvas &cnv, float factor = 1.0f) {
        const int r = (int)radius;
        const int r2 = r * r;
        const int x_start = center_x - r;
        const int x_end   = center_x + r;
        const int y_start = center_y - r;
        const int y_end   = center_y + r;

        for (int y = y_start; y <= y_end; y++)
            for (int x = x_start; x <= x_end; x++) {
                const int dx = x - center_x;
                const int dy = y - center_y;
                const int d2 = dx*dx + dy*dy;
                if (d2 <= r2) {
                    const float d = sqrtf((float)d2);
                    cnv.setPixel(x, y, color, factor * opacity * (d <= full_radius ? 1.0f : smoothStep(1.0f, 0.0f, (d - full_radius) / falloff)));
                }
            }
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }
};

SlimApp* createApp() {
    return new PaintApp();
}