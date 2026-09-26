#version 450
// A port of stdshaders/watercheap_vs20.fxc (Water_DX90's cheap vertex stage).
// Combos (fxctmp9/WaterCheap_vs20.inc): static BLEND (1). The world tangent
// frame (the brush's TANGENTS/TANGENTT streams and the normal through the
// MODEL rotation, as the vertex record carries them), the world eye vector,
// the refraction texture position and the two extra scrolled bump
// coordinates.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 normalMapTexCoord;
layout( location = 1 ) out vec3 worldVertToEyeVector;
layout( location = 2 ) out vec3 tangentSpaceTranspose0;
layout( location = 3 ) out vec3 tangentSpaceTranspose1;
layout( location = 4 ) out vec3 tangentSpaceTranspose2;
layout( location = 5 ) out vec4 vRefract_W_ProjZ;
layout( location = 6 ) out vec4 vExtraBumpTexCoord;
layout( location = 9 ) out vec4 fogFactorW;

#define TexOffsets VS_C( 51 )
#define VSHADER_VECT_SCALE 1.0

void main()
{
	const bool BLEND = STATIC_VS_COMBO( 1, 2 ) != 0;

	const vec3 worldPos = inWorldPos;
	const vec4 projPos = LegacyProject( worldPos );
	gl_Position = projPos;

	vRefract_W_ProjZ = vec4( 0.0 );
	if ( BLEND )
	{
		// Map projected position to the reflection texture
		vRefract_W_ProjZ.x = projPos.x;
		vRefract_W_ProjZ.y = -projPos.y; // invert Y
		vRefract_W_ProjZ.xy = ( vRefract_W_ProjZ.xy + projPos.w ) * 0.5;
		vRefract_W_ProjZ.z = projPos.w;
	}
	vRefract_W_ProjZ.w = projPos.z;

	tangentSpaceTranspose0 = inWorldTangentS.xyz;
	tangentSpaceTranspose1 = LegacyTangentT();
	tangentSpaceTranspose2 = inWorldNormal;

	worldVertToEyeVector = VSHADER_VECT_SCALE * ( cEyePos - worldPos );

	normalMapTexCoord = inTexCoord0;

	const float f45x = inTexCoord0.x + inTexCoord0.y;
	const float f45y = inTexCoord0.y - inTexCoord0.x;
	vExtraBumpTexCoord.x = f45x * 0.1 + TexOffsets.x;
	vExtraBumpTexCoord.y = f45y * 0.1 + TexOffsets.y;
	vExtraBumpTexCoord.z = inTexCoord0.y * 0.45 + TexOffsets.z;
	vExtraBumpTexCoord.w = inTexCoord0.x * 0.45 + TexOffsets.w;

	fogFactorW = vec4( RangeFog( projPos.xyz ) );
}
