#pragma once

#include "./brush.h"

void runOnCPU(const Image &image, const Image &current, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush, bool displace) {
    for (i32 y = relevant_bounds.top;
         y <= relevant_bounds.bottom;
         y++) {
        for (i32 x = relevant_bounds.left;
             x <= relevant_bounds.right;
             x++) {
            i32 pixel_offset = (i32)image.width * y + x;
            Pixel &pixel = current.pixels[pixel_offset];
            vec2 &displacement = displacement_map[pixel_offset];

            if (displace)
                brush.undisplace(pixel, x, y, image, displacement);
            else
                brush.displace(pixel, x, y, image, displacement);
        }
    }
};