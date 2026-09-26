#version 450
// A port of stdshaders/shatteredglass_vs20.fxc. Combos
// (fxctmp9/ShatteredGlass_vs20.inc): static ENVMAP_MASK (2, no effect on the
// outputs); dynamic DOWATERFOG (1), whose vs_2_0 fog factor is 1. The detail
// and envmap-mask coordinates come from TEXCOORD2.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 baseTexCoord;
layout( location = 1 ) out vec2 detailTexCoord;
layout( location = 2 ) out vec2 lightmapTexCoord;
layout( location = 3 ) out vec2 envmapMaskTexCoord;
layout( location = 4 ) out vec4 worldPos_projPosZ;
layout( location = 5 ) out vec3 worldNormal;
layout( location = 8 ) out vec4 vertexColor;
layout( location = 9 ) out vec4 fogFactorW;

#define cBaseTexCoordTransform_0 VS_C( 48 )
#define cBaseTexCoordTransform_1 VS_C( 49 )
#define cDetailTexCoordTransform_0 VS_C( 50 )
#define cDetailTexCoordTransform_1 VS_C( 51 )

// dot( float2 uv, float4 row ) + row.w: fxc truncates the dot to two terms.
float Dot2PlusW( vec2 uv, vec4 row )
{
	return dot( uv, row.xy ) + row.w;
}

void main()
{
	const bool DOWATERFOG = DYNAMIC_VS_COMBO( 1, 2 ) != 0;

	const vec3 worldPos = inWorldPos;
	vec4 projPos = LegacyProject( worldPos );
	gl_Position = projPos;
	projPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ

	worldPos_projPosZ = vec4( worldPos, projPos.z );
	worldNormal = inWorldNormal;
	baseTexCoord = vec2( Dot2PlusW( inTexCoord0, cBaseTexCoordTransform_0 ),
	    Dot2PlusW( inTexCoord0, cBaseTexCoordTransform_1 ) );
	const vec2 vDetailTexCoord = LegacyTexCoord2();
	detailTexCoord = vec2( Dot2PlusW( vDetailTexCoord, cDetailTexCoordTransform_0 ),
	    Dot2PlusW( vDetailTexCoord, cDetailTexCoordTransform_1 ) );
	envmapMaskTexCoord = detailTexCoord;
	lightmapTexCoord = inTexCoord1;

	fogFactorW = vec4( DOWATERFOG ? 1.0 : RangeFog( projPos.xyz ) );
	vertexColor = cModulationColor;
}
