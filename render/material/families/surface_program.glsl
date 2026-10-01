// render.material program `surface` (RFC 0016 K11 "Model assembly"): the one
// surface program. Each lighting-model term is a specialization constant, so
// a neutral term costs nothing, and the legacy families are points of it:
// - the `lightmapped` point is LightmappedGeneric's arithmetic (the port's
//   lightmapped.frag, from lightmappedgeneric_ps2_3_x.h) in linear light: the
//   flat lightmap, or the three bumped pages weighted by the normal map (RNM)
//   or by an ssbump's basis weights; the env map with its mask (base alpha,
//   normal map alpha or $envmapmask), tint, contrast, saturation and fresnel;
//   detail (TextureCombine) and self-illumination;
// - the `pbr` point (kPbr) is the RFC 0007 layered metal/roughness BRDF
//   (render/shaders/common/pbr_brdf.glsl, the one GLSL copy of
//   public/render/pbr_brdf.h) under the draw's model lighting: the model
//   port's arithmetic (model_pbr.frag) without map probes, environment maps,
//   the probe volume or clear coat. Units follow Source's model lighting: a
//   local light is incident radiance pi * color * attenuation, the ambient
//   cube a Lambertian return, and the cube in the reflected direction the
//   specular image light. On a world surface (kBakedLightmap) the indirect
//   diffuse is the draw's lightmap basis (lightmap_basis.glsl) at the mapped
//   normal in place of the ambient cube: the flat page, or with
//   kDirectionalLightmap the page and its gradient page. The draw's page is
//   the LMAP layer the indirect policy picks (the indirect layer where the
//   core draws the direct light, so no light counts twice). With
//   kProbeVolume a surface without a lightmap takes its indirect diffuse
//   from the map's probe volume (probe_volume.glsl, with visibility), the
//   ambient cube where no grid covers the point; with kReflectionProbes the
//   specular image light is the map's reflection probes
//   (reflection_probes.glsl: blended, parallax-corrected, relit), the
//   ambient cube in the reflected direction where none carries light. Both
//   are weighted as before (the diffuse color; the split-sum directional
//   albedo).
// - the `unlit` point (kUnlit) is UnlitGeneric: the lightmapped point with
//   the lighting fixed at one and UnlitGeneric's vertex color and alpha;
// - the `vertexlit` point (kVertexLit) is VertexLitGeneric's lit path: the
//   model vertex stage evaluates Source's per-vertex lighting (DoLighting),
//   and the pixel is the base texture times $color times that lighting;
// - the `water` point (kWater, render/material/water_family.h) is Portal 2's
//   water_ps2x above water without refraction: flowing normal maps, the
//   flowing sludge layer, the lightmapped water fog color and a fresnel
//   reflection of the view's reflection target or of the env map.
// All end in the view's fog and the output encoding. Blending is pipeline
// state. The debug views and lighting-model controls (RFC 0014) come from
// debug_view.glsl; at their neutral values they are dead code. In the pbr
// point the local lights answer to the `clustered` term, the ambient cube to
// `probes` and the cube in the reflected direction to `ibl`.

#include "../../shaders/common/color_encoding.glsl"
#include "../../shaders/common/debug_view.glsl"
#include "../../shaders/common/pbr_brdf.glsl"
#include "../../shaders/common/ltc.glsl"
#include "../../shaders/common/runtime_light.glsl"
#include "../../shaders/common/shadow_sample.glsl"
#include "../../shaders/common/projected_light.glsl"
#include "../../shaders/common/lightmap_basis.glsl"
#include "surface_lighting.glsl"

// The terms (the port's static combo bits where they exist).
layout( constant_id = 0 ) const int kTerms = 0;
layout( constant_id = 1 ) const int kDetailMode = 0;
const int kUnlit = 1;
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
const int kSelfIllumMask = 536870912;
const int kPhongExponentTexture = 1073741824;
const int kTransmission = int( 0x80000000u );
// The vertexlit point (its vertex stage reads kHalfLambert, 16).
const int kVertexLit = 8192;
// The view's clustered runtime lights (the view group).
const int kClustered = 16384;
// The pbr point on a world surface: its indirect diffuse from the draw's
// lightmap page, and the page's gradient page when it is directional.
const int kBakedLightmap = 32768;
const int kDirectionalLightmap = 65536;
// The pbr point with the map's probe volume and reflection probes (frame
// group bindings 5 to 10).
const int kProbeVolume = 131072;
const int kReflectionProbes = 262144;
// The view's ambient occlusion (render.pass.ao), the SSR targets, the
// depth-and-normal prepass and the projected lights' bounce
// (surface_program.h).
const int kAmbientOcclusion = 524288;
const int kSsrTargets = 1048576;
const int kDepthNormal = 2097152;
const int kProbeBounce = 4194304;
const int kMeshDirect = 8388608;
const int kMraoTexture = 268435456;
// A reflective shadow map's albedo (render.pass.bounce): attachment 0 holds
// the surface's diffuse reflectance only.
const int kRsm = 16777216;
// The water point (the flow map and noise at material bindings 5 and 11,
// the view's planar reflection at view binding 12).
const int kWater = 33554432;
// Runtime direct light on a world surface (surface_program.h): the basis
// reads the indirect layer, and every light's direct light is drawn here.
const int kRuntimeDirect = 134217728;

// An area light (render.area-light.v1, area_light::AreaLight): its
// rectangle, its radiance and its reach.
struct AreaLight
{
	vec4 center; // w: 1 when two-sided
	vec4 halfU;  // w: the reach
	vec4 halfV;
	vec4 radiance;
};
const int kMaxAreaLights = 64;

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
	vec4 areaCount; // x: the frame's area lights (the pbr point reads them)
	// The sun: towards it (w the tangent of its disc's angular radius), its
	// diffuse light (w 1 when that is in the baked light) and its shadow (x
	// the first cascade tile or -1, y the cascades, z 1 to read the baked
	// mask in the lightmap page's alpha).
	vec4 sunDirection;
	vec4 sunColor;
	vec4 sunShadow;
	// The water point: x the shaders' time in seconds, y its reflection
	// tint's scale (4 in integer HDR), zw the camera's right in the water
	// plane.
	vec4 water;
	vec4 viewport; // the view's x, y, 1 / width, 1 / height in the target
	AreaLight areas[kMaxAreaLights];
} frame;
// The split-sum table (RFC 0007, pbr_split_sum_table.h), read by the pbr point.
layout( set = 0, binding = 1 ) uniform texture2D splitSumTexture;
layout( set = 0, binding = 2 ) uniform sampler splitSumSampler;
// The view group: the view's clustered runtime lights (render.pass.lights
// lists render.light-set.v1's point and spot lights per froxel). Read only
// with kClustered.
struct RuntimeLightRecord
{
	vec4 position;  // w: radius (0 unbounded)
	vec4 color;     // w: minLight
	vec4 direction; // w: outerCos, below -1 for a point light
	vec4 cone;      // innerCos, 1 for an inverse-square falloff, sourceRadius,
	                // the shadow tile or -1
	vec4 spot;      // the cone ramp's exponent, baked diffuse, shadow tiles
	vec4 attenuation; // vrad's c, l, q (cone.y 2)
};
layout( set = 1, binding = 0 ) uniform ClusterView
{
	uvec4 grid;         // tilesX, tilesY, slices, tile size in pixels
	vec4 slices;        // sliceScale, sliceBias, nearZ
	vec4 viewDistance;  // a world point's view distance: dot( xyz, p ) + w
	vec4 counts;        // x: the projected lights (binding 7)
} clusterView;
layout( set = 1, binding = 1, std430 ) readonly buffer ClusterFroxels
{
	uvec2 froxelRanges[]; // offset, count into the index list
};
layout( set = 1, binding = 2, std430 ) readonly buffer ClusterIndices
{
	uvec4 clusterHeader;
	uint clusterIndices[];
};
layout( set = 1, binding = 3, std430 ) readonly buffer ClusterLights
{
	RuntimeLightRecord runtimeLights[];
};
// The view's shadow tiles (ShadowTileGpu) and atlas: a light whose cone.w is
// a tile index (not -1) is shadowed by that tile (render.shadows.v1).
layout( set = 1, binding = 4, std430, row_major ) readonly buffer ShadowTiles
{
	ShadowTile shadowTiles[];
};
// The view's projected lights (render.projected-light.v1), their cookies,
// and the view's ambient occlusion (fetched per pixel).
layout( set = 1, binding = 5, std430 ) readonly buffer ProjectedLights
{
	ProjectedLight projectors[];
};
layout( set = 1, binding = 6 ) uniform texture2D shadowAtlas;
layout( set = 1, binding = 7 ) uniform sampler shadowSampler;
#include "../../shaders/common/shadow_faces.glsl"
layout( set = 1, binding = 8 ) uniform texture2DArray cookieTexture;
layout( set = 1, binding = 9 ) uniform sampler cookieSampler;
layout( set = 1, binding = 10 ) uniform texture2D occlusionTexture;
layout( set = 1, binding = 11 ) uniform sampler occlusionSampler;
// The view's planar reflection (the water point's reflection target).
layout( set = 1, binding = 12 ) uniform texture2D reflectionTexture;
layout( set = 1, binding = 13 ) uniform sampler reflectionSampler;
// Linear scene color behind thin transmission, before output transforms.
layout( set = 1, binding = 14 ) uniform texture2D sceneColorTexture;
layout( set = 1, binding = 15 ) uniform sampler sceneColorSampler;

