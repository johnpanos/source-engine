#version 450
// A port of stdshaders/weapon_sheen_pass_vs20.fxc (VertexLitGeneric's
// $sheenpassenabled pass). Combos (fxctmp9/weapon_sheen_pass_vs20.inc): static
// BUMPMAP (4); dynamic COMPRESSED_VERTS (1), SKINNING (2), which the shader
// API's vertex record already applied. The model-space position is the
// POSITION stream as the mesh holds it (object_position_extra).
#include "legacy_vs.glsl"
#include "legacy_blended_pass_vs.glsl"

layout( location = 7 ) out vec4 vModelSpacePos;

void main()
{
	BlendedPassVertex( STATIC_VS_COMBO( 4, 2 ) != 0 );
	vModelSpacePos = vec4( inExtra.xyz, 0.0 );
}
