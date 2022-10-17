#pragma once

#include "./brush.h"

template <typename T>
void runOnCPU(const Image<T> &image, const Image<T> &current, TiledGridInfo &grid, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush) {
    for (i32 y = relevant_bounds.top; y <= relevant_bounds.bottom; y++)
        for (i32 x = relevant_bounds.left; x <= relevant_bounds.right; x++) {
            u32 offset = grid.getOffset(x, y);
            brush.apply(current.content[offset], x, y, image, grid, displacement_map[offset]);
        }

//    u32 offset = 0;
//    for (grid.row = 0; grid.row < grid.rows; grid.row++) {
//        u32 tile_height = grid.row == grid.bottom_row ? grid.bottom_row_tile_height : image.tile_height;
//
//        for (grid.column = 0; grid.column < grid.columns; grid.column++) {
//            u32 tile_width = grid.column == grid.right_column ? grid.right_column_tile_stride : image.tile_width;
//
//            for (grid.tile_y = 0; grid.tile_y < tile_height; grid.tile_y++) {
//                for (grid.tile_x = 0; grid.tile_x < tile_width; grid.tile_x++, offset++) {
//                    grid.updateGlobalCoords();
//                    if (relevant_bounds.contains((i32)grid.x, (i32)grid.y)) {
//                        T &pixel = current.content[offset];
//                        vec2 &displacement = displacement_map[offset];
//                        brush.apply(pixel, grid.x, grid.y, image, grid, displacement);
//                    }
//                }
//            }
//        }
//    }
}