// The GGX LTC table (public/render/pbr_ltc_table.h), read by the pbr point.
layout( set = 0, binding = 3 ) uniform texture2D ltcTexture;
layout( set = 0, binding = 4 ) uniform sampler ltcSampler;
// The map's probe volume (PRBV: its atlas and grid table) and reflection
// probes (RPRB, WriteReflectionProbeTexture's form), read under
// kProbeVolume and kReflectionProbes.
layout( set = 0, binding = 5 ) uniform texture2D probeAtlas;
layout( set = 0, binding = 6 ) uniform sampler probeAtlasSampler;
layout( set = 0, binding = 7 ) uniform texture2D probeGrids;
layout( set = 0, binding = 8 ) uniform sampler probeGridsSampler;
layout( set = 0, binding = 9 ) uniform texture2D reflectionProbes;
layout( set = 0, binding = 10 ) uniform sampler reflectionProbesSampler;
// The projected lights' bounce: an atlas of the probe atlas's layout, with
// its sampler (linear, clamped, as the probe atlas's).
layout( set = 0, binding = 11 ) uniform texture2D probeSecondAtlas;
layout( set = 0, binding = 12 ) uniform sampler probeSecondSampler;
#define PROBE_VOLUME_SECOND
#define PROBE_VOLUME_SECOND_SAMPLER probeSecondSampler
#include "../../shaders/common/probe_volume.glsl"

vec4 ReflectionProbesFetch( ivec2 texel )
{
	return texelFetch( sampler2D( reflectionProbes, reflectionProbesSampler ), texel, 0 );
}

