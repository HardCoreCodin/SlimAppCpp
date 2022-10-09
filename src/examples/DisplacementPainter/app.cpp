#include "../../slim.h"

struct Brush {
    static constexpr i32 MAX_BRUSH_RADIUS = 60;
    static constexpr i32 MIN_BRUSH_RADIUS = 30;

    enum Param {Radius, InnerRadius, Opacity};

    f32 magnitudes[MAX_BRUSH_RADIUS*MAX_BRUSH_RADIUS*4];
    f32 opacity = 0.2f;
    i32 radius = MAX_BRUSH_RADIUS;
    i32 inner_radius = MIN_BRUSH_RADIUS;

    void drawTo(const Canvas &canvas, const RectI &draw_bounds) {
        if (draw_bounds.right < 0 ||
            draw_bounds.bottom < 0 ||
            draw_bounds.left >= canvas.dimensions.width ||
            draw_bounds.top >= canvas.dimensions.height)
            return;

        i32 draw_width = draw_bounds.right - draw_bounds.left - 1;
        i32 draw_height = draw_bounds.bottom - draw_bounds.top - 1;
        if (draw_width > MAX_BRUSH_RADIUS*2) draw_width = MAX_BRUSH_RADIUS*2;
        if (draw_height > MAX_BRUSH_RADIUS*2) draw_height = MAX_BRUSH_RADIUS*2;
        for (i32 y = 0, Y = draw_bounds.top; y < draw_height; y++, Y++)
            for (i32 x = 0, X = draw_bounds.left; x < draw_width; x++, X++)
                canvas.setPixel(X, Y, White, magnitudes[MAX_BRUSH_RADIUS * 2 * y + x]);
    }

    void update() {
        const i32 size = MAX_BRUSH_RADIUS * 2;
        for (i32 y = 0; y < size; y++) {
            for (i32 x = 0; x < size; x++) {
                f32 X = (f32)(x - MAX_BRUSH_RADIUS);
                f32 Y = (f32)(y - MAX_BRUSH_RADIUS);
                f32 r = sqrtf(X * X + Y * Y);
                magnitudes[size * y + x] = opacity * (1.0f - smoothStep((f32)inner_radius, (f32)radius, r));
            }
        }
    }

    void increment(Param param, f32 by) {
        switch (param) {
            case Param::Opacity: opacity = clampedValue(by + opacity); break;
            case Param::Radius: radius = clampedValue((i32)by + radius, MAX_BRUSH_RADIUS); break;
            case Param::InnerRadius: inner_radius = clampedValue((i32)by + inner_radius, MAX_BRUSH_RADIUS); break;
        }
        update();
    }
};

struct ParticleBrush {
    static constexpr i32 MAX_RADIUS = 60;
    static constexpr i32 MAX_PARTICLES = 8;

    vec2 particle_positions[MAX_PARTICLES];
    f32 radius = 60.0f;
    f32 factor = 1.0f;
    u8 active_particles = 1;

    INLINE_XPU f32 sample(f32 distance) const {
        return (1.0f - smoothStep(distance / radius)) * factor;
    }

    INLINE_XPU f32 sample(vec2 source, vec2 target) const {
        return sample((target - source).length());
    }

    INLINE_XPU f32 sample(i32 particle_index, vec2 target) const {
        return sample(particle_positions[particle_index], target);
    }

    void drawToWindow(RectI draw_bounds) {
        if (draw_bounds.right < 0 ||
            draw_bounds.bottom < 0 ||
            draw_bounds.left >= window::width ||
            draw_bounds.top >= window::height)
            return;

        draw_bounds.right = min(draw_bounds.right, window::width - 1);
        draw_bounds.bottom = min(draw_bounds.bottom, window::height - 1);

        i32 x, y;
        vec2 v;
        for (v.y = 0.5f - MAX_RADIUS, y = draw_bounds.top; y <= draw_bounds.bottom; y++, v.y += 1.0f) {
            for (v.x = 0.5f - MAX_RADIUS, x = draw_bounds.left; x <= draw_bounds.right; x++, v.x += 1.0f) {
                u8 A = (u8)(FLOAT_TO_COLOR_COMPONENT * sample(v.length()));
                window::content[window::width * y + x] = A << 16 | A << 8 | A;
            }
        }
    }

    void drawToCanvas(const Canvas &canvas, vec2i at, const Color &color) {
        vec2 brush_start{0.5f - radius};
        RectI bounds;
        bounds.left = bounds.top = -(i32)radius;
        bounds.right = bounds.bottom = (i32)radius - 1;
        bounds.x_range += at.x;
        bounds.y_range += at.y;
        if (bounds.x_range.first < 0) brush_start.x -= (f32)bounds.x_range.first;
        if (bounds.y_range.first < 0) brush_start.y -= (f32)bounds.y_range.first;

        bounds -= RectI{0, canvas.dimensions.width, 0, canvas.dimensions.height};
        if (!bounds) return;

        i32 x, y;
        vec2 v;
        for (v.y = brush_start.y, y = bounds.top; y <= bounds.bottom; y++, v.y += 1.0f)
            for (v.x = brush_start.x, x = bounds.left; x <= bounds.right; x++, v.x += 1.0f)
                canvas.setPixel(x, y, color, sample(v.length()));
    }
};


struct DisplacementPainter : SlimApp {
    static constexpr f32 MouseBrushSensitivity = 0.05f;
    static constexpr f32 BrushOpacitySensitivity = 0.01f;

