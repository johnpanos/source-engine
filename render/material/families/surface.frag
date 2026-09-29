// render.material program `surface` (RFC 0016 K11 "Model assembly"): the one
// surface program. Each lighting-model term is a specialization constant, so
// a neutral term costs nothing, and the legacy families are points of it:
// - the `lightmapped` point is LightmappedGeneric's arithmetic (the port's
//   lightmapped.frag, from lightmappedgeneric_ps2_3_x.h) in linear light: the
//   flat lightmap, or the three bumped pages weighted by the normal map (RNM)
//   or by an ssbump's basis weights; the env map with its mask (base alpha,
//   normal map alpha or $envmapmask), tint, contrast, saturation and fresnel;
//   detail (TextureCombine) and self-illumination. Unlit is this point with
//   the lighting fixed at one;
// - the `pbr` point (kPbr) is the RFC 0007 layered metal/roughness BRDF
//   (render/shaders/common/pbr_brdf.glsl, the one GLSL copy of
//   public/render/pbr_brdf.h) under the draw's model lighting: the model
//   port's arithmetic (model_pbr.frag) without map probes, environment maps,
//   the probe volume or clear coat. Units follow Source's model lighting: a
//   local light is incident radiance pi * color * attenuation, the ambient
//   cube a Lambertian return, and the cube in the reflected direction the
//   specular image light.
// Both end in the view's fog and the output encoding. Blending is pipeline
// state. The debug views and lighting-model controls (RFC 0014) come from
// debug_view.glsl; at their neutral values they are dead code. In the pbr
// point the local lights answer to the `clustered` term, the ambient cube to
// `probes` and the cube in the reflected direction to `ibl`.
#version 450

#include "../../shaders/common/color_encoding.glsl"
#include "../../shaders/common/debug_view.glsl"
#include "../../shaders/common/pbr_brdf.glsl"
#include "surface_lighting.glsl"

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
// The pbr point: the metal/roughness BRDF; kBumpmap is then its tangent-space
// normal map (two channels), and kEmissionTexture its emission texture.
const int kPbr = 2048;
const int kEmissionTexture = 4096;

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
// The split-sum table (RFC 0007, pbr_split_sum_table.h), read by the pbr point.
layout( set = 0, binding = 1 ) uniform texture2D splitSumTexture;
layout( set = 0, binding = 2 ) uniform sampler splitSumSampler;
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
	vec4 emission;      // x: $emissionscale (the pbr point)
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
layout( set = 2, binding = 11 ) uniform texture2D mraoTexture;
layout( set = 2, binding = 12 ) uniform sampler mraoSampler;
layout( set = 2, binding = 13 ) uniform texture2D emissionTexture;
layout( set = 2, binding = 14 ) uniform sampler emissionSampler;
// The lightmap page is the draw's: surfaces of one material share pages
// with others. So is the model lighting (surface_lighting.glsl, binding 2).
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
layout( location = 9 ) in vec4 lightAtten;      // each model light's vertex attenuation
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

// The debug view's pixel, encoded as the output is.
vec4 DebugOutput( DebugInputs inputs )
{
	vec4 view = DebugViewOutput( inputs );
	if ( frame.light.z != 0.0 )
		view.rgb = LinearToSrgb( view.rgb );
	return view;
}

// Every point's output: the tone-mapping scale, then the view's fog (its
// color is scaled too): range fog squares its factor; a fully opaque surface
// under height fog writes the factor to alpha. Then the encoding.
vec4 Output( vec3 lit, float alpha )
{
	lit *= frame.light.y;
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
	return vec4( lit, alpha );
}

// PixelShaderAmbientLight: the faces weighted by the squared normal. In the
// furnace (RFC 0014) every face is a uniform radiance of 1.
vec3 AmbientCube( vec3 n )
{
	const vec3 squared = n * n;
	if ( DebugFurnace() )
		return vec3( squared.x + squared.y + squared.z );
	const bvec3 positive = greaterThanEqual( n, vec3( 0.0 ) );
	return squared.x * ( positive.x ? lighting.cube[0] : lighting.cube[1] ).rgb +
	       squared.y * ( positive.y ? lighting.cube[2] : lighting.cube[3] ).rgb +
	       squared.z * ( positive.z ? lighting.cube[4] : lighting.cube[5] ).rgb;
}

