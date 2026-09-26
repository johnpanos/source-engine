// Vertex-stage inputs of the legacy shader ports: the shared vertex record the
// shader API fills (vulkan_legacy_programs.h documents it). Positions arrive in
// world space, skinned or through the MODEL matrix, as the D3D9 vertex shaders
// compute them with SkinPosition; normals and tangents are rotated the same way
// and not normalized. A port's vertex stage does the rest of its vs20/vs30.
#ifndef LEGACY_VS_GLSL
#define LEGACY_VS_GLSL

#include "legacy_common.glsl"

layout( location = 0 ) in vec3 inWorldPos;
layout( location = 1 ) in vec3 inColor;   // vertex color RGB
layout( location = 2 ) in vec2 inTexCoord0;
layout( location = 3 ) in vec2 inTexCoord1;
layout( location = 4 ) in vec3 inWorldNormal;
// w: the TANGENT sign; with VERTEX_TANGENT_S (brushes), TEXCOORD2.x.
layout( location = 5 ) in vec4 inWorldTangentS;
layout( location = 6 ) in float inAlpha;        // vertex color alpha
// With VERTEX_TANGENT_S (brushes): the TANGENTT stream in xyz and TEXCOORD2.y
// in w; else TEXCOORD2.xy (or the object-space position with
// object_position_extra).
layout( location = 7 ) in vec4 inExtra;

out float gl_ClipDistance[2];

// Whether the pass's vertex format has the brush tangent streams (VERTEX_TANGENT_S),
// which places tangent T and TEXCOORD2 in the record (vulkan_legacy_programs.h).
bool LegacyBrushTangents()
{
	return ( lc.bools.w & 0x10 ) != 0;
}
// TEXCOORD2 (e.g. the bumped lightmap offset) wherever the record holds it.
vec2 LegacyTexCoord2()
{
	return LegacyBrushTangents() ? vec2( inWorldTangentS.w, inExtra.w ) : inExtra.xy;
}
// TANGENT's w (the model tangent's sign). With VERTEX_TANGENT_S, D3D9's
// declaration binds the three-component TANGENTS stream as TANGENT, so w reads 1.
float LegacyTangentSign()
{
	return LegacyBrushTangents() ? 1.0 : inWorldTangentS.w;
}
// The TANGENTT stream, rotated like the normal (brushes).
vec3 LegacyTangentT()
{
	return inExtra.xyz;
}

// common_vs_fxc.h's registers (c0 is the compile-time ( 0, 1, 2, 0.5 )).
#define cConstants1 VS_C( 1 )
#define cOOGamma ( cConstants1.x )
#define cOneThird ( cConstants1.z )
#define cEyePosWaterZ VS_C( 2 )
#define cEyePos ( cEyePosWaterZ.xyz )
#define cFlexScale VS_C( 3 )
#define cFogParams VS_C( 16 )
#define cFogEndOverFogRange ( cFogParams.x )
#define cFogOne ( cFogParams.y )
#define cFogMaxDensity ( cFogParams.z )
#define cOOFogRange ( cFogParams.w )
#define cModulationColor VS_C( 47 )

// The clip-space position of a world position (mul( float4( worldPos, 1 ),
// cViewProj )), with the user clip planes applied to it.
vec4 LegacyProject( vec3 worldPos )
{
	vec4 projPos = pc.viewProj * vec4( worldPos, 1.0 );
	gl_ClipDistance[0] = dot( pc.clipPlanes[0], projPos );
	gl_ClipDistance[1] = dot( pc.clipPlanes[1], projPos );
	return projPos;
}

// The two ways Source's vertex shaders apply a texture transform held as two
// registers (SetVertexShaderTextureTransform's rows); each port uses the one
// its HLSL spells, as fxc compiled it:
//   dot( float4( uv, 0, 1 ), row ) per row (e.g. cBaseTexCoordTransform[i]):
vec2 DotTexTransform( vec2 uv, vec4 row0, vec4 row1 )
{
	const vec4 t = vec4( uv, 0.0, 1.0 );
	return vec2( dot( t, row0 ), dot( t, row1 ) );
}
//   mul( float4 texCoord, (float2x4)transform ).xy, which fxc truncates to
//   u * row0 + v * row1 (no translation):
vec2 MulFloat2x4TexTransform( vec2 uv, vec4 row0, vec4 row1 )
{
	return uv.x * row0.xy + uv.y * row1.xy;
}

// common_vs_fxc.h's RangeFog (the vertex fog factor vs20 shaders output).
float RangeFog( vec3 projPos )
{
	return max( cFogMaxDensity, ( -projPos.z * cOOFogRange + cFogEndOverFogRange ) );
}

#endif // LEGACY_VS_GLSL
