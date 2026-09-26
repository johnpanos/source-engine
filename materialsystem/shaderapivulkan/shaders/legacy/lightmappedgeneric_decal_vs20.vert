#version 450
// A port of stdshaders/lightmappedgeneric_decal_vs20.fxc
// (DecalBaseTimesLightmapAlphaBlendSelfIllum's first pass). Combos
// (fxctmp9/lightmappedgeneric_decal_vs20.inc): dynamic DOWATERFOG (1), whose
// vs_2_0 fog factor is 1. The three bumped lightmap coordinates step by
// TEXCOORD2.x.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 vTexCoord0;
layout( location = 1 ) out vec2 vTexCoord1;
layout( location = 2 ) out vec2 vTexCoord2;
layout( location = 3 ) out vec2 vTexCoord3;
layout( location = 4 ) out vec4 worldPos_projPosZ;
layout( location = 8 ) out vec4 vColor;
layout( location = 9 ) out vec4 fogFactorW;

#define cShaderConst0 VS_C( 48 )
#define cShaderConst1 VS_C( 49 )

void main()
{
	const bool DOWATERFOG = DYNAMIC_VS_COMBO( 1, 2 ) != 0;

	const vec3 worldPos = inWorldPos;
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	worldPos_projPosZ = vec4( worldPos, vProjPos.z );

	fogFactorW = vec4( DOWATERFOG ? 1.0 : RangeFog( vProjPos.xyz ) );

	// Compute the texture coordinates given the offset between
	// each bumped lightmap
	const vec2 offset = vec2( LegacyTexCoord2().x, 0.0 );

	const vec4 texCoord0 = vec4( inTexCoord0, 0.0, 1.0 );
	vTexCoord0 = vec2( dot( texCoord0, cShaderConst0 ), dot( texCoord0, cShaderConst1 ) );

	vTexCoord1 = offset + inTexCoord1;
	vTexCoord2 = ( offset * 2.0 ) + inTexCoord1;
	vTexCoord3 = ( offset * 3.0 ) + inTexCoord1;

	vColor = vec4( inColor, inAlpha );
}
