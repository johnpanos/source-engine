// Vertex-stage helpers of the screen-space legacy ports: vertex shaders that
// output POSITION as is (a clip-space quad) or through cModelViewProj. Their
// ports register flags=object_position, so inWorldPos is the POSITION stream.
#ifndef LEGACY_SCREEN_VS_GLSL
#define LEGACY_SCREEN_VS_GLSL

#include "legacy_vs.glsl"

// The vertex shader's oPos, with the user clip planes applied to it.
vec4 LegacyClipSpace( vec4 projPos )
{
	gl_ClipDistance[0] = dot( pc.clipPlanes[0], projPos );
	gl_ClipDistance[1] = dot( pc.clipPlanes[1], projPos );
	return projPos;
}

// mul( pos, cModelViewProj ): c4..c7 hold the matrix's columns.
vec4 MulModelViewProj( vec4 pos )
{
	return vec4( dot( pos, VS_C( 4 ) ), dot( pos, VS_C( 5 ) ), dot( pos, VS_C( 6 ) ),
	    dot( pos, VS_C( 7 ) ) );
}

#endif // LEGACY_SCREEN_VS_GLSL
