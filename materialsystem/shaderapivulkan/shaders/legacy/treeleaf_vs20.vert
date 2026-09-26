#version 450
// A port of stdshaders/treeleaf_vs20.fxc (TreeLeaf's vertex stage): a
// camera-facing leaf quad expanded from the cluster point along the view's x
// and y axes by TEXCOORD0, lit (half-Lambert) with the normal from the leaf
// center to the point; the texture coordinate is the NORMAL stream's xy.
// Combos (fxctmp9/treeleaf_vs20.inc): static HALFLAMBERT (12),
// USE_STATIC_CONTROL_FLOW (24); dynamic DYNAMIC_LIGHT (1), STATIC_LIGHT (2),
// NUM_LIGHTS (4, 0..2). The position arrives untransformed (object_position)
// because the normal is taken from the object-space position; cModel[0]
// places it. The NORMAL stream is the vertex record's normal, which the
// MODEL rotation (the identity for the world's leaves) has turned.
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"

layout( location = 0 ) out vec2 texCoord;
layout( location = 8 ) out vec3 color;

#define cLeafCenter VS_C( 48 )
#define cModel0_0 VS_C( 58 )
#define cModel0_1 VS_C( 59 )
#define cModel0_2 VS_C( 60 )

void main()
{
	const bool g_bHalfLambert = STATIC_VS_COMBO( 12, 2 ) != 0;
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 24, 2 ) != 0;
	const bool bDynamicLight = DYNAMIC_VS_COMBO( 1, 2 ) != 0;
	const bool bStaticLight = DYNAMIC_VS_COMBO( 2, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 4, 3 );

	const vec4 vPos = vec4( inWorldPos, 1.0 );
	vec3 worldPos = vec3( dot( vPos, cModel0_0 ), dot( vPos, cModel0_1 ), dot( vPos, cModel0_2 ) );

	const vec3 normal = normalize( vPos.xyz - cLeafCenter.xyz );
	const vec3 worldNormal = vec3( dot( normal, cModel0_0.xyz ), dot( normal, cModel0_1.xyz ),
	    dot( normal, cModel0_2.xyz ) );

	vec3 lighting;
	if ( USE_STATIC_CONTROL_FLOW )
		lighting = DoLighting(
		    worldPos, worldNormal, vec3( 0.0 ), bStaticLight, bDynamicLight, g_bHalfLambert );
	else
		lighting = DoLightingUnrolled( worldPos, worldNormal, vec3( 0.0 ), bStaticLight,
		    bDynamicLight, g_bHalfLambert, NUM_LIGHTS );

	// float3( cViewModel[0].x, cViewModel[1].x, cViewModel[2].x ) and the y
	// column: registers c17 and c18.
	const vec3 xAxis = VS_C( 17 ).xyz;
	const vec3 yAxis = VS_C( 18 ).xyz;

	worldPos += xAxis * inTexCoord0.x;
	worldPos += yAxis * ( 1.0 - inTexCoord0.y );

	gl_Position = LegacyProject( worldPos );
	texCoord = inWorldNormal.xy;
	color = lighting;
}
