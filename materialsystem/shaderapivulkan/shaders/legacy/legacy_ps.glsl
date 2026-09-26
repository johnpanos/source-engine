// Pixel-stage helpers of the legacy shader ports: common_ps_fxc.h's FinalOutput
// family, the sRGB conversions D3D9 does in fixed function (SRGBTEXTURE,
// SRGBWRITEENABLE) where the hardware view cannot, and the fixed-function
// alpha test. Every port ends with LegacyWrite( FinalOutput( ... ) ).
#ifndef LEGACY_PS_GLSL
#define LEGACY_PS_GLSL

#include "legacy_common.glsl"

layout( location = 0 ) out vec4 outColor;

// common_ps_fxc.h's registers.
#define g_LinearFogColor PS_C( 29 )
#define OO_DESTALPHA_DEPTH_RANGE ( g_LinearFogColor.w )
#define cLightScale PS_C( 30 )
#define LINEAR_LIGHT_SCALE ( cLightScale.x )
#define LIGHT_MAP_SCALE ( cLightScale.y )
#define ENV_MAP_SCALE ( cLightScale.z )
#define GAMMA_LIGHT_SCALE ( cLightScale.w )
#define cFlashlightColor PS_C( 28 )
#define cFlashlightScreenScale PS_C( 31 )
#define HDR_INPUT_MAP_SCALE 16.0

// shader_constant_register_map.h.
#define PSREG_SELFILLUMTINT 0
#define PSREG_DIFFUSE_MODULATION 1
#define PSREG_ENVMAP_TINT_SHADOW_TWEAKS 2
#define PSREG_SELFILLUM_SCALE_BIAS_EXP 3
#define PSREG_AMBIENT_CUBE 4
#define PSREG_ENVMAP_FRESNEL_SELFILLUMMASK 10
#define PSREG_EYEPOS_SPEC_EXPONENT 11
#define PSREG_FOG_PARAMS 12
#define PSREG_FLASHLIGHT_ATTENUATION 13
#define PSREG_FLASHLIGHT_POSITION_RIM_BOOST 14
#define PSREG_FLASHLIGHT_TO_WORLD_TEXTURE 15
#define PSREG_FRESNEL_SPEC_PARAMS 19
#define PSREG_LIGHT_INFO_ARRAY 20
#define PSREG_SPEC_RIM_PARAMS 26

#define TONEMAP_SCALE_NONE 0
#define TONEMAP_SCALE_LINEAR 1
#define TONEMAP_SCALE_GAMMA 2
#define PIXEL_FOG_TYPE_NONE -1
#define PIXEL_FOG_TYPE_RANGE 0
#define PIXEL_FOG_TYPE_HEIGHT 1

vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}

// A texel of the sampler in slot `slot` (its binding in set 0), decoded from
// sRGB when the material reads it as sRGB and the bound view could not.
vec4 LegacyTexel( int slot, vec4 texel )
{
	if ( ( int( pc.params.z ) & ( 1 << slot ) ) != 0 )
		texel.rgb = SrgbToLinear( texel.rgb );
	return texel;
}
#define tex2D( set, s, uv ) LegacyTexel( ( set ), texture( ( s ), ( uv ) ) )
#define tex2Dlod( set, s, uv, lod ) LegacyTexel( ( set ), textureLod( ( s ), ( uv ), ( lod ) ) )
#define tex2Dbias( set, s, uv, bias ) LegacyTexel( ( set ), texture( ( s ), ( uv ), ( bias ) ) )
#define texCUBE( set, s, dir ) LegacyTexel( ( set ), texture( ( s ), ( dir ) ) )
#define tex3D( set, s, uvw ) LegacyTexel( ( set ), texture( ( s ), ( uvw ) ) )
// tex2Dproj: the coordinate divided by its w.
#define tex2Dproj( set, s, uvw ) LegacyTexel( ( set ), texture( ( s ), ( uvw ).xy / ( uvw ).w ) )

float CalcWaterFogAlpha( float flWaterZ, float flEyePosZ, float flWorldPosZ, float flProjPosZ,
    float flFogOORange )
{
	float flDepthFromWater = flWaterZ - flWorldPosZ;
	float flDepthFromEye = flEyePosZ - flWorldPosZ;
	float f = saturate( flDepthFromWater * ( 1.0 / flDepthFromEye ) );
	return saturate( f * flProjPosZ * flFogOORange );
}

