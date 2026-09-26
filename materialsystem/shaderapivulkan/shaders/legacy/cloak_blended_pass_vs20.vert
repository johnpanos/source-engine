#version 450
// A port of stdshaders/cloak_blended_pass_vs20.fxc (VertexLitGeneric's
// $cloakpassenabled pass). Combos (fxctmp9/cloak_blended_pass_vs20.inc):
// static BUMPMAP (4); dynamic COMPRESSED_VERTS (1), SKINNING (2), which the
// shader API's vertex record already applied. Without BUMPMAP the HLSL
// outputs no tangent frame or texture coordinate; nothing reads them then.
#include "legacy_vs.glsl"
#include "legacy_blended_pass_vs.glsl"

void main()
{
	BlendedPassVertex( STATIC_VS_COMBO( 4, 2 ) != 0 );
}
