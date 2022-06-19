#define SLIMMER
#define SLIM_DISABLE_ALL_CANVAS_DRAWING
#define SLIM_ENABLE_CANVAS_LINE_DRAWING
#define SLIM_ENABLE_CANVAS_TRIANGLE_DRAWING
#define SLIM_ENABLE_CANVAS_CIRCLE_DRAWING
#define SLIM_ENABLE_CANVAS_TEXT_DRAWING
#define SLIM_ENABLE_CANVAS_HUD_DRAWING

#include "../../slim/math/vec2.h"
#include "../../slim/app.h"
#include "../../slim/draw/line.h"
#include "../../slim/draw/circle.h"
#include "../../slim/draw/triangle.h"
#include "../../slim/draw/hud.h"

struct VizApp : SlimApp {
    Canvas canvas;

    i32 step = 0;

    // HUD:
    HUDLine Hits{ (char*)"Hits  : "};
    HUDLine Step{ (char*)"Step 1: "};
    HUDSettings hud_settings{
            2,
            1.2f,
            BrightGrey
    };
    HUD hud{hud_settings, &Hits};

    struct PlayButton {
        static constexpr i32 top{20};
        static constexpr i32 size{40};

        struct Triangle {
            vec2i A, B, C;
            Color color;

            void update(const RectI &bounds) {
                A.x = C.x = bounds.left;
                B.x = bounds.right;

                A.y = bounds.top;
                C.y = bounds.bottom;
                B.y = (A.y + C.y) / 2;
            }

            void render(Canvas &cnv) {
                cnv.fillTriangle(A, B, C, color);
            }
        } triangle;

        RectI rect{};
        bool pressed{false};
        bool clicked{false};
        bool hovered{false};

        PlayButton() {
            rect.top = top;
            rect.bottom = top + size;
            update(window::width);
        }

        void update(i32 width) {
            rect.left = width / 2 - size / 2;
            rect.right = width / 2 + size / 2;
            triangle.update(rect);
        }

        void OnMouseMoved(i32 x, i32 y) {
            hovered = rect.contains(x, y);
        }

        void OnMouseDown(mouse::Button &mouse_button) {
            if (hovered && &mouse_button == &mouse::left_button)
                pressed = true;
        }

        void OnMouseUp(mouse::Button &mouse_button) {
            if (&mouse_button == &mouse::left_button) {
                if (pressed)
                    clicked = true;
                pressed = false;
            }
        }

        bool OnUpdate() {
            triangle.color = pressed ? White : (hovered ? BrightGrey : Grey);
            if (clicked) {
                clicked = false;
                return true;
            }
            return false;
        }
    } play_button;

    struct Circle {
        vec2 center, hit_point1, hit_point2;
        i32 radius;
        Color color = BrightGrey;
        f32 opacity = 0.5f;
        i32 hit_count = 0;

        Circle() {
            update(window::width, window::height);
        }

        void update(i32 width, i32 height) {
            radius = width / 8;
            center.x = (f32)width / 2;
            center.y = (f32)height / 2;
        }

        void render(Canvas &cnv) {
            cnv.fillCircle(center, radius, color, opacity);
            cnv.fillCircle(center, radius - 2, Black);
        }
    } circle;

    struct Ray {
        vec2 origin, target, direction, arrow_left, arrow_right, arrow_head;
        i32 origin_radius;
        Color color = BrightRed;
        Color origin_color = BrightMagenta;
        f32 opacity = 0.25f;
        RectI arrow_rect;
        bool pressed = false;

        Ray() {
            update(window::width, window::height);
        }

        void update(i32 width, i32 height) {
            f32 w = (f32)width;
            f32 h = (f32)height;
            origin.x = w * (1.0f / 2.0f + 1.0f / 8.0f);
            origin.y = h * (1.0f - (1.0f / 8.0f));
            origin_radius = (i32)(w * (1.0f / 100.0f));
            target.x = w * (1.0f - (1.0f / 16.0f));
            target.y = 0;
            direction = (target - origin).normalized();
            updateArrow();
        }

