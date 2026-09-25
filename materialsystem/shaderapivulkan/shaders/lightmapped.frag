#version 450
// LightmappedGeneric's pixel stage: a port of stdshaders/lightmappedgeneric_ps2_3_x.h
// (the ps20b build, with common_lightmappedgeneric_fxc.h, common_ps_fxc.h's
// TextureCombine and FinalOutput), for the non-flashlight pass. WorldVertexTransition
// is the same shader with BASETEXTURE2.
//
// The static combos come as flags (params.y; kLightmapped* in vulkan_device.h):
// 1 BASETEXTURE2, 2 DETAILTEXTURE, 4 BUMPMAP, 8 BUMPMAP == 2 (ssbump), 16
// BUMPMAP2, 32 CUBEMAP, 64 ENVMAPMASK, 128 BASEALPHAENVMAPMASK, 256 SELFILLUM,
// 512 NORMALMAPALPHAENVMAPMASK, 1024 DIFFUSEBUMPMAP, 2048 BASETEXTURENOENVMAP,
// 4096 BASETEXTURE2NOENVMAP, 8192 BUMPMASK, 16384 MASKEDBLENDING. WARPLIGHTING,
// FANCY_BLENDING, SEAMLESS, OUTLINE and SOFTEDGES are not ported (the shader API
// reports them). NORMAL_DECODE_MODE is always NONE on PC.
//
// The uniform block holds the pixel shader registers as the material's dynamic
// state wrote them (lightmappedgeneric_dx9_helper.cpp): c0 g_EnvmapTint, c2..c4
// (contrast, saturation, fresnel) and c7 g_SelfIllumTint outside the fast path,
// c8 the detail tint and blend factor, c10 g_EyePos, c12 the tint and lightmap
// scale with $alpha2 in w, c29 g_LinearFogColor (w: OO_DESTALPHA_DEPTH_RANGE),
// c30 cLightScale. The flashlight registers c13..c23 are unused by this pass
// and carry vertex-stage state instead: c13/c14 cBaseTexCoordTransform, c15/c16
// cDetailOrBumpTexCoordTransform, c17/c18 cEnvmapMaskTexCoordTransform, c22 (
// cModulationColor.a, the vertex shader's static combos (1 VERTEXCOLOR, 2
// VERTEXALPHATEXBLENDFACTOR), the pixel shader's dynamic combo index, the
// vertex shader's FASTPATH), c23.x DETAIL_BLEND_MODE.
//
// params: x alpha-test reference (< 0 disables), y combo flags, z kColor* flags
// (1 sRGB base, 2 sRGB lightmap, 4 sRGB output, 8 GREATER alpha test, 64 sRGB
// envmap, 4194304 sRGB base 2, 8388608 sRGB detail), w FinalOutput's linear light
// scale.
layout( location = 0 ) in vec2 vUv;
layout( location = 1 ) in vec4 vColor;
layout( location = 2 ) in vec2 vLightmapUv;
layout( location = 3 ) in float vLightmapOffset;
layout( location = 4 ) in vec3 vWorldPos;
layout( location = 5 ) in vec3 vTangentS;
layout( location = 6 ) in vec3 vTangentT;
layout( location = 7 ) in vec3 vWorldNormal;
layout( location = 8 ) flat in vec4 fragFogColor;
layout( location = 9 ) flat in vec4 fragFogParams;
layout( location = 10 ) flat in vec4 fragFogMisc;
layout( location = 11 ) in vec2 fragFogDepth;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;    // s0
layout( set = 1, binding = 0 ) uniform sampler2D LightmapSampler;       // s1
layout( set = 2, binding = 0 ) uniform samplerCube EnvmapSampler;       // s2
layout( set = 3, binding = 0 ) uniform sampler2D BumpmapSampler;        // s4
layout( set = 4, binding = 0 ) uniform sampler2D Bumpmap2OrMaskSampler; // s5
layout( set = 5, binding = 0 ) uniform sampler2D BaseTextureSampler2;   // s7
layout( set = 6, binding = 0 ) uniform PixelShaderConstants
{
	vec4 c[32];
}
ps;
layout( set = 7, binding = 0 ) uniform sampler2D BumpMaskSampler; // s8
layout( set = 8, binding = 0 ) uniform sampler2D DetailSampler;   // s12
layout( push_constant ) uniform Constants
{
	mat4 viewProj;
	vec4 unused0;
	vec4 unused1;
	vec4 eyePos;
	vec4 params;
	vec4 params2;
	vec4 clipPlanes[2];
}
consts;

