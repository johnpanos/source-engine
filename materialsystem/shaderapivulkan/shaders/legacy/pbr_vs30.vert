#version 450
// The PBR shader's vertex stage (materialsystem/stdshaders/pbr_dx9.cpp, which
// selects the vs30/ps30 pair on shader model 3): a port of
// stdshaders/pbr_vs30.fxc. Combos (fxctmp9/pbr_vs30.inc): dynamic
// COMPRESSED_VERTS (1), DOWATERFOG (2), SKINNING (4), LIGHTING_PREVIEW (8),
// NUM_LIGHTS (16, 0..4). The vertex record arrives skinned and decompressed;
// the fog output feeds only fixed-function fog, which the pixel stage replaces.
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"

layout( location = 0 ) out vec2 baseTexCoord;
layout( location = 1 ) out vec4 lightAtten;
layout( location = 2 ) out vec3 worldNormal;
layout( location = 3 ) out vec3 worldPosOut;
layout( location = 4 ) out vec3 projPos;
layout( location = 5 ) out vec4 lightmapTexCoord1And2;
layout( location = 6 ) out vec4 lightmapTexCoord3;

#define cBaseTexCoordTransform_0 VS_C( 48 )
#define cBaseTexCoordTransform_1 VS_C( 49 )
#define cViewProjZ VS_C( 13 )

void main()
{
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 16, 5 );

	// dot( float2 vTexCoord0, float4 transform ): the float2 dot product.
	const vec2 transformed = vec2( dot( inTexCoord0, cBaseTexCoordTransform_0.xy ),
	    dot( inTexCoord0, cBaseTexCoordTransform_1.xy ) );
	lightmapTexCoord3.zw = transformed + vec2( cBaseTexCoordTransform_0.w, cBaseTexCoordTransform_1.w );

	const vec2 offset = LegacyTexCoord2();
	lightmapTexCoord1And2.xy = inTexCoord1 + offset;
	const vec2 lightmapTexCoord2 = lightmapTexCoord1And2.xy + offset;
	const vec2 lightmapTexCoord3xy = lightmapTexCoord2 + offset;
	// Reversed component order
	lightmapTexCoord1And2.w = lightmapTexCoord2.x;
	lightmapTexCoord1And2.z = lightmapTexCoord2.y;
	lightmapTexCoord3.xy = lightmapTexCoord3xy;

	const vec3 worldPos = inWorldPos;
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), cViewProjZ );
	projPos = vProjPos.xyz;

	worldPosOut = worldPos;
	worldNormal = normalize( inWorldNormal );

	// Scalar attenuations for four lights
	lightAtten = vec4( 0.0 );
	if ( NUM_LIGHTS > 0 )
		lightAtten.x = GetVertexAttenForLight( worldPos, 0, false );
	if ( NUM_LIGHTS > 1 )
		lightAtten.y = GetVertexAttenForLight( worldPos, 1, false );
	if ( NUM_LIGHTS > 2 )
		lightAtten.z = GetVertexAttenForLight( worldPos, 2, false );
	if ( NUM_LIGHTS > 3 )
		lightAtten.w = GetVertexAttenForLight( worldPos, 3, false );

	baseTexCoord = transformed;
}
