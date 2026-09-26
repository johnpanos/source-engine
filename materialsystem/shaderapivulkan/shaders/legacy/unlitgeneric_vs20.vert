#version 450
// A port of stdshaders/unlitgeneric_vs20.fxc (Modulate and other unlit
// shaders' vertex stage). Combos (fxctmp9/unlitgeneric_vs20.inc): static
// VERTEXCOLOR (8); dynamic COMPRESSED_VERTS (1), DOWATERFOG (2), SKINNING (4),
// which the shader API's vertex record already applied.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 vTexCoord0;
layout( location = 1 ) out vec2 vTexCoord1;
layout( location = 2 ) out vec2 vTexCoord2;
layout( location = 3 ) out vec2 vTexCoord3;
layout( location = 7 ) out vec4 worldPos_projPosZ;
layout( location = 8 ) out vec4 vColor;
layout( location = 9 ) out vec4 fogFactorW;

#define cBaseTextureTransform_0 VS_C( 48 )
#define cBaseTextureTransform_1 VS_C( 49 )
#define cDetailTextureTransform_0 VS_C( 52 )
#define cDetailTextureTransform_1 VS_C( 53 )
#define g_vVertexColor VS_C( 54 )

void main()
{
	const bool VERTEXCOLOR = STATIC_VS_COMBO( 8, 2 ) != 0;
	const vec3 worldPos = inWorldPos;
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	worldPos_projPosZ = vec4( worldPos, vProjPos.z );
	fogFactorW = vec4( RangeFog( vProjPos.xyz ) );

	// mul( v.vTexCoord0, (float2x4)cBaseTextureTransform ) and the detail's.
	vTexCoord0 =
	    MulFloat2x4TexTransform( inTexCoord0, cBaseTextureTransform_0, cBaseTextureTransform_1 );
	vTexCoord1 = vec2( 0.0 );
	vTexCoord2 = vec2( 0.0 );
	vTexCoord3 = MulFloat2x4TexTransform(
	    inTexCoord0, cDetailTextureTransform_0, cDetailTextureTransform_1 );

	vColor = cModulationColor;
	if ( VERTEXCOLOR )
		vColor = mix( vColor, vColor * vec4( inColor, inAlpha ), g_vVertexColor.x );
}
