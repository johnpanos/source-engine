#version 450
// A port of stdshaders/cloak_vs20.fxc (Cloak_DX90's vertex stage). Combos
// (fxctmp9/cloak_vs20.inc): static MODEL (24), USE_STATIC_CONTROL_FLOW (48);
// dynamic COMPRESSED_VERTS (1), DOWATERFOG (2), SKINNING (4), NUM_LIGHTS (8,
// 0..2). MODEL selects nothing in the vs20 code (TANGENT is the user data of a
// model, the TANGENTS stream of a brush, as the vertex record carries it). The
// vertex record already applied the skinning and the vertex decompression; the
// fixed-function fog output is unused (pixel fog).
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"

layout( location = 0 ) out vec2 vBaseTexCoord;
layout( location = 1 ) out vec3 tangentSpaceTranspose0;
layout( location = 2 ) out vec3 tangentSpaceTranspose1;
layout( location = 3 ) out vec3 tangentSpaceTranspose2;
layout( location = 4 ) out vec3 worldPosOut;
layout( location = 5 ) out vec3 projPosOut;
layout( location = 6 ) out vec4 lightAtten;
layout( location = 7 ) out vec3 vRefract;

void main()
{
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 48, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 8, 3 );

	// SkinPositionNormalAndTangentSpace: T = cross( N, S ) * TANGENT.w.
	vec3 worldPos = inWorldPos;
	vec3 worldNormal = inWorldNormal;
	vec3 worldTangentS = inWorldTangentS.xyz;
	vec3 worldTangentT = cross( worldNormal, worldTangentS ) * LegacyTangentSign();

	// Always normalize since flex path is controlled by runtime
	// constant not a shader combo and will always generate the normalization
	worldNormal = normalize( worldNormal );
	worldTangentS = normalize( worldTangentS );
	worldTangentT = normalize( worldTangentT );

	// Projected position
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;

	// Map projected position to the refraction texture
	vec2 vRefractPos;
	vRefractPos.x = vProjPos.x;
	vRefractPos.y = -vProjPos.y; // invert Y
	vRefractPos = ( vRefractPos + vProjPos.w ) * 0.5;

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

	// World position
	worldPosOut = worldPos;

	// Refract position
	projPosOut = vec3( vRefractPos.x, vRefractPos.y, vProjPos.w );

	vBaseTexCoord = inTexCoord0;
	vRefract = vec3( 0.0 );

	// Tangent space transform: a float3x3 of rows ( S.x, T.x, N.x ), ...; the
	// registers hold its columns.
	tangentSpaceTranspose0 = worldTangentS;
	tangentSpaceTranspose1 = worldTangentT;
	tangentSpaceTranspose2 = worldNormal;
}
