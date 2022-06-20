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

    PaintApp() {
        painting.dimensions.update(MAX_WIDTH, MAX_HEIGHT);
//        painting.antialias = canvas.antialias = SSAA;
    }

    void OnRender() override {
        canvas.clear();

        int width = canvas.dimensions.width;
        int height = canvas.dimensions.height;
//        if (canvas.antialias == SSAA) {
//            width *= 2;
//            height *= 2;
//        }
        float one_over_width = 1.0f / (f32)width;
        float one_over_height = 1.0f / (f32)height;

        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++) {
                Color color;
                color.green = 0;
                color.red = (float)x * one_over_width;
                color.blue = (float)y * one_over_height;
                color.gammaCorrect();
                canvas.setPixel(x, y, color);
            }

        Color color = Green;
        i32 radius = 15;
        vec2i center{mouse::pos_x, mouse::pos_y};
        if (mouse::left_button.is_pressed)
            painting.fillCircle(center, radius, color, 0.2);

        canvas.drawFrom(painting, true);
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