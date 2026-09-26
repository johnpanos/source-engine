#version 450
// A port of stdshaders/shadowmodel_vs20.fxc (ShadowModel_DX9's vertex stage).
// Combos (fxctmp9/shadowmodel_vs20.inc): dynamic DOWATERFOG (1), which only
// feeds the fixed-function fog, and SKINNING (2), which the vertex record
// already applied. The shader writes TEXCOORD0's xyz only; its w reads 0 here,
// as D3D9's unwritten components do in the oracle's model of the pipeline.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec4 T0;
layout( location = 1 ) out vec3 T1;
layout( location = 2 ) out vec3 T2;
layout( location = 3 ) out float T3;
layout( location = 8 ) out vec4 vColor; // COLOR0

#define cShadowTextureMatrix( n ) VS_C( 48 + ( n ) )
#define cTexOrigin VS_C( 51 )
#define cTexScale VS_C( 52 )
#define cShadowConstants ( VS_C( 53 ).xyz )
#define flShadowFalloffOffset ( cShadowConstants.x )
#define flOneOverShadowDist ( cShadowConstants.y )
#define flShadowScale ( cShadowConstants.z )

void main()
{
	vec3 worldPos = inWorldPos;
	vec3 worldNormal = inWorldNormal;

	// Transform into projection space
	gl_Position = LegacyProject( worldPos );

	// Transform position into texture space (from 0 to 1)
	vec3 vTexturePos;
	vTexturePos.x = dot( worldPos.xyz, cShadowTextureMatrix( 0 ).xyz );
	vTexturePos.y = dot( worldPos.xyz, cShadowTextureMatrix( 1 ).xyz );
	vTexturePos.z = dot( worldPos.xyz, cShadowTextureMatrix( 2 ).xyz );

	// Figure out the shadow fade amount
	float flShadowFade = ( vTexturePos.z - flShadowFalloffOffset ) * flOneOverShadowDist;

	// Offset it into the texture
	T0 = vec4( vTexturePos * cTexScale.xyz + cTexOrigin.xyz, 0.0 );

	// We're doing clipping by using texkill
	T1.xyz = vTexturePos.xyz; // Also clips when shadow z < 0 !
	T2.xyz = vec3( 1.0 ) - vTexturePos.xyz;
	T2.z = 1.0 - flShadowFade; // Clips when shadow z > shadow distance

	// Transform z component of normal in texture space
	// If it's negative, then don't draw the pixel
	T3 = dot( worldNormal, -cShadowTextureMatrix( 2 ).xyz );

	// Shadow color, falloff (COLOR0 saturates)
	vColor.xyz = saturate( cModulationColor.xyz );
	vColor.w = saturate( flShadowFade * flShadowScale );
}
