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

__global__ void d_run(u32 width, u32 height, u32 pixel_count, RectI relevant_bounds, ParticleBrush brush, TiledGridInfo grid,
                      ByteColor *image_pixels, ByteColor *current_pixels, vec2 *displacement_map, vec2 *particle_positions) {
    u32 i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i >= pixel_count)
        return;

    i32 stride = relevant_bounds.right - relevant_bounds.left + 1;
    i32 x = relevant_bounds.left + (i32)i % stride;
    i32 y = relevant_bounds.top + (i32)i / stride;

    ByteColorImage image;
    image.width = width;
    image.height = height;
    image.content = image_pixels;
    brush.particle_positions = particle_positions;

    u32 pixel_offset = grid.getOffset(x, y);
    ByteColor &pixel = current_pixels[pixel_offset];
    vec2 &displacement = displacement_map[pixel_offset];

    brush.apply(pixel, x, y, image, grid, displacement);
}

template <typename T> void runOnGPU(const Image<T> &image, const Image<T> &current, TiledGridInfo &grid, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush) {
    u32 pixel_count = (relevant_bounds.right - relevant_bounds.left + 1) * (relevant_bounds.bottom - relevant_bounds.top + 1);
    u32 threads = 512;
    u32 blocks  = pixel_count / threads;
    if (pixel_count < threads) {
        threads = pixel_count;
        blocks = 1;
    } else if (pixel_count % threads)
        blocks++;

    d_run<<<blocks, threads>>>(image.width, image.height, pixel_count, relevant_bounds, brush, grid,
                               d_image_pixels, d_current_pixels, d_displacement_map, d_particle_positions);

    checkErrors()

    downloadN(d_current_pixels, current.content, current.width * current.height)
    downloadN(d_displacement_map, displacement_map, current.width * current.height)
}