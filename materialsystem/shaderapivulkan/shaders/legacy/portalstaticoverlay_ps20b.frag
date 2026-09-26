#version 450
// PortalStaticOverlay's pixel stage: a port of
// stdshaders/portalstaticoverlay_ps2x.fxc (ps20b). The static texture (or
// gray) with the static amount as alpha, limited by the alpha mask. Combos
// (fxctmp9/portalstaticoverlay_ps20b.inc): static CONVERT_TO_SRGB (4, always 0
// here), HASALPHAMASK (8), HASSTATICTEXTURE (16); dynamic HDRENABLED (1,
// unused by the shader), PIXELFOGTYPE (2). With a static texture it is on s0
// and the mask on s1; without one the mask is on s0.
// @legacy program=portalstaticoverlay ps=portalstaticoverlay_ps20b
//         vs=portalstaticoverlay_vs20 vert=portalstaticoverlay_vs20 samplers=0:2d,1:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D Sampler0; // s0
layout( set = 0, binding = 1 ) uniform sampler2D Sampler1; // s1

layout( location = 0 ) in vec2 vTexCoord0;
layout( location = 1 ) in vec2 vTexCoord1;
layout( location = 7 ) in vec4 worldPos_projPosZ;

#define g_StaticAmount PS_C( 0 ) // x is static, y is 1.0 - static
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const bool HASALPHAMASK = STATIC_PS_COMBO( 8, 2 ) != 0;
	const bool HASSTATICTEXTURE = STATIC_PS_COMBO( 16, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 2, 2 );

	vec4 result;
	if ( HASSTATICTEXTURE )
		result.rgb = tex2D( 0, Sampler0, vTexCoord0 ).rgb; // StaticTextureSampler
	else
		result.rgb = vec3( 0.25 ); // without a static texture, just be gray

	if ( HASALPHAMASK )
	{
		// when static reaches 0, fades away completely, also never exceeds the mask's alpha
		const float maskAlpha = HASSTATICTEXTURE ? tex2D( 1, Sampler1, vTexCoord1 ).a
		                                         : tex2D( 0, Sampler0, vTexCoord0 ).a;
		result.a = min( g_StaticAmount.x, maskAlpha );
	}
	else
	{
		result.a = g_StaticAmount.x; // when static reaches 0, fades away completely
	}

	const float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( result, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