// The pbr point. Base and emission are sampled as sRGB, MRAO and the normal
// map as linear data.
void PbrSurface()
{
	const bool furnace = DebugFurnace();
	const bool normalMap = Term( kBumpmap );
	const bool emissive = Term( kEmissionTexture );
	const vec2 uv = baseUv;
	const vec4 baseSample = texture( sampler2D( baseTexture, baseSampler ), uv );
	const vec3 base = furnace ? vec3( 1.0 ) : baseSample.rgb;
	const vec3 mrao = texture( sampler2D( mraoTexture, mraoSampler ), uv ).rgb;
	const float metalness =
	    kDebugForceMetalness >= 0.0 ? kDebugForceMetalness : clamp( mrao.r, 0.0, 1.0 );
	const float roughness =
	    max( kDebugForceRoughness >= 0.0 ? kDebugForceRoughness : mrao.g, 0.02 );
	const float occlusion = DebugTermOn( kDebugTermAo ) ? clamp( mrao.b, 0.0, 1.0 ) : 1.0;

	const vec3 view = normalize( frame.eye.xyz - worldPosition );
	vec3 normal = dot( worldNormal, worldNormal ) > 1e-12 ? normalize( worldNormal ) : view;
	vec3 mapped = vec3( 0.0, 0.0, 1.0 );
	if ( normalMap )
	{
		const vec2 xy = texture( sampler2D( bumpTexture, bumpSampler ), uv ).rg * 2.0 - 1.0;
		mapped = vec3( xy, sqrt( max( 0.0, 1.0 - dot( xy, xy ) ) ) );
		normal = normalize( normalize( tangentS ) * mapped.x + normalize( tangentT ) * mapped.y +
		                    normal * mapped.z );
	}
	const float normalDotView = max( dot( normal, view ), 0.0 );
	const vec3 f0 = mix( vec3( 0.04 ), base, metalness );
	const vec2 splitSum = texture( sampler2D( splitSumTexture, splitSumSampler ),
	    PbrSplitSumCoordinate( vec2( textureSize( sampler2D( splitSumTexture, splitSumSampler ), 0 ) ),
	        normalDotView, roughness ) )
	                          .rg;
	// cl_render_debug_brdf 3: multiple-scattering compensation off.
	const bool compensate = kDebugBrdf != kDebugBrdfNoEnergyCompensation;
	const vec3 compensation = compensate ? PbrEnergyCompensation( f0, splitSum ) : vec3( 1.0 );
	const vec3 directionalAlbedo = compensate
	                                   ? PbrDirectionalAlbedo( f0, splitSum )
	                                   : min( vec3( 1.0 ), f0 * splitSum.x + vec3( splitSum.y ) );
	const vec3 diffuseColor = base * ( 1.0 - metalness ) * ( vec3( 1.0 ) - directionalAlbedo );
	// cl_render_debug_brdf 1 and 2: one lobe.
	const bool diffuseLobe = kDebugBrdf != kDebugBrdfSpecularOnly;
	const bool specularLobe = kDebugBrdf != kDebugBrdfDiffuseOnly;

	vec3 color = diffuseLobe && DebugTermOn( kDebugTermProbes )
	                 ? diffuseColor * AmbientCube( normal ) * occlusion
	                 : vec3( 0.0 );
	vec3 direct = vec3( 0.0 );
	const int count = DebugTermOn( kDebugTermClustered ) && !furnace ? int( lighting.eye.w ) : 0;
	for ( int i = 0; i < 4; ++i )
	{
		if ( i >= count )
			break;
		// A directional light shines along its direction. (The port's pixel
		// constants place it 10,000 units from the lighting origin against
		// that direction, CommitPixelShaderLighting; the vertex term reads
		// the light's own position, which is 1 for it.)
		const vec3 light = lighting.lights[i].color.w > 0.5
		                       ? -normalize( lighting.lights[i].direction.xyz )
		                       : normalize( lighting.lights[i].position.xyz - worldPosition );
		const float normalDotLight = max( dot( normal, light ), 0.0 );
		if ( normalDotLight <= 0.0 )
			continue;
		const vec3 incident = lighting.lights[i].color.rgb * lightAtten[i];
		if ( diffuseLobe )
		{
			const vec3 diffuse = diffuseColor * incident * normalDotLight;
			color += diffuse;
			direct += diffuse;
		}
		if ( specularLobe )
		{
			const vec3 specular = kPi * incident * PbrSpecular( normal, view, light, f0, roughness ) *
			                      compensation * normalDotLight;
			color += specular;
			direct += specular;
		}
	}
	vec3 imageSpecular = vec3( 0.0 );
	if ( specularLobe && DebugTermOn( kDebugTermIbl ) )
	{
		imageSpecular = AmbientCube( reflect( -view, normal ) ) * directionalAlbedo * occlusion;
		color += imageSpecular;
	}
	vec3 emission = vec3( 0.0 );
	if ( emissive && DebugTermOn( kDebugTermEmission ) && !furnace )
	{
		emission =
		    texture( sampler2D( emissionTexture, emissionSampler ), uv ).rgb * material.emission.x;
		color += emission;
	}

	if ( DebugViewActive() )
	{
		DebugInputs inputs = DebugInputsNone();
		inputs.mask = kDebugHasAlbedo | kDebugHasNormal | kDebugHasRoughness | kDebugHasMetalness |
		              kDebugHasAo | kDebugHasDirect | kDebugHasImageSpecular | kDebugHasUv0;
		inputs.albedo = base;
		inputs.normal = normal;
		if ( normalMap )
		{
			inputs.mask |= kDebugHasNormalMap;
			inputs.normalMap = mapped;
		}
		inputs.roughness = roughness;
		inputs.metalness = metalness;
		inputs.ao = occlusion;
		inputs.direct = direct;
		inputs.imageSpecular = imageSpecular;
		if ( emissive )
		{
			inputs.mask |= kDebugHasEmission;
			inputs.emission = emission;
		}
		inputs.uv0 = uv;
		inputs.final = color;
		outColor = DebugOutput( inputs );
		return;
	}
	// cl_render_debug_brdf 4: the split-sum table's sample as red and green.
	if ( kDebugBrdf == kDebugBrdfSplitSumSample )
	{
		outColor = vec4( splitSum, 0.0, 1.0 );
		return;
	}
	outColor = Output( color, baseSample.a );
}