    Canvas canvas;
//    Brush brush;
    ParticleBrush particle_brush;

    char *files[2]{(char*)"input.image", (char*)"input.image"};
    Image image, current;
    ImagePack image_pack{2, &image, files, (char*)__FILE__};
    RectI image_bounds{0, (i32)image.width - 1, 0, (i32)image.height - 1};
    vec2 *displacement_map = new vec2[image.width * image.height];

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
//        brush.update();
    }

    void OnRender() override {
        canvas.clear();

        drawImage(current, canvas, image_bounds);

        u32 offset;
        f32 magnitude, factor;
        vec2 direction, from, to, displacement;
        for (u32 y = 0; y < image.height; y++)
            for (u32 x = 0; x < image.width; x++) {
                offset = image.width * y + x;
                displacement = displacement_map[offset];
                if (displacement.nonZero()) {
                    magnitude = displacement.length();
                    direction = displacement / magnitude;
                    factor = magnitude * 0.05f;
                    offset = canvas.dimensions.width * (image.height + y) + x;
                    canvas.pixels[offset].color.r = clampedValue(factor * (direction.x * 0.5f + 0.5f));
                    canvas.pixels[offset].color.g = clampedValue(factor * (direction.y * 0.5f + 0.5f));
                }
            }

        if (image_bounds.contains(mouse::pos_x, mouse::pos_y)) {
            Color color = controls::is_pressed::ctrl ? Green : Red;
            if (mouse::left_button.is_pressed)
                canvas.drawCircle(mouse::pos_x, mouse::pos_y, (i32)particle_brush.radius, color);
            else
                particle_brush.drawToCanvas(canvas, {mouse::pos_x, mouse::pos_y}, color);
//            canvas.drawCircle(mouse::pos_x, mouse::pos_y, brush.radius, color, brush.opacity);
//            canvas.drawCircle(mouse::pos_x, mouse::pos_y, brush.inner_radius, color, brush.opacity);
        }

        canvas.drawToWindow();

        RectI draw_bounds;
        draw_bounds.left = (i32)image.width;
        draw_bounds.bottom = Brush::MAX_BRUSH_RADIUS * 2;
        draw_bounds.right = draw_bounds.left + draw_bounds.bottom;
        particle_brush.drawToWindow(draw_bounds);
    }

    void OnUpdate(float delta_time) override {
        if (mouse::wheel_scrolled) {
//            Brush::Param param = Brush::Param::Radius;
            f32 by = mouse::wheel_scroll_amount * MouseBrushSensitivity;
            if (controls::is_pressed::shift) {
                by *= BrushOpacitySensitivity;
//                param = Brush::Param::Opacity;
                particle_brush.factor =  clampedValue(by +particle_brush.factor);
            }
//            else if (controls::is_pressed::alt)
//                param = Brush::Param::InnerRadius;
            else
                particle_brush.radius = clampedValue(by + particle_brush.radius, (f32)ParticleBrush::MAX_RADIUS);
//            brush.increment(param, by);
        }

        if ((controls::is_pressed::ctrl || mouse::moved) && mouse::left_button.is_pressed && image_bounds.contains(mouse::pos_x, mouse::pos_y)) {
            // Update displacement map using the brush and the mouse position and movement, then update current image:

            RectI bounds;
            bounds.left = bounds.top = -(i32)particle_brush.radius;
            bounds.right = bounds.bottom = (i32)particle_brush.radius - 1;
            bounds.x_range += mouse::pos_x;
            bounds.y_range += mouse::pos_y;
            bounds -= image_bounds;
            if (!bounds) return;

            vec2i screen_coord, pull, target, movement{mouse::movement_x, mouse::movement_y};
            vec2 particle_sample, displacement, scaled_movement;
            i32 pixel_offset;
            f32 opacity;

            for (screen_coord.y = bounds.top, particle_sample.y = (f32)(bounds.top - mouse::pos_y) + 0.5f;
                 screen_coord.y <= bounds.bottom;
                 screen_coord.y++, particle_sample.y += 1.0f) {
                for (screen_coord.x = bounds.left, particle_sample.x = (f32)(bounds.left - mouse::pos_x) + 0.5f;
                     screen_coord.x <= bounds.right;
                     screen_coord.x++, particle_sample.x += 1.0f) {

                    opacity = particle_brush.sample(particle_sample.length());
                    if (opacity)
                        scaled_movement = vec2{movement} * opacity;
                    else
                        continue;

                    pixel_offset = -1;
                    if (controls::is_pressed::ctrl) {
                        pixel_offset = (i32)image.width * screen_coord.y + screen_coord.x;
                        displacement = displacement_map[pixel_offset] * (1.0f - opacity);
                    } else {
                        target = screen_coord + scaled_movement;
                        if (image_bounds.contains(target.x, target.y)) {
                            pixel_offset = (i32)image.width * target.y + target.x;
                            displacement = displacement_map[pixel_offset] - scaled_movement;
                        }
                    }

                    if (pixel_offset != -1) {
                        displacement_map[pixel_offset] = displacement;
                        pull = screen_coord + displacement;
                        if (image_bounds.contains(pull.x, pull.y))
                            current.pixels[pixel_offset] = image.pixels[(i32)image.width * pull.y + pull.x];
                        else
                            current.pixels[pixel_offset].color = Black;
                    }
                }
            }
        }
    }
};

SlimApp* createApp() {
    return new DisplacementPainter();
}