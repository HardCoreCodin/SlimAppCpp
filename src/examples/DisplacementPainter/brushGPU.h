#pragma once

#include "./brush.h"

Pixel *d_image_pixels;
Pixel *d_current_pixels;
vec2 *d_particle_positions;
vec2 *d_displacement_map;

void allocateDeviceMemory(const Image &image) {
    gpuErrchk(cudaMalloc(&d_image_pixels,     sizeof(Pixel)  * image.width * image.height))
    gpuErrchk(cudaMalloc(&d_current_pixels,   sizeof(Pixel)  * image.width * image.height))
    gpuErrchk(cudaMalloc(&d_displacement_map, sizeof(vec2)   * image.width * image.height))
    gpuErrchk(cudaMalloc(&d_particle_positions, sizeof(vec2)   * ParticleBrush::MAX_PARTICLES))
}

void uploadImage(const Image &image) {
    uploadN(image.pixels, d_image_pixels, image.width * image.height)
}

void uploadCurrent(const Image &current, const vec2 *displacement_map) {
    uploadN(current.pixels, d_current_pixels, current.width * current.height)
    uploadN(displacement_map, d_displacement_map, current.width * current.height)
}

void uploadBrushParticlePositions(vec2 *particle_positions) {
    uploadN(particle_positions, d_particle_positions, ParticleBrush::MAX_PARTICLES)
}

__global__ void d_run(u32 width, u32 height, u32 pixel_count, RectI relevant_bounds, ParticleBrush brush,
                      Pixel *image_pixels, Pixel *current_pixels, vec2 *displacement_map, vec2 *particle_positions) {
    u32 i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i >= pixel_count)
        return;

    i32 stride = relevant_bounds.right - relevant_bounds.left + 1;
    i32 x = relevant_bounds.left + (i32)i % stride;
    i32 y = relevant_bounds.top + (i32)i / stride;

    Image image;
    image.width = width;
    image.height = height;
    image.pixels = image_pixels;
    brush.particle_positions = particle_positions;

    i32 pixel_offset = (i32)width * y + x;
    Pixel &pixel = current_pixels[pixel_offset];
    vec2 &displacement = displacement_map[pixel_offset];

    brush.apply(pixel, x, y, image, displacement);
}

void runOnGPU(const Image &image, const Image &current, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush) {
    u32 pixel_count = (relevant_bounds.right - relevant_bounds.left + 1) * (relevant_bounds.bottom - relevant_bounds.top + 1);
    u32 threads = 512;
    u32 blocks  = pixel_count / threads;
    if (pixel_count < threads) {
        threads = pixel_count;
        blocks = 1;
    } else if (pixel_count % threads)
        blocks++;

    d_run<<<blocks, threads>>>(image.width, image.height, pixel_count, relevant_bounds, brush,
                               d_image_pixels, d_current_pixels, d_displacement_map, d_particle_positions);

    checkErrors()

    downloadN(d_current_pixels, current.pixels, current.width * current.height)
    downloadN(d_displacement_map, displacement_map, current.width * current.height)
}