#version 450
// A port of stdshaders/water_vs20.fxc (Water_DX90's expensive vertex stage).
// Combos (fxctmp9/Water_vs20.inc): static BASETEXTURE (1), MULTITEXTURE (2).
// The projected position mapped to the reflection and refraction textures,
// the tangent-space eye vector, the bump coordinates (and the two extra
// scrolled bump coordinates, or the bumped lightmap coordinates). The tangent
// frame is the brush's TANGENTS/TANGENTT streams and the normal as the vertex
// record carries them (through the MODEL rotation, which is the identity for
// world brushes); TEXCOORD2 is the bumped lightmap offset.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 vBumpTexCoord;
layout( location = 1 ) out vec3 vTangentEyeVect;
layout( location = 2 ) out vec4 vReflectXY_vRefractYX;
layout( location = 3 ) out float W;
layout( location = 4 ) out vec4 vProjPosOut;
layout( location = 5 ) out float screenCoord;
layout( location = 6 ) out vec4 vExtraBumpTexCoord_lightmapTexCoord1And2;
layout( location = 7 ) out vec4 lightmapTexCoord3;
layout( location = 9 ) out vec4 fogFactorW;

#define cBumpTexCoordTransform_0 VS_C( 49 )
#define cBumpTexCoordTransform_1 VS_C( 50 )
#define TexOffsets VS_C( 51 )

void main()
{
	const bool BASETEXTURE = STATIC_VS_COMBO( 1, 2 ) != 0;
	const bool MULTITEXTURE = STATIC_VS_COMBO( 2, 2 ) != 0;

	const vec3 vWorldPos = inWorldPos;
	const vec4 vProjPos = LegacyProject( vWorldPos );
	gl_Position = vProjPos;
	vProjPosOut = vProjPos;

	// Map projected position to the reflection texture
	const vec2 vReflectPos = ( vProjPos.xy + vProjPos.w ) * 0.5;

	// Map projected position to the refraction texture
	vec2 vRefractPos = vec2( vProjPos.x, -vProjPos.y ); // invert Y
	vRefractPos = ( vRefractPos + vProjPos.w ) * 0.5;

	// Reflection transform
	vReflectXY_vRefractYX = vec4( vReflectPos.x, vReflectPos.y, vRefractPos.y, vRefractPos.x );
	W = vProjPos.w;
	screenCoord = vProjPos.x;

	// Compute fog based on the position
	fogFactorW = vec4( RangeFog( vProjPos.xyz ) );

	// Eye vector, transformed to the tangent space
	const vec3 vWorldEyeVect = cEyePos - vWorldPos;
	vTangentEyeVect.x = dot( vWorldEyeVect, inWorldTangentS.xyz );
	vTangentEyeVect.y = dot( vWorldEyeVect, LegacyTangentT() );
	vTangentEyeVect.z = dot( vWorldEyeVect, inWorldNormal );

	// Tranform bump coordinates
	const vec4 vBaseTexCoord = vec4( inTexCoord0, 0.0, 1.0 );
	vBumpTexCoord.x = dot( vBaseTexCoord, cBumpTexCoordTransform_0 );
	vBumpTexCoord.y = dot( vBaseTexCoord, cBumpTexCoordTransform_1 );
	const float f45x = vBaseTexCoord.x + vBaseTexCoord.y;
	const float f45y = vBaseTexCoord.y - vBaseTexCoord.x;

	vExtraBumpTexCoord_lightmapTexCoord1And2 = vec4( 0.0 );
	lightmapTexCoord3 = vec4( 0.0 );
	if ( MULTITEXTURE )
	{
		vExtraBumpTexCoord_lightmapTexCoord1And2 = vec4( f45x * 0.1 + TexOffsets.x,
		    f45y * 0.1 + TexOffsets.y, vBaseTexCoord.y * 0.45 + TexOffsets.z,
		    vBaseTexCoord.x * 0.45 + TexOffsets.w );
	}
	if ( BASETEXTURE )
	{
		const vec2 vLightmapTexCoordOffset = LegacyTexCoord2();
		vec4 lightmapTexCoord1And2;
		lightmapTexCoord1And2.xy = inTexCoord1 + vLightmapTexCoordOffset;
		const vec2 lightmapTexCoord2 = lightmapTexCoord1And2.xy + vLightmapTexCoordOffset;
		const vec2 lightmapTexCoord3xy = lightmapTexCoord2 + vLightmapTexCoordOffset;
		// reversed component order
		lightmapTexCoord1And2.w = lightmapTexCoord2.x;
		lightmapTexCoord1And2.z = lightmapTexCoord2.y;
		vExtraBumpTexCoord_lightmapTexCoord1And2 = lightmapTexCoord1And2;
		lightmapTexCoord3.xy = lightmapTexCoord3xy;
	}
}
