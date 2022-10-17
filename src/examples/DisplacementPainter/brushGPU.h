#pragma once

#include "./brush.h"

ByteColor *d_image_pixels;
ByteColor *d_current_pixels;
vec2 *d_particle_positions;
vec2 *d_displacement_map;

template <typename T>
void allocateDeviceMemory(const Image<T> &image) {
    gpuErrchk(cudaMalloc(&d_image_pixels,     sizeof(T)  * image.width * image.height))
    gpuErrchk(cudaMalloc(&d_current_pixels,   sizeof(T)  * image.width * image.height))
    gpuErrchk(cudaMalloc(&d_displacement_map, sizeof(vec2)   * image.width * image.height))
    gpuErrchk(cudaMalloc(&d_particle_positions, sizeof(vec2)   * ParticleBrush::MAX_PARTICLES))
}

template <typename T>
void uploadImage(const Image<T> &image) {
    uploadN(image.content, d_image_pixels, image.width * image.height)
}

template <typename T>
void uploadCurrent(const Image<T> &current, const vec2 *displacement_map) {
    uploadN(current.content, d_current_pixels, current.width * current.height)
    uploadN(displacement_map, d_displacement_map, current.width * current.height)
}

void uploadBrushParticlePositions(vec2 *particle_positions) {
    uploadN(particle_positions, d_particle_positions, ParticleBrush::MAX_PARTICLES)
}

__global__ void d_run(u32 width, u32 height, RectI relevant_bounds, ParticleBrush brush, TiledGridInfo grid,
                      ByteColor *image_pixels, ByteColor *current_pixels, vec2 *displacement_map, vec2 *particle_positions) {

    grid.tile_x = threadIdx.x;
    grid.tile_y = threadIdx.y;
    grid.column = blockIdx.x;
    grid.row    = blockIdx.y;
    grid.updateGlobalCoords();
    const i32 x = (i32)grid.x;
    const i32 y = (i32)grid.y;
    if (!relevant_bounds.contains(x, y))
        return;

    ByteColorImage image;
    image.width = width;
    image.height = height;
    image.content = image_pixels;
    brush.particle_positions = particle_positions;

    u32 pixel_offset = grid.getOffset();
    ByteColor &pixel = current_pixels[pixel_offset];
    vec2 &displacement = displacement_map[pixel_offset];

    brush.apply(pixel, x, y, image, grid, displacement);
}

template <typename T> void runOnGPU(const Image<T> &image, const Image<T> &current, TiledGridInfo &tiled_grid, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush) {
    dim3 dimBlock, dimGrid;
    dimBlock.x = image.tile_width;
    dimBlock.y= image.tile_height;
    dimGrid.x = tiled_grid.columns;
    dimGrid.y = tiled_grid.rows;

    d_run<<<dimGrid, dimBlock>>>(image.width, image.height, relevant_bounds, brush, tiled_grid,
                               d_image_pixels, d_current_pixels, d_displacement_map, d_particle_positions);

    checkErrors()

    downloadN(d_current_pixels, current.content, current.width * current.height)
    downloadN(d_displacement_map, displacement_map, current.width * current.height)
}