const int kBaseTexture2 = 1;
const int kDetailTexture = 2;
const int kBumpmap = 4;
const int kSsbump = 8;
const int kBumpmap2 = 16;
const int kCubemap = 32;
const int kEnvmapMask = 64;
const int kBaseAlphaEnvmapMask = 128;
const int kSelfIllum = 256;
const int kNormalMapAlphaEnvmapMask = 512;
const int kDiffuseBumpmap = 1024;
const int kBaseTextureNoEnvmap = 2048;
const int kBaseTexture2NoEnvmap = 4096;
const int kBumpMask = 8192;
const int kMaskedBlending = 16384;

const int kSrgbBase = 1;
const int kSrgbLightmap = 2;
const int kSrgbOutput = 4;
const int kAlphaGreater = 8;
const int kSrgbEnvmap = 64;
const int kSrgbBase2 = 4194304;
const int kSrgbDetail = 8388608;

// common_fxc.h
const float OO_SQRT_3 = 0.57735025882720947;
const vec3 bumpBasis[3] = vec3[3]( vec3( 0.81649661064147949, 0.0, OO_SQRT_3 ),
    vec3( -0.40824833512306213, 0.70710676908493042, OO_SQRT_3 ),
    vec3( -0.40824821591377258, -0.7071068286895752, OO_SQRT_3 ) );

vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}

// A sampler the material reads as sRGB samples its sRGB view when the texture
// has one (the replay clears the flag); otherwise it is decoded here.
vec4 Sample( sampler2D s, vec2 uv, int srgbFlag )
{
	vec4 texel = texture( s, uv );
	if ( ( int( consts.params.z ) & srgbFlag ) != 0 )
		texel.rgb = SrgbToLinear( texel.rgb );
	return texel;
}

// NORM_DECODE_NONE.
vec4 DecompressNormal( sampler2D s, vec2 uv )
{
	vec4 texel = texture( s, uv );
	return vec4( texel.xyz * 2.0 - 1.0, texel.a );
}

// cBaseTexCoordTransform-style rows: dot( float2 uv, row ) + row.w.
vec2 Transform( vec2 uv, vec4 row0, vec4 row1 )
{
	return vec2( dot( uv, row0.xy ) + row0.w, dot( uv, row1.xy ) + row1.w );
}

// common_ps_fxc.h TextureCombine for the modes this shader's SKIP lines allow
// (0, 1 and 10 with a bump map, 11 without).
vec4 TextureCombine( vec4 baseColor, vec4 detailColor, int mode, float blendFactor )
{
	if ( mode == 7 )
	{
		vec3 dc = vec3( mix( detailColor.r, detailColor.a, baseColor.a ) );
		baseColor.rgb *= mix( vec3( 1.0 ), 2.0 * dc, blendFactor );
	}
	if ( mode == 0 )
		baseColor.rgb *= mix( vec3( 1.0 ), 2.0 * detailColor.rgb, blendFactor );
	if ( mode == 1 )
		baseColor.rgb += blendFactor * detailColor.rgb;
	if ( mode == 2 )
		baseColor.rgb = mix( baseColor.rgb, detailColor.rgb, blendFactor * detailColor.a );
	if ( mode == 3 )
		baseColor = mix( baseColor, detailColor, blendFactor );
	if ( mode == 4 )
	{
		baseColor.rgb = mix( baseColor.rgb, detailColor.rgb, blendFactor * ( 1.0 - baseColor.a ) );
		baseColor.a = detailColor.a;
	}
	if ( mode == 8 )
		baseColor = mix( baseColor, baseColor * detailColor, blendFactor );
	if ( mode == 9 )
		baseColor.a = mix( baseColor.a, baseColor.a * detailColor.a, blendFactor );
	if ( mode == 11 )
		baseColor.rgb = baseColor.rgb * dot( detailColor.rgb, vec3( 2.0 / 3.0 ) );
	return baseColor;
}

