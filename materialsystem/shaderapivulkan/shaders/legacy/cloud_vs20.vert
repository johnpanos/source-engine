#version 450
// A port of stdshaders/cloud_vs20.fxc (Cloud_dx9's vertex stage; no combos):
// the base and cloud-alpha texture transforms and the range fog factor the
// pixel stage turns into alpha.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 baseCoords;
layout( location = 1 ) out vec2 cloudAlphaCoords;
layout( location = 2 ) out float fogFactor;

#define g_matBaseTexCoordTransform_0 VS_C( 48 )
#define g_matBaseTexCoordTransform_1 VS_C( 49 )
#define g_matCloudTexCoordTransform_0 VS_C( 50 )
#define g_matCloudTexCoordTransform_1 VS_C( 51 )

void main()
{
	vec4 projPos = LegacyProject( inWorldPos );
	gl_Position = projPos;

	// CalcFog( vWorldPos, o.projPos, FOGTYPE_RANGE ): RangeFog on the
	// cModelViewProj z row.
	projPos.z = dot( vec4( inWorldPos, 1.0 ), VS_C( 10 ) ); // cViewProj[2]
	fogFactor = RangeFog( projPos.xyz );

	baseCoords = DotTexTransform(
	    inTexCoord0, g_matBaseTexCoordTransform_0, g_matBaseTexCoordTransform_1 );
	cloudAlphaCoords = DotTexTransform(
	    inTexCoord1, g_matCloudTexCoordTransform_0, g_matCloudTexCoordTransform_1 );
}