vec4 ReflectionProbesSample( vec2 texel )
{
	return textureLod( sampler2D( reflectionProbes, reflectionProbesSampler ),
	    texel / vec2( textureSize( sampler2D( reflectionProbes, reflectionProbesSampler ), 0 ) ),
	    0.0 );
}
#include "../../shaders/common/reflection_probes.glsl"
layout( set = 2, binding = 0 ) uniform Material
{
	vec4 tint;  // rgb: $color, a: $alpha
	vec4 flags; // x: $vertexcolor, y: $alphatest, z: $alphatestreference
	// x: 1 when fully opaque (height fog's factor is the output alpha), y:
	// gamma vertex color (vertex stage), z: the ssbump weights' scale
	// (0.57735 with $ssbumpmathfix, else 1), w: $vertexalpha (the unlit point)
	vec4 state;
	vec4 envTint;       // rgb: $envmaptint, a: $fresnelreflection
	vec4 envContrast;   // rgb: the contrast in effect, a: 1 - $fresnelreflection
	vec4 envSaturation; // legacy saturation; PBR rgb: probe tint, a: inverted Phong mask
	vec4 selfIllumTint; // rgb: $selfillumtint
	vec4 detailTint;    // rgb: $detailtint, a: $detailblendfactor
	vec4 detailScale;   // xy: $detailscale
	vec4 envLightScale; // x: min, y: min + max, z: $envmaplightscale (Portal 2)
	vec4 emission;      // x: $emissionscale, y: $emissiononesided (the pbr point)
	// The water point (SurfaceConstants::water*, water_family.h).
	vec4 waterFlow;      // 1/$flow_worlduvscale, 1/$flow_normaluvscale, bump strength, displacement
	vec4 waterFlowTime;  // interval, scroll distance, noise scale, 1 with a flow map
	vec4 waterColorFlow; // 1/$color_flow_uvscale, interval, scroll distance, lerp exponent
	vec4 waterReflect;   // rgb: $reflecttint (linear); a: $waterblendfactor
	vec4 waterFog;       // rgb: $fogcolor (linear); a: $reflectamount
	vec4 waterMode;      // x: reflection target, y: sludge, z: $lightmapwaterfog, w: $forcefresnel
	vec4 pbrFactors;     // rgb: metalness, roughness, AO without MRAO texture
	vec4 meshControls;   // VertexLit mesh: base-alpha Phong mask, albedo tint, rim boost/exponent
	vec4 meshProbeMasks; // VertexLit mesh: base-alpha and normal-alpha probe masks
	vec4 meshProbeFresnel; // VertexLit mesh: env Fresnel weight and min/max/exp
	vec4 meshModes; // light warp, authored env map, Phong enable
	vec4 meshProbeColor; // contrast, saturation, Phong warp, RGB probe mask
	vec4 transmission; // thin: fraction/IOR; Refract: amount/blur/mode/fade
	vec4 emissionCone; // inner/outer cosine, exponent, enabled
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
// The water point reads its flow map through the env map mask's binding and
// its flow noise through MRAO's (a group has 16 GL slots; it reads neither).
#define flowTexture envmapMaskTexture
#define flowSampler envmapMaskSampler
#define flowNoiseTexture mraoTexture
#define flowNoiseSampler mraoSampler
// The lightmap page is the draw's: surfaces of one material share pages
// with others. So is the model lighting (surface_lighting.glsl, binding 2).
layout( set = 3, binding = 0 ) uniform texture2D lightmap;
layout( set = 3, binding = 1 ) uniform sampler lightmapSampler;
// A directional page's gradient page (kDirectionalLightmap).
layout( set = 3, binding = 3 ) uniform texture2D lightmapGradient;
layout( set = 3, binding = 4 ) uniform sampler lightmapGradientSampler;
// The bake's indirect layer of the same page (kAmbientOcclusion).
layout( set = 3, binding = 5 ) uniform texture2D lightmapIndirect;
layout( set = 3, binding = 6 ) uniform sampler lightmapIndirectSampler;

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
layout( location = 10 ) in vec3 vertexLighting; // the vertexlit point's DoLighting
layout( location = 0 ) out vec4 outColor;
#ifdef SURFACE_SSR_TARGETS
// render.pass.ssr's inputs (ssr.h): the octahedral normal and roughness, the
// image-specular radiance and its weight.
layout( location = 1 ) out vec4 outNormalRoughness;
layout( location = 2 ) out vec4 outIblRadiance;
layout( location = 3 ) out vec4 outSpecularWeight;
#endif

// ssr.h's Octahedral (OctEncode).
vec2 SurfaceOctEncode( vec3 n )
{
	n /= abs( n.x ) + abs( n.y ) + abs( n.z );
	vec2 p = n.xy;
	if ( n.z < 0.0 )
		p = ( vec2( 1.0 ) - abs( p.yx ) ) * vec2( p.x >= 0.0 ? 1.0 : -1.0, p.y >= 0.0 ? 1.0 : -1.0 );
	return p;
}

void WriteSsrTargets( vec3 normal, float roughness, vec3 iblRadiance, vec3 weight,
    bool cameraOnlyEmitter )
{
#ifdef SURFACE_SSR_TARGETS
	outNormalRoughness = vec4( SurfaceOctEncode( normal ), roughness,
	    cameraOnlyEmitter ? 2.0 : 1.0 );
	outIblRadiance = vec4( iblRadiance, 1.0 );
	outSpecularWeight = vec4( weight, 1.0 );
#endif
}

// Interleaved gradient noise (Jimenez 2014): a per-pixel angle that turns
// the soft shadows' discs.
float PixelRotation()
{
	return 6.2831853 *
	       fract( 52.9829189 * fract( dot( gl_FragCoord.xy, vec2( 0.06711056, 0.00583715 ) ) ) );
}

// Multi-bounce ambient occlusion (Jimenez et al. 2016, "Practical Real-Time
// Strategies for Accurate Indirect Occlusion", the cubic fit): the
// visibility with the light the occluders' own albedo returns.
vec3 MultiBounceOcclusion( float visibility, vec3 albedo )
{
	const vec3 a = 2.0404 * albedo - 0.3324;
	const vec3 b = -4.7951 * albedo + 0.6417;
	const vec3 c = 2.7552 * albedo + 0.6903;
	return max( vec3( visibility ), ( ( visibility * a + b ) * visibility + c ) * visibility );
}

// Specular occlusion from the ambient occlusion (Lagarde and de Rousiers
// 2014, "Moving Frostbite to PBR").
float SpecularOcclusion( float normalDotView, float visibility, float roughness )
{
	return clamp( pow( normalDotView + visibility, exp2( -16.0 * roughness - 1.0 ) ) - 1.0 +
	                  visibility,
	    0.0, 1.0 );
}

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

vec3 MeshDiffuseFactor( float normalDotLight )
{
	const float halfLambert = clamp( 0.5 * normalDotLight + 0.5, 0.0, 1.0 );
	const float scalar = material.flags.w > 0.5 ? halfLambert : max( normalDotLight, 0.0 );
	if ( material.meshModes.x > 0.5 )
		return 2.0 * texture( sampler2D( mraoTexture, mraoSampler ),
		           vec2( scalar, 0.5 ) )
		                 .rgb;
	return vec3( material.flags.w > 0.5 ? scalar * scalar : scalar );
}

// VertexLitGeneric's specular warp is a 2D data lookup: horizontal is the
// authored Phong highlight, vertical is its three-range Fresnel response.
// The result colors the PBR direct specular lobe, leaving native IBL alone.
vec3 MeshSpecularWarp( vec3 normal, vec3 view, vec3 light, float exponent )
{
	if ( material.meshProbeColor.z <= 0.5 )
		return vec3( 1.0 );
	const vec3 ranges = material.envContrast.rgb;
	float fresnel = clamp( 1.0 - dot( normal, view ), 0.0, 1.0 );
	fresnel = fresnel * fresnel - 0.5;
	fresnel = ranges.y +
	          ( fresnel >= 0.0 ? 2.0 * ( ranges.z - ranges.y )
	                           : 2.0 * ( ranges.y - ranges.x ) ) *
	              fresnel;
	const float highlight =
	    pow( max( dot( reflect( -view, normal ), light ), 0.0 ), max( exponent, 0.0 ) );
	return texture( sampler2D( mraoTexture, mraoSampler ),
	    vec2( highlight, clamp( fresnel, 0.0, 1.0 ) ) )
	    .rgb;
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

// render.pass.lights FroxelAt: the froxel of a pixel position (x right, y
// down) and a view distance, clamped to the grid.
uint ClusterAxis( float pixel, uint count )
{
	const float t = floor( pixel / float( clusterView.grid.w ) );
	if ( !( t > 0.0 ) )
		return 0u;
	return t < float( count - 1u ) ? uint( t ) : count - 1u;
}

uint ClusterFroxel( vec2 pixel, float distance )
{
	const uint slices = clusterView.grid.z;
	uint slice = 0u;
	if ( distance > clusterView.slices.z )
	{
		const float s = floor( log( distance ) * clusterView.slices.x + clusterView.slices.y );
#ifdef SEEDED_CLUSTER_SLICE_OFF_BY_ONE
		slice = s + 1.0 < float( slices - 1u ) ? ( s + 1.0 > 0.0 ? uint( s + 1.0 ) : 0u ) : slices - 1u;
#else
		slice = s < float( slices - 1u ) ? ( s > 0.0 ? uint( s ) : 0u ) : slices - 1u;
#endif
	}
	const uint x = ClusterAxis( pixel.x, clusterView.grid.x );
	const uint y = ClusterAxis( pixel.y, clusterView.grid.y );
	return ( slice * clusterView.grid.y + y ) * clusterView.grid.x + x;
}

// An area light's corners, counterclockwise seen from its front.
void AreaLightCorners( AreaLight light, out vec3 corners[4] )
{
	corners[0] = light.center.xyz - light.halfU.xyz - light.halfV.xyz;
	corners[1] = light.center.xyz + light.halfU.xyz - light.halfV.xyz;
	corners[2] = light.center.xyz + light.halfU.xyz + light.halfV.xyz;
	corners[3] = light.center.xyz - light.halfU.xyz + light.halfV.xyz;
}

// The irradiance the frame's area lights whose diffuse light is not in the
// bake (the emitting surfaces) give a point with normal n, in the lightmap's
// units: each light's radiance times its form factor (the cosine's integral
// over the rectangle, clipped to the horizon) and its window. Unshadowed.
vec3 AreaLightIrradiance( vec3 n, vec3 p )
{
	vec3 sum = vec3( 0.0 );
	const int areaCount = DebugTermOn( kDebugTermArea ) ? min( int( frame.areaCount.x ), kMaxAreaLights ) : 0;
	for ( int i = 0; i < kMaxAreaLights; ++i )
	{
		if ( i >= areaCount )
			break;
		const AreaLight light = frame.areas[i];
		if ( light.halfV.w >= 0.5 )
			continue;
		const float window = AreaLightWindow(
		    light.center.xyz, light.halfU.xyz, light.halfV.xyz, light.halfU.w, p );
		if ( window <= 0.0 )
			continue;
		vec3 corners[4];
		AreaLightCorners( light, corners );
		sum += light.radiance.rgb * window *
		       LtcRectangle( n, normalize( frame.eye.xyz - p ), p, mat3( 1.0 ), corners,
		           light.center.w > 0.5 );
	}
	return sum;
}

// The ambient cube (surface_lighting.glsl). In the furnace (RFC 0014) every
// face is a uniform radiance of 1.
vec3 AmbientCube( vec3 n )
{
	if ( DebugFurnace() )
	{
		const vec3 squared = n * n;
		return vec3( squared.x + squared.y + squared.z );
	}
	return ModelAmbientCube( n );
}

// Refract_DX90's model point. The color input is a linear snapshot behind the
// surface; its authored normal alpha controls both the warp and reflection.
vec3 RefractSceneColor( vec2 uv )
{
	if ( material.meshModes.x > 0.5 )
		return texture( sampler2D( baseTexture, baseSampler ), uv ).rgb;
	return texture( sampler2D( sceneColorTexture, sceneColorSampler ), uv ).rgb;
}

void RefractSurface()
{
	const vec4 bump = texture( sampler2D( bumpTexture, bumpSampler ), baseUv );
	const vec3 mapped = bump.rgb * 2.0 - 1.0;
	const vec2 unwarped = ( gl_FragCoord.xy - frame.viewport.xy ) * frame.viewport.zw;
	const vec2 warped = unwarped + mapped.xy * bump.a * material.transmission.x;
	vec3 behind = RefractSceneColor( warped );
	if ( material.transmission.y > 0.5 )
	{
		const vec2 halfBlur = vec2( 0.5 / 512.0 );
		const vec2 fullBlur = vec2( 1.0 / 512.0 );
		behind = RefractSceneColor( warped - halfBlur ) * 0.4444444;
		behind += RefractSceneColor( warped + vec2( fullBlur.x, -halfBlur.y ) ) * 0.2222222;
		behind += RefractSceneColor( warped + vec2( -halfBlur.x, fullBlur.y ) ) * 0.2222222;
		behind += RefractSceneColor( warped + fullBlur ) * 0.1111111;
	}
	const vec3 eye = normalize( frame.eye.xyz - worldPosition );
	const vec3 smoothNormal = normalize( worldNormal );
	float fade = 1.0;
	if ( material.transmission.w > 0.5 )
		fade = pow( clamp( dot( eye, smoothNormal ), 0.0, 1.0 ), 3.0 );
	vec3 result = mix( RefractSceneColor( unwarped ),
	    behind * material.tint.rgb, fade );
	if ( material.meshModes.y > 0.5 && DebugTermOn( kDebugTermIbl ) )
	{
		const vec3 normal = normalize( normalize( tangentS ) * mapped.x +
		    normalize( tangentT ) * mapped.y + smoothNormal * mapped.z );
		const vec3 reflected = reflect( -eye, normal );
		vec3 radiance = vec3( 0.0 );
		if ( Term( kReflectionProbes ) )
		{
			if ( !ReflectionProbesRadiance(
			         worldPosition, smoothNormal, reflected, 0.02, radiance ) )
				radiance = AmbientCube( reflected );
		}
		else
			radiance = texture( samplerCube( envmapTexture, envmapSampler ), reflected ).rgb;
		vec3 specular = radiance * bump.a * material.envTint.rgb;
		specular = mix( specular, specular * specular, material.meshProbeColor.x );
		const vec3 grey = vec3( dot( specular, vec3( 0.299, 0.587, 0.114 ) ) );
		result += mix( grey, specular, material.envSaturation.rgb );
	}
	if ( DebugViewActive() )
	{
		DebugInputs inputs = DebugInputsNone();
		inputs.mask = kDebugHasAlbedo | kDebugHasUv0;
		inputs.albedo = material.tint.rgb;
		inputs.uv0 = baseUv;
		inputs.final = result;
		outColor = DebugOutput( inputs );
		return;
	}
	outColor = Output( result, material.meshModes.y > 0.5 ? 1.0 : bump.a );
}

// The pbr point. Base and emission are sampled as sRGB, MRAO and the normal
// map as linear data.
void PbrSurface()
{
	const bool furnace = DebugFurnace();
	const bool normalMap = Term( kBumpmap | kSsbump );
	const bool ssbump = Term( kSsbump );
	const bool emissive = Term( kEmissionTexture );
	const vec2 uv = baseUv;
	vec4 baseSample = texture( sampler2D( baseTexture, baseSampler ), uv );
	if ( Term( kDetailTexture ) )
	{
		const vec4 detail = vec4( material.detailTint.rgb, 1.0 ) *
		                    texture( sampler2D( detailTexture, detailSampler ),
		                        uv * material.detailScale.xy );
		baseSample = TextureCombine( baseSample, detail, material.detailTint.a );
	}
	if ( material.flags.y != 0.0 && baseSample.a * material.tint.a < material.flags.z )
		discard;
	vec3 base = furnace ? vec3( 1.0 ) : baseSample.rgb * material.tint.rgb;
	if ( material.meshProbeMasks.z > 0.5 && !furnace )
	{
		const vec3 tinted = mix( base, material.tint.rgb, material.meshProbeMasks.w );
		base = mix( baseSample.rgb, tinted, baseSample.a );
	}
	const bool unlitMesh = material.meshModes.w > 0.5;
	const vec3 mrao = Term( kMraoTexture )
	                      ? texture( sampler2D( mraoTexture, mraoSampler ), uv ).rgb
	                      : material.pbrFactors.rgb;
	const float metalness =
	    kDebugForceMetalness >= 0.0 ? kDebugForceMetalness : clamp( mrao.r, 0.0, 1.0 );
	const vec4 exponentSample = Term( kPhongExponentTexture )
	                                ? texture( sampler2D( envmapMaskTexture, envmapMaskSampler ), uv )
	                                : vec4( 1.0 );
	const float exponent = material.pbrFactors.w > 0.0
	                           ? material.pbrFactors.w
	                           : 1.0 + 149.0 * exponentSample.r;
	const float meshRoughness = Term( kPhongExponentTexture ) && material.pbrFactors.w <= 0.0
	                                ? sqrt( 2.0 / ( exponent + 2.0 ) )
	                                : mrao.g;
	const float roughness =
	    max( kDebugForceRoughness >= 0.0 ? kDebugForceRoughness : meshRoughness, 0.02 );
	const float occlusion = DebugTermOn( kDebugTermAo ) ? clamp( mrao.b, 0.0, 1.0 ) : 1.0;
	if ( Term( kRsm ) )
	{
		outColor = vec4( base * ( 1.0 - metalness ), 1.0 );
		return;
	}
	// The view's occlusion of the indirect light (render.pass.ao).
	const float screenOcclusion =
	    Term( kAmbientOcclusion ) && DebugTermOn( kDebugTermAo )
	        ? clamp( texelFetch( sampler2D( occlusionTexture, occlusionSampler ),
	                     clamp( ivec2( gl_FragCoord.xy ), ivec2( 0 ),
	                         textureSize( sampler2D( occlusionTexture, occlusionSampler ), 0 ) - 1 ),
	                     0 )
	                     .r,
	              0.0, 1.0 )
	        : 1.0;

	const vec3 view = normalize( frame.eye.xyz - worldPosition );
	vec3 normal = dot( worldNormal, worldNormal ) > 1e-12 ? normalize( worldNormal ) : view;
	const vec3 smoothNormal = normal;
	vec3 mapped = vec3( 0.0, 0.0, 1.0 );
	float directSpecularMask =
	    material.meshModes.z > 0.5 && material.meshControls.x > 0.5 ? baseSample.a : 1.0;
	vec3 probeSpecularMask = vec3(
	    material.meshModes.y > 0.5 || material.meshProbeMasks.x > 0.5 ? baseSample.a : 1.0 );
	if ( material.meshProbeColor.w > 0.5 )
		probeSpecularMask *=
		    texture( sampler2D( envmapMaskTexture, envmapMaskSampler ), uv ).rgb;
	if ( normalMap )
	{
		const vec4 normalSample = texture( sampler2D( bumpTexture, bumpSampler ), uv );
		if ( ssbump )
			mapped = normalize( bumpBasis[0] * normalSample.r +
			                    bumpBasis[1] * normalSample.g + bumpBasis[2] * normalSample.b );
		else
		{
			const vec2 xy = normalSample.rg * 2.0 - 1.0;
			mapped = vec3( xy, sqrt( max( 0.0, 1.0 - dot( xy, xy ) ) ) );
		}
		normal = normalize( normalize( tangentS ) * mapped.x + normalize( tangentT ) * mapped.y +
		                    normal * mapped.z );
		if ( material.meshModes.z > 0.5 && material.meshControls.x <= 0.5 )
			directSpecularMask = normalSample.a;
		if ( material.meshProbeMasks.y > 0.5 )
			probeSpecularMask = vec3( normalSample.a );
	}
	if ( material.meshModes.z > 0.5 && material.envSaturation.a > 0.5 )
		directSpecularMask = 1.0 - directSpecularMask;
	if ( material.meshModes.y > 0.5 && material.envSaturation.a > 0.5 )
		probeSpecularMask = 1.0 - probeSpecularMask;
	const float normalDotView = max( dot( normal, view ), 0.0 );
	if ( Term( kDepthNormal ) )
	{
		// render.pass.ao's radius: two lightmap texels' world size on a
		// lightmapped surface (the bake holds coarser occlusion), else 0
		// (the pass's radius).
		float radius = 0.0;
		if ( Term( kBakedLightmap ) )
		{
			const vec2 size = vec2( textureSize( sampler2D( lightmap, lightmapSampler ), 0 ) );
			const float texelsX = length( dFdx( lightmapUv ) * size );
			const float texelsY = length( dFdy( lightmapUv ) * size );
			const float worldX = length( dFdx( worldPosition ) );
			const float worldY = length( dFdy( worldPosition ) );
			radius = worldX / max( texelsX, 1e-6 ) + worldY / max( texelsY, 1e-6 );
		}
		outColor = vec4( SurfaceOctEncode( normal ), roughness, radius );
		return;
	}
	// VertexLitGeneric's authored Fresnel triplet offsets the physical
	// dielectric baseline at front, middle and grazing angles. Native PBR
	// materials leave the control disabled.
	const vec3 rangeOffset = material.envContrast.rgb - vec3( 0.0, 0.5, 1.0 );
	const float angle = clamp( ( 1.0 - normalDotView ) * 2.0, 0.0, 2.0 );
	const float authoredOffset = angle < 1.0
	                                 ? mix( rangeOffset.x, rangeOffset.y, angle )
	                                 : mix( rangeOffset.y, rangeOffset.z, angle - 1.0 );
	const float rangeGain = material.envContrast.a > 0.5
	                            ? clamp( 1.0 + authoredOffset, 0.0, 25.0 )
	                            : 1.0;
	const float ior = max( material.transmission.y, 1.0 );
	const float dielectricF0 = Term( kTransmission )
	                               ? pow( ( ior - 1.0 ) / ( ior + 1.0 ), 2.0 )
	                               : 0.04;
	vec3 f0 = mix( clamp( vec3( dielectricF0 * rangeGain ) * material.envTint.rgb, 0.0,
	                   0.9 ),
	    base, metalness );
	if ( material.meshControls.y > 0.5 )
		f0 *= mix( vec3( 1.0 ), base,
		    Term( kPhongExponentTexture ) ? exponentSample.g : 1.0 );
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
	const vec3 baseDiffuse = base * ( 1.0 - metalness ) * ( vec3( 1.0 ) - directionalAlbedo );
	const float transmitted =
	    Term( kTransmission ) ? clamp( material.transmission.x, 0.0, 1.0 ) : 0.0;
	const vec3 diffuseColor = baseDiffuse * ( 1.0 - transmitted );
	// cl_render_debug_brdf 1 and 2: one lobe.
	const bool diffuseLobe = kDebugBrdf != kDebugBrdfSpecularOnly && !unlitMesh;
	const bool specularLobe = kDebugBrdf != kDebugBrdfDiffuseOnly &&
	                          ( !unlitMesh || material.meshModes.y > 0.5 );

	// Indirect diffuse: the lightmap basis on a world surface (the `baked`
	// term), else the ambient cube (`probes`).
	const bool lightmapped = Term( kBakedLightmap );
	// kRuntimeDirect: the bake's indirect layer is the basis (the gradient
	// page is its own), and the loops below draw every light's diffuse lobe.
	const bool runtimeDirect = lightmapped && Term( kRuntimeDirect );
	vec3 baked = vec3( 0.0 );
	if ( lightmapped )
	{
		baked = runtimeDirect
		            ? LightmapPageSample( lightmapIndirect, lightmapIndirectSampler, lightmapUv )
		            : LightmapPageSample( lightmap, lightmapSampler, lightmapUv );
		if ( Term( kDirectionalLightmap ) )
			baked = LightmapDirectional( baked,
			    LightmapPageSample( lightmapGradient, lightmapGradientSampler, lightmapUv ), normal,
			    smoothNormal );
		baked = furnace ? vec3( 1.0 ) : baked * frame.light.x;
	}
	// The indirect light's occlusion: the material's, times the view's with
	// its interreflection (the occluders' albedo taken as the surface's).
	// kMeshDirect: a surface without a lightmap takes the baked lights'
	// direct light at runtime (shadowed, both lobes) over the probe volume's
	// indirect layer, when the volume carries one (RFC 0011's layers: total,
	// indirect).
	const bool meshDirect = Term( kMeshDirect ) && !lightmapped && Term( kProbeVolume ) &&
	                        ProbeGridRow( 5, 0 ).x >= 2.0 && !furnace;
	const vec3 indirectOcclusion =
	    occlusion * MultiBounceOcclusion( screenOcclusion, diffuseColor + directionalAlbedo );
	vec3 color = vec3( 0.0 );
	vec3 diffuseIrradiance = vec3( 1.0 );
	if ( diffuseLobe && lightmapped && DebugTermOn( kDebugTermBaked ) )
	{
		// The view's occlusion darkens the bake's indirect layer only: the
		// page less the part of its indirect light the occlusion removes,
		// carried to the mapped normal as the page is.
		vec3 light = baked;
		if ( Term( kAmbientOcclusion ) && !furnace )
		{
			vec3 indirect =
			    LightmapPageSample( lightmapIndirect, lightmapIndirectSampler, lightmapUv );
			if ( Term( kDirectionalLightmap ) )
				indirect = LightmapDirectional( indirect,
				    LightmapPageSample( lightmapGradient, lightmapGradientSampler, lightmapUv ),
				    normal, smoothNormal );
			light = max( baked - indirect * frame.light.x *
			                         ( vec3( 1.0 ) - MultiBounceOcclusion( screenOcclusion,
			                                             diffuseColor + directionalAlbedo ) ),
			    vec3( 0.0 ) );
		}
		color = diffuseColor * light * occlusion;
		diffuseIrradiance = light;
	}
	else if ( diffuseLobe && !lightmapped && DebugTermOn( kDebugTermProbes ) )
	{
		// A surface the probe volume lights reads its indirect layer when it
		// has one, and then takes every light's direct light itself
		// (meshDirect below); else the total layer holds the baked lights.
		vec3 irradiance;
		if ( furnace || !Term( kProbeVolume ) ||
		     !ProbeIrradiance( worldPosition, normal, meshDirect ? 1 : 0, true, irradiance ) )
			irradiance = AmbientCube( normal );
		color = diffuseColor * irradiance * indirectOcclusion;
		diffuseIrradiance = irradiance;
	}
	// The projected lights' bounce, on every surface the volume covers.
	if ( diffuseLobe && Term( kProbeBounce ) && Term( kProbeVolume ) && !furnace &&
	     DebugTermOn( kDebugTermProjected ) )
	{
		vec3 unused;
		vec3 bounce;
		if ( ProbeIrradiancePair( worldPosition, normal, 0, true, unused, bounce ) )
			color += diffuseColor * bounce * indirectOcclusion;
	}
	const float rotation = PixelRotation();
	// The facet's normal (the shadows' receiver offset), facing the view as
	// the interpolated normal does.
	vec3 geometricNormal = normalize( cross( dFdx( worldPosition ), dFdy( worldPosition ) ) );
	if ( dot( geometricNormal, smoothNormal ) < 0.0 )
		geometricNormal = -geometricNormal;
	vec3 direct = vec3( 0.0 );
	const int count = DebugTermOn( kDebugTermClustered ) && !furnace && !Term( kMeshDirect )
	                      ? int( lighting.eye.w )
	                      : 0;
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
			const vec3 diffuse = diffuseColor * incident * MeshDiffuseFactor( normalDotLight );
			color += diffuse;
			direct += diffuse;
		}
		if ( specularLobe && !unlitMesh )
		{
			const vec3 specular = kPi * incident * PbrSpecular( normal, view, light, f0, roughness ) *
			                      compensation * normalDotLight * directSpecularMask *
			                      MeshSpecularWarp( normal, view, light, exponent );
			color += specular;
			direct += specular;
		}
	}
	// The view's clustered runtime lights (render.light-set.v1): each light
	// of the fragment's froxel, its falloff and cone (runtime_light.glsl),
	// both lobes. A light's color is its diffuse light on a surface facing it
	// (the lightmap unit), so its irradiance is pi times that.
	if ( Term( kClustered ) && DebugTermOn( kDebugTermClustered ) && !furnace )
	{
		const float distance =
		    dot( clusterView.viewDistance.xyz, worldPosition ) + clusterView.viewDistance.w;
		const uvec2 range = froxelRanges[ClusterFroxel( gl_FragCoord.xy, distance )];
		for ( uint k = 0u; k < range.y; ++k )
		{
#ifdef SEEDED_CLUSTER_SKIPS_FIRST
			if ( k == 0u )
				continue;
#endif
			const RuntimeLightRecord runtime = runtimeLights[clusterIndices[range.x + k]];
			const vec3 toLight = runtime.position.xyz - worldPosition;
			const float distanceSquared = dot( toLight, toLight );
#ifdef SEEDED_RUNTIME_FALLOFF_UNWINDOWED
			float falloff = runtime.cone.y > 0.5
			                    ? 1e4 / max( distanceSquared, runtime.cone.z * runtime.cone.z )
			                    : RuntimeLightFalloffLegacy(
			                          distanceSquared, runtime.position.w, runtime.color.w );
#else
			float falloff =
			    runtime.cone.y > 1.5
			        ? RuntimeLightFalloffAttenuated(
			              distanceSquared, runtime.position.w, runtime.attenuation.xyz )
			    : runtime.cone.y > 0.5
			        ? RuntimeLightFalloffInverseSquare(
			              distanceSquared, runtime.position.w, runtime.cone.z )
			        : RuntimeLightFalloffLegacy( distanceSquared, runtime.position.w, runtime.color.w );
#endif
			if ( falloff <= 0.0 )
				continue;
			const vec3 light = toLight * inversesqrt( max( distanceSquared, 1e-8 ) );
			if ( runtime.direction.w >= -1.0 )
				falloff *= RuntimeLightSpot( dot( -light, normalize( runtime.direction.xyz ) ),
				    runtime.cone.x, runtime.direction.w, runtime.spot.x );
			const float normalDotLight = max( dot( normal, light ), 0.0 );
			if ( falloff <= 0.0 || normalDotLight <= 0.0 )
				continue;
			const int tile = int( runtime.cone.w );
			const int tiles = int( runtime.spot.z );
#ifdef SEEDED_SHADOW_TILE_NEXT
			if ( tile >= 0 )
				falloff *= ShadowVisibility( shadowAtlas, shadowSampler,
				    shadowTiles[( tile + 1 ) % shadowTiles.length()], worldPosition );
#elif !defined( SEEDED_SHADOW_IGNORED )
			if ( tile >= 0 && tiles > 1 )
				falloff *= ShadowTerminatorFade( smoothNormal, light,
				    ShadowFacesVisibility( shadowAtlas, shadowSampler, tile, tiles, worldPosition,
				        ShadowReceiverOffset( geometricNormal, light ), max( runtime.cone.z, 0.5 ),
				        rotation ) );
			else if ( tile >= 0 )
				falloff *= ShadowTerminatorFade( smoothNormal, light,
				    ShadowVisibilitySoft( shadowAtlas, shadowSampler, shadowTiles[tile],
				        worldPosition, ShadowReceiverOffset( geometricNormal, light ),
				        max( runtime.cone.z, 0.5 ), rotation ) );
#endif
			const vec3 incident = runtime.color.rgb * falloff;
			// A baked light's diffuse light is already in the bake, unless
			// the bake is the indirect layer (runtime direct light).
			if ( diffuseLobe && ( runtime.spot.y < 0.5 || meshDirect || runtimeDirect ) )
			{
				const vec3 diffuse =
				    diffuseColor * incident * MeshDiffuseFactor( normalDotLight );
				color += diffuse;
				direct += diffuse;
			}
			if ( specularLobe && !unlitMesh )
			{
				const vec3 specular = kPi * incident *
				                      PbrSpecular( normal, view, light, f0, roughness ) * compensation *
				                      normalDotLight * directSpecularMask *
				                      MeshSpecularWarp( normal, view, light, exponent );
				color += specular;
				direct += specular;
			}
		}
	}
	// Area lights (render.area-light.v1): both lobes by linearly transformed
	// cosines, windowed by each light's reach. The GGX lobe's magnitude and
	// Fresnel split are the split-sum's A and B, as the image light's are.
	const int areaCount = DebugTermOn( kDebugTermArea ) && !furnace
	                          ? min( int( frame.areaCount.x ), kMaxAreaLights )
	                          : 0;
	if ( areaCount > 0 )
	{
		uvec2 areaMask = uvec2( 0xffffffffu );
		if ( clusterHeader.w != 0u )
		{
			const float distance =
			    dot( clusterView.viewDistance.xyz, worldPosition ) + clusterView.viewDistance.w;
			const uint offset = clusterHeader.w - 1u +
			                    2u * ClusterFroxel( gl_FragCoord.xy, distance );
			areaMask = uvec2( clusterIndices[offset], clusterIndices[offset + 1u] );
		}
		const mat3 ltc =
		    LtcInverse( LtcLookup( ltcTexture, ltcSampler, roughness, normalDotView ) );
#ifdef SEEDED_LTC_NO_MAGNITUDE
		const vec3 areaSpecular = vec3( 1.0 );
#else
		const vec3 areaSpecular = ( f0 * splitSum.x + vec3( splitSum.y ) ) * compensation;
#endif
		while ( any( notEqual( areaMask, uvec2( 0u ) ) ) )
		{
			const int word = areaMask.x != 0u ? 0 : 1;
			const int bit = findLSB( areaMask[word] );
			areaMask[word] &= areaMask[word] - 1u;
			const int i = word * 32 + bit;
			if ( i >= areaCount )
				break;
			const AreaLight light = frame.areas[i];
			const float window = AreaLightWindow(
			    light.center.xyz, light.halfU.xyz, light.halfV.xyz, light.halfU.w, worldPosition );
			if ( window <= 0.0 )
				continue;
			vec3 corners[4];
			AreaLightCorners( light, corners );
			const bool twoSided = light.center.w > 0.5;
			float visibility = 1.0;
			const int firstTile = int( light.radiance.w );
			if ( firstTile >= 0 )
			{
				// The rectangle as a disc of its area as the point sees it: its
				// extent across the ray shrinks with the cosine of the ray's
				// angle to the rectangle's normal, so the disc of the same
				// projected area has sqrt( cosine ) of the radius.
				const vec3 toCenter = normalize( light.center.xyz - worldPosition );
				const vec3 facing = normalize( cross( light.halfU.xyz, light.halfV.xyz ) );
				const float size =
				    sqrt( 4.0 * length( light.halfU.xyz ) * length( light.halfV.xyz ) / kPi ) *
				    sqrt( max( abs( dot( facing, toCenter ) ), 0.05 ) );
				visibility = ShadowTerminatorFade( smoothNormal, toCenter,
				    ShadowFacesVisibility( shadowAtlas, shadowSampler, firstTile,
				        twoSided ? 6 : 5, worldPosition,
				        ShadowReceiverOffset( geometricNormal, toCenter ), size, rotation ) );
				if ( visibility <= 0.0 )
					continue;
			}
			const vec3 radiance = light.radiance.rgb * window * visibility;
			// A baked light's diffuse light is already in the bake, unless
			// the bake is the indirect layer (runtime direct light).
			if ( diffuseLobe && ( light.halfV.w < 0.5 || meshDirect || runtimeDirect ) )
			{
				const vec3 diffuse = diffuseColor * radiance *
				                     LtcRectangle( normal, view, worldPosition, mat3( 1.0 ),
				                         corners, twoSided );
				color += diffuse;
				direct += diffuse;
			}
			if ( specularLobe && !unlitMesh )
			{
				const vec3 specular =
				    areaSpecular * radiance * directSpecularMask *
				    LtcRectangle( normal, view, worldPosition, ltc, corners, twoSided ) *
				    MeshSpecularWarp( normal, view,
				        normalize( light.center.xyz - worldPosition ), exponent );
				color += specular;
				direct += specular;
			}
		}
	}
	// The sun: both lobes (the specular only when its diffuse light is
	// baked), its visibility the bake's mask on a world surface, else its
	// cascades; its disc widens the specular lobe (Karis 2013).
	if ( dot( frame.sunColor.rgb, vec3( 1.0 ) ) > 0.0 && DebugTermOn( kDebugTermSun ) && !furnace )
	{
		const vec3 light = frame.sunDirection.xyz;
		const float normalDotLight = max( dot( normal, light ), 0.0 );
		if ( normalDotLight > 0.0 )
		{
			float visibility = 1.0;
			if ( lightmapped && frame.sunShadow.z > 0.5 )
				visibility = clamp(
				    textureLod( sampler2D( lightmap, lightmapSampler ), lightmapUv, 0.0 ).a, 0.0,
				    1.0 );
			else if ( frame.sunShadow.x >= 0.0 )
			{
				// The first cascade holding the point.
				const int first = int( frame.sunShadow.x );
				const int count = int( frame.sunShadow.y );
				for ( int c = 0; c < count; ++c )
				{
					const ShadowTile tile = shadowTiles[first + c];
					const vec4 h = tile.viewProjection * vec4( worldPosition, 1.0 );
					const vec3 ndc = h.xyz / h.w;
					if ( all( lessThan( abs( ndc.xy ), vec2( 0.98 ) ) ) && ndc.z <= 1.0 )
					{
						visibility = ShadowTerminatorFade( smoothNormal, light,
						    ShadowVisibilitySoft( shadowAtlas, shadowSampler, tile,
						        worldPosition, ShadowReceiverOffset( geometricNormal, light ),
						        frame.sunDirection.w, rotation ) );
						break;
					}
				}
			}
			const vec3 incident = frame.sunColor.rgb * visibility;
			if ( diffuseLobe && ( frame.sunColor.w < 0.5 || meshDirect || runtimeDirect ) )
			{
				const vec3 diffuse =
				    diffuseColor * incident * MeshDiffuseFactor( normalDotLight );
				color += diffuse;
				direct += diffuse;
			}
			if ( specularLobe && !unlitMesh && visibility > 0.0 )
			{
				const float alpha = roughness * roughness;
				const float widened = min( alpha + 0.5 * frame.sunDirection.w, 1.0 );
				const vec3 specular = kPi * incident *
				                      PbrSpecular( normal, view, light, f0, sqrt( widened ) ) *
				                      compensation * normalDotLight * directSpecularMask * ( alpha * alpha ) /
				                      ( widened * widened ) *
				                      MeshSpecularWarp( normal, view, light, exponent );
				color += specular;
				direct += specular;
			}
		}
	}
	// The view's projected lights: never baked, so both lobes, shadowed by
	// their tiles (a lens of atten.w).
	const int projectorCount = Term( kClustered ) && DebugTermOn( kDebugTermProjected ) && !furnace
	                               ? int( clusterView.counts.x )
	                               : 0;
	for ( int i = 0; i < projectorCount; ++i )
	{
		const ProjectedLight projector = projectors[i];
		vec2 cookieUv;
		float distance;
		if ( !ProjectorProject( projector, worldPosition, cookieUv, distance ) )
			continue;
		const vec3 light = ( projector.origin.xyz - worldPosition ) / max( distance, 1e-4 );
		const float normalDotLight = max( dot( normal, light ), 0.0 );
		float scale = ProjectorAttenuation( projector, distance );
		if ( normalDotLight <= 0.0 || scale <= 0.0 )
			continue;
		const int tile = int( projector.color.w );
		if ( tile >= 0 )
			scale *= ShadowTerminatorFade( smoothNormal, light,
			    ShadowVisibilitySoft( shadowAtlas, shadowSampler, shadowTiles[tile],
			        worldPosition, ShadowReceiverOffset( geometricNormal, light ),
			        projector.atten.w, rotation ) );
		const vec3 cookie =
		    textureLod( sampler2DArray( cookieTexture, cookieSampler ),
		        vec3( cookieUv, projector.origin.w ), 0.0 )
		        .rgb;
		const vec3 incident = projector.color.rgb * cookie * scale;
		if ( diffuseLobe )
		{
			const vec3 diffuse = diffuseColor * incident * MeshDiffuseFactor( normalDotLight );
			color += diffuse;
			direct += diffuse;
		}
		if ( specularLobe && !unlitMesh )
		{
			const vec3 specular = kPi * incident *
			                      PbrSpecular( normal, view, light, f0, roughness ) * compensation *
			                      normalDotLight * directSpecularMask *
			                      MeshSpecularWarp( normal, view, light, exponent );
			color += specular;
			direct += specular;
		}
	}
	vec3 imageSpecular = vec3( 0.0 );
	vec3 iblRadiance = vec3( 0.0 );
	vec3 iblWeight = vec3( 0.0 );
	vec3 probeRadiance = vec3( 0.0 );
	vec4 probeSelection = vec4( -1.0, -1.0, 0.0, 0.0 );
	vec3 probeHeader = vec3( 0.0 );
	float probeRankDivisor = 32.0;
	if ( specularLobe && DebugTermOn( kDebugTermIbl ) )
	{
		const vec3 reflected = reflect( -view, normal );
		vec3 radiance;
		if ( ( kDebugView == 24 || kDebugView == 27 ) && !furnace && Term( kReflectionProbes ) )
		{
			const vec4 header = ReflectionProbesFetch( ivec2( 0, 0 ) );
			const vec4 mode = ReflectionProbesFetch( ivec2( 1, 0 ) );
			probeRankDivisor = header.x > 16.0 ? 128.0 : 32.0;
			probeHeader = vec3( clamp( header.x / 64.0, 0.0, 1.0 ),
			    clamp( mode.x / 7.0, 0.0, 1.0 ),
			    header.w == kReflectionProbesMarker ? 1.0 : 0.0 );
		}
		if ( furnace || !Term( kReflectionProbes ) )
		{
			probeSelection.x = -2.0;
			radiance = AmbientCube( reflected );
		}
		else if ( !ReflectionProbesRadianceDebug( worldPosition, smoothNormal, reflected,
		              roughness, radiance, probeSelection ) )
		{
			const vec4 header = ReflectionProbesFetch( ivec2( 0, 0 ) );
			if ( header.w == kReflectionProbesMarker )
				probeSelection.x = header.x < 1.0 ? -3.0 : -4.0;
			radiance = AmbientCube( reflected );
		}
		probeRadiance = radiance;
		// The specular occlusion from the view's occlusion (one without it).
		const float specularOcclusion =
		    Term( kAmbientOcclusion ) && DebugTermOn( kDebugTermSpecularOcclusion ) && !furnace
		        ? SpecularOcclusion( normalDotView, screenOcclusion, roughness )
		        : 1.0;
		iblRadiance = radiance * material.envSaturation.rgb;
		iblRadiance = mix( iblRadiance, iblRadiance * iblRadiance,
		    material.meshProbeColor.x );
		const vec3 grey = vec3( dot( iblRadiance, vec3( 0.299, 0.587, 0.114 ) ) );
		iblRadiance = mix( grey, iblRadiance, material.meshProbeColor.y );
		if ( material.envLightScale.z > 0.0 )
		{
			const vec3 lightMask = clamp( ( diffuseIrradiance - material.envLightScale.x ) *
			                                  material.envLightScale.y, 0.0, 1.0 );
			iblRadiance *= mix( vec3( 1.0 ), lightMask, material.envLightScale.z );
		}
		const float angularMask = mix( material.meshProbeFresnel.y,
		    material.meshProbeFresnel.z,
		    pow( 1.0 - normalDotView, material.meshProbeFresnel.w ) );
		iblWeight = mix( f0, directionalAlbedo, material.meshProbeFresnel.x ) *
		            occlusion * specularOcclusion * probeSpecularMask * angularMask;
		imageSpecular = iblRadiance * iblWeight;
		color += imageSpecular;
		if ( material.meshControls.z > 0.0 )
		{
			// VertexLitGeneric's authored rim is a grazing image-light
			// lobe in the shared model; it reads the same RPRB radiance.
			const vec3 rim = iblRadiance * base * material.meshControls.z *
			                 pow( 1.0 - normalDotView, material.meshControls.w ) * occlusion;
			imageSpecular += rim;
			color += rim;
		}
	}
	WriteSsrTargets( normal, roughness, iblRadiance, iblWeight,
	    emissive && material.emission.z > 0.5 );
	vec3 emission = vec3( 0.0 );
	if ( emissive && DebugTermOn( kDebugTermEmission ) && !furnace &&
	     ( material.emission.y <= 0.5 || dot( smoothNormal, view ) > 0.0 ) )
	{
		emission =
		    texture( sampler2D( emissionTexture, emissionSampler ), uv ).rgb * material.emission.x;
		if ( material.emissionCone.w > 0.5 )
		{
			const float cosine = dot( smoothNormal, view );
			const float cone = clamp( ( cosine - material.emissionCone.y ) /
			    max( material.emissionCone.x - material.emissionCone.y, 1e-6 ), 0.0, 1.0 );
			emission *= material.emissionCone.z == 0.0 || material.emissionCone.z == 1.0
			                ? cone
			                : pow( cone, material.emissionCone.z );
		}
		color += emission;
	}
	if ( Term( kSelfIllum ) && DebugTermOn( kDebugTermEmission ) && !furnace )
	{
		// Source model base alpha selects the self-lit surface instead of
		// adding a second copy of its albedo on top of direct lighting.
		const vec3 selfLit = base * material.selfIllumTint.rgb;
		const vec3 mask = Term( kSelfIllumMask )
		                      ? texture( sampler2D( emissionTexture, emissionSampler ), uv ).rgb
		                      : vec3( baseSample.a );
		emission += selfLit * mask;
		color = mix( color, selfLit, mask );
	}
	if ( unlitMesh && DebugTermOn( kDebugTermEmission ) && !furnace )
	{
		emission += base;
		color += base;
	}
	if ( Term( kTransmission ) && !furnace )
	{
		const ivec2 texel = clamp( ivec2( gl_FragCoord.xy ), ivec2( 0 ),
		    textureSize( sampler2D( sceneColorTexture, sceneColorSampler ), 0 ) - 1 );
		const vec3 behind = texelFetch(
		    sampler2D( sceneColorTexture, sceneColorSampler ), texel, 0 ).rgb;
		color += behind * baseDiffuse * transmitted;
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
		if ( specularLobe )
		{
			inputs.mask |= kDebugHasReflectionProbe;
			inputs.probeRankDivisor = probeRankDivisor;
			inputs.probeSelection = probeSelection;
			inputs.probeRadiance = probeRadiance;
			inputs.probeWeight = iblWeight;
			inputs.probeHeader = probeHeader;
		}
		if ( lightmapped )
		{
			inputs.mask |= kDebugHasBaked;
			inputs.baked = baked;
		}
		if ( emissive || Term( kSelfIllum ) || unlitMesh )
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
	outColor = Output( color,
	    Term( kTransmission ) ? 1.0
	    : ( Term( kSelfIllum ) || material.meshControls.x > 0.5 ) && material.state.x > 0.5
	        ? 1.0
	        : baseSample.a * material.tint.a );
}

// The vertexlit point: the vertexlit_and_unlit_generic port's DIFFUSELIGHTING
// path, the vertex lighting mixing the ambient cube and the lights, so no
// light term is separable; the furnace takes albedo 1 under a uniform
// radiance of 1.
void VertexLitSurface()
{
	const bool furnace = DebugFurnace();
	const vec4 baseColor = texture( sampler2D( baseTexture, baseSampler ), baseUv );
	const vec3 albedo = furnace ? vec3( 1.0 ) : baseColor.rgb * material.tint.rgb;
	const float alpha = material.tint.a * baseColor.a;
	if ( material.flags.y != 0.0 && alpha < material.flags.z )
		discard;
	const vec3 lit = albedo * ( furnace ? vec3( 1.0 ) : vertexLighting );
	if ( DebugViewActive() )
	{
		DebugInputs inputs = DebugInputsNone();
		inputs.mask = kDebugHasAlbedo | kDebugHasAo | kDebugHasUv0;
		inputs.albedo = albedo;
		inputs.uv0 = baseUv;
		inputs.final = lit;
		outColor = DebugOutput( inputs );
		return;
	}
	outColor = Output( lit, alpha );
}

// The water point: water_ps2x's main (Portal 2's; the CS:GO source has its
// parameter set) with REFLECT or the forced env map, BASETEXTURE with
// FLOWMAP or neither, LIGHTMAPWATERFOG, ABOVEWATER and no REFRACT. It
// computes the shader's output (TONEMAP_SCALE_NONE: the reflection is not
// scaled again) and hands Output its value before the tone-map scale.
void WaterSurface()
{
	const vec3 toEye = normalize( frame.eye.xyz - worldPosition );
	const vec2 worldUv = vec2( worldPosition.x, -worldPosition.y );
	vec4 normal;
	vec4 flowColor = vec4( 0.0 );
	if ( material.waterFlowTime.w != 0.0 )
	{
		// The noise offsets each point's interval, so the layers don't pulse.
		const float noise = texture( sampler2D( flowNoiseTexture, flowNoiseSampler ),
		    worldUv * material.waterFlowTime.z )
		                        .g;
		const vec4 flowTexel =
		    texture( sampler2D( flowTexture, flowSampler ), baseUv * material.waterFlow.x );
		const vec2 flowVector = flowTexel.rg * 2.0 - 1.0;
		const float intervals = frame.water.x / ( material.waterFlowTime.x * 2.0 ) + noise;
		const vec2 uv = worldUv * material.waterFlow.y;
		const vec2 flowUv1 = uv + floor( intervals ) * 0.311 +
		                     fract( intervals ) * ( material.waterFlowTime.y * flowVector );
		const vec2 flowUv2 = uv + ( floor( intervals + 0.5 ) * 0.311 + 0.5 ) +
		                     fract( intervals + 0.5 ) * ( material.waterFlowTime.y * flowVector );
		const float weight2 = abs( 2.0 * fract( intervals ) - 1.0 );
		const vec4 normal1 = texture( sampler2D( bumpTexture, bumpSampler ), flowUv1 );
		const vec4 normal2 = texture( sampler2D( bumpTexture, bumpSampler ), flowUv2 );
		vec2 xy = mix( normal1.xy, normal2.xy, weight2 ) * 2.0 - 1.0;
		// The bump strength follows the flow's speed.
		xy *= ( dot( flowVector, flowVector ) + 0.1 ) * material.waterFlow.z;
		normal = vec4( xy, sqrt( clamp( 1.0 - dot( xy, xy ), 0.0, 1.0 ) ), 1.0 );
		if ( material.waterMode.y != 0.0 )
		{
			// The sludge: the base texture scrolled the same way, displaced
			// along the view by the normal maps' alpha.
			const float parallax =
			    mix( normal1.a, normal2.a, weight2 ) * material.waterFlow.w;
			const vec2 colorUv =
			    worldUv * material.waterColorFlow.x + ( toEye.xy - normal.xy ) * parallax;
			const float colorIntervals =
			    frame.water.x / ( material.waterColorFlow.y * 2.0 ) + noise;
			const vec2 colorUv1 = colorUv + floor( colorIntervals ) * 0.311 +
			                      ( fract( colorIntervals ) - 0.5 ) *
			                          ( material.waterColorFlow.z * flowVector );
			const vec2 colorUv2 = colorUv + ( floor( colorIntervals + 0.5 ) * 0.311 + 0.5 ) +
			                      ( fract( colorIntervals + 0.5 ) - 0.5 ) *
			                          ( material.waterColorFlow.z * flowVector );
			const float colorWeight1 = pow(
			    abs( 2.0 * fract( colorIntervals + 0.5 ) - 1.0 ), material.waterColorFlow.w );
			const float colorWeight2 =
			    pow( abs( 2.0 * fract( colorIntervals ) - 1.0 ), material.waterColorFlow.w );
			flowColor = texture( sampler2D( baseTexture, baseSampler ), colorUv1 ) * colorWeight1 +
			            texture( sampler2D( baseTexture, baseSampler ), colorUv2 ) * colorWeight2;
			flowColor *= flowTexel.a; // the flow map's alpha masks the sludge
		}
	}
	else
	{
		// DecompressNormal( NORM_DECODE_NONE ) at the surface's coordinates.
		const vec4 texel = texture( sampler2D( bumpTexture, bumpSampler ), baseUv );
		normal = vec4( texel.xyz * 2.0 - 1.0, texel.a );
	}

	// The reflection: the view's reflection target at the fragment's view
	// position (flipped vertically: the reflected view), offset along the
	// camera's right and forward in the water plane by the normal; or the
	// env map in the reflected direction.
	vec3 reflection;
	if ( material.waterMode.x != 0.0 )
	{
		const vec2 view = ( gl_FragCoord.xy - frame.viewport.xy ) * frame.viewport.zw;
		const vec2 right = frame.water.zw;
		const vec2 forward = vec2( -right.y, right.x );
		const vec2 offset =
		    vec2( dot( right, normal.xy ), dot( forward, normal.xy ) ) * normal.a * material.waterFog.a;
		reflection = texture( sampler2D( reflectionTexture, reflectionSampler ),
		    vec2( view.x, 1.0 - view.y ) + offset )
		                 .rgb;
	}
	else
	{
		const vec3 reflected = 2.0 * dot( normal.xyz, toEye ) * normal.xyz -
		                       dot( normal.xyz, normal.xyz ) * toEye;
		reflection =
		    frame.eye.w * texture( samplerCube( envmapTexture, envmapSampler ), reflected ).rgb;
	}
	reflection *= material.waterReflect.rgb * frame.water.y;

	const float fresnel =
	    material.waterMode.w != -1.0
	        ? material.waterMode.w
	        : 0.2 + 0.8 * pow( 1.0 - clamp( dot( toEye, normal.xyz ), 0.0, 1.0 ), 5.0 );
	// LIGHT_MAP_SCALE * LINEAR_LIGHT_SCALE: the lightmap in the output's
	// scale, as the shader's output is.
	vec3 light = vec3( 1.0 );
	vec3 fog = material.waterFog.rgb;
	if ( material.waterMode.z != 0.0 )
	{
		light = texture( sampler2D( lightmap, lightmapSampler ), lightmapUv ).rgb * frame.light.x *
		        frame.light.y;
		fog *= light;
	}
	vec3 lit;
	if ( material.waterMode.y != 0.0 )
	{
		// The sludge's alpha: 0 to 0.5 its translucency in the water, 0.5
		// to 0.7 floating above it, where nothing reflects.
		const vec3 underWater =
		    mix( fog, flowColor.rgb * light, clamp( flowColor.a * 2.0, 0.0, 1.0 ) );
		const float aboveWater = smoothstep( 0.5, 0.7, flowColor.a );
		lit = mix( underWater, reflection, clamp( fresnel * ( 1.0 - aboveWater ), 0.0, 1.0 ) );
	}
	else
	{
		lit = mix( fog, reflection, fresnel );
	}
	if ( DebugViewActive() )
	{
		DebugInputs inputs = DebugInputsNone();
		inputs.mask = kDebugHasAlbedo | kDebugHasNormal | kDebugHasUv0;
		inputs.albedo = material.waterMode.y != 0.0 ? flowColor.rgb : material.waterFog.rgb;
		inputs.normal = normal.xyz;
		inputs.uv0 = baseUv;
		if ( material.waterMode.z != 0.0 )
		{
			inputs.mask |= kDebugHasBaked;
			inputs.baked = light / frame.light.y;
		}
		inputs.final = lit / frame.light.y;
		outColor = DebugOutput( inputs );
		return;
	}
	outColor = Output( lit / frame.light.y, material.waterReflect.a );
}

void main()
{
	// Points without image specular leave the SSR targets empty (weight 0:
	// render.pass.ssr leaves their pixels unchanged).
	WriteSsrTargets( vec3( 0.0, 0.0, 1.0 ), 1.0, vec3( 0.0 ), vec3( 0.0 ), false );
	if ( Term( kPbr ) )
	{
		if ( Term( kTransmission ) && material.transmission.z > 0.5 )
		{
			RefractSurface();
			return;
		}
		PbrSurface();
		return;
	}
	if ( Term( kWater ) )
	{
		WaterSurface();
		return;
	}
	if ( Term( kRsm ) )
	{
		// A legacy point's reflectance: its base texture times its tint.
		outColor = vec4(
		    texture( sampler2D( baseTexture, baseSampler ), baseUv ).rgb * material.tint.rgb, 1.0 );
		return;
	}
	if ( Term( kDepthNormal ) )
	{
		const vec3 n = dot( worldNormal, worldNormal ) > 1e-12 ? normalize( worldNormal )
		                                                     : vec3( 0.0, 0.0, 1.0 );
		outColor = vec4( SurfaceOctEncode( n ), 1.0, 0.0 );
		return;
	}
	if ( Term( kVertexLit ) )
	{
		VertexLitSurface();
		return;
	}
	const bool bumpmap = Term( kBumpmap | kSsbump );
	const bool ssbump = Term( kSsbump );
	const bool diffuseBumpmap = bumpmap && Term( kDiffuseBumpmap );
	const bool lightingOne = Term( kUnlit );

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
	if ( lightingOne )
	{
		// UnlitGeneric: $alpha once, the vertex color and the vertex alpha
		// each by its own flag.
		if ( material.flags.x != 0.0 )
			albedo *= color.rgb;
		if ( material.state.w != 0.0 )
			alpha *= color.a;
	}
	else if ( material.flags.x != 0.0 )
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
	// The frame's emitting surfaces (render.area-light.v1), which the
	// engine leaves out of the pages of the surfaces the core draws: their
	// irradiance at the mapped normal, tinted as the lightmap is.
	if ( !lightingOne && !furnace && frame.areaCount.x > 0.0 &&
	     dot( worldNormal, worldNormal ) > 0.0 )
	{
		// The shading normal below: an ssbump's weights are a tangent-space
		// normal by now.
		const vec3 n = normalize( bumpmap ? normalSample.x * tangentS + normalSample.y * tangentT +
		                                        normalSample.z * worldNormal
		                                  : worldNormal );
		diffuse += tint * AreaLightIrradiance( n, worldPosition );
	}

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
