#pragma once

#include "../../slim.h"

struct ParticleBrush {
    static constexpr i32 MAX_RADIUS = 60;
    static constexpr i32 MAX_PARTICLES = 8;

    enum Operation {Displace, Undisplace, Pinch, Expand, Twirl};

    Operation operation = Displace;

    mat2 twirl_rotation = mat2{}.rotated_by(10 * DEG_TO_RAD);
    vec2 movement, *particle_positions;
    f32 radius = 60.0f;
    f32 opacity = 0.2f;
    u8 active_particles = 1;

    INLINE_XPU f32 sample(f32 distance) const {
        return 1.0f - smoothStep(distance / radius);
    }

    INLINE_XPU f32 sample(vec2 at, vec2 center) const {
        return sample((at - center).length());
    }

    INLINE_XPU f32 sample(vec2 at, i32 particle_index) const {
        return sample(at, particle_positions[particle_index]);
    }

    INLINE_XPU static void updatePixel(Pixel &target_pixel, vec2 at, const Image &image) {
        i32 x = (i32)at.x;
        i32 y = (i32)at.y;

        if (x >= 0 && x < (i32)image.width && y >= 0 && y < (i32)image.height)
            target_pixel = image.pixels[image.width * (u32)y + (u32)x];
        else
            target_pixel.color = Black;
    }

    INLINE_XPU void displace(Pixel &pixel, i32 x, i32 y, const Image &image, vec2 &displacement) {
        vec2 at{(f32)x + 0.5f, (f32)y + 0.5f};
        vec2 new_displacement{0.0f};
        for (i32 particle_index = active_particles - 1; particle_index >= 0; particle_index--) {
            f32 magnitude = sample(at + new_displacement, particle_index);
            if (magnitude == 0.0f)
                break;

            new_displacement -= movement * magnitude;
        }

        if (new_displacement.nonZero()) {
            new_displacement += displacement;
            displacement = new_displacement;

            ParticleBrush::updatePixel(pixel, at + new_displacement, image);
        }
    }

    INLINE_XPU void undisplace(Pixel &pixel, i32 x, i32 y, const Image &image, vec2 &displacement) {
        vec2 at{(f32)x + 0.5f, (f32)y + 0.5f};

        f32 magnitude = 0;
        for (u8 particle_index = 0; particle_index < active_particles; particle_index++)
            magnitude += sample(at, particle_index);

        if (magnitude) {
            vec2 new_displacement{displacement};
            new_displacement *= 1.0f - clampedValue(magnitude * opacity);
            displacement = new_displacement;

            updatePixel(pixel, at + new_displacement, image);
        }
    }

    INLINE_XPU void pinch(Pixel &pixel, i32 x, i32 y, const Image &image, vec2 &displacement) {
        vec2 at{(f32)x + 0.5f, (f32)y + 0.5f};
        vec2 new_displacement{0.0f};
        for (i32 particle_index = active_particles - 1; particle_index >= 0; particle_index--) {
            f32 magnitude = sample(at + new_displacement, particle_index);
            if (magnitude == 0.0f)
                break;

            vec2 v = at - particle_positions[particle_index];
            new_displacement += v * magnitude * opacity * 0.1f;
        }

        if (new_displacement.nonZero()) {
            new_displacement += displacement;
            displacement = new_displacement;

            ParticleBrush::updatePixel(pixel, at + new_displacement, image);
        }
    }

    INLINE_XPU void expand(Pixel &pixel, i32 x, i32 y, const Image &image, vec2 &displacement) {
        vec2 at{(f32)x + 0.5f, (f32)y + 0.5f};
        vec2 new_displacement{0.0f};
        for (i32 particle_index = active_particles - 1; particle_index >= 0; particle_index--) {
            f32 magnitude = sample(at + new_displacement, particle_index);
            if (magnitude == 0.0f)
                break;

            vec2 v = at - particle_positions[particle_index];
            new_displacement -= v * magnitude * opacity * 0.1f;
        }

        if (new_displacement.nonZero()) {
            new_displacement += displacement;
            displacement = new_displacement;

            ParticleBrush::updatePixel(pixel, at + new_displacement, image);
        }
    }

    INLINE_XPU void twirl(Pixel &pixel, i32 x, i32 y, const Image &image, vec2 &displacement) {
        vec2 at{(f32)x + 0.5f, (f32)y + 0.5f};
        vec2 new_displacement{0.0f};
        for (i32 particle_index = active_particles - 1; particle_index >= 0; particle_index--) {
            f32 magnitude = sample(at + new_displacement, particle_index);
            if (magnitude == 0.0f)
                break;

            vec2 v1 = at - particle_positions[particle_index];
            vec2 v2 = twirl_rotation * v1;
            vec2 v = v2 - v1;
            new_displacement += v * magnitude * 0.1f;
        }

        if (new_displacement.nonZero()) {
            new_displacement += displacement;
            displacement = new_displacement;

            ParticleBrush::updatePixel(pixel, at + new_displacement, image);
        }
    }

    XPU void apply(Pixel &pixel, i32 x, i32 y, const Image &image, vec2 &displacement) {
        switch (operation) {
            case Displace: return displace(pixel, x, y, image, displacement);
            case Undisplace: return undisplace(pixel, x, y, image, displacement);
            case Pinch: return pinch(pixel, x, y, image, displacement);
            case Expand: return expand(pixel, x, y, image, displacement);
            case Twirl: return twirl(pixel, x, y, image, displacement);
        }
    }

    void drawToWindow(RectI draw_bounds) {
        if (draw_bounds.right < 0 ||
            draw_bounds.bottom < 0 ||
            draw_bounds.left >= window::width ||
            draw_bounds.top >= window::height)
            return;

        if (draw_bounds.right > window::width - 1) draw_bounds.right = window::width - 1;
        if (draw_bounds.bottom > window::height - 1) draw_bounds.bottom = window::height - 1;

        i32 x, y;
        vec2 v;
        for (v.y = 0.5f - MAX_RADIUS, y = draw_bounds.top; y <= draw_bounds.bottom; y++, v.y += 1.0f) {
            for (v.x = 0.5f - MAX_RADIUS, x = draw_bounds.left; x <= draw_bounds.right; x++, v.x += 1.0f) {
                u8 A = (u8)(FLOAT_TO_COLOR_COMPONENT * opacity * sample(v.length()));
                window::content[window::width * y + x] = A << 16 | A << 8 | A;
            }
        }
    }

    void drawToCanvas(const Canvas &canvas, vec2i at, const Color &color, f32 draw_opacity = 1.0f) {
        draw_opacity *= opacity;
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
                canvas.setPixel(x, y, color, draw_opacity * sample(v.length()));
    }
};