        void updateArrow() {
            vec2 perp{direction.perp()};
            arrow_head = origin + direction * 100.0f;
            arrow_left = arrow_head - direction * 10.0f + perp * 10.0f;
            arrow_right = arrow_head - direction * 10.0f - perp * 10.0f;

            arrow_rect.left = (i32)(arrow_left.x < arrow_right.x ? arrow_left.x : arrow_right.x);
            arrow_rect.right = (i32)(arrow_left.x < arrow_right.x ? arrow_left.x : arrow_right.x);
            if ((i32)arrow_head.x < arrow_rect.left) arrow_rect.left = (i32)arrow_head.x;
            if ((i32)arrow_head.x > arrow_rect.right) arrow_rect.right = (i32)arrow_head.x;

            arrow_rect.top = (i32)(arrow_left.y < arrow_right.y ? arrow_left.y : arrow_right.y);
            arrow_rect.bottom = (i32)(arrow_left.y < arrow_right.y ? arrow_left.y : arrow_right.y);
            if ((i32)arrow_head.y < arrow_rect.top) arrow_rect.top = (i32)arrow_head.y;
            if ((i32)arrow_head.y > arrow_rect.bottom) arrow_rect.bottom = (i32)arrow_head.y;

            i32 arrow_width = arrow_rect.right - arrow_rect.left;
            i32 arrow_height = arrow_rect.bottom - arrow_rect.top;
            i32 arrow_center_x = (arrow_rect.right + arrow_rect.left) / 2;
            i32 arrow_center_y = (arrow_rect.bottom + arrow_rect.top) / 2;
            arrow_rect.left = arrow_center_x - arrow_width;
            arrow_rect.right = arrow_center_x + arrow_width;
            arrow_rect.top = arrow_center_y - arrow_height;
            arrow_rect.bottom = arrow_center_y + arrow_height;
        }

        void render(Canvas &cnv, i32 step) {
            cnv.drawLine(origin, target, color, opacity);
            cnv.fillCircle(origin, origin_radius, origin_color);
            if (step >= 5) {
                cnv.drawLine(origin, arrow_head, Magenta, opacity*2.0f, 2);
                cnv.drawLine(arrow_head, arrow_left, Magenta, opacity*2.0f, 2);
                cnv.drawLine(arrow_head, arrow_right, Magenta, opacity*2.0f, 2);
            }
        }

        void OnMouseMoved(i32 x, i32 y, i32 step) {
            if (pressed && step == 5) {
                direction.x = (f32)x - origin.x;
                direction.y = (f32)y - origin.y;
                direction = direction.normalized();
                target = origin + (2000.0f) * direction;
                updateArrow();
            }
        }

        void OnMouseDown(mouse::Button &mouse_button) {
            if (&mouse_button == &mouse::left_button && arrow_rect.contains(mouse_button.down_pos_x, mouse_button.down_pos_y))
                pressed = true;
        }

        void OnMouseUp(mouse::Button &mouse_button) {
            if (&mouse_button == &mouse::left_button) {
                pressed = false;
            }
        }
    } ray;

    struct ClosestPoint {
        Color circle_color, color;
        vec2 position, circle_center;
        i32 circle_radius, radius;

        void update(i32 width, const Ray &ray, const Circle &circle) {
            radius = width / 200;
            position = ray.origin + (ray.direction * (circle.center - ray.origin).dot(ray.direction));
            circle_center = circle.center;
            circle_radius = (i32)(circle_center - position).length();
            if (circle_radius > circle.radius) {
                circle_color = BrightRed;
                color = Red;
            } else {
                circle_color = BrightGreen;
                color = Green;
            }
        }

        void render(Canvas &cnv, i32 step) {
            if (step > 0 && step < 6) {
                cnv.drawLine(circle_center, position, circle_color, 0.25f);
                cnv.drawCircle(circle_center, circle_radius, circle_color, 0.25f);
            }
            cnv.fillCircle(position, radius, color);
        }
    } closest_point;