float CalcRangeFog(
    float flProjPosZ, float flFogStartOverRange, float flFogMaxDensity, float flFogOORange )
{
	return saturate( min( flFogMaxDensity, ( flProjPosZ * flFogOORange ) - flFogStartOverRange ) );
}

float CalcPixelFogFactor(
    int iPIXELFOGTYPE, vec4 fogParams, float flEyePosZ, float flWorldPosZ, float flProjPosZ )
{
	float retVal = 0.0;
	if ( iPIXELFOGTYPE == PIXEL_FOG_TYPE_RANGE )
		retVal = CalcRangeFog( flProjPosZ, fogParams.x, fogParams.z, fogParams.w );
	else if ( iPIXELFOGTYPE == PIXEL_FOG_TYPE_HEIGHT )
		retVal = CalcWaterFogAlpha( fogParams.y, flEyePosZ, flWorldPosZ, flProjPosZ, fogParams.w );
	return retVal;
}

vec3 BlendPixelFog( vec3 vShaderColor, float pixelFogFactor, vec3 vFogColor, int iPIXELFOGTYPE )
{
	if ( iPIXELFOGTYPE == PIXEL_FOG_TYPE_RANGE )
	{
		pixelFogFactor = saturate( pixelFogFactor );
		return mix( vShaderColor, vFogColor, pixelFogFactor * pixelFogFactor );
	}
	if ( iPIXELFOGTYPE == PIXEL_FOG_TYPE_HEIGHT )
		return mix( vShaderColor, vFogColor, saturate( pixelFogFactor ) );
	return vShaderColor;
}

float DepthToDestAlpha( float flProjZ )
{
	return flProjZ * OO_DESTALPHA_DEPTH_RANGE;
}

// FinalOutput( color, fog factor, fog type, tone map type[, depth to dest
// alpha, projected z] ). SRGBOutput is the identity: this backend reports no
// shader sRGB conversion, so CONVERT_TO_SRGB is 0 in every combo it selects.
vec4 FinalOutput( vec4 vShaderColor, float pixelFogFactor, int iPIXELFOGTYPE,
    int iTONEMAP_SCALE_TYPE, bool bWriteDepthToDestAlpha, float flProjZ )
{
	vec4 result;
	if ( iTONEMAP_SCALE_TYPE == TONEMAP_SCALE_LINEAR )
		result.rgb = vShaderColor.rgb * LINEAR_LIGHT_SCALE;
	else if ( iTONEMAP_SCALE_TYPE == TONEMAP_SCALE_GAMMA )
		result.rgb = vShaderColor.rgb * GAMMA_LIGHT_SCALE;
	else
		result.rgb = vShaderColor.rgb;
	result.a = bWriteDepthToDestAlpha ? DepthToDestAlpha( flProjZ ) : vShaderColor.a;
	result.rgb = BlendPixelFog( result.rgb, pixelFogFactor, g_LinearFogColor.rgb, iPIXELFOGTYPE );
	return result;
}
vec4 FinalOutput( vec4 vShaderColor, float pixelFogFactor, int iPIXELFOGTYPE, int iTONEMAP_SCALE_TYPE )
{
	return FinalOutput( vShaderColor, pixelFogFactor, iPIXELFOGTYPE, iTONEMAP_SCALE_TYPE, false, 1.0 );
}

// NORM_DECODE_NONE's DecompressNormal (the only mode Source selects on this
// backend, which reports no ATI2N support).
vec4 DecompressNormal( int set, sampler2D normalSampler, vec2 tc )
{
	vec4 normalTexel = tex2D( set, normalSampler, tc );
	return vec4( normalTexel.xyz * 2.0 - 1.0, normalTexel.a );
}

// The pixel shader's oC0 through D3D9's fixed-function tail: the alpha test,
// then the sRGB write encode when the attachment's view does not encode.
void LegacyWrite( vec4 color )
{
	const float ref = pc.params.x;
	if ( ref >= 0.0 && ( pc.params.y != 0.0 ? color.a <= ref : color.a < ref ) )
		discard;
	if ( ( int( pc.params.z ) & 65536 ) != 0 )
		color.rgb = LinearToSrgb( color.rgb );
	outColor = color;
}

#endif // LEGACY_PS_GLSL
