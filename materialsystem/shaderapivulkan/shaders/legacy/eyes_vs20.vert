#version 450
// A port of stdshaders/Eyes_vs20.fxc (Eyes_dx9's vertex stage). Combos
// (fxctmp9/eyes_vs20.inc): static INTRO (96), HALFLAMBERT (192),
// USE_STATIC_CONTROL_FLOW (384); dynamic COMPRESSED_VERTS (1), SKINNING (2),
// DOWATERFOG (4), DYNAMIC_LIGHT (8), STATIC_LIGHT (16), NUM_LIGHTS (32, 0..2).
// The vertex record already applied the flex deltas, the skinning and the
// vertex decompression; the fixed-function fog output is unused (pixel fog).
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"
#include "legacy_vortwarp_vs.glsl"

layout( location = 0 ) out vec2 baseTC;
layout( location = 1 ) out vec2 irisTC;
layout( location = 2 ) out vec2 glintTC;
layout( location = 3 ) out vec3 vColor;
layout( location = 7 ) out vec4 worldPos_projPosZ;

#define cEyeOrigin ( VS_C( 48 ).xyz )
#define cHalfEyeballUp ( VS_C( 49 ).xyz )
#define cIrisProjectionU VS_C( 50 )
#define cIrisProjectionV VS_C( 51 )
#define cGlintProjectionU VS_C( 52 )
#define cGlintProjectionV VS_C( 53 )
#define const4 VS_C( 54 )
#define g_Time ( const4.w )
#define modelOrigin ( const4.xyz )

void main()
{
	const bool INTRO = STATIC_VS_COMBO( 96, 2 ) != 0;
	const bool g_bHalfLambert = STATIC_VS_COMBO( 192, 2 ) != 0;
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 384, 2 ) != 0;
	const bool bDynamicLight = DYNAMIC_VS_COMBO( 8, 2 ) != 0;
	const bool bStaticLight = DYNAMIC_VS_COMBO( 16, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 32, 3 );

	vec3 worldPos = inWorldPos;
	vec3 dummy = worldPos;
	if ( INTRO )
		WorldSpaceVertexProcess( g_Time, modelOrigin, worldPos, dummy, dummy, dummy );

	// Transform into projection space
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	worldPos_projPosZ = vec4( worldPos.xyz, vProjPos.z );

	// Normal = (Pos - Eye origin) - just step on dummy normal created above
	vec3 worldNormal = worldPos - cEyeOrigin;

	// Normal -= 0.5f * (Normal dot Eye Up) * Eye Up
	float normalDotUp = -dot( worldNormal, cHalfEyeballUp ) * 0.5;
	worldNormal = normalize( normalDotUp * cHalfEyeballUp + worldNormal );

	// Vertex lighting
	if ( USE_STATIC_CONTROL_FLOW )
		vColor = DoLighting( worldPos, worldNormal, vec3( 0.0 ), bStaticLight, bDynamicLight,
		    g_bHalfLambert );
	else
		vColor = DoLightingUnrolled( worldPos, worldNormal, vec3( 0.0 ), bStaticLight,
		    bDynamicLight, g_bHalfLambert, NUM_LIGHTS );

	// Texture 0 is the base texture
	// Texture 1 is a planar projection used for the iris
	// Texture 2 is a planar projection used for the glint
	baseTC = inTexCoord0;
	irisTC.x = dot( cIrisProjectionU, vec4( worldPos, 1.0 ) );
	irisTC.y = dot( cIrisProjectionV, vec4( worldPos, 1.0 ) );
	glintTC.x = dot( cGlintProjectionU, vec4( worldPos, 1.0 ) );
	glintTC.y = dot( cGlintProjectionV, vec4( worldPos, 1.0 ) );
}
