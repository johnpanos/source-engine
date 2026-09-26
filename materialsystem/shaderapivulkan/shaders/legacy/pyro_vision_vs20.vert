#version 450
// A port of stdshaders/pyro_vision_vs20.fxc (pyro_vision's vertex stage). Combos
// (fxctmp9/pyro_vision_vs20.inc): static EFFECT (48, 0..3), VERTEXCOLOR (192),
// HALFLAMBERT (384), VERTEX_LIT (768), FULLBRIGHT (1536),
// USE_STATIC_CONTROL_FLOW (3072), BASETEXTURE2 (6144), STRIPES (12288),
// STRIPES_USE_NORMAL2 (24576); dynamic COMPRESSED_VERTS (1) and SKINNING (2),
// which the shader API's vertex record already applied, DYNAMIC_LIGHT (4),
// STATIC_LIGHT (8), NUM_LIGHTS (16, 0..2).
//
// The record's position is the POSITION stream skinned or through the MODEL
// matrix: EFFECT 2 and 3 output POSITION as is, which it equals for the
// screen-space quads they draw (identity model matrix). The record has no
// COLOR1 (the static prop lighting stream STATIC_LIGHT adds), so the port
// reads it as black.
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"

layout( location = 0 ) out vec4 vBaseAndSeamlessTexCoord; // EFFECT 2, 3: vBaseTexCoord in xy
layout( location = 1 ) out vec2 vStripeSeamlessTexCoord;
layout( location = 2 ) out vec2 vLightmapBlendTexCoord;
layout( location = 3 ) out vec3 vBlendFactor;
layout( location = 4 ) out vec4 worldPos_projPosZ;
layout( location = 5 ) out vec3 vWorldNormalOut;
layout( location = 8 ) out vec4 vVertexColor;

#define g_vPyroParms1 VS_C( 48 )
#define g_vStripeScale ( g_vPyroParms1.xyz )
#define g_vPyroParms2 VS_C( 49 )
#define g_vCanvasScale ( g_vPyroParms2.xyz )
#define cBlendMaskTexCoordTransform_0 VS_C( 14 )
#define cBlendMaskTexCoordTransform_1 VS_C( 15 )

void main()
{
	const int EFFECT = STATIC_VS_COMBO( 48, 4 );
	const bool VERTEXCOLOR = STATIC_VS_COMBO( 192, 2 ) != 0;
	const bool HALFLAMBERT = STATIC_VS_COMBO( 384, 2 ) != 0;
	const bool VERTEX_LIT = STATIC_VS_COMBO( 768, 2 ) != 0;
	const bool FULLBRIGHT = STATIC_VS_COMBO( 1536, 2 ) != 0;
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 3072, 2 ) != 0;
	const bool BASETEXTURE2 = STATIC_VS_COMBO( 6144, 2 ) != 0;
	const bool STRIPES = STATIC_VS_COMBO( 12288, 2 ) != 0;
	const bool DYNAMIC_LIGHT = DYNAMIC_VS_COMBO( 4, 2 ) != 0;
	const bool STATIC_LIGHT = DYNAMIC_VS_COMBO( 8, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 16, 3 );

	vBaseAndSeamlessTexCoord = vec4( 0.0 );
	vStripeSeamlessTexCoord = vec2( 0.0 );
	vLightmapBlendTexCoord = vec2( 0.0 );
	vBlendFactor = vec3( 0.0 );
	worldPos_projPosZ = vec4( 0.0 );
	vWorldNormalOut = vec3( 0.0 );
	vVertexColor = vec4( 0.0 );

	if ( EFFECT == 2 || EFFECT == 3 )
	{
		gl_Position = LegacyProject( inWorldPos );
		vBaseAndSeamlessTexCoord.xy = inTexCoord0;
		return;
	}

	// SkinPositionAndNormal of the NORMAL stream (VERTEX_LIT) or of ( 0, 0, 1 ).
	const vec3 vWorldPos = inWorldPos;
	vec3 vWorldNormal =
	    VERTEX_LIT ? inWorldNormal : vec3( VS_C( 58 ).z, VS_C( 59 ).z, VS_C( 60 ).z );

	vec4 vProjPos = LegacyProject( vWorldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( vWorldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ

	worldPos_projPosZ = vec4( vWorldPos, vProjPos.z );

	vWorldNormal = normalize( vWorldNormal );
	vWorldNormalOut = vWorldNormal;

	vBaseAndSeamlessTexCoord.xy = inTexCoord0;
	float flZFactor = vWorldPos.z * g_vCanvasScale.z;
	vBaseAndSeamlessTexCoord.zw =
	    vec2( vWorldPos.x + flZFactor, vWorldPos.y - flZFactor ) * g_vCanvasScale.xy;

	if ( STRIPES )
	{
		vec3 vWorldStripeCoord = vWorldPos * g_vStripeScale;
		vStripeSeamlessTexCoord.xy =
		    vWorldStripeCoord.yz + vWorldStripeCoord.xz + vWorldStripeCoord.xy;
	}

	if ( !VERTEX_LIT )
		vLightmapBlendTexCoord.xy = inTexCoord1;

	if ( BASETEXTURE2 )
	{
		vBlendFactor.x = dot( inTexCoord0, cBlendMaskTexCoordTransform_0.xy ) +
		                 cBlendMaskTexCoordTransform_0.w;
		vBlendFactor.y = dot( inTexCoord0, cBlendMaskTexCoordTransform_1.xy ) +
		                 cBlendMaskTexCoordTransform_1.w;
		vBlendFactor.z = inAlpha;
	}

	if ( VERTEXCOLOR )
	{
		vVertexColor = vec4( inColor, inAlpha );
	}
	else if ( VERTEX_LIT && !FULLBRIGHT )
	{
		const vec3 vSpecular = vec3( 0.0 ); // COLOR1, absent from the vertex record
		if ( USE_STATIC_CONTROL_FLOW )
			vVertexColor.xyz = DoLighting( vWorldPos, vWorldNormal, vSpecular, STATIC_LIGHT,
			    DYNAMIC_LIGHT, HALFLAMBERT );
		else
			vVertexColor.xyz = DoLightingUnrolled( vWorldPos, vWorldNormal, vSpecular,
			    STATIC_LIGHT, DYNAMIC_LIGHT, HALFLAMBERT, NUM_LIGHTS );
		vVertexColor.a = 1.0;
	}
	else
	{
		vVertexColor = vec4( 1.0 );
	}
}
