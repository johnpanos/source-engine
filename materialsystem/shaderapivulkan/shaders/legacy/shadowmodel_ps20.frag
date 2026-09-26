#version 450
// ShadowModel_DX9's pixel stage: a port of stdshaders/shadowmodel_ps20.fxc (no
// combos). Texkill clips against the shadow volume and back faces; the color
// lerps from white by TEXCOORD0.w, which the vertex shader never writes.
// @legacy program=shadowmodel ps=shadowmodel_ps20 vs=shadowmodel_vs20 vert=shadowmodel_vs20
//         samplers=0:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0 (not read)

layout( location = 0 ) in vec4 T0;
layout( location = 1 ) in vec3 T1;
layout( location = 2 ) in vec3 T2;
layout( location = 3 ) in float T3;
layout( location = 8 ) in vec4 vColor; // COLOR0

void main()
{
	// Kill pixel against various fields computed in vertex shader
	if ( any( lessThan( T1, vec3( 0.0 ) ) ) || any( lessThan( T2, vec3( 0.0 ) ) ) || T3 < 0.0 )
		discard;

	LegacyWrite( vec4( mix( vec3( 1.0 ), vColor.xyz, T0.a ), 1.0 ) );
}
