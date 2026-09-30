// render.material program `surface`: the entry point with the SSR targets
// (kSurfaceSsrTargets: attachments 1 to 3, render.pass.ssr's inputs). The
// program is surface_program.glsl.
#version 450

#define SURFACE_SSR_TARGETS
#include "surface_program.glsl"
