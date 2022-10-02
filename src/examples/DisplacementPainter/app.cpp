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

struct DisplacementPainter : SlimApp {
    static constexpr f32 MouseBrushSensitivity = 0.05f;
    static constexpr f32 BrushOpacitySensitivity = 0.01f;

    Canvas canvas;
    Brush brush;

    char *files[2]{(char*)"input.image", (char*)"input.image"};
    Image image, current;
    ImagePack image_pack{2, &image, files, (char*)__FILE__};
    RectI image_bounds{0, (i32)image.width - 1, 0, (i32)image.height - 1};
    vec2 *displacement_map = new vec2[image.width * image.height];

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
        brush.update();
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

        i32 brush_size = Brush::MAX_BRUSH_RADIUS * 2;
        RectI draw_bounds;
        draw_bounds.bottom = brush_size;
        draw_bounds.left = (i32)image.width;
        draw_bounds.right = (i32)image.width + brush_size;
        brush.drawTo(canvas, draw_bounds);

        if (image_bounds.contains(mouse::pos_x, mouse::pos_y)) {
            Color color = controls::is_pressed::ctrl ? Green : Red;
            canvas.drawCircle(mouse::pos_x, mouse::pos_y, brush.radius, color, brush.opacity);
            canvas.drawCircle(mouse::pos_x, mouse::pos_y, brush.inner_radius, color, brush.opacity);
        }

        canvas.drawToWindow();
    }

    void OnUpdate(float delta_time) override {
        if (mouse::wheel_scrolled) {
            Brush::Param param = Brush::Param::Radius;
            f32 by = mouse::wheel_scroll_amount * MouseBrushSensitivity;
            if (controls::is_pressed::shift) {
                by *= BrushOpacitySensitivity;
                param = Brush::Param::Opacity;
            } else if (controls::is_pressed::alt)
                param = Brush::Param::InnerRadius;

            brush.increment(param, by);
        }

        if ((controls::is_pressed::ctrl || mouse::moved) && mouse::left_button.is_pressed && image_bounds.contains(mouse::pos_x, mouse::pos_y)) {
            // Update displacement map using the brush and the mouse position and movement, then update current image:
            vec2 displacement_inc, old_displacement, new_displacement;
            for (i32 dy = -brush.radius; dy < brush.radius; dy++)
                for (i32 dx = -brush.radius; dx < brush.radius; dx++) {
                    i32 x = mouse::pos_x + dx;
                    i32 y = mouse::pos_y + dy;
                    if (!image_bounds.contains(x, y))
                        continue;

                    i32 brush_x = Brush::MAX_BRUSH_RADIUS + dx;
                    i32 brush_y = Brush::MAX_BRUSH_RADIUS + dy;
                    f32 magnitude = brush.magnitudes[Brush::MAX_BRUSH_RADIUS*2 * brush_y + brush_x];
                    if (controls::is_pressed::ctrl) {
                        if (magnitude) {
                            i32 pixel_offset = (i32)image.width * y + x;
                            Pixel &target_pixel = current.pixels[pixel_offset];
                            vec2 &current_displacement = displacement_map[pixel_offset];
                            new_displacement = current_displacement * (1.0f - magnitude);
                            i32 pull_x = (i32)new_displacement.x + x;
                            i32 pull_y = (i32)new_displacement.y + y;
                            if (image_bounds.contains(pull_x, pull_y))
                                target_pixel = image.pixels[(i32)image.width * pull_y + pull_x];
                            else
                                target_pixel.color = Black;
                            current_displacement = new_displacement;
                        }
                    } else {
                        displacement_inc = vec2{mouse::movement_x, mouse::movement_y} * magnitude;
                        i32 target_x = (i32)displacement_inc.x + x;
                        i32 target_y = (i32)displacement_inc.y + y;
                        if (image_bounds.contains(target_x, target_y)) {
                            i32 target_pixel_offset = (i32)image.width * target_y + target_x;
                            Pixel &target_pixel = current.pixels[target_pixel_offset];
                            vec2 &current_displacement = displacement_map[target_pixel_offset];
                            new_displacement = current_displacement - displacement_inc;

                            i32 pull_x = (i32)new_displacement.x + x;
                            i32 pull_y = (i32)new_displacement.y + y;
                            if (image_bounds.contains(pull_x, pull_y))
                                target_pixel = image.pixels[(i32)image.width * pull_y + pull_x];
                            else
                                target_pixel.color = Black;

                            current_displacement = new_displacement;
                        }
                    }
                }
        }
    }
};

SlimApp* createApp() {
    return new DisplacementPainter();
}