#define SLIMMER
#define SLIM_DISABLE_ALL_CANVAS_DRAWING
#define SLIM_ENABLE_CANVAS_CIRCLE_DRAWING

#include "../slim/math/vec2.h"
#include "../slim/app.h"
#include "../slim/draw/circle.h"

// Or using the single-header file:
//#include "../slim.h"

struct PaintApp : SlimApp {
    Canvas canvas{SSAA}, painting{MAX_WIDTH, MAX_HEIGHT, SSAA};

    void OnRender() override {
        canvas.clear();

        int width = canvas.dimensions.width * 2;
        int height = canvas.dimensions.height * 2;
        float one_over_width = 1.0f / (f32)width;
        float one_over_height = 1.0f / (f32)height;

        Color color;
        color.green = 0;
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++) {
                color.red = (float)x * one_over_width;
                color.blue = (float)y * one_over_height;
                canvas.setPixel(x, y, color);
            }

        color = Green;
        i32 radius = 5;
        vec2i center{mouse::pos_x, mouse::pos_y};
        if (mouse::left_button.is_pressed)
            painting.fillCircle(center, radius, color, 0.2f);

        canvas.drawFrom(painting);
        canvas.drawCircle(center, radius, color);
        canvas.drawToWindow();
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }
};

SlimApp* createApp() {
    return new PaintApp();
}