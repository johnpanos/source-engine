#version 450
#define main SurfaceMain
#include "surface_program.glsl"
#undef main
layout( location = 11 ) in vec4 motionCurrent;
layout( location = 12 ) in vec4 motionPrevious;
layout( location = 1 ) out vec2 outMotion;
layout( location = 2 ) out float outMotionDepth;
void main()
{
    // Preserve the exact material's coverage, clip planes and visible color.
    SurfaceMain();
    // Blended surfaces can write motion without writing the scene depth buffer.
    // Keep the temporal input depth paired with that same visible surface.
    outMotionDepth = gl_FragCoord.z;
    const bool valid = frame.motionExtent.z > 0.5 && motionCurrent.w > 0.0 &&
        motionPrevious.w > 0.0 && !any( isnan( motionPrevious ) ) &&
        !any( isinf( motionPrevious ) );
    // Missing correspondence is explicit out-of-bounds motion, never stationary.
    outMotion = valid ? ( motionPrevious.xy / motionPrevious.w -
        motionCurrent.xy / motionCurrent.w ) * vec2( 0.5, -0.5 ) * frame.motionExtent.xy
        : vec2( 65504.0 );
}
