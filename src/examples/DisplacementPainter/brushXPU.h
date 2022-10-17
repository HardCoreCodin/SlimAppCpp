#pragma once

#include "./brushCPU.h"

#ifdef __CUDACC__
#include "./brushGPU.h"
#define USE_GPU_BY_DEFAULT true

template <typename T> void runOnXPU(const Image<T> &image, const Image<T> &current, TiledGridInfo &grid, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush, bool on_gpu = false) {
    if (on_gpu) runOnGPU(image, current, grid, displacement_map, relevant_bounds, brush);
    else        runOnCPU(image, current, grid, displacement_map, relevant_bounds, brush);
}

#else
#define USE_GPU_BY_DEFAULT false

template <typename T> void allocateDeviceMemory(const Image<T> &image) {}
template <typename T> void uploadImage(const Image<T> &image) {}
template <typename T> void uploadCurrent(const Image<T> &current, const vec2 *displacement_map) {}
void uploadBrushParticlePositions(vec2 *particle_positions) {}

template <typename T> void runOnXPU(const Image<T> &image, const Image<T> &current, TiledGridInfo &grid, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush, bool on_gpu = false) {
    runOnCPU(image, current, grid, displacement_map, relevant_bounds, brush);
}
#endif