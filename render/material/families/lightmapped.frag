// render.material family `lightmapped` (RFC 0016 K4, K5 surface model): the
// lit surface term, LightmappedGeneric's arithmetic (the port's
// lightmapped.frag, from lightmappedgeneric_ps2_3_x.h) in linear light. Each
// term is a specialization constant, so a neutral term costs nothing:
// - the flat lightmap, or the three bumped pages weighted by the normal map
//   (RNM) or by an ssbump's basis weights;
// - the env map with its mask (base alpha, normal map alpha or $envmapmask),
//   tint, contrast, saturation and fresnel;
// - detail (TextureCombine) and self-illumination;
// - the view's fog.
// Unlit is this term with the lighting fixed at one. Blending is pipeline state.
#version 450

#include "../../shaders/common/color_encoding.glsl"

// The terms (the port's static combo bits where they exist).
layout( constant_id = 0 ) const int kTerms = 0;
layout( constant_id = 1 ) const int kDetailMode = 0;
const int kDetailTexture = 2;
const int kBumpmap = 4;
const int kSsbump = 8;
const int kCubemap = 32;
const int kEnvmapMask = 64;
const int kBaseAlphaEnvmapMask = 128;
const int kSelfIllum = 256;
const int kNormalMapAlphaEnvmapMask = 512;
const int kDiffuseBumpmap = 1024;

layout( set = 0, binding = 0 ) uniform Frame
{
	// x: the lightmap scale for how the pages encode light (2^2.2 for LDR
	// gamma pages, 16 for integer-HDR pages); y: the output's linear scale
	// (the frame's tone-mapping scale, 1 without HDR); z: 1 when the target
	// has no sRGB view and the shader encodes the output itself; w: 1 when
	// specular shows (mat_specular), else the env map's tint is zero.
	vec4 light;
	// The view's fog (common_ps_fxc.h CalcPixelFogFactor, BlendPixelFog):
	// color with its type in w (-1 none, 0 range, 1 height), parameters
	// (range: start / range, water z, max density, 1 / range; height: 0,
	// water z, 1, 1 / range), and the eye's world z in misc.x.
	vec4 fogColor;
	vec4 fogParams;
	vec4 fogMisc; // x: the eye's world z, y: 1 when the game scales every ssbump by 1/sqrt(3)
	// xyz: the eye's world position (c10); w: ENV_MAP_SCALE (16 in integer
	// HDR, where cube maps hold light / 16, else 1).
	vec4 eye;
} frame;
layout( set = 2, binding = 0 ) uniform Material
{
	vec4 tint;  // rgb: $color, a: $alpha
	vec4 flags; // x: $vertexcolor, y: $alphatest, z: $alphatestreference, w: 1 when lighting is one (unlit)
	// x: 1 when fully opaque (height fog's factor is the output alpha), y:
	// gamma vertex color (vertex stage), z: the ssbump weights' scale
	// (0.57735 with $ssbumpmathfix, else 1), w: unused
	vec4 state;
	vec4 envTint;       // rgb: $envmaptint, a: $fresnelreflection
	vec4 envContrast;   // rgb: the contrast in effect, a: 1 - $fresnelreflection
	vec4 envSaturation; // rgb: the saturation in effect
	vec4 selfIllumTint; // rgb: $selfillumtint
	vec4 detailTint;    // rgb: $detailtint, a: $detailblendfactor
	vec4 detailScale;   // xy: $detailscale
	vec4 envLightScale; // x: min, y: min + max, z: $envmaplightscale (Portal 2)
} material;
layout( set = 2, binding = 1 ) uniform texture2D baseTexture;
layout( set = 2, binding = 2 ) uniform sampler baseSampler;
layout( set = 2, binding = 3 ) uniform textureCube envmapTexture;
layout( set = 2, binding = 4 ) uniform sampler envmapSampler;
layout( set = 2, binding = 5 ) uniform texture2D envmapMaskTexture;
layout( set = 2, binding = 6 ) uniform sampler envmapMaskSampler;
layout( set = 2, binding = 7 ) uniform texture2D bumpTexture;
layout( set = 2, binding = 8 ) uniform sampler bumpSampler;
layout( set = 2, binding = 9 ) uniform texture2D detailTexture;
layout( set = 2, binding = 10 ) uniform sampler detailSampler;
// The lightmap page is the draw's: surfaces of one material share pages
// with others.
layout( set = 3, binding = 0 ) uniform texture2D lightmap;
layout( set = 3, binding = 1 ) uniform sampler lightmapSampler;

layout( location = 0 ) in vec2 baseUv;
layout( location = 1 ) in vec2 lightmapUv;
layout( location = 2 ) in vec4 color;
layout( location = 3 ) in vec2 fogDepth; // the clip-space z (D3D9's projPos.z) and world z
layout( location = 4 ) in vec3 worldPosition;
layout( location = 5 ) in vec3 worldNormal;
layout( location = 6 ) in vec3 tangentS;
layout( location = 7 ) in vec3 tangentT;
layout( location = 8 ) in float lightmapOffset; // the bumped pages' offset (TEXCOORD2.x)
layout( location = 0 ) out vec4 outColor;

