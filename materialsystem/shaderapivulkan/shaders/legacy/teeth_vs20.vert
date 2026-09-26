#version 450
// A port of stdshaders/teeth_vs20.fxc (Teeth_DX9's vertex stage without a bump
// map). Combos (fxctmp9/teeth_vs20.inc): static INTRO (96),
// USE_STATIC_CONTROL_FLOW (192); dynamic COMPRESSED_VERTS (1), DOWATERFOG (2),
// SKINNING (4), DYNAMIC_LIGHT (8), STATIC_LIGHT (16), NUM_LIGHTS (32, 0..2).
// The vertex record already applied the skinning and the vertex decompression;
// its world normal is the model normal rotated, which is the HLSL's
// normalize( normal ) rotated for the unit-length normals models carry. The
// fixed-function fog output is unused (pixel fog).
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"
#include "legacy_vortwarp_vs.glsl"

layout( location = 0 ) out vec2 baseTexCoord;
layout( location = 1 ) out vec3 vertAtten;
layout( location = 7 ) out vec4 worldPos_projPosZ;

#define cTeethLighting VS_C( 48 )
#define const4 VS_C( 49 )
#define g_Time ( const4.w )
#define modelOrigin ( const4.xyz )

void main()
{
	const bool INTRO = STATIC_VS_COMBO( 96, 2 ) != 0;
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 192, 2 ) != 0;
	const bool bDynamicLight = DYNAMIC_VS_COMBO( 8, 2 ) != 0;
	const bool bStaticLight = DYNAMIC_VS_COMBO( 16, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 32, 3 );

	vec3 worldPos = inWorldPos;
	vec3 worldNormal = inWorldNormal;
	if ( INTRO )
	{
		vec3 dummy = vec3( 0.0 );
		WorldSpaceVertexProcess( g_Time, modelOrigin, worldPos, worldNormal, dummy, dummy );
	}

	// Transform into projection space
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	worldPos_projPosZ = vec4( worldPos.xyz, vProjPos.z );

	// Compute lighting
	vec3 linearColor;
	if ( USE_STATIC_CONTROL_FLOW )
		linearColor = DoLighting( worldPos, worldNormal, vec3( 0.0 ), bStaticLight, bDynamicLight, false );
	else
		linearColor = DoLightingUnrolled( worldPos, worldNormal, vec3( 0.0 ), bStaticLight,
		    bDynamicLight, false, NUM_LIGHTS );

	// Forward vector
	vec3 vForward = cTeethLighting.xyz;
	float fIllumFactor = cTeethLighting.w;

	// Darken by forward dot normal and illumination factor
	linearColor *= fIllumFactor * saturate( dot( worldNormal, vForward ) );

	vertAtten = linearColor;
	baseTexCoord = inTexCoord0;
}
