#version 450
// A port of stdshaders/emissive_scroll_blended_pass_vs20.fxc (VertexLitGeneric's
// $emissiveblendenabled pass). Combos (fxctmp9/
// emissive_scroll_blended_pass_vs20.inc): dynamic COMPRESSED_VERTS (1),
// SKINNING (2), which the shader API's vertex record already applied.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 vTexCoord0;

void main()
{
	gl_Position = LegacyProject( inWorldPos );
	vTexCoord0 = inTexCoord0;
}
