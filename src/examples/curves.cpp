#define SLIMMER

#include "../slim/math/vec2.h"
#include "../slim/draw/circle.h"
#include "../slim/draw/line.h"
#include "../slim/app.h"

// Or using the single-header file:
//#include "../slim.h"

struct App : SlimApp {
    Canvas canvas;
    Color color;

    vec2 P0{100.0f, 300.0f};
    vec2 P1{150.0f, 250.0f};
    vec2 P2{200.0f, 220.0f};
    vec2 P3{250.0f, 330.0f};
    vec2 *Ps = &P0;
    vec2 *selected_point = nullptr;
    vec2 *hovered_point = nullptr;
    vec2 selection_offset;
    i32 point_radius = 5;
    i32 line_segments = 800;
    vec2 v[3];

    void updateCoefficientVectore() {
        vec2 _3P0 = 3.0f*P0;
        vec2 _3P1 = 3.0f*P1;
        vec2 _3P2 = 3.0f*P2;
        vec2 _3P3 = 3.0f*P3;
        v[0] = _3P1 - _3P0;
        v[1] = _3P0 + _3P2 - 2.0f*_3P1;
        v[2] = _3P1 + P3 - (_3P2 + P0);
    }

    bool isOverPoint(vec2 *P) const {
        return RectI{
                (i32)P->x - point_radius,
                (i32)P->x + point_radius,
                (i32)P->y - point_radius,
                (i32)P->y + point_radius
        }.contains(mouse::pos_x, mouse::pos_y);
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
        updateCoefficientVectore();
    }

    void OnMouseButtonDown(mouse::Button &mouse_button) override {
        selected_point = hovered_point;
        if (selected_point) selection_offset = vec2{mouse::pos_x, mouse::pos_y} - *selected_point;
    }

    void OnMouseButtonUp(mouse::Button &mouse_button) override {
        selected_point = hovered_point;
    }

    void OnUpdate(float delta_time) override {
        bool is_dragging = mouse::left_button.is_pressed && selected_point != nullptr;
        if (is_dragging) {
            if (mouse::moved) {
                *hovered_point = vec2{mouse::pos_x, mouse::pos_y} - selection_offset;
                updateCoefficientVectore();
            }
        } else {
            hovered_point = nullptr;
            vec2 *P = Ps;
            for (u8 i = 0; i < 4; i++, P++) if (isOverPoint(P)) hovered_point = P;
        }
    }

    void OnRender() override {
        canvas.clear();

        vec2 from, to{P0};
        f32 t_inc = 1.0f / (f32)line_segments;
        f32 t, T = t_inc;
        for (i32 i = 0; i < line_segments; i++, T += t_inc) {
            from = to;
            from = to;
            to = P0;
            t = T;
            for (u8 j = 0; j < 3; j++, t *= T) to += t * v[j];
            canvas.drawLine(from, to, Green, 0.2f);
        }

        vec2 *P = Ps;
        for (u8 i = 0; i < 4; i++, P++)
            canvas.fillCircle(*P, point_radius,
                              P == selected_point ? White : (P == hovered_point ? BrightGrey : Grey));

        canvas.drawToWindow();
    }
};

SlimApp* createApp() {
    return new App();
}