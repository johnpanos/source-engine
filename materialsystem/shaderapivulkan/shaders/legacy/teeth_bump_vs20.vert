#version 450
// A port of stdshaders/teeth_bump_vs20.fxc (Teeth_DX9's vertex stage with a
// bump map). Combos (fxctmp9/teeth_bump_vs20.inc): static INTRO (48),
// USE_STATIC_CONTROL_FLOW (96); dynamic COMPRESSED_VERTS (1), DOWATERFOG (2),
// SKINNING (4), STATIC_LIGHT (8), NUM_LIGHTS (16, 0..2). The vertex record
// already applied the skinning and the vertex decompression; the fixed-function
// fog output is unused (pixel fog).
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"
#include "legacy_vortwarp_vs.glsl"

layout( location = 0 ) out vec2 baseTexCoord;
layout( location = 1 ) out vec4 worldVertToEyeVector_Darkening;
layout( location = 2 ) out vec3 tangentSpaceTranspose0;
layout( location = 3 ) out vec3 tangentSpaceTranspose1;
layout( location = 4 ) out vec3 tangentSpaceTranspose2;
layout( location = 5 ) out vec4 worldPos_projPosZ;
layout( location = 6 ) out vec2 lightAtten01;
layout( location = 7 ) out vec2 lightAtten23;

#define cTeethLighting VS_C( 48 )
#define const4 VS_C( 49 )
#define g_Time ( const4.w )
#define modelOrigin ( const4.xyz )

void main()
{
	const bool INTRO = STATIC_VS_COMBO( 48, 2 ) != 0;
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 96, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 16, 3 );

	// SkinPositionNormalAndTangentSpace: T = cross( N, S ) * TANGENT.w.
	vec3 worldPos = inWorldPos;
	vec3 worldNormal = inWorldNormal;
	vec3 worldTangentS = inWorldTangentS.xyz;
	vec3 worldTangentT = cross( worldNormal, worldTangentS ) * LegacyTangentSign();

	if ( INTRO )
		WorldSpaceVertexProcess( g_Time, modelOrigin, worldPos, worldNormal, worldTangentS, worldTangentT );

	// Always normalize since flex path is controlled by runtime
	// constant not a shader combo and will always generate the normalization
	worldNormal = normalize( worldNormal );
	worldTangentS = normalize( worldTangentS );
	worldTangentT = normalize( worldTangentT );

	// Transform into projection space
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	worldPos_projPosZ = vec4( worldPos, vProjPos.z );

	// Needed for specular
	worldVertToEyeVector_Darkening.xyz = cEyePos - worldPos;

	// Special darkening of lights for mouth open/close
	worldVertToEyeVector_Darkening.w = cTeethLighting.w * saturate( dot( worldNormal, cTeethLighting.xyz ) );

	// Scalar light attenuation (mouth darkening applied in pixel shader)
	if ( !USE_STATIC_CONTROL_FLOW )
	{
		lightAtten01 = vec2( 0.0 );
		lightAtten23 = vec2( 0.0 );
		if ( NUM_LIGHTS > 0 )
			lightAtten01.x = GetVertexAttenForLight( worldPos, 0, false );
		if ( NUM_LIGHTS > 1 )
			lightAtten01.y = GetVertexAttenForLight( worldPos, 1, false );
		if ( NUM_LIGHTS > 2 )
			lightAtten23.x = GetVertexAttenForLight( worldPos, 2, false );
		if ( NUM_LIGHTS > 3 )
			lightAtten23.y = GetVertexAttenForLight( worldPos, 3, false );
	}
	else
	{
		lightAtten01.x = GetVertexAttenForLight( worldPos, 0, true );
		lightAtten01.y = GetVertexAttenForLight( worldPos, 1, true );
		lightAtten23.x = GetVertexAttenForLight( worldPos, 2, true );
		lightAtten23.y = GetVertexAttenForLight( worldPos, 3, true );
	}

	baseTexCoord = inTexCoord0;

	// Tangent space transform: a float3x3 of rows ( S.x, T.x, N.x ), ...; the
	// registers hold its columns.
	tangentSpaceTranspose0 = worldTangentS;
	tangentSpaceTranspose1 = worldTangentT;
	tangentSpaceTranspose2 = worldNormal;
}
