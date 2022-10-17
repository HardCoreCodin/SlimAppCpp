#pragma once

#include "./brush.h"

template <typename T>
void runOnCPU(const Image<T> &image, const Image<T> &current, TiledGridInfo &grid, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush) {
    for (i32 y = relevant_bounds.top;
         y <= relevant_bounds.bottom;
         y++) {
        for (i32 x = relevant_bounds.left;
             x <= relevant_bounds.right;
             x++) {
            u32 offset = grid.getOffset(x, y);
            T &pixel = current.content[offset];
            vec2 &displacement = displacement_map[offset];

            brush.apply(pixel, x, y, image, grid, displacement);
        }
    }
};