// common_ps_fxc.h CalcPixelFogFactor for the draw's fog (the same inputs as
// demo_dyn_tex.frag's ApplyPixelFog): type -1 (none), 0 range, 1 height.
float PixelFogFactor()
{
	const float fogType = fragFogColor.w;
	if ( fogType < -0.5 )
		return 0.0;
	const float projZ = fragFogDepth.x;
	if ( fogType < 0.5 )
		return clamp( min( fragFogParams.z, projZ * fragFogParams.w - fragFogParams.x ), 0.0, 1.0 );
	const float worldZ = fragFogDepth.y;
	const float depthFromWater = fragFogParams.y - worldZ;
	const float depthFromEye = fragFogMisc.x - worldZ;
	const float f = clamp( depthFromWater * ( 1.0 / depthFromEye ), 0.0, 1.0 );
	return clamp( f * projZ * fragFogParams.w, 0.0, 1.0 );
}

// BlendPixelFog: range fog squares its factor.
vec3 BlendPixelFog( vec3 color, float factor )
{
	const float fogType = fragFogColor.w;
	if ( fogType < -0.5 )
		return color;
	if ( fogType < 0.5 )
		factor *= factor;
	return mix( color, fragFogColor.rgb, factor );
}

void main()
{
	const int combos = int( consts.params.y );
	const int flags = int( consts.params.z );
	const bool bBaseTexture2 = ( combos & kBaseTexture2 ) != 0;
	const bool bDetailTexture = ( combos & kDetailTexture ) != 0;
	const bool bBumpmap = ( combos & ( kBumpmap | kSsbump ) ) != 0;
	const bool bSsbump = ( combos & kSsbump ) != 0;
	const bool bBumpmap2 = ( combos & kBumpmap2 ) != 0;
	const bool bCubemap = ( combos & kCubemap ) != 0;
	const bool bEnvmapMask = ( combos & kEnvmapMask ) != 0;
	const bool bBaseAlphaEnvmapMask = ( combos & kBaseAlphaEnvmapMask ) != 0;
	const bool bSelfIllum = ( combos & kSelfIllum ) != 0;
	const bool bNormalMapAlphaEnvmapMask = ( combos & kNormalMapAlphaEnvmapMask ) != 0;
	const bool bDiffuseBumpmap = ( combos & kDiffuseBumpmap ) != 0;
	const bool bBaseTextureNoEnvmap = ( combos & kBaseTextureNoEnvmap ) != 0;
	const bool bBaseTexture2NoEnvmap = ( combos & kBaseTexture2NoEnvmap ) != 0;
	const bool bBumpMask = ( combos & kBumpMask ) != 0;
	const bool bMaskedBlending = ( combos & kMaskedBlending ) != 0;
	const int vsCombos = int( ps.c[22].y );
	const int psDynamic = int( ps.c[22].z );
	const bool vsFastPath = ps.c[22].w > 0.5;
	const int detailBlendMode = int( ps.c[23].x );

	// lightmappedgeneric_vs20: the texture coordinates, raw on its FASTPATH.
	const vec2 baseTexCoord = vsFastPath ? vUv : Transform( vUv, ps.c[13], ps.c[14] );
	const vec2 detailOrBumpTexCoord = vsFastPath ? vUv : Transform( vUv, ps.c[15], ps.c[16] );
	const vec2 envmapMaskTexCoord = vsFastPath ? vUv : Transform( vUv, ps.c[17], ps.c[18] );
	// Its vertex color: the modulation alpha, and the vertex color with VERTEXCOLOR
	// (raw on the fast path, else with VERTEXALPHATEXBLENDFACTOR the alpha
	// replaced by the modulation's, or multiplied by it).
	const float modulationAlpha = ps.c[22].x;
	vec4 vertexColor = vec4( 1.0, 1.0, 1.0, modulationAlpha );
	if ( ( vsCombos & 1 ) != 0 )
	{
		if ( vsFastPath )
			vertexColor = vColor;
		else if ( ( vsCombos & 2 ) != 0 )
			vertexColor = vec4( vColor.rgb, modulationAlpha );
		else
			vertexColor = vec4( vColor.rgb, vColor.a * modulationAlpha );
	}
	// vertexBlendX_fogFactorW.r: the vertex alpha with VERTEXALPHATEXBLENDFACTOR.
	const float vertexBlendX = ( vsCombos & 2 ) != 0 ? vColor.a : 0.0;

	// The pixel shader's fast path: fixed envmap contrast (FASTPATHENVMAPCONTRAST),
	// saturation, fresnel and self-illumination tint.
	const bool psFastPath = ( ( psDynamic / 2 ) % 2 ) != 0;
	vec3 envmapContrast = vec3( ( psDynamic % 2 ) != 0 ? 1.0 : 0.0 );
	vec3 envmapSaturation = vec3( 1.0 );
	float fresnelReflection = 1.0;
	float oneMinusFresnelReflection = 0.0;
	vec3 selfIllumTint = vec3( 1.0 );
	if ( !psFastPath )
	{
		envmapContrast = ps.c[2].rgb;
		envmapSaturation = ps.c[3].rgb;
		fresnelReflection = ps.c[4].a;
		oneMinusFresnelReflection = ps.c[4].b;
		selfIllumTint = ps.c[7].rgb;
	}

	// GetBaseTextureAndNormal: the bump map is read at the base coordinates.
	vec4 baseColor = Sample( BaseTextureSampler, baseTexCoord, kSrgbBase );
	vec4 baseColor2 = vec4( 0.0 );
	if ( bBaseTexture2 )
		baseColor2 = Sample( BaseTextureSampler2, baseTexCoord, kSrgbBase2 );
	vec4 vNormal = vec4( 0.0, 0.0, 1.0, 1.0 );
	if ( bBumpmap || bNormalMapAlphaEnvmapMask )
		vNormal = texture( BumpmapSampler, baseTexCoord );
	if ( bBumpmap && !bSsbump )
		vNormal.xyz = vNormal.xyz * 2.0 - 1.0;

	vec3 lightmapColor1 = vec3( 1.0 );
	vec3 lightmapColor2 = vec3( 1.0 );
	vec3 lightmapColor3 = vec3( 1.0 );
	// The bumped pages follow the flat one at TEXCOORD2's offset.
	const vec2 lightmapOffset = vec2( vLightmapOffset, 0.0 );
	if ( bBumpmap && bDiffuseBumpmap )
	{
		lightmapColor1 = Sample( LightmapSampler, vLightmapUv + lightmapOffset, kSrgbLightmap ).rgb;
		lightmapColor2 =
		    Sample( LightmapSampler, vLightmapUv + 2.0 * lightmapOffset, kSrgbLightmap ).rgb;
		lightmapColor3 =
		    Sample( LightmapSampler, vLightmapUv + 3.0 * lightmapOffset, kSrgbLightmap ).rgb;
	}
	else
	{
		lightmapColor1 = Sample( LightmapSampler, vLightmapUv, kSrgbLightmap ).rgb;
	}

	vec2 detailTexCoord = vec2( 0.0 );
	vec2 bumpmapTexCoord = detailOrBumpTexCoord;
	vec2 bumpmap2TexCoord = envmapMaskTexCoord;
	if ( bDetailTexture )
	{
		detailTexCoord = detailOrBumpTexCoord;
		bumpmapTexCoord = baseTexCoord;
	}
	vec4 detailColor = vec4( 1.0 );
	if ( bDetailTexture )
		detailColor = vec4( ps.c[8].rgb, 1.0 ) * Sample( DetailSampler, detailTexCoord, kSrgbDetail );

	float blendedAlpha = baseColor.a;
	const float blendfactor = bMaskedBlending ? 0.5 : vertexBlendX;
	if ( bBaseTexture2 )
	{
		baseColor.rgb = mix( baseColor.rgb, baseColor2.rgb, blendfactor );
		blendedAlpha = mix( baseColor.a, baseColor2.a, blendfactor );
	}

	vec3 specularFactor = vec3( 1.0 );
	vec4 vNormalMask = vec4( 0.0, 0.0, 1.0, 1.0 );
	if ( bBumpmap )
	{
		if ( bBaseTextureNoEnvmap )
			vNormal.a = 0.0;
		if ( bBumpmap2 )
		{
			const vec2 b2TexCoord = bBumpMask ? bumpmap2TexCoord : bumpmapTexCoord;
			vec4 vNormal2 = bSsbump ? texture( Bumpmap2OrMaskSampler, b2TexCoord )
			                        : DecompressNormal( Bumpmap2OrMaskSampler, b2TexCoord );
			if ( bBaseTexture2NoEnvmap )
				vNormal2.a = 0.0;
			if ( bBumpMask )
			{
				const vec3 vNormal1 = DecompressNormal( BumpmapSampler, detailOrBumpTexCoord ).xyz;
				vNormal.xyz = normalize( vNormal1 + vNormal2.xyz );
				vNormalMask = DecompressNormal( BumpMaskSampler, baseTexCoord );
				vNormal.xyz = mix( vNormalMask.xyz, vNormal.xyz, vNormalMask.a );
				specularFactor = vec3( vNormalMask.a );
			}
			else
			{
				vNormal.xyz = mix( vNormal.xyz, vNormal2.xyz, blendfactor );
			}
		}
		if ( bNormalMapAlphaEnvmapMask )
			specularFactor *= vNormal.a;
	}
	else if ( bNormalMapAlphaEnvmapMask )
	{
		specularFactor *= vNormal.a;
	}
	if ( !bBumpmap2 && bEnvmapMask )
		specularFactor *= texture( Bumpmap2OrMaskSampler, envmapMaskTexCoord ).xyz;
	if ( bBaseAlphaEnvmapMask )
		specularFactor *= 1.0 - blendedAlpha;

	vec4 albedo = baseColor;
	float alpha = 1.0;
	if ( !bBaseAlphaEnvmapMask && !bSelfIllum )
		alpha *= baseColor.a;
	if ( bDetailTexture )
		albedo = TextureCombine( albedo, detailColor, detailBlendMode, ps.c[8].w );
	albedo.xyz *= vertexColor.rgb;
	alpha *= vertexColor.a * ps.c[12].w;

	vec3 diffuseLighting;
	if ( bBumpmap && bDiffuseBumpmap )
	{
		if ( bSsbump )
		{
			// Portal 2's TCOMBINE_SSBUMP_BUMP on an ssbump: the detail ssbump
			// scales the basis weights (then alpha is 1).
			if ( bDetailTexture && detailBlendMode == 10 )
			{
				vNormal.xyz *= mix( vec3( 1.0 ), 2.0 * detailColor.rgb, alpha );
				alpha = 1.0;
			}
			diffuseLighting = vNormal.x * lightmapColor1 + vNormal.y * lightmapColor2 +
			                  vNormal.z * lightmapColor3;
			// $ssbumpmathfix: the weights of a flat ssbump sum to 1.733, not 1
			// (c23.y is 0.57735 with the fix, 1 without).
			diffuseLighting *= ps.c[23].y;
			diffuseLighting *= ps.c[12].rgb;
			vNormal.xyz = normalize( bumpBasis[0] * vNormal.x + bumpBasis[1] * vNormal.y +
			                         bumpBasis[2] * vNormal.z );
		}
		else
		{
			vec3 dp;
			dp.x = clamp( dot( vNormal.xyz, bumpBasis[0] ), 0.0, 1.0 );
			dp.y = clamp( dot( vNormal.xyz, bumpBasis[1] ), 0.0, 1.0 );
			dp.z = clamp( dot( vNormal.xyz, bumpBasis[2] ), 0.0, 1.0 );
			dp *= dp;
			if ( detailBlendMode == 10 )
				dp *= 2.0 * detailColor.rgb;
			diffuseLighting = dp.x * lightmapColor1 + dp.y * lightmapColor2 + dp.z * lightmapColor3;
			const float sum = dot( dp, vec3( 1.0 ) );
			diffuseLighting *= ps.c[12].rgb / sum;
		}
	}
	else
	{
		diffuseLighting = lightmapColor1 * ps.c[12].rgb;
	}

	vec3 diffuseComponent = albedo.xyz * diffuseLighting;
	if ( bSelfIllum )
		diffuseComponent = mix( diffuseComponent, selfIllumTint * albedo.xyz, baseColor.a );

	vec3 specularLighting = vec3( 0.0 );
	if ( bCubemap )
	{
		// mul( vNormal, tangentSpaceTranspose ): rows S, T, N.
		const vec3 worldSpaceNormal =
		    vNormal.x * vTangentS + vNormal.y * vTangentT + vNormal.z * vWorldNormal;
		const vec3 worldVertToEyeVector = ps.c[10].xyz - vWorldPos;
		const vec3 reflectVect = 2.0 * dot( worldSpaceNormal, worldVertToEyeVector ) * worldSpaceNormal -
		                         dot( worldSpaceNormal, worldSpaceNormal ) * worldVertToEyeVector;
		const vec3 eyeVect = normalize( worldVertToEyeVector );
		float fresnel = 1.0 - dot( worldSpaceNormal, eyeVect );
		fresnel = pow( fresnel, 5.0 );
		fresnel = fresnel * oneMinusFresnelReflection + fresnelReflection;
		vec3 envmap = texture( EnvmapSampler, reflectVect ).rgb;
		if ( ( flags & kSrgbEnvmap ) != 0 )
			envmap = SrgbToLinear( envmap );
		specularLighting = ps.c[30].z * envmap; // ENV_MAP_SCALE
		// $envmaplightscale (c23.z, min/max in c21.xy): the cubemap darkened
		// where the lightmap is dark.
		if ( ps.c[23].z > 0.0 )
		{
			const vec3 cubemapLight = clamp( ( diffuseLighting - ps.c[21].x ) * ps.c[21].y, 0.0, 1.0 );
			specularLighting = mix( specularLighting, specularLighting * cubemapLight, ps.c[23].z );
		}
		specularLighting *= specularFactor;
		specularLighting *= ps.c[0].rgb;
		const vec3 specularLightingSquared = specularLighting * specularLighting;
		specularLighting = mix( specularLighting, specularLightingSquared, envmapContrast );
		const vec3 greyScale = vec3( dot( specularLighting, vec3( 0.299, 0.587, 0.114 ) ) );
		specularLighting = mix( greyScale, specularLighting, envmapSaturation );
		specularLighting *= fresnel;
	}

	vec3 result = diffuseComponent + specularLighting;

	// WRITE_DEPTH_TO_DESTALPHA unless WRITEWATERFOGTODESTALPHA; with the latter
	// and height fog the alpha is the fog factor.
	const bool writeWaterFogToDestAlpha = ( ( psDynamic / 4 ) % 2 ) != 0;
	const bool writeDepthToAlpha = ( ( psDynamic / 48 ) % 2 ) != 0 && !writeWaterFogToDestAlpha;
	const float fogFactor = PixelFogFactor();
	if ( writeWaterFogToDestAlpha && fragFogColor.w > 0.5 )
		alpha = fogFactor;

	// FinalOutput( ..., TONEMAP_SCALE_LINEAR, bWriteDepthToAlpha, projZ ).
	result *= consts.params.w;
	const float outAlpha = writeDepthToAlpha ? fragFogDepth.x * ps.c[29].w : alpha;
	// The D3D9 fixed-function alpha test on the output: GREATEREQUAL, or GREATER.
	if ( consts.params.x >= 0.0 && ( ( flags & kAlphaGreater ) != 0 ? outAlpha <= consts.params.x
	                                                                 : outAlpha < consts.params.x ) )
		discard;
	result = BlendPixelFog( result, fogFactor );
	if ( ( flags & kSrgbOutput ) != 0 )
		result = LinearToSrgb( result );
	outColor = vec4( result, outAlpha );
}
