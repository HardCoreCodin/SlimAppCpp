#define SLIM_ENABLE_CANVAS_LINE_DRAWING
#define SLIM_ENABLE_CANVAS_RECTANGLE_DRAWING
#define SLIM_ENABLE_CANVAS_TRIANGLE_DRAWING
#define SLIM_ENABLE_CANVAS_CIRCLE_DRAWING

#include "../slim/math/vec2.h"
#include "../slim/app.h"
#include "../slim/draw/line.h"
#include "../slim/draw/circle.h"
#include "../slim/draw/triangle.h"
#include "../slim/draw/rectangle.h"

// Or using the single-header file:
//#include "../slim.h"

struct ShapesApp : SlimApp {
    void OnRender() override {
        using namespace window;

        // Draw and fill a rectangle with different colors:
        RectI rect{100, 300, 100, 300};
        canvas.fillRect(rect, DarkBlue);
        canvas.drawRect(rect);

        // Draw and fill a triangle with different colors:
        vec2i p1{150, 150};
        vec2i p2{250, 150};
        vec2i p3{200, 275};
        canvas.fillTriangle(p1, p2, p3, Cyan);
        p1.y += 20;
        p2.y += 20;
        p3.y += 20;
        canvas.drawTriangle(p1, p2, p3, Magenta);

        // Draw and fill a circle with different colors:
        i32 radius = 18;
        vec2i center{200, 175};
        canvas.fillCircle(center, radius, Yellow);
        center.y += 25;
        radius += 2;
        canvas.drawCircle(center, radius, Magenta);

        // Draw horizontal and vertical lines with different colors:
        rect.left = rect.top = 90;
        rect.right = rect.bottom = 310;
        canvas.drawHLine(rect.x_range, rect.top,    Green);
        canvas.drawHLine(rect.x_range, rect.bottom, Green);
        canvas.drawVLine(rect.y_range, rect.left,   Magenta);
        canvas.drawVLine(rect.y_range, rect.right,  Magenta);
    }
};
SlimApp* createApp() {
    return new ShapesApp();
}