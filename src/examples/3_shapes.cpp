#include "../slim/app.h"
#include "../slim/math/vec2.h"
#include "../slim/draw/line.h"
#include "../slim/draw/circle.h"
#include "../slim/draw/triangle.h"
#include "../slim/draw/rectangle.h"
// Or using the single-header file:
// #include "../slim.h"

struct ShapesApp : SlimApp {
    void OnRender() override {
        Canvas &canvas = window::canvas;

        // Draw and fill a rectangle with different colors:
        RectI rect{100, 300, 100, 300};
        fill(rect, canvas, DarkBlue);
        draw(rect, canvas);

        // Draw and fill a triangle with different colors:
        vec2i p1{150, 150};
        vec2i p2{250, 150};
        vec2i p3{200, 275};
        fillTriangle(p1, p2, p3, canvas, Cyan);
        p1.y += 20;
        p2.y += 20;
        p3.y += 20;
        drawTriangle(p1, p2, p3, canvas, Magenta);

        // Draw and fill a circle with different colors:
        i32 radius = 18;
        vec2i center{200, 175};
        fillCircle(center, radius, canvas, Yellow);
        center.y += 25;
        radius += 2;
        drawCircle(center, radius, canvas, Magenta);

        // Draw horizontal and vertical lines with different colors:
        rect.left = rect.top = 90;
        rect.right = rect.bottom = 310;
        drawHLine(rect.x_range, rect.top,    canvas, Green);
        drawHLine(rect.x_range, rect.bottom, canvas, Green);
        drawVLine(rect.y_range, rect.left,   canvas, Magenta);
        drawVLine(rect.y_range, rect.right,  canvas, Magenta);
    }
};
SlimApp* createApp() {
    return new ShapesApp();
}