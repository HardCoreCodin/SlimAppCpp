#pragma once

#include "./brushCPU.h"

#ifdef __CUDACC__
#include "./brushGPU.h"
#define USE_GPU_BY_DEFAULT true

void runOnXPU(const Image &image, const Image &current, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush, bool on_gpu = false) {
    if (on_gpu) runOnGPU(image, current, displacement_map, relevant_bounds, brush);
    else        runOnCPU(image, current, displacement_map, relevant_bounds, brush);
}

#else
#define USE_GPU_BY_DEFAULT false

void allocateDeviceMemory(const Image &image) {}
void uploadImage(const Image &image) {}
void uploadCurrent(const Image &current, const vec2 *displacement_map) {}
void uploadBrushParticlePositions(const ParticleBrush &brush) {}

void runOnXPU(const Image &image, const Image &current, vec2 *displacement_map, const RectI &relevant_bounds, ParticleBrush &brush, bool on_gpu = false) {
    runOnCPU(image, current, displacement_map, relevant_bounds, brush);
}
#endif