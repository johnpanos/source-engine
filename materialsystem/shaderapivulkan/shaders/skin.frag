#version 450
// VertexLitGeneric's $phong pixel stage: a port of stdshaders/skin_ps20b.fxc
// with common_vertexlitgeneric_dx9.h's lighting, for the non-flashlight pass.
//
// The pixel shader constants are the D3D9 registers the material's dynamic
// state writes (skin_dx9_helper.cpp), c0..c31 as the shader API holds them,
// read under their register names below. The static combos come as flags
// (params.y): 1 FASTPATH_NOBUMP, 2 LIGHTWARPTEXTURE, 4 PHONGWARPTEXTURE,
// 8 SELFILLUM, 16 SELFILLUMFRESNEL, 32 RIMLIGHT, 64 BLENDTINTBYBASEALPHA.
// CUBEMAP, DETAILTEXTURE, WRINKLEMAP and fog are not ported (the shader API
// reports those combos unimplemented).
//
// params: x alpha-test reference (< 0 disables), y combo flags, z kColor*
// flags (1 sRGB base, 4 sRGB output, 8 GREATER alpha test), w FinalOutput's
// linear light scale. params2.x: NUM_LIGHTS.
layout( location = 0 ) in vec2 vBaseUv;
layout( location = 1 ) in vec4 vLightAtten;
layout( location = 2 ) in vec3 vWorldVertToEye;
layout( location = 3 ) in vec3 vTangentS;
layout( location = 4 ) in vec3 vTangentT;
layout( location = 5 ) in vec3 vNormal;
layout( location = 6 ) in vec3 vWorldPos;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;    // s0
layout( set = 1, binding = 0 ) uniform sampler2D SpecularWarpSampler;   // s1
layout( set = 2, binding = 0 ) uniform sampler2D DiffuseWarpSampler;    // s2
layout( set = 3, binding = 0 ) uniform sampler2D NormalMapSampler;      // s3
layout( set = 4, binding = 0 ) uniform sampler2D SpecExponentSampler;   // s7
layout( set = 5, binding = 0 ) uniform sampler2D SelfIllumMaskSampler;  // s14
layout( set = 6, binding = 0 ) uniform PixelShaderConstants
{
	vec4 c[32];
}
ps;
layout( push_constant ) uniform Constants
{
	mat4 viewProj;
	vec4 texXform0;
	vec4 texXform1;
	vec4 eyePos;
	vec4 params;
	vec4 params2;
	vec4 clipPlanes[2];
}
consts;

#define g_SelfIllumTint_and_DetailBlendFactor ps.c[0]
#define g_DiffuseModulation ps.c[1]
#define g_SelfIllumScaleBiasExpBrightness ps.c[3]
#define g_EnvMapFresnel ps.c[10]
#define g_EyePos_SpecExponent ps.c[11]
#define g_FlashlightAttenuationFactors_RimMask ps.c[13]
#define g_FlashlightPos_RimBoost ps.c[14]
#define g_FresnelSpecParams ps.c[19]
#define g_SpecularRimParams ps.c[26]
#define g_ShaderControls ps.c[27]
#define g_fRimBoost g_FlashlightPos_RimBoost.w
#define g_FresnelRanges g_FresnelSpecParams.xyz
#define g_SpecularBoost g_FresnelSpecParams.w
#define g_SpecularTint g_SpecularRimParams.xyz
#define g_RimExponent g_SpecularRimParams.w
#define g_RimMaskControl g_FlashlightAttenuationFactors_RimMask.x
#define g_SelfIllumMaskControl g_EnvMapFresnel.w
#define g_fBaseMapAlphaPhongMask g_ShaderControls.x
#define g_fTintReplacementControl g_ShaderControls.z

const int kFastPathNoBump = 1;
const int kLightWarp = 2;
const int kPhongWarp = 4;
const int kSelfIllum = 8;
const int kSelfIllumFresnel = 16;
const int kRimLight = 32;
const int kBlendTintByBaseAlpha = 64;

vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}

// cAmbientCube: c4..c9.
vec3 PixelShaderAmbientLight( vec3 worldNormal )
{
	vec3 nSquared = worldNormal * worldNormal;
	vec3 isNegative = mix( nSquared, vec3( 0.0 ), greaterThanEqual( worldNormal, vec3( 0.0 ) ) );
	vec3 isPositive = mix( vec3( 0.0 ), nSquared, greaterThanEqual( worldNormal, vec3( 0.0 ) ) );
	return isPositive.x * ps.c[4].xyz + isNegative.x * ps.c[5].xyz + isPositive.y * ps.c[6].xyz +
	       isNegative.y * ps.c[7].xyz + isPositive.z * ps.c[8].xyz + isNegative.z * ps.c[9].xyz;
}

// cLightInfo: c20..c25, the fourth light spread across the w components.
vec3 PixelShaderGetLightColor( int n )
{
	if ( n == 3 )
		return vec3( ps.c[20].w, ps.c[21].w, ps.c[22].w );
	return ps.c[20 + 2 * n].xyz;
}
vec3 PixelShaderGetLightPosition( int n )
{
	if ( n == 3 )
		return vec3( ps.c[23].w, ps.c[24].w, ps.c[25].w );
	return ps.c[21 + 2 * n].xyz;
}