    struct Point {
        Color circle_color{BrightCyan}, color{Cyan};
        vec2 position, circle_center;
        RectI rect;
        i32 circle_radius, radius;
        bool pressed{false};

        Point() {
            position.x = (f32)window::width / 2.0f + (f32)window::width / 8.0f;
            position.y = (f32)window::height - (f32)window::height / 8.0f;
        }

        void updatePositionAndColor(i32 step, f32 t, const Ray &ray, const Circle &circle, const ClosestPoint &closest_point) {
            if (step == 1 || step == 2 || step == 5) {
                position = ray.origin.lerpTo(closest_point.position, t);

                if (t > 1.0f) t = 2.0f - t;
                if (t < 0.0f) t = 0.0f;
                color = Color(BrightCyan).lerpTo(closest_point.color, t);
                circle_color = Color(BrightCyan).lerpTo(closest_point.circle_color, t);
            } else if (step == 3 || step == 4) {
                if (t < 0.0f) t = 0.0f;
                vec2 p = circle.center - closest_point.position;
                f32 d = p.length();
                p *= (d - (f32)circle.radius) / d;
                p += closest_point.position;
                position = closest_point.position.lerpTo(p, t);
                if (t > 1.0f) t = 1.0f;
                color = closest_point.color.lerpTo(Color(Green), t);
                circle_color = closest_point.circle_color.lerpTo(Color(Green), t);
            } else if (step == 6) {
                position = closest_point.position.lerpTo(circle.center, t);
            } else if (step == 8) {
                position = closest_point.position.lerpTo(circle.hit_point1, t);
                color = Color(Green).lerpTo(Color(Magenta), t);
            } else if (step == 10) {
                position = closest_point.position.lerpTo(circle.hit_point2, t);
                color = Color(Green).lerpTo(Color(Cyan), t);
            }
        }

        void update(i32 width, const Circle &circle) {
            radius = width / 200;
            circle_center = circle.center;
            circle_radius = (i32)(circle_center - position).length();

            rect.left = (i32)position.x - radius;
            rect.top = (i32)position.y - radius;
            rect.right = rect.left + radius;
            rect.bottom = rect.top + radius;
            width = rect.right - rect.left;
            i32 height = rect.bottom - rect.top;
            i32 x = (rect.right + rect.left) / 2;
            i32 y = (rect.bottom + rect.top) / 2;
            rect.left = x - width;
            rect.right = x + width;
            rect.top = y - height;
            rect.bottom = y + height;
        }

        void render(Canvas &cnv, i32 step) {
            if (step > 0 && step < 6) {
                cnv.drawLine(circle_center, position, circle_color, 0.25f);
                cnv.drawCircle(circle_center, circle_radius, circle_color, 0.25f);
            }
            cnv.fillCircle(position, radius, color);
        }

        void OnMouseMoved(i32 x, i32 y, i32 width, i32 step, const Ray &ray, const Circle &circle, const ClosestPoint &closest_point) {
            if (pressed) {
                f32 t = 0;
                if (step == 1) {
                    t = (vec2{x, y} - ray.origin).dot(ray.direction);
                    t /= (closest_point.position - ray.origin).length();
                } else if (step == 3) {
                    vec2 closest_point_to_circle_center = circle.center - closest_point.position;
                    f32 d = closest_point_to_circle_center.length();
                    closest_point_to_circle_center /= d;
                    t = (vec2{x, y} - closest_point.position).dot(closest_point_to_circle_center);
                    t /= d - (f32)circle.radius;
                }

                updatePositionAndColor(step, t, ray, circle, closest_point);
                update(width, circle);
            }
        }

        void OnMouseDown(mouse::Button &mouse_button) {
            if (&mouse_button == &mouse::left_button && rect.contains(mouse_button.down_pos_x, mouse_button.down_pos_y))
                pressed = true;
        }

        void OnMouseUp(mouse::Button &mouse_button) {
            if (&mouse_button == &mouse::left_button) {
                pressed = false;
            }
        }
    } point;

    struct Transition {
        bool active = false;
        f32 t = 0, speed = 1.0f, eased_t = 0;