// common_fxc.h
const float OO_SQRT_3 = 0.57735025882720947;
const vec3 bumpBasis[3] = vec3[3]( vec3( 0.81649661064147949, 0.0, OO_SQRT_3 ),
    vec3( -0.40824833512306213, 0.70710676908493042, OO_SQRT_3 ),
    vec3( -0.40824821591377258, -0.7071068286895752, OO_SQRT_3 ) );

bool Term( int term )
{
	return ( kTerms & term ) != 0;
}

float FogFactor()
{
	const float type = frame.fogColor.w;
	if ( type < -0.5 )
		return 0.0;
	const float projZ = fogDepth.x;
	if ( type < 0.5 )
		return clamp( min( frame.fogParams.z, projZ * frame.fogParams.w - frame.fogParams.x ), 0.0,
		    1.0 );
	const float depthFromWater = frame.fogParams.y - fogDepth.y;
	const float depthFromEye = frame.fogMisc.x - fogDepth.y;
	const float f = clamp( depthFromWater * ( 1.0 / depthFromEye ), 0.0, 1.0 );
	return clamp( f * projZ * frame.fogParams.w, 0.0, 1.0 );
}

// common_ps_fxc.h TextureCombine for the modes the family claims.
vec4 TextureCombine( vec4 baseColor, vec4 detailColor, float blendFactor )
{
	if ( kDetailMode == 0 )
		baseColor.rgb *= mix( vec3( 1.0 ), 2.0 * detailColor.rgb, blendFactor );
	if ( kDetailMode == 1 )
		baseColor.rgb += blendFactor * detailColor.rgb;
	if ( kDetailMode == 2 )
		baseColor.rgb = mix( baseColor.rgb, detailColor.rgb, blendFactor * detailColor.a );
	if ( kDetailMode == 3 )
		baseColor = mix( baseColor, detailColor, blendFactor );
	if ( kDetailMode == 4 )
	{
		baseColor.rgb = mix( baseColor.rgb, detailColor.rgb, blendFactor * ( 1.0 - baseColor.a ) );
		baseColor.a = detailColor.a;
	}
	if ( kDetailMode == 7 )
	{
		vec3 dc = vec3( mix( detailColor.r, detailColor.a, baseColor.a ) );
		baseColor.rgb *= mix( vec3( 1.0 ), 2.0 * dc, blendFactor );
	}
	if ( kDetailMode == 8 )
		baseColor = mix( baseColor, baseColor * detailColor, blendFactor );
	if ( kDetailMode == 9 )
		baseColor.a = mix( baseColor.a, baseColor.a * detailColor.a, blendFactor );
	return baseColor;
}

void main()
{
	const bool bumpmap = Term( kBumpmap | kSsbump );
	const bool ssbump = Term( kSsbump );
	const bool diffuseBumpmap = bumpmap && Term( kDiffuseBumpmap );
	const bool lightingOne = material.flags.w != 0.0;

	const vec4 base = texture( sampler2D( baseTexture, baseSampler ), baseUv );
	// GetBaseTextureAndNormal: the bump map is read at the base coordinates;
	// with only $normalmapalphaenvmapmask its texels are used undecoded, as
	// the port does.
	vec4 normalSample = vec4( 0.0, 0.0, 1.0, 1.0 );
	if ( bumpmap || Term( kNormalMapAlphaEnvmapMask ) )
		normalSample = texture( sampler2D( bumpTexture, bumpSampler ), baseUv );
	if ( bumpmap && !ssbump )
		normalSample.xyz = normalSample.xyz * 2.0 - 1.0;

	vec3 albedo = base.rgb;
	// The port's vertex fast path (no texture transform): the vertex color
	// replaces the modulation alpha, which otherwise applies $alpha a second
	// time; self-illumination and a base-alpha env map mask use the base
	// alpha as their mask instead.
	float alpha = Term( kBaseAlphaEnvmapMask ) || Term( kSelfIllum ) ? 1.0 : base.a;
	alpha *= material.tint.a;
	if ( Term( kDetailTexture ) )
	{
		const vec4 detail = vec4( material.detailTint.rgb, 1.0 ) *
		                    texture( sampler2D( detailTexture, detailSampler ),
		                        baseUv * material.detailScale.xy );
		albedo = TextureCombine( vec4( albedo, base.a ), detail, material.detailTint.a ).rgb;
	}
	if ( material.flags.x != 0.0 )
	{
		// Off the vertex fast path (detail) the modulation alpha multiplies
		// the vertex alpha instead of being replaced by it.
		albedo *= color.rgb;
		alpha *= color.a * ( Term( kDetailTexture ) ? material.tint.a : 1.0 );
	}
	else
	{
		alpha *= material.tint.a;
	}
	if ( material.flags.y != 0.0 && alpha < material.flags.z )
		discard;

	// The diffuse light: c12 is the tint times the lightmap scale.
	const vec3 c12 = material.tint.rgb * ( lightingOne ? 1.0 : frame.light.x );
	vec3 diffuse;
	if ( lightingOne )
	{
		diffuse = c12;
	}
	else if ( diffuseBumpmap )
	{
		const vec2 offset = vec2( lightmapOffset, 0.0 );
		const vec3 light1 =
		    texture( sampler2D( lightmap, lightmapSampler ), lightmapUv + offset ).rgb;
		const vec3 light2 =
		    texture( sampler2D( lightmap, lightmapSampler ), lightmapUv + 2.0 * offset ).rgb;
		const vec3 light3 =
		    texture( sampler2D( lightmap, lightmapSampler ), lightmapUv + 3.0 * offset ).rgb;
		if ( ssbump )
		{
			diffuse = normalSample.x * light1 + normalSample.y * light2 + normalSample.z * light3;
			// The running game's shaders may scale every ssbump (Portal 2's
			// do); else $ssbumpmathfix does.
			diffuse *= ( frame.fogMisc.y != 0.0 ? OO_SQRT_3 : material.state.z ) * c12;
			normalSample.xyz = normalize( bumpBasis[0] * normalSample.x +
			                              bumpBasis[1] * normalSample.y +
			                              bumpBasis[2] * normalSample.z );
		}
		else
		{
			vec3 dp;
			dp.x = clamp( dot( normalSample.xyz, bumpBasis[0] ), 0.0, 1.0 );
			dp.y = clamp( dot( normalSample.xyz, bumpBasis[1] ), 0.0, 1.0 );
			dp.z = clamp( dot( normalSample.xyz, bumpBasis[2] ), 0.0, 1.0 );
			dp *= dp;
			diffuse = dp.x * light1 + dp.y * light2 + dp.z * light3;
			diffuse *= c12 / dot( dp, vec3( 1.0 ) );
		}
	}
	else
	{
		diffuse = texture( sampler2D( lightmap, lightmapSampler ), lightmapUv ).rgb * c12;
	}

	vec3 lit = albedo * diffuse;
	if ( Term( kSelfIllum ) )
		lit = mix( lit, material.selfIllumTint.rgb * albedo, base.a );

	if ( Term( kCubemap ) )
	{
		vec3 specularFactor = vec3( 1.0 );
		if ( Term( kNormalMapAlphaEnvmapMask ) )
			specularFactor *= normalSample.a;
		if ( Term( kEnvmapMask ) )
			specularFactor *=
			    texture( sampler2D( envmapMaskTexture, envmapMaskSampler ), baseUv ).xyz;
		if ( Term( kBaseAlphaEnvmapMask ) )
			specularFactor *= 1.0 - base.a;
		// mul( vNormal, tangentSpaceTranspose ): rows S, T, N.
		const vec3 normal = normalSample.x * tangentS + normalSample.y * tangentT +
		                    normalSample.z * worldNormal;
		const vec3 toEye = frame.eye.xyz - worldPosition;
		const vec3 reflected =
		    2.0 * dot( normal, toEye ) * normal - dot( normal, normal ) * toEye;
		float fresnel = pow( 1.0 - dot( normal, normalize( toEye ) ), 5.0 );
		fresnel = fresnel * material.envContrast.a + material.envTint.a;
		vec3 specular =
		    frame.eye.w * texture( samplerCube( envmapTexture, envmapSampler ), reflected ).rgb;
		// Portal 2's $envmaplightscale: darker where the diffuse light is.
		if ( material.envLightScale.z > 0.0 )
		{
			const vec3 cubemapLight =
			    clamp( ( diffuse - material.envLightScale.x ) * material.envLightScale.y, 0.0, 1.0 );
			specular = mix( specular, specular * cubemapLight, material.envLightScale.z );
		}
		specular *= specularFactor * material.envTint.rgb * frame.light.w;
		specular = mix( specular, specular * specular, material.envContrast.rgb );
		const vec3 grey = vec3( dot( specular, vec3( 0.299, 0.587, 0.114 ) ) );
		specular = mix( grey, specular, material.envSaturation.rgb );
		// The port adds the specular term after the lightmap scale; the
		// tint's lightmap scale is in c12 only.
		lit += specular * fresnel;
	}

	lit *= frame.light.y;
	// The view's fog, after the tone-mapping scale (its color is scaled too):
	// range fog squares its factor; a fully opaque surface under height fog
	// writes the factor to alpha.
	const float fogType = frame.fogColor.w;
	if ( fogType > -0.5 )
	{
		const float factor = FogFactor();
		if ( fogType > 0.5 && material.state.x != 0.0 )
			alpha = factor;
		lit = mix( lit, frame.fogColor.rgb, fogType < 0.5 ? factor * factor : factor );
	}
	if ( frame.light.z != 0.0 )
		lit = LinearToSrgb( lit );
	outColor = vec4( lit, alpha );
}
