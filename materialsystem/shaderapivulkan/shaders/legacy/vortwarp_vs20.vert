#version 450
// A port of stdshaders/vortwarp_vs20.fxc (VortWarp_DX9's vertex stage). Combos
// (fxctmp9/vortwarp_vs20.inc): static HALFLAMBERT (24), USE_STATIC_CONTROL_FLOW
// (48); dynamic COMPRESSED_VERTS (1), DOWATERFOG (2, skipped when 1), SKINNING
// (4), NUM_LIGHTS (8, 0..2). The vertex record already applied the skinning and
// the vertex decompression.
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"
#include "legacy_vortwarp_vs.glsl"

layout( location = 0 ) out vec4 baseTexCoord2_tangentSpaceVertToEyeVectorXY;
layout( location = 1 ) out vec4 lightAtten;
layout( location = 2 ) out vec4 worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ;
// float3x3 tangentSpaceTranspose : TEXCOORD3..5, rows ( S.x, T.x, N.x ), ...;
// the registers hold its columns.
layout( location = 3 ) out vec3 tangentSpaceTranspose0;
layout( location = 4 ) out vec3 tangentSpaceTranspose1;
layout( location = 5 ) out vec3 tangentSpaceTranspose2;
layout( location = 6 ) out vec4 worldPos_projPosZ;
layout( location = 9 ) out vec4 fogFactorW; // COLOR1

#define cBaseTexCoordTransform_0 VS_C( 48 )
#define cBaseTexCoordTransform_1 VS_C( 49 )
#define const4 VS_C( 52 )
#define g_Time ( const4.w )
#define modelOrigin ( const4.xyz )

void main()
{
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 48, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 8, 3 );

	// SkinPositionNormalAndTangentSpace: T = cross( N, S ) * TANGENT.w.
	vec3 worldPos = inWorldPos;
	vec3 worldNormal = inWorldNormal;
	vec3 worldTangentS = inWorldTangentS.xyz;
	vec3 worldTangentT = cross( worldNormal, worldTangentS ) * LegacyTangentSign();

	WorldSpaceVertexProcess( g_Time, modelOrigin, worldPos, worldNormal, worldTangentS, worldTangentT );

	// Always normalize since flex path is controlled by runtime
	// constant not a shader combo and will always generate the normalization
	worldNormal = normalize( worldNormal );
	worldTangentS = normalize( worldTangentS );
	worldTangentT = normalize( worldTangentT );

	// Transform into projection space
	vec4 projPos = LegacyProject( worldPos );
	gl_Position = projPos;
	projPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	fogFactorW = vec4( RangeFog( projPos.xyz ) );
	worldPos_projPosZ = vec4( worldPos, projPos.z );

	// Needed for cubemapping + parallax mapping
	worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ = vec4( cEyePos - worldPos, 0.0 );

	if ( !USE_STATIC_CONTROL_FLOW )
	{
		lightAtten = vec4( 0.0 );
		if ( NUM_LIGHTS > 0 )
			lightAtten.x = GetVertexAttenForLight( worldPos, 0, false );
		if ( NUM_LIGHTS > 1 )
			lightAtten.y = GetVertexAttenForLight( worldPos, 1, false );
		if ( NUM_LIGHTS > 2 )
			lightAtten.z = GetVertexAttenForLight( worldPos, 2, false );
		if ( NUM_LIGHTS > 3 )
			lightAtten.w = GetVertexAttenForLight( worldPos, 3, false );
	}
	else
	{
		// Scalar light attenuation
		lightAtten.x = GetVertexAttenForLight( worldPos, 0, true );
		lightAtten.y = GetVertexAttenForLight( worldPos, 1, true );
		lightAtten.z = GetVertexAttenForLight( worldPos, 2, true );
		lightAtten.w = GetVertexAttenForLight( worldPos, 3, true );
	}

	// Base texture coordinate transform
	baseTexCoord2_tangentSpaceVertToEyeVectorXY = vec4(
	    DotTexTransform( inTexCoord0, cBaseTexCoordTransform_0, cBaseTexCoordTransform_1 ), 0.0, 0.0 );

	tangentSpaceTranspose0 = worldTangentS;
	tangentSpaceTranspose1 = worldTangentT;
	tangentSpaceTranspose2 = worldNormal;
}