// DiffuseTerm with bHalfLambert (skin always passes it) and no ambient occlusion.
vec3 DiffuseTerm( vec3 worldNormal, vec3 lightDir, bool bDoLightingWarp )
{
	float fResult = clamp( dot( worldNormal, lightDir ) * 0.5 + 0.5, 0.0, 1.0 );
	if ( !bDoLightingWarp )
		fResult *= fResult;
	vec3 fOut = vec3( fResult );
	if ( bDoLightingWarp )
		fOut = 2.0 * texture( DiffuseWarpSampler, vec2( fResult, 0.5 ) ).rgb; // tex1D
	return fOut;
}

// Custom Fresnel, ranges encoded ( ( mid - min ) * 2, mid, ( max - mid ) * 2 ).
float Fresnel( vec3 vNormal, vec3 vEyeDir, vec3 vRanges )
{
	float f = clamp( 1.0 - dot( vNormal, vEyeDir ), 0.0, 1.0 );
	f = f * f - 0.5;
	return vRanges.y + ( f >= 0.0 ? vRanges.z : vRanges.x ) * f;
}
float Fresnel4( vec3 vNormal, vec3 vEyeDir )
{
	float fresnel = clamp( 1.0 - dot( vNormal, vEyeDir ), 0.0, 1.0 );
	fresnel = fresnel * fresnel;
	return fresnel * fresnel;
}

void main()
{
	const int combos = int( consts.params.y );
	const int flags = int( consts.params.z );
	const int nNumLights = int( consts.params2.x );
	const bool bFastPath = ( combos & kFastPathNoBump ) != 0;
	const bool bDoDiffuseWarp = ( combos & kLightWarp ) != 0;
	const bool bDoSpecularWarp = ( combos & kPhongWarp ) != 0;
	const bool bSelfIllum = ( combos & kSelfIllum ) != 0;
	const bool bSelfIllumFresnel = ( combos & kSelfIllumFresnel ) != 0;
	const bool bDoRimLighting = ( combos & kRimLight ) != 0;
	const bool bBlendTintByBaseAlpha = ( combos & kBlendTintByBaseAlpha ) != 0;
	const float lightAtten[4] =
	    float[4]( vLightAtten.x, vLightAtten.y, vLightAtten.z, vLightAtten.w );

	vec4 baseColor = texture( BaseTextureSampler, vBaseUv );
	if ( ( flags & 1 ) != 0 )
		baseColor.rgb = SrgbToLinear( baseColor.rgb );

	vec3 vEyeDir = normalize( vWorldVertToEye );
	vec3 vRimAmbientCubeColor = PixelShaderAmbientLight( vEyeDir );

	vec3 tangentSpaceNormal;
	float fSpecMask = 1.0;
	vec4 normalTexel = texture( NormalMapSampler, vBaseUv );
	if ( !bFastPath )
	{
		tangentSpaceNormal =
		    mix( 2.0 * normalTexel.xyz - 1.0, vec3( 0.0, 0.0, 1.0 ), g_fBaseMapAlphaPhongMask );
		fSpecMask = mix( normalTexel.a, baseColor.a, g_fBaseMapAlphaPhongMask );
	}
	else
	{
		tangentSpaceNormal = vec3( 0.0, 0.0, 1.0 );
		fSpecMask = baseColor.a;
	}

	// mul( tangentSpaceTranspose, tangentSpaceNormal ): rows ( S.x, T.x, N.x ), ...
	vec3 worldSpaceNormal = normalize( vTangentS * tangentSpaceNormal.x +
	                                   vTangentT * tangentSpaceNormal.y +
	                                   vNormal * tangentSpaceNormal.z );

	float fFresnelRanges = Fresnel( worldSpaceNormal, vEyeDir, g_FresnelRanges );
	float fRimFresnel = Fresnel4( worldSpaceNormal, vEyeDir );

	// PixelShaderDoLighting: the ambient cube, then each local light.
	vec3 diffuseLighting = PixelShaderAmbientLight( worldSpaceNormal );
	for ( int i = 0; i < 4; ++i )
	{
		if ( i >= nNumLights )
			break;
		vec3 lightDir = normalize( PixelShaderGetLightPosition( i ) - vWorldPos );
		diffuseLighting += PixelShaderGetLightColor( i ) * lightAtten[i] *
		                   DiffuseTerm( worldSpaceNormal, lightDir, bDoDiffuseWarp );
	}

	vec3 specularLighting = vec3( 0.0 );
	vec3 rimLighting = vec3( 0.0 );
	vec3 vSpecularTint = vec3( 1.0 );
	float fRimMask = 0.0;
	float fSpecExp = 1.0;
	if ( !bFastPath )
	{
		vec4 vSpecExpMap = texture( SpecExponentSampler, vBaseUv );
		fRimMask = mix( 1.0, vSpecExpMap.a, g_RimMaskControl );
		fSpecExp = ( g_EyePos_SpecExponent.w >= 0.0 ) ? g_EyePos_SpecExponent.w
		                                               : ( 1.0 + 149.0 * vSpecExpMap.r );
		vSpecularTint = mix( vec3( 1.0 ), baseColor.rgb, vSpecExpMap.g );
		vSpecularTint = ( g_SpecularTint.r >= 0.0 ) ? g_SpecularTint.rgb : vSpecularTint;
	}
	else
	{
		fSpecExp = max( g_EyePos_SpecExponent.w, 0.0 );
	}

	vec3 albedo = baseColor.rgb;

	// PixelShaderDoSpecularLighting: SpecularAndRimTerms of each local light.
	vec3 vReflect = 2.0 * worldSpaceNormal * dot( worldSpaceNormal, vEyeDir ) - vEyeDir;
	for ( int i = 0; i < 4; ++i )
	{
		if ( i >= nNumLights )
			break;
		vec3 vLightDir = normalize( PixelShaderGetLightPosition( i ) - vWorldPos );
		vec3 color = PixelShaderGetLightColor( i ) * lightAtten[i];
		float LdotR = clamp( dot( vReflect, vLightDir ), 0.0, 1.0 );
		vec3 specular = vec3( pow( LdotR, fSpecExp ) );
		if ( bDoSpecularWarp )
			specular *= texture( SpecularWarpSampler, vec2( specular.x, fFresnelRanges ) ).rgb;
		float NdotL = clamp( dot( worldSpaceNormal, vLightDir ), 0.0, 1.0 );
		specularLighting += specular * NdotL * color;
		if ( bDoRimLighting )
			rimLighting += pow( LdotR, g_RimExponent ) * NdotL * color;
	}

	if ( !bDoSpecularWarp )
		fSpecMask *= fFresnelRanges;
	specularLighting *= fSpecMask * g_SpecularBoost;

	if ( bBlendTintByBaseAlpha )
	{
		vec3 tintedColor = albedo * g_DiffuseModulation.rgb;
		tintedColor = mix( tintedColor, g_DiffuseModulation.rgb, g_fTintReplacementControl );
		albedo = mix( albedo, tintedColor, baseColor.a );
	}
	else
	{
		albedo = albedo * g_DiffuseModulation.rgb;
	}

	vec3 diffuseComponent = albedo * diffuseLighting;
	if ( bSelfIllum )
	{
		if ( bSelfIllumFresnel )
		{
			vec3 vVertexNormal = normalize( vNormal );
			float flSelfIllumFresnel =
			    ( pow( clamp( dot( vVertexNormal, vEyeDir ), 0.0, 1.0 ),
			          g_SelfIllumScaleBiasExpBrightness.z ) *
			        g_SelfIllumScaleBiasExpBrightness.x ) +
			    g_SelfIllumScaleBiasExpBrightness.y;
			diffuseComponent = mix( diffuseComponent,
			    g_SelfIllumTint_and_DetailBlendFactor.rgb * albedo *
			        g_SelfIllumScaleBiasExpBrightness.w,
			    baseColor.a * clamp( flSelfIllumFresnel, 0.0, 1.0 ) );
		}
		else
		{
			vec3 vSelfIllumMask = texture( SelfIllumMaskSampler, vBaseUv ).rgb;
			vSelfIllumMask = mix( baseColor.aaa, vSelfIllumMask, g_SelfIllumMaskControl );
			diffuseComponent = mix( diffuseComponent,
			    g_SelfIllumTint_and_DetailBlendFactor.rgb * albedo, vSelfIllumMask );
		}
		diffuseComponent = max( vec3( 0.0 ), diffuseComponent );
	}

	if ( bDoRimLighting )
	{
		float fRimMultiply = fRimMask * fRimFresnel;
		rimLighting *= fRimMultiply;
		specularLighting = max( specularLighting, rimLighting );
		specularLighting += ( vRimAmbientCubeColor * g_fRimBoost ) *
		                    clamp( fRimMultiply * worldSpaceNormal.z, 0.0, 1.0 );
	}

	vec3 result = specularLighting * vSpecularTint + diffuseComponent;

	float alpha = g_DiffuseModulation.a;
	if ( !bSelfIllum && !bBlendTintByBaseAlpha )
		alpha = mix( baseColor.a * alpha, alpha, g_fBaseMapAlphaPhongMask );

	// The D3D9 fixed-function alpha test (GREATEREQUAL, or GREATER with flag 8).
	if ( consts.params.x >= 0.0 &&
	     ( ( flags & 8 ) != 0 ? alpha <= consts.params.x : alpha < consts.params.x ) )
		discard;
	// FinalOutput( ..., TONEMAP_SCALE_LINEAR ).
	result *= consts.params.w;
	if ( ( flags & 4 ) != 0 )
		result = LinearToSrgb( result );
	outColor = vec4( result, alpha );
}
