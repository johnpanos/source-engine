#version 450
// VertexLitGeneric and UnlitGeneric's bumped vertex stage ($bumpmap or
// $lightwarptexture without $phong): a port of
// stdshaders/vertexlit_and_unlit_generic_bump_vs20.fxc. Combos
// (fxctmp9/VertexLit_and_unlit_Generic_bump_vs20.inc): static HALFLAMBERT
// (24), USE_WITH_2B (48), USE_STATIC_CONTROL_FLOW (96); dynamic
// COMPRESSED_VERTS (1), DOWATERFOG (2), SKINNING (4), NUM_LIGHTS (8, 0..2).
// The vertex record arrives skinned and decompressed, so COMPRESSED_VERTS and
// SKINNING need nothing here. The record carries no POSITION1/NORMAL1 flex
// stream: ApplyMorph adds deltas of zero, as D3D9 does without a flex mesh
// (and with cFlexScale zero). The fog outputs (FOG, COLOR1) feed only
// fixed-function fog, which the ps20b pixel stage replaces with its own.
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"
#include "legacy_vs_bumped_model.glsl"

layout( location = 0 ) out vec4 baseTexCoord2_tangentSpaceVertToEyeVectorXY;
layout( location = 1 ) out vec3 lightAtten;
layout( location = 2 ) out vec4 worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ;
layout( location = 3 ) out vec3 vWorldNormal;
layout( location = 4 ) out vec4 vWorldTangent;
// vProjPos with USE_WITH_2B, else the binormal.
layout( location = 5 ) out vec4 vProjPos_vWorldBinormal;
layout( location = 6 ) out vec4 worldPos_projPosZ;
layout( location = 7 ) out vec3 detailTexCoord_atten3;
layout( location = 9 ) out vec4 fogFactorW;

#define cBaseTexCoordTransform_0 VS_C( 48 )
#define cBaseTexCoordTransform_1 VS_C( 49 )
#define cDetailTexCoordTransform_0 VS_C( 52 )
#define cDetailTexCoordTransform_1 VS_C( 53 )
#define cViewProjZ VS_C( 13 )

void main()
{
	const bool USE_WITH_2B = STATIC_VS_COMBO( 48, 2 ) != 0;
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 96, 2 ) != 0;
	const bool DOWATERFOG = DYNAMIC_VS_COMBO( 2, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 8, 3 );

	// Perform skinning (the record's), then the tangent frame.
	vec3 worldNormal, worldTangentS, worldTangentT;
	LegacyModelTangentFrame( worldNormal, worldTangentS, worldTangentT );
	const vec3 worldPos = inWorldPos;

	vWorldNormal = worldNormal;
	// Propagate binormal sign in world tangent.w
	vWorldTangent = vec4( worldTangentS, LegacyTangentSign() );

	// Transform into projection space
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), cViewProjZ );

	if ( USE_WITH_2B )
		vProjPos_vWorldBinormal = vProjPos;
	else
		vProjPos_vWorldBinormal = vec4( worldTangentT, 0.0 );

	// CalcFog: RangeFog, or in vs20 1 for water fog (done in the pixel shader).
	fogFactorW = vec4( DOWATERFOG ? 1.0 : RangeFog( vProjPos.xyz ) );

	worldPos_projPosZ = vec4( worldPos, vProjPos.z );

	worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ =
	    vec4( normalize( cEyePos - worldPos ), 0.0 );

	lightAtten = vec3( 0.0 );
	detailTexCoord_atten3.z = 0.0;
	if ( !USE_STATIC_CONTROL_FLOW )
	{
		if ( NUM_LIGHTS > 0 )
			lightAtten.x = GetVertexAttenForLight( worldPos, 0, false );
		if ( NUM_LIGHTS > 1 )
			lightAtten.y = GetVertexAttenForLight( worldPos, 1, false );
		if ( NUM_LIGHTS > 2 )
			lightAtten.z = GetVertexAttenForLight( worldPos, 2, false );
		if ( NUM_LIGHTS > 3 )
			detailTexCoord_atten3.z = GetVertexAttenForLight( worldPos, 3, false );
	}
	else
	{
		// Scalar light attenuation
		lightAtten.x = GetVertexAttenForLight( worldPos, 0, true );
		lightAtten.y = GetVertexAttenForLight( worldPos, 1, true );
		lightAtten.z = GetVertexAttenForLight( worldPos, 2, true );
		detailTexCoord_atten3.z = GetVertexAttenForLight( worldPos, 3, true );
	}

	// Base texture coordinate transform
	baseTexCoord2_tangentSpaceVertToEyeVectorXY = vec4(
	    DotTexTransform( inTexCoord0, cBaseTexCoordTransform_0, cBaseTexCoordTransform_1 ), 0.0,
	    0.0 );

	// Detail texture coordinate transform
	detailTexCoord_atten3.xy =
	    DotTexTransform( inTexCoord0, cDetailTexCoordTransform_0, cDetailTexCoordTransform_1 );
}
