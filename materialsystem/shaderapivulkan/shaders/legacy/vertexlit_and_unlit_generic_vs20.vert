#version 450
// UnlitGeneric and VertexLitGeneric's vertex stage: a port of
// stdshaders/vertexlit_and_unlit_generic_vs20.fxc. Combos
// (fxctmp9/vertexlit_and_unlit_generic_vs20.inc): static VERTEXCOLOR (192),
// CUBEMAP (384), HALFLAMBERT (768), FLASHLIGHT (1536), SEAMLESS_BASE (3072),
// SEAMLESS_DETAIL (6144), SEPARATE_DETAIL_UVS (12288), USE_STATIC_CONTROL_FLOW
// (24576), DONT_GAMMA_CONVERT_VERTEX_COLOR (49152); dynamic COMPRESSED_VERTS
// (1), DYNAMIC_LIGHT (2), STATIC_LIGHT (4), DOWATERFOG (8), SKINNING (16),
// LIGHTING_PREVIEW (32), NUM_LIGHTS (64, 0..2).
// The vertex record arrives skinned and decompressed (world position and
// normal), so COMPRESSED_VERTS and SKINNING need nothing here. vSpecular (the
// static-prop color mesh, COLOR1) is the record's color when the pass's format
// has no vertex color. The seamless coordinates read the object-space POSITION
// (object_position_extra); their weights use the record's normal, which is the
// object-space normal the HLSL reads only when the MODEL rotation is the
// identity. The fog outputs (FOG, COLOR1) feed only fixed-function fog, which
// the ps20b pixel stage replaces with its own.
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"

layout( location = 0 ) out vec3 baseTexCoord;   // xy, or xyz with SEAMLESS_BASE
layout( location = 1 ) out vec3 detailTexCoord; // xy, or xyz with SEAMLESS_DETAIL
layout( location = 2 ) out vec4 color;          // vertex color (from lighting or unlit)
layout( location = 3 ) out vec3 worldVertToEyeVector;
layout( location = 4 ) out vec3 worldSpaceNormal;
layout( location = 6 ) out vec4 vProjPosOut;
layout( location = 7 ) out vec4 worldPos_ProjPosZ;
layout( location = 8 ) out vec3 SeamlessWeights;
layout( location = 9 ) out vec4 fogFactorW;

#define cBaseTexCoordTransform_0 VS_C( 48 )
#define cBaseTexCoordTransform_1 VS_C( 49 )
#define cSeamlessScale ( VS_C( 50 ).x )
#define cDetailTexCoordTransform_0 VS_C( 52 )
#define cDetailTexCoordTransform_1 VS_C( 53 )
#define cViewProjZ VS_C( 13 )

void main()
{
	const bool g_bVertexColor = STATIC_VS_COMBO( 192, 2 ) != 0;
	const bool CUBEMAP = STATIC_VS_COMBO( 384, 2 ) != 0;
	const bool g_bHalfLambert = STATIC_VS_COMBO( 768, 2 ) != 0;
	const bool FLASHLIGHT = STATIC_VS_COMBO( 1536, 2 ) != 0;
	const bool SEAMLESS_BASE = STATIC_VS_COMBO( 3072, 2 ) != 0;
	const bool SEAMLESS_DETAIL = STATIC_VS_COMBO( 6144, 2 ) != 0;
	const bool SEPARATE_DETAIL_UVS = STATIC_VS_COMBO( 12288, 2 ) != 0;
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 24576, 2 ) != 0;
	const bool DONT_GAMMA_CONVERT_VERTEX_COLOR = STATIC_VS_COMBO( 49152, 2 ) != 0;
	const bool bDynamicLight = DYNAMIC_VS_COMBO( 2, 2 ) != 0;
	const bool bStaticLight = DYNAMIC_VS_COMBO( 4, 2 ) != 0;
	const bool DOWATERFOG = DYNAMIC_VS_COMBO( 8, 2 ) != 0;
	const bool LIGHTING_PREVIEW = DYNAMIC_VS_COMBO( 32, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 64, 3 );

	const vec3 objectPos = inExtra.xyz;
	if ( SEAMLESS_BASE || SEAMLESS_DETAIL )
	{
		const vec3 NNormal = normalize( inWorldNormal );
		SeamlessWeights = NNormal * NNormal; // sums to 1.
	}
	else
	{
		SeamlessWeights = vec3( 0.0 );
	}

	const vec3 worldPos = inWorldPos;
	vec3 worldNormal = inWorldNormal;
	if ( !g_bVertexColor )
		worldNormal = normalize( worldNormal );
	worldSpaceNormal = worldNormal;

	// Transform into projection space.
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), cViewProjZ );
	vProjPosOut = vProjPos;
	// CalcFog: RangeFog, or in vs20 1 for water fog (done in the pixel shader).
	fogFactorW = vec4( 0.0, 0.0, 0.0, DOWATERFOG ? 1.0 : RangeFog( vProjPos.xyz ) );
	worldPos_ProjPosZ = vec4( worldPos, vProjPos.z );

	worldVertToEyeVector = CUBEMAP ? cEyePos - worldPos : vec3( 0.0 );

	if ( FLASHLIGHT )
	{
		color = vec4( 0.0 );
	}
	else if ( g_bVertexColor )
	{
		// Assume that this is unlitgeneric if you are using vertex color.
		color.rgb = DONT_GAMMA_CONVERT_VERTEX_COLOR ? inColor : GammaToLinear( inColor );
		color.a = inAlpha;
	}
	else
	{
		if ( USE_STATIC_CONTROL_FLOW )
			color.xyz = DoLighting(
			    worldPos, worldNormal, inColor, bStaticLight, bDynamicLight, g_bHalfLambert );
		else
			color.xyz = DoLightingUnrolled( worldPos, worldNormal, inColor, bStaticLight,
			    bDynamicLight, g_bHalfLambert, NUM_LIGHTS );
		color.w = 0.0;
	}

	if ( SEAMLESS_BASE )
		baseTexCoord = cSeamlessScale * objectPos;
	else
		baseTexCoord = vec3(
		    DotTexTransform( inTexCoord0, cBaseTexCoordTransform_0, cBaseTexCoordTransform_1 ),
		    0.0 );

	if ( SEAMLESS_DETAIL )
		detailTexCoord = ( cSeamlessScale * cDetailTexCoordTransform_0.x ) * objectPos;
	else
		detailTexCoord = vec3(
		    DotTexTransform( inTexCoord0, cDetailTexCoordTransform_0, cDetailTexCoordTransform_1 ),
		    0.0 );
	if ( SEPARATE_DETAIL_UVS )
		detailTexCoord.xy = inTexCoord1;

	// float dot = 0.5 + 0.5 * worldNormal * float3( 0.7071, 0.7071, 0 ), which
	// fxc truncates to the x component.
	if ( LIGHTING_PREVIEW )
		color.xyz = vec3( 0.5 + 0.5 * worldNormal.x * 0.7071 );
}