        bool increment(f32 amount) {
            t += amount * speed;
            if (t > 1) {
                t = eased_t = 1;
            } else
                eased_t = smoothstep(0, 1, t);

            return active;

        }
    } transition;

    VizApp() {
        Hits.value = (i32)0;
        Step.value = step;
        canvas.antialias = SSAA;

        point.update(window::width, circle);
        closest_point.update(window::width, ray, circle);

        hud.top = (i32)((f32)play_button.triangle.C.y + hud.settings.line_height * 2 * FONT_HEIGHT);
    };

    void OnUpdate(f32 delta_time) override {
        if (play_button.OnUpdate() && !(step == 5 && circle.hit_count != 2)) {
            Step.value = ++step;
            if (step) {
                transition.active = true;
                transition.t = 0;
            }
        }

        if (step) {
            if (step == 5) {
                if (ray.pressed) {
                    point.updatePositionAndColor(step, 1.0f, ray, circle, closest_point);
                    point.update(canvas.dimensions.width, circle);
                    closest_point.update(canvas.dimensions.width, ray, circle);
                }
            } else if (transition.active && transition.increment(delta_time)) {
                if (step == 2) {
                    point.updatePositionAndColor(step, transition.eased_t, ray, circle, closest_point);
                    point.update(canvas.dimensions.width, circle);
                    if (transition.t == 1.0f) {
                        step++;
                        transition.active = false;
                        closest_point.color = closest_point.circle_color = BrightRed;
                        point.color = point.circle_color = BrightCyan;
                    }
                } else if (step == 4 || step >= 6) {
                    point.updatePositionAndColor(step, transition.eased_t, ray, circle, closest_point);
                    point.update(canvas.dimensions.width, circle);
                    if (transition.t == 1.0f)
                        transition.speed = -transition.speed;
                    if (transition.speed < 0 && transition.t <= 0) {
                        transition.speed = -transition.speed;
                        transition.t = 0;
                        transition.active = false;
                        step++;
                    }
                }
            }
        }
        if (step >= 5) {
            circle.hit_count = (i32)(
                    closest_point.circle_radius > circle.radius ? 0 : (
                            closest_point.circle_radius == circle.radius ? 1 : 2)
            );

            if (circle.hit_count == 2) {
                f32 t = (closest_point.position - ray.origin).length();
                f32 dt = sqrtf((f32)circle.radius*(f32)circle.radius - (circle.center - closest_point.position).squaredLength());
                f32 t1 = t - dt;
                f32 t2 = t + dt;
                circle.hit_point1 = ray.origin+ray.direction*t1;
                circle.hit_point2 = ray.origin+ray.direction*t2;
            }
            Hits.value = circle.hit_count;
        }
    }

    void OnKeyChanged(u8 key, bool is_pressed) override {
        if (is_pressed) return;
        if (key == controls::key_map::tab)
            hud.enabled = !hud.enabled;
    }

    void OnRender() override {
        canvas.clear();
        circle.render(canvas);
        ray.render(canvas, step);
        if (step >= 6) {
            canvas.drawLine(circle.center, circle.hit_point1, White, 0.5f, 2);
            canvas.drawLine(circle.center, circle.hit_point2, White, 0.5f, 2);
            canvas.drawLine(closest_point.position, circle.hit_point1, Magenta, 0.5f, 2);
            canvas.drawLine(closest_point.position, circle.hit_point2, Cyan, 0.5f, 2);
            canvas.drawLine(closest_point.position, circle.center, Green, 0.5f, 2);
        }
        if (step) {
            closest_point.render(canvas, step);
            point.render(canvas, step);

            if (circle.hit_count == 0) Hits.value_color = Grey;
            if (circle.hit_count == 1) Hits.value_color = Green;
            if (circle.hit_count == 2) {
                Hits.value_color = Yellow;
                canvas.fillCircle(circle.hit_point1, point.radius, Magenta);
                canvas.fillCircle(circle.hit_point2, point.radius, Cyan);
            }
        }
        play_button.triangle.render(canvas);
        if (step) {
            if (step == 1 || step == 2) {
                Step.title = (char*)"Step 1: ";
                Step.value.string = (char*)"Find the point along the ray that is closest to the circle's center";
            } else if (step == 3 || step == 4) {
                Step.title = (char*)"Step 2: ";
                Step.value.string = (char*)"Check if that closest point is 'on' or 'within' the circle";
                vec2i extra_message_pos{
                        (i32)hud.left + (i32)(Step.title.length * FONT_WIDTH),
                        (i32)hud.top + (i32)(hud.settings.line_height * 2 * FONT_HEIGHT)
                };
                canvas.drawText((char*)"by comparing it's distance from the circle's center to it's radius", extra_message_pos, Step.value_color);
            } else if (step == 5) {
                Step.title = (char*)"Step 3: ";
                Step.value.string = (char*)"When the distance is grater than the radius, there is no intersection point";
                vec2i extra_message_pos{
                        (i32)hud.left + (i32)(Step.title.length * FONT_WIDTH),
                        (i32)hud.top + (i32)(hud.settings.line_height * 2 * FONT_HEIGHT)
                };
                canvas.drawText((char*)"When it's equal to the radius, there is a single intersection point", extra_message_pos, Step.value_color);
                canvas.drawText((char*)"When it's smaller than the radius, there are 2 intersection points", extra_message_pos + vec2{0, hud.settings.line_height * FONT_HEIGHT}, Step.value_color);
            } if (step >= 6) {
                Step.title = (char*)"Step 4: ";
                Step.value.string = (char*)"The 2 intersection points are at the same distance from the closest-point";
                vec2i extra_message_pos{
                        (i32)hud.left + (i32)(Step.title.length * FONT_WIDTH),
                        (i32)hud.top + (i32)(hud.settings.line_height * 2 * FONT_HEIGHT)
                };
                canvas.drawText((char*)"The pythagoras theorem is used to determine this distance", extra_message_pos, Step.value_color);
                canvas.drawText((char*)"using the radius and the distance of the closest point from the center", extra_message_pos + vec2{0, hud.settings.line_height * FONT_HEIGHT}, Step.value_color);
            }

            canvas.drawHUD(hud);
            if (step == 1) {
                if (point.position == ray.origin)
                    canvas.drawText((char*)"Click and drag the point", (i32)ray.origin.x - 100, (i32)ray.origin.y + point.radius * 3);
            } else if (step == 3) {
                if (point.position == closest_point.position)
                    canvas.drawText((char*)"Click and drag the point", (i32)closest_point.position.x - 100, (i32)closest_point.position.y + point.radius * 4);
            } else if (step == 5) {
                canvas.drawText((char*)"Drag the ray direction arrow to form 2 intersections", (i32)ray.origin.x - 200, (i32)ray.origin.y + point.radius * 4);
            }
        }
        canvas.drawToWindow();
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
        play_button.update(width);
        circle.update(width, height);
        ray.update(width, height);
        closest_point.update(width, ray, circle);
        point.updatePositionAndColor(1, 0, ray, circle, closest_point);
        point.update(width, circle);
    }

    void OnMouseButtonDown(mouse::Button &mouse_button) override {
        play_button.OnMouseDown(mouse_button);
        point.OnMouseDown(mouse_button);
        ray.OnMouseDown(mouse_button);
    }
    void OnMouseButtonUp(mouse::Button &mouse_button) override {
        play_button.OnMouseUp(mouse_button);
        point.OnMouseUp(mouse_button);
        ray.OnMouseUp(mouse_button);
    }
    void OnMousePositionSet(i32 x, i32 y) override {
        play_button.OnMouseMoved(x, y);
        point.OnMouseMoved(x, y, canvas.dimensions.width, step, ray, circle, closest_point);
        ray.OnMouseMoved(x, y, step);
    }
};


SlimApp* createApp() {
    window::title = (char*)"Ray/Circle Intersection Visualization";
    window::width = 1024;
    window::height = 1024;
    return new VizApp();
}