void main()
{
	if ( Term( kPbr ) )
	{
		PbrSurface();
		return;
	}
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

	const bool furnace = DebugFurnace();
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

	// The furnace (RFC 0014): albedo 1 after every modulation, the light a
	// uniform radiance of 1 in place of the lightmap and the env map.
	vec3 tint = material.tint.rgb;
	if ( furnace )
	{
		albedo = vec3( 1.0 );
		tint = vec3( 1.0 );
	}
	// The diffuse light: c12 is the tint times the lightmap scale.
	const vec3 c12 = tint * ( lightingOne ? 1.0 : frame.light.x );
	// The baked light with albedo 1 (the debug view), before the tint.
	vec3 baked = vec3( 0.0 );
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
			const float weightScale = frame.fogMisc.y != 0.0 ? OO_SQRT_3 : material.state.z;
			baked = diffuse * weightScale * frame.light.x;
			diffuse *= weightScale * c12;
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
			baked = diffuse / dot( dp, vec3( 1.0 ) ) * frame.light.x;
			diffuse *= c12 / dot( dp, vec3( 1.0 ) );
		}
	}
	else
	{
		const vec3 page = texture( sampler2D( lightmap, lightmapSampler ), lightmapUv ).rgb;
		baked = page * frame.light.x;
		diffuse = page * c12;
	}
	// cl_render_debug_term baked: the frame of a zero lightmap page;
	// cl_render_debug_brdf 2 (specular only) drops the diffuse lobe.
	if ( !lightingOne && furnace )
		diffuse = baked = vec3( 1.0 );
#ifndef SEEDED_DEBUG_TERM_IGNORED
	if ( !lightingOne && !DebugTermOn( kDebugTermBaked ) )
		diffuse = vec3( 0.0 );
#endif

	vec3 lit = kDebugBrdf == kDebugBrdfSpecularOnly ? vec3( 0.0 ) : albedo * diffuse;
	// Self-illumination replaces the diffuse term by its tint times albedo
	// where base alpha is set. Its emission is that tint's share; turning the
	// term off is the frame of $selfillumtint 0.
	vec3 emission = vec3( 0.0 );
	if ( Term( kSelfIllum ) )
	{
		const vec3 selfIllum =
		    DebugTermOn( kDebugTermEmission ) && !furnace ? material.selfIllumTint.rgb : vec3( 0.0 );
		emission = selfIllum * albedo * base.a;
		lit = mix( lit, selfIllum * albedo, base.a );
	}

	vec3 imageSpecular = vec3( 0.0 );
	vec3 shadingNormal = vec3( 0.0 );
	const bool hasNormal = dot( worldNormal, worldNormal ) > 0.0;
	if ( hasNormal )
	{
		shadingNormal = normalize( bumpmap ? normalSample.x * tangentS + normalSample.y * tangentT +
		                                         normalSample.z * worldNormal
		                                   : worldNormal );
	}
	if ( Term( kCubemap ) && DebugTermOn( kDebugTermIbl ) &&
	     kDebugBrdf != kDebugBrdfDiffuseOnly )
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
		vec3 specular = furnace ? vec3( 1.0 )
		                        : frame.eye.w * texture( samplerCube( envmapTexture, envmapSampler ),
		                                            reflected )
		                                            .rgb;
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
		imageSpecular = specular * fresnel;
		lit += imageSpecular;
	}

	if ( DebugViewActive() )
	{
		DebugInputs inputs = DebugInputsNone();
		inputs.mask = kDebugHasAlbedo | kDebugHasAo | kDebugHasUv0 | kDebugHasVertexColor;
		if ( Term( kCubemap ) )
			inputs.mask |= kDebugHasImageSpecular;
		if ( Term( kSelfIllum ) )
			inputs.mask |= kDebugHasEmission;
		inputs.albedo = albedo * tint;
		if ( hasNormal )
		{
			inputs.mask |= kDebugHasNormal;
			inputs.normal = shadingNormal;
		}
		if ( bumpmap )
		{
			inputs.mask |= kDebugHasNormalMap;
			inputs.normalMap = normalSample.xyz;
		}
		if ( !lightingOne )
		{
			inputs.mask |= kDebugHasBaked;
			inputs.baked = baked;
		}
		inputs.imageSpecular = imageSpecular;
		inputs.emission = emission;
		inputs.uv0 = baseUv;
		inputs.vertexColor = color;
		inputs.final = lit;
		outColor = DebugOutput( inputs );
		return;
	}
	outColor = Output( lit, alpha );
}
