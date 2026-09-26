#version 450
// VertexLitGeneric's $phong vertex stage: a port of stdshaders/skin_vs20.fxc.
// Combos (fxctmp9/skin_vs20.inc): static USE_STATIC_CONTROL_FLOW (48);
// dynamic COMPRESSED_VERTS (1), DOWATERFOG (2), SKINNING (4), LIGHTING_PREVIEW
// (8), NUM_LIGHTS (16, 0..2). The vertex record arrives skinned and
// decompressed, so COMPRESSED_VERTS and SKINNING need nothing here;
// LIGHTING_PREVIEW and DOWATERFOG change only the fixed-function FOG output,
// which the ps20b pixel stage replaces with its own fog.
//
// The record carries no POSITION1/NORMAL1 flex stream, so ApplyMorph adds deltas
// of zero and the wrinkle weight it outputs (vPosFlex.w * cFlexScale.y, the
// flex stream's w) is zero, as D3D9 computes it without a flex mesh. Wrinkle
// maps need that stream's w per vertex in the record, and cFlexScale (c3) set
// as D3D9's mesh sets it when a flex mesh is bound.
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"
#include "legacy_vs_bumped_model.glsl"

layout( location = 0 ) out vec4 baseTexCoord; // includes detail tex coord
layout( location = 1 ) out vec3 lightAtten;
layout( location = 2 ) out vec3 worldVertToEyeVector;
// tangentSpaceTranspose (float3x3 at TEXCOORD3..5), whose rows
// ( S.x, T.x, N.x ), ... D3D9 packs by column: the tangent S, tangent T and
// normal.
layout( location = 3 ) out vec3 tangentSpaceTransposeColumn0;
layout( location = 4 ) out vec3 tangentSpaceTransposeColumn1;
layout( location = 5 ) out vec3 tangentSpaceTransposeColumn2;
layout( location = 6 ) out vec4 worldPos_atten3;
layout( location = 7 ) out vec4 projPos_fWrinkleWeight;

#define cBaseTexCoordTransform_0 VS_C( 48 )
#define cBaseTexCoordTransform_1 VS_C( 49 )
#define cDetailTexCoordTransform_0 VS_C( 52 )
#define cDetailTexCoordTransform_1 VS_C( 53 )
#define cViewProjZ VS_C( 13 )

void main()
{
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 48, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 16, 3 );

	// ApplyMorph: the flex stream (POSITION1) is absent, read as zero.
	const vec4 vPosFlex = vec4( 0.0 );
	projPos_fWrinkleWeight.w = vPosFlex.w * cFlexScale.y;

	// Perform skinning (the record's), then the tangent frame.
	vec3 worldNormal, worldTangentS, worldTangentT;
	LegacyModelTangentFrame( worldNormal, worldTangentS, worldTangentT );
	const vec3 worldPos = inWorldPos;

	// Transform into projection space
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), cViewProjZ );

	projPos_fWrinkleWeight.xyz = vProjPos.xyz;

	// Needed for water fog alpha and diffuse lighting
	worldPos_atten3.xyz = worldPos;

	// Needed for specular
	worldVertToEyeVector = cEyePos - worldPos;

	// Compute bumped lighting
	if ( !USE_STATIC_CONTROL_FLOW )
	{
		lightAtten = vec3( 0.0 );
		worldPos_atten3.w = 0.0;
		if ( NUM_LIGHTS > 0 )
			lightAtten.x = GetVertexAttenForLight( worldPos, 0, false );
		if ( NUM_LIGHTS > 1 )
			lightAtten.y = GetVertexAttenForLight( worldPos, 1, false );
		if ( NUM_LIGHTS > 2 )
			lightAtten.z = GetVertexAttenForLight( worldPos, 2, false );
		if ( NUM_LIGHTS > 3 )
			worldPos_atten3.w = GetVertexAttenForLight( worldPos, 3, false );
	}
	else
	{
		lightAtten.x = GetVertexAttenForLight( worldPos, 0, true );
		lightAtten.y = GetVertexAttenForLight( worldPos, 1, true );
		lightAtten.z = GetVertexAttenForLight( worldPos, 2, true );
		worldPos_atten3.w = GetVertexAttenForLight( worldPos, 3, true );
	}

	// Base texture coordinate transform
	baseTexCoord.x = dot( vec4( inTexCoord0, 0.0, 1.0 ), cBaseTexCoordTransform_0 );
	baseTexCoord.y = dot( vec4( inTexCoord0, 0.0, 1.0 ), cBaseTexCoordTransform_1 );
	baseTexCoord.z = dot( vec4( inTexCoord0, 0.0, 1.0 ), cDetailTexCoordTransform_0 );
	baseTexCoord.w = dot( vec4( inTexCoord0, 0.0, 1.0 ), cDetailTexCoordTransform_1 );

	// Tangent space transform
	tangentSpaceTransposeColumn0 = worldTangentS;
	tangentSpaceTransposeColumn1 = worldTangentT;
	tangentSpaceTransposeColumn2 = worldNormal;
}
