#version 450
#define SURFACE_INSTANCED
#include "surface_model_vertex.glsl"
// The model vertex program drawn GPU-driven (SurfaceVariant::instanced):
// the draw matrices per instance, from vertex attributes 8-15.
