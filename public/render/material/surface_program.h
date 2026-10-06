//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The one surface program (RFC 0016 K11 "Model assembly"): the
//			owner of surface.frag's bind groups, vertex stages and pipelines on
//			render.device.v2. Each lighting-model term is a specialization
//			constant (clause D20), so a term at its neutral value costs
//			nothing, and the legacy families are points of the program: the
//			`lightmapped` family (lightmapped_family.h) claims
//			LightmappedGeneric's terms, the `pbr` family (pbr_family.h) the
//			RFC 0007 metal/roughness point (kSurfacePbr). A family owns its
//			claim and the packing of its parameters into SurfaceConstants; the
//			program owns everything the GPU sees.
//
//			Bind groups, one set for every point (a point that does not read
//			an input binds the neutral one: an input named empty takes a 1x1
//			white texture or cube, material_programs.h):
//			- frame (role kFrame): binding 0 the SurfaceFrame block, 1 the
//			  split-sum table (SplitSumTable, read by the pbr point) and 2
//			  its linear, clamped sampler, 3 the LTC table (LtcTable, the
//			  pbr point's area lights) and 4 its sampler;
//			- material (role kMaterial): binding 0 SurfaceConstants; 1 base,
//			  3 env map (a cube), 5 env map mask, 7 bump or normal map, 9
//			  detail, 11 MRAO and 13 emission, each with its sampler after it
//			  (the water point binds its flow map at 5 and its flow noise at
//			  11: a group has 16 GL slots, and it reads neither of those);
//			- view (role kView): the view's clustered runtime lights
//			  (render.light-set.v1's point and spot lights; render.pass.lights
//			  lists them per froxel, RFC 0016 K7): binding 0 SurfaceViewGpu,
//			  1 the froxels' ranges, 2 the index list, 3 SurfaceLightGpu
//			  records, in the order the index list counts them, 4 the
//			  ShadowTileGpu records a light's shadow tile indexes, 5 the view's
//			  shadow atlas (render.shadows.v1, a depth texture) and 6 its point
//			  sampler. Only a variant with kSurfaceClustered reads them; the
//			  others bind the neutral view group (NeutralViewGroup), which
//			  MaterialPrograms keeps. Binding 12 is the view's planar
//			  reflection (SurfaceScreenInputs), which the water point reads,
	//			  and 13 its sampler; 14 is the pre-output linear scene color
	//			  behind a transmitting surface and 15 its sampler;
//			- draw (role kDraw): binding 0 the draw's lightmap page, 1 its
//			  sampler, 2 its model lighting (model_lighting.h; neutral for a
//			  world surface).
//			The draw constants are the FamilyDrawConstants prefix: world-to-
//			clip for the flat and world vertices, object-to-clip and object-
//			to-world for the model vertex.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_SURFACE_PROGRAM_H
#define RENDER_MATERIAL_SURFACE_PROGRAM_H

#include "foundation/expected.h"
#include "render/area_light.h"
#include "render/device/device.h"
#include "render/light_set.h"
#include "render/material/material_programs.h"
#include "render/material/model_lighting.h"
#include "render/projected_light.h"
#include "render/shadow_tile.h"
#include "render/shaderlib/debug_view.h"

#include <cstdint>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace render::material
{

// The lightmap scale in linear light: the port's 2.0 overbright, gamma to
// linear (pow 2.2).
inline constexpr float kLightmapScaleLinear = 4.5947938f;

// The program's material constants (std140, the Material block of
// surface.frag).
struct SurfaceConstants
{
	float tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // $color, $alpha
	float flags[4] = {}; // $vertexcolor, $alphatest, reference, mesh half-Lambert
	// x: 1 when the material is fully opaque (no blend, no alpha test), where
	// height fog writes its factor to the output alpha (the port's
	// WRITEWATERFOGTODESTALPHA); set by Request from the variant. y: 1 when
	// the vertex color is gamma-encoded and decoded per vertex (pow 2.2, as
	// UnlitGeneric's port reads it); LightmappedGeneric's is used unconverted.
	// z: the ssbump weights' scale (0.57735 with $ssbumpmathfix, else 1).
	// w: $vertexalpha (the unlit point).
	float state[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float envTint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };       // $envmaptint, $fresnelreflection
	float envContrast[4] = { 0.0f, 0.0f, 0.0f, 0.0f };   // in effect; a: 1 - $fresnelreflection
	// RGB: env saturation on legacy points, native mesh probe tint on PBR;
	// alpha: the native mesh's inverted normal-alpha Phong mask.
	float envSaturation[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	float selfIllumTint[4] = { 1.0f, 1.0f, 1.0f, 0.0f }; // in effect
	float detailTint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };    // $detailtint, $detailblendfactor
	float detailScale[4] = { 4.0f, 4.0f, 0.0f, 0.0f };   // $detailscale
	// Portal 2's $envmaplightscale: the cube map darkened where the diffuse
	// light is dark. x: the min of $envmaplightscaleminmax, y: min + max (as
	// the Portal 2 helper packs them), z: $envmaplightscale (0 off).
	float envLightScale[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	// x: scale, y: one-sided emission, z: camera-only analytic emitter mesh
	// (the analytic light supplies its glossy rays; SSR cannot sample its pixel).
	float emission[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	// The water point (kSurfaceWater, water_family.h): water_ps2x's
	// constants. Flow: 1 / $flow_worlduvscale, 1 / $flow_normaluvscale,
	// $flow_bumpstrength, $color_flow_displacebynormalstrength.
	float waterFlow[4] = { 1.0f, 1.0f, 1.0f, 0.0025f };
	// $flow_timeintervalinseconds, $flow_uvscrolldistance, $flow_noise_scale,
	// and 1 with a flow map (else the normal map at the surface coordinates).
	float waterFlowTime[4] = { 0.4f, 0.2f, 0.0002f, 0.0f };
	// 1 / $color_flow_uvscale, $color_flow_timeintervalinseconds,
	// $color_flow_uvscrolldistance, $color_flow_lerpexp.
	float waterColorFlow[4] = { 1.0f, 0.4f, 0.2f, 1.0f };
	// rgb: $reflecttint (Source's GammaToLinear); w: $waterblendfactor, the
	// output alpha.
	float waterReflect[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	// rgb: $fogcolor (the sRGB curve); w: $reflectamount.
	float waterFog[4] = { 1.0f, 0.0f, 0.0f, 0.8f };
	// x: 1 when the reflection is the view's reflection target, 0 the env
	// map (binding 3); y: 1 with the sludge ($basetexture and a flow map); z:
	// $lightmapwaterfog; w: $forcefresnel (-1: the fresnel term).
	float waterMode[4] = { 1.0f, 0.0f, 0.0f, -1.0f };
	// Modern mesh point without an MRAO texture: metalness, roughness and AO.
	// kSurfaceMraoTexture selects the texture instead at pipeline creation.
	float pbrFactors[4] = { 0.0f, 0.55f, 1.0f, 0.0f };
	// Native VertexLit mesh point: x base-alpha Phong mask, y albedo-tinted
	// Phong, z rim boost (0 off), w rim exponent. Neutral for other points.
	float meshControls[4] = { 0.0f, 0.0f, 0.0f, 4.0f };
	// Native probe reflectance masks on VertexLit meshes: base alpha and
	// normal alpha. Other points leave both off.
	float meshProbeMasks[4] = {};
	// VertexLit probe response: legacy env Fresnel contribution, followed by
	// the authored minimum, maximum and exponent of its angular mask.
	float meshProbeFresnel[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float meshModes[4] = {}; // light warp, authored env map, Phong enable
	// Native mesh probe contrast, saturation, Phong-warp enable, and authored
	// RGB $envmapmask enable. The last reads the data texture at material binding 5.
	float meshProbeColor[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	// Thin dielectric transmission on the shared PBR point: x is the
	// transmitted fraction, y the index of refraction. The scene input is
	// linear light before output scale, fog and encoding.
	float transmission[4] = { 0.0f, 1.5f, 0.0f, 0.0f };
	// Visible one-sided emitter shape: inner/outer cosine, falloff exponent,
	// and 1 when the authored cone is present. Neutral for other PBR surfaces.
	float emissionCone[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	// UnlitTwoTexture: the second texture's independent UV transform.
	float texture2Transform[8] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
	float surfaceControls[4] = { 0.0f, 1.0f, 0.0f, 1.0f }; // nofog, HDR scale, portal radius, alpha2
	// The unlit particle point uses z/w as depthblend enabled/scale.
	// VertexLitGeneric's $selfillumfresnelminmaxexp (min, max, exp) and 1
	// when $selfillumfresnel weights the self-illuminated region
	// (vertexlit_family.h). Neutral (w 0) for every other point.
	float selfIllumFresnel[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
	// tree_sway.glsl: geometry, motion, falloff curves, wind controls/mode.
	float treeGeometry[4] = { 1000.0f, 0.1f, 300.0f, 0.2f };
	float treeMotion[4] = { 1.0f, 10.0f, 12.0f, 10.0f };
	float treeCurves[4] = { 2.0f, 1.0f, 1.5f, 5.0f };
	float treeWind[4] = { 3.0f, 6.0f, 0.0f, 0.0f }; // lerp start/end, static, mode
	float baseTransform[8] = { 1, 0, 0, 0, 0, 1, 0, 0 }; // independent base texture UV rows
	// The base texture's encoding on the unlit point. x: 1 for RGBS (the
	// Sky shader's $hdrcompressedtexture: color times alpha, filtered as
	// premultiplied texels, sky_hdr_compressed_rgbs_ps2x); its scale is
	// surfaceControls.y. Neutral (0) for every other point.
	float baseDecode[4] = {};
};
static_assert( sizeof( SurfaceConstants ) == 576 );

// An area light as the frame block holds it (render.area-light.v1: the
// rectangle, its radiance and its reach).
struct SurfaceAreaLight
{
	float center[4] = {}; // w: 1 when two-sided
	float halfU[4] = {};  // w: the reach
	float halfV[4] = {};  // w: 1 when its diffuse light is in the surface's baked light
	// rgb: the radiance; w: the first of its shadow tiles in the view's list
	// (the faces of its hemicube, five, or six when two-sided), or -1
	float radiance[4] = {};
};
static_assert( sizeof( SurfaceAreaLight ) == 64 );
inline constexpr int kSurfaceMaxAreaLights = 64;

// Packs an area light (area_light::AreaLight, its reach set). diffuseInBake:
// the surfaces' baked light (lightmap, probe volume) already holds its
// diffuse light, so the program adds its specular lobe only. firstTile: its
// shadow faces in the view's tile list (SurfaceShadows), or -1 unshadowed.
SurfaceAreaLight PackAreaLight(
    const area_light::AreaLight &light, bool diffuseInBake = false, int firstTile = -1 );

// The view group's parameters (std140, binding 0): the cluster grid a
// fragment's froxel is found in (render.pass.lights FroxelAt) and the view
// distance of a world point.
struct SurfaceViewGpu
{
	std::uint32_t grid[4] = {}; // tilesX, tilesY, slices, tile size in pixels
	float slices[4] = {};       // sliceScale, sliceBias, nearZ, 0
	// The view distance of a world point p: dot( xyz, p ) + w (the negated z
	// row of world-to-view).
	float viewDistance[4] = {};
	// x: the view's projected lights (binding 7), y: the pixel-to-texel
	// scale of the view's screen inputs (1), z, w: viewport origin in pixels
	float counts[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	// Soft particles read the copied scene's alpha = clamp(clip Z / range).
	float depthAlpha[4] = {}; // range, 1/source width, 1/source height, reserved
};
static_assert( sizeof( SurfaceViewGpu ) == 80 );

// A runtime point or spot light as the view group holds it (std430,
// binding 3), light_set::RuntimeLight packed.
struct SurfaceLightGpu
{
	float position[4] = {};  // w: radius (0 unbounded)
	float color[4] = {};     // linear, times the style scalar; w: minLight
	float direction[4] = {}; // the spot's axis; w: outerCos, or -2 for a point light
	// innerCos, the falloff (0 Source's dlight, 1 inverse square, 2 vrad's
	// attenuation), sourceRadius, and the
	// light's shadow tile in the view's tile list (binding 4), or -1 when it
	// has none (unshadowed: its visibility is one).
	float cone[4] = {};
	// x: the cone ramp's exponent (light_set::SpotFactor); y: 1 when its
	// diffuse light is in the surface's baked light (the specular lobe only);
	// z: RuntimeShadowLayout (single tile or world-aligned cube from cone.w)
	float spot[4] = {};
	// xyz: vrad's constant, linear and quadratic terms (cone.y 2: an
	// Attenuated world light, light_set::AttenuatedFalloff)
	float attenuation[4] = {};
};
static_assert( sizeof( SurfaceLightGpu ) == 96 );

SurfaceLightGpu PackSurfaceLight( const light_set::RuntimeLight &light, int shadowTile = -1,
    RuntimeShadowLayout shadowLayout = RuntimeShadowLayout::kSingle, bool diffuseInBake = false );

// A view's shadows as the view group binds them: the atlas the view's shadow
// passes drew (render.pass.shadows; kSampled wherever the group is read, and
// alive past its last use) and its tiles. No atlas binds a neutral texture no
// tile may index.
struct SurfaceShadows
{
	device::TextureId atlas;
	device::TextureDesc atlasDesc;
	std::span<const ShadowTileGpu> tiles;
};

// A view's projected lights (render.projected-light.v1, both lobes, never
// baked): their records (projected_light::PackLightGpu, the cookie layer and
// shadow tile set) and the cookies as one 2D array texture (at least two
// layers), made and kept by the owner in kSampled where the group is read.
struct SurfaceProjectors
{
	std::span<const projected_light::LightGpu> lights;
	device::TextureId cookies;
	device::TextureDesc cookiesDesc;
};

// A view's screen inputs (made by passes before the surface pass, kSampled
// while it runs): the ambient occlusion (render.pass.ao, at the view's size),
// read under kSurfaceAmbientOcclusion. Its receiver metadata prevents applying
// another surface's visibility. Inputs without the b = -1 marker retain the
// scalar visibility contract (including the neutral white texture).
struct SurfaceScreenInputs
{
	device::TextureId ambientOcclusion;
	device::TextureDesc ambientOcclusionDesc;
	// The view's planar reflection (the image a reflection view drew before
	// this pass; its sRGB view), read by the water point
	// (kSurfaceWater, binding 12, filtered and clamped).
	device::TextureId planarReflection;
	device::TextureDesc planarReflectionDesc;
	// Snapshot of the opaque attachment, after its output scale and fog. sRGB
	// images decode while sampling; manually encoded unorm images are decoded
	// by the surface's frame setting. Transmission must not expose/fog it again.
	device::TextureId sceneColor;
	device::TextureDesc sceneColorDesc;
	// Ordered copy of opaque scene depth, encoded in alpha as clip Z / range.
	// The image is sampled linearly without sRGB conversion. It is distinct
	// from the currently bound writable depth attachment. The unlit point
	// shares sceneColor's binding; callers bind separate per-point view groups.
	device::TextureId depthAlpha;
	device::TextureDesc depthAlphaDesc;
	float depthAlphaRange = 0.0f;
	// The full source attachment's extent before the depth copy resamples it.
	std::uint32_t depthAlphaSourceWidth = 0;
	std::uint32_t depthAlphaSourceHeight = 0;
};

// The frame's terms (std140, the Frame block of surface.frag): one lightmap
// term whose scale depends on how the pages encode light, and the output's
// linear scale. LDR pages hold gamma light at half overbright (scale 2^2.2
// after sRGB decode); integer-HDR pages hold linear light / 16 (scale 16);
// the output scale is the frame's linear tone-mapping scale (1 without HDR).
// The defaults are LDR's, where the family's pixel cases sit.
//
// The view's fog is a frame term too (legacy::CorePassFog): its color (linear,
// tone-scaled in integer HDR) with its type in w (-1 none, 0 range, 1 height),
// its parameters, and the eye's world z. The default is no fog.
struct SurfaceFrame
{
	// lightmap scale, output scale, 1 to encode sRGB in the shader (a target
	// without an sRGB view), 1 when specular shows
	float light[4] = { kLightmapScaleLinear, 1.0f, 0.0f, 1.0f };
	float fogColor[4] = { 0.0f, 0.0f, 0.0f, -1.0f };
	float fogParams[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	// x: the eye's world z; y: 1 when the running game's shaders scale every
	// ssbump's basis weights by 1/sqrt(3) (Portal 2's do; this SDK's only
	// with $ssbumpmathfix: the backend's SsbumpBasisNormalized owns it).
	float fogMisc[4] = {};
	// xyz: the eye's world position (the env map's reflection, the pbr
	// point's view direction); w: ENV_MAP_SCALE, 16 in integer HDR (cube
	// maps hold light / 16), else 1.
	float eye[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	// The frame's area lights (the pbr point reads them): x their count.
	float areaCount[4] = {};
	// The sun (render.shadows.v1; the pbr point reads it): xyz towards the
	// sun, w the tangent of its disc's angular radius.
	float sunDirection[4] = {};
	// rgb its diffuse light on a surface facing it (the lightmap unit); w 1
	// when that diffuse light is in the surfaces' baked light.
	float sunColor[4] = {};
	// x: the first of its cascade tiles in the view's list, or -1; y: the
	// cascades' count; z: 1 when a world surface's visibility is the baked
	// mask in its gradient page's alpha (LMAP --sun-visibility); w: 0.
	float sunShadow[4] = { -1.0f, 0.0f, 0.0f, 0.0f };
	// The water point's frame terms: x the shaders' time in seconds (the
	// backend's CurrentTime, which water_ps2x's flow reads); y the scale of
	// its reflection tint (4 in integer HDR, where the client draws the
	// water views at a quarter of the tone-map scale, SetLightmapScaleForWater;
	// 1 otherwise); z, w: the camera's right in the water plane (the view's
	// x axis projected on the z = 0 plane, normalized: water_ps2x's
	// g_vWorldToViewWater0), along which the reflection is offset.
	float water[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
	// The view's viewport in the target: x, y, 1 / width, 1 / height (a
	// fragment's position in the view, where the water point samples its
	// reflection target).
	float viewport[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	SurfaceAreaLight areas[kSurfaceMaxAreaLights];
	float clipPlanes[6][4] = {};
	float motionCurrentToClip[16] = {};
	float motionPreviousToClip[16] = {};
	float motionExtent[4] = {};
	// Client-owned horizontal wind and animation clock: xy wind, z time.
	// Previous sample is captured with the same frame as the previous view.
	float foliage[2][4] = {};
};
static_assert( sizeof( SurfaceFrame ) == 448 + 64 * kSurfaceMaxAreaLights );

// The flat vertex (surface_flat.vert): position, base and lightmap
// coordinates, and color as UNORM8x4 (RGBA).
struct SurfaceFlatVertex
{
	float position[3] = {};
	float uv[2] = {};
	float lightmapUv[2] = {};
	std::uint8_t color[4] = { 255, 255, 255, 255 };
};
static_assert( sizeof( SurfaceFlatVertex ) == 32 );

// The world vertex (surface_world.vert): the flat vertex, then the world
// normal, tangents S and T, and the bumped lightmap pages' offset in the page
// (lightmappedgeneric_vs20's TEXCOORD2.x: the flat page's width, as a
// coordinate), read with tangent T as one four-component attribute.
struct SurfaceWorldVertex
{
	float position[3] = {};
	float uv[2] = {};
	float lightmapUv[2] = {};
	std::uint8_t color[4] = { 255, 255, 255, 255 };
	float normal[3] = { 0.0f, 0.0f, 1.0f };
	float tangentS[3] = { 1.0f, 0.0f, 0.0f };
	float tangentT[3] = { 0.0f, 1.0f, 0.0f };
	float lightmapOffset = 0.0f;
};
static_assert( sizeof( SurfaceWorldVertex ) == 72 );

// The model vertex (surface_model.vert): position, normal, tangent (w: the
// bitangent's sign) and uv0 of a mesh, in object space.
struct SurfaceModelVertex
{
	float position[3] = {};
	float normal[3] = {};
	float tangent[4] = {};
	float uv[2] = {};
};
static_assert( sizeof( SurfaceModelVertex ) == 48 );

enum class SurfaceVertexLayout : std::uint8_t
{
	kFlat,  // SurfaceFlatVertex: the terms that read no normal
	kWorld, // SurfaceWorldVertex: every term
	kModel  // SurfaceModelVertex: every term; no lightmap coordinates
};

std::uint32_t SurfaceVertexStride( SurfaceVertexLayout layout );
// The FamilyDrawConstants prefix the layout's vertex stage reads.
std::uint32_t SurfaceDrawConstantBytes( SurfaceVertexLayout layout );

// The terms (surface.frag's kTerms bits, the port's static combo bits where
// they exist).
// The unlit point: the lighting fixed at one, UnlitGeneric's vertex color
// and alpha.
inline constexpr std::uint32_t kSurfaceUnlit = 1;
inline constexpr std::uint32_t kSurfaceDetail = 2;
inline constexpr std::uint32_t kSurfaceBump = 4; // the pbr point's normal map too
inline constexpr std::uint32_t kSurfaceSsbump = 8;
inline constexpr std::uint32_t kSurfaceHalfLambert = 16; // with kSurfaceVertexLit
inline constexpr std::uint32_t kSurfaceEnvmap = 32;
inline constexpr std::uint32_t kSurfaceEnvmapMask = 64;
inline constexpr std::uint32_t kSurfaceBaseAlphaEnvmapMask = 128;
inline constexpr std::uint32_t kSurfaceSelfIllum = 256;
inline constexpr std::uint32_t kSurfaceNormalMapAlphaEnvmapMask = 512;
inline constexpr std::uint32_t kSurfaceDiffuseBump = 1024;
inline constexpr std::uint32_t kSurfacePbr = 2048;
inline constexpr std::uint32_t kSurfaceEmissionTexture = 4096;
// The vertexlit point: Source's per-vertex model lighting (the model vertex).
inline constexpr std::uint32_t kSurfaceVertexLit = 8192;
// The view's clustered runtime lights (the view group), both lobes.
inline constexpr std::uint32_t kSurfaceClustered = 16384;
// The pbr point on a world surface: the indirect diffuse is the draw's
// lightmap basis (render/shaders/common/lightmap_basis.glsl) in place of the
// ambient cube; with kSurfaceDirectionalLightmap the draw group's gradient
// page is read too (a directional LMAP page's right half).
inline constexpr std::uint32_t kSurfaceBakedLightmap = 32768;
inline constexpr std::uint32_t kSurfaceDirectionalLightmap = 65536;
// The pbr point with the map's probe volume (indirect diffuse where no
// lightmap is read) and reflection probes (the specular image light): the
// frame group's SurfaceMapTextures.
inline constexpr std::uint32_t kSurfaceProbeVolume = 131072;
inline constexpr std::uint32_t kSurfaceReflectionProbes = 262144;
// Ambient occlusion (render.pass.ao): the view's occlusion multiplies the
// indirect light only, and derives the specular occlusion; on a world
// surface it darkens the bake's indirect layer (the draw group's third page),
// never its direct light.
inline constexpr std::uint32_t kSurfaceAmbientOcclusion = 524288;
// Screen-space reflection targets (render.pass.ssr): attachments 1 to 3
// hold the octahedral normal and roughness, the image-specular radiance and
// its weight, as ssr.h names them.
inline constexpr std::uint32_t kSurfaceSsrTargets = 1048576;
// The depth-and-normal prepass: attachment 0 holds the octahedral normal and
// roughness only.
inline constexpr std::uint32_t kSurfaceDepthNormal = 2097152;
// The projected lights' bounce (the frame group's bounce atlas, a probe
// atlas of the probe volume's layout sampled with its weights), added to the
// indirect diffuse light of every surface.
inline constexpr std::uint32_t kSurfaceProbeBounce = 4194304;
// A dynamic mesh's direct light: a surface without a lightmap reads the
// probe volume's indirect layer (when it has one) and takes every light's
// diffuse lobe at runtime, shadowed, baked lights included.
inline constexpr std::uint32_t kSurfaceMeshDirect = 8388608;
// A reflective shadow map's pass (render.pass.bounce): attachment 0 holds
// the surface's diffuse reflectance (RGBA16F).
inline constexpr std::uint32_t kSurfaceRsm = 16777216;
// The water point (water_family.h): Portal 2's water_ps2x above water, with
// the material group's flow map, flow noise and reflection target.
inline constexpr std::uint32_t kSurfaceWater = 33554432;
// A depth pass into the lit pass's own targets (with kSurfaceDepthNormal's
// fragment program, so alpha tests discard as the lit pass does): attachment
// 0 is the target's color format with nothing written, and depth is written
// for opaque programs. Drawn before the lit surfaces, the lit pass's depth
// test (less-equal) then shades each pixel once (Doom 2016's prepass).
inline constexpr std::uint32_t kSurfaceDepthOnly = 67108864;
// Runtime direct light on a world surface (RFC 0016's runtime direct light,
// id Tech's split): with kSurfaceBakedLightmap the lightmap basis reads the
// bake's indirect layer (the draw group's third page, directional with the
// gradient page the group then binds, the indirect layer's own), and every
// light's direct light is drawn at runtime, shadowed, baked lights included.
// The draw's page stays the bake's total layer (the sun's baked mask).
inline constexpr std::uint32_t kSurfaceRuntimeDirect = 134217728;
// The PBR point reads its MRAO texture; absent for an imported dielectric
// whose constant metalness, roughness and AO are in SurfaceConstants.
inline constexpr std::uint32_t kSurfaceMraoTexture = 268435456;
// A VertexLit mesh uses its separate RGB self-illumination mask in the
// material emission slot, instead of base alpha.
inline constexpr std::uint32_t kSurfaceSelfIllumMask = 536870912;
// VertexLit's Phong exponent texture uses the material mask slot. Its red
// channel supplies the exponent when the constant is zero; green controls
// albedo tint and alpha may mask the rim.
inline constexpr std::uint32_t kSurfacePhongExponentTexture = 1073741824;
// Thin dielectric transmission from the view's scene color (binding 14).
inline constexpr std::uint32_t kSurfaceTransmission = 0x80000000u;
// The terms that read the normal (not on the flat vertex), and those the
// model vertex alone evaluates.
inline constexpr std::uint32_t kSurfaceNormalTerms =
    kSurfaceBump | kSurfaceSsbump | kSurfaceEnvmap | kSurfacePbr | kSurfaceVertexLit;
inline constexpr std::uint32_t kSurfaceModelTerms = kSurfaceVertexLit;
// The terms that read the lightmap coordinates (not on the model vertex).
inline constexpr std::uint32_t kSurfaceLightmapTerms =
    kSurfaceBakedLightmap | kSurfaceDirectionalLightmap | kSurfaceRuntimeDirect;
// The terms that read the map's probes (the pbr point's).
inline constexpr std::uint32_t kSurfaceMapProbeTerms =
    kSurfaceProbeVolume | kSurfaceReflectionProbes;

// The map's textures a frame group names (TextureCache names; empty when
// the map has none, which the terms must then not ask for): the PRBV atlas
// (RGBA16F) and grid table (RGBA32F, WriteProbeGridTable), and the RPRB
// texture (RGBA16F, WriteReflectionProbeTexture).
struct SurfaceMapTextures
{
	std::string probeAtlas;
	std::string probeGrids;
	std::string reflectionProbes;
	// The projected lights' bounce atlas (kSurfaceProbeBounce): a texture
	// the owner made (render.pass.bounce), bound as it is.
	device::TextureId probeBounce;
	device::TextureDesc probeBounceDesc;
};

// Ordered view/portal state, independent of material semantics. Default state
// preserves the material's depth policy. Portal masks can override it explicitly.
struct SurfaceDrawState
{
	device::StencilState stencil;
	device::CullMode cull = device::CullMode::kNone;
	bool overrideDepth = false;
	bool depthTest = true;
	bool depthWrite = true;
	device::CompareOp depthCompare = device::CompareOp::kLessEqual;
	std::uint8_t colorWrite = device::kColorWriteAll;
	float depthBiasConstant = 0.0f;
	float depthBiasSlope = 0.0f;
	auto operator<=>( const SurfaceDrawState & ) const = default;
};

// Material predicates whose values belong to the immutable material request.
// Raw Pipeline callers may retain uniform evaluation; Request derives these
// from the same constants it uploads. Authored warp materials retain uniform
// evaluation for image compatibility. Numeric parameters remain uniforms.
inline constexpr std::uint32_t kSurfaceDynamicMaterialFeatures = 0xffffffffu;
inline constexpr std::uint32_t kSurfaceMaterialAlphaTest = 1u;
inline constexpr std::uint32_t kSurfaceMaterialHalfLambert = 2u;
inline constexpr std::uint32_t kSurfaceMaterialDiffuseWarp = 4u;
inline constexpr std::uint32_t kSurfaceMaterialSpecularWarp = 8u;
inline constexpr std::uint32_t kSurfaceMaterialUnlitMesh = 16u;
std::uint32_t SurfaceMaterialFeatures( const SurfaceConstants &constants );

// A view may remove only absent lighting families. The default supports every
// family, including raw lab users whose bindings vary between draws.
inline constexpr std::uint32_t kSurfaceViewSun = 1u;
inline constexpr std::uint32_t kSurfaceViewProjectors = 2u;
inline constexpr std::uint32_t kSurfaceViewAreas = 4u;
// The view's user clip planes (FrameTerms::clipPlanes, a water or portal
// view's). Without it the fragment's clip test, whose discard keeps the
// driver from testing depth early, is compiled out; only a view whose planes
// are all zero (which never clip) may omit it.
inline constexpr std::uint32_t kSurfaceViewClipPlanes = 8u;
inline constexpr std::uint32_t kSurfaceAllViewFeatures = 15u;

// One point of the program: its pipeline state and specialization.
struct SurfaceVariant
{
	device::BlendMode blend = device::BlendMode::kOpaque;
	// Whether the draw writes destination alpha: the port leaves it for
	// translucent and alpha-tested draws (write mask, clause D17).
	bool alphaWrite = true;
	std::uint32_t terms = 0;      // kSurface* bits
	std::uint32_t detailMode = 0; // $detailblendmode, with kSurfaceDetail
	SurfaceVertexLayout layout = SurfaceVertexLayout::kFlat;
	bool ignoreDepth = false;
	SurfaceDrawState drawState{};
	bool portalMask = false;
	bool temporal = false;
	std::uint32_t materialFeatures = kSurfaceDynamicMaterialFeatures;
	std::uint32_t viewFeatures = kSurfaceAllViewFeatures;
	std::uint32_t treeSwayMode = 0; // vertex specialization; 0 has no deformation cost
	bool alphaToCoverage = false;   // authored request; effective only with multiple samples
	bool decalModulate = false;     // dimensionless factors, no exposure or output encoding
	bool cable = false;             // fragment specialization: expanded Source rope ribbon
	// A single-sample shadow atlas: the same vertex deformation and alpha
	// coverage as the visible point, with no color attachments or lighting.
	bool shadowDepth = false;

	auto operator<=>( const SurfaceVariant & ) const = default;
	bool operator==( const SurfaceVariant & ) const = default;
};

// A point's textures by TextureCache name (empty when absent: the neutral
// texture). Base, env map and emission are gamma images (sRGB views; an HDR
// cube holds linear light and passes as it is); the mask, the bump or normal
// map and MRAO are data. A detail texture reads through sRGB only in mode 1
// (additive): the other modes combine its gamma values as they are
// (lightmappedgeneric_dx9_helper.cpp: EnableSRGBRead( SAMPLER12, mode == 1 )).
struct SurfaceTextures
{
	std::string base;
	std::string envmap; // a cube map
	std::string envmapMask; // or VertexLit's Phong exponent texture
	std::string bump;
	std::string detail;
	std::string mrao;     // or VertexLit's light-warp lookup (no MRAO term on that point)
	std::string emission; // or VertexLit's self-illumination mask
	// The water point's flow map and flow noise (data), bound in the env map
	// mask's and MRAO's places (the water point reads neither).
	std::string flowmap;
	std::string flowNoise;
	bool baseSrgb = true; // Sprite's $nosrgb selects an undecoded base image
};

// The split-sum table (RFC 0007, pbr_split_sum_table.h): the texels for a
// kRGBA32Float texture sampled linearly with clamped addressing, which the
// frame group names for the pbr point.
struct PbrSplitSumTable
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	device::Format format = device::Format::kRGBA32Float;
	std::vector<float> texels; // RGBA per texel
};
PbrSplitSumTable SplitSumTable();
// The GGX linearly-transformed-cosine table (public/render/pbr_ltc_table.h)
// in the same form: kRGBA32Float, sampled linearly with clamped addressing,
// named by the frame group for the pbr point's area lights.
PbrSplitSumTable LtcTable();

enum class SurfaceStatus : std::uint8_t
{
	kDevice = 1, // a layout or pipeline was refused
	// the variant's terms read the normal and the layout is flat, or they
	// need the model vertex and the layout is another
	kInvalidRequest
};

class SurfaceProgram
{
public:
	// Replacement color/depth fragments for seeded suites, borrowed until this
	// program dies. Empty spans use the program's own entry points.
	static foundation::Expected<std::unique_ptr<SurfaceProgram>, SurfaceStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
	    std::uint32_t sampleCount = 1, std::span<const std::uint32_t> fragmentModule = {},
	    std::span<const std::uint32_t> shadowFragmentModule = {} );
	~SurfaceProgram();
	SurfaceProgram( const SurfaceProgram & ) = delete;
	SurfaceProgram &operator=( const SurfaceProgram & ) = delete;

	device::BindGroupLayoutId FrameLayout() const { return m_FrameLayout; }
	device::BindGroupLayoutId ViewLayout() const { return m_ViewLayout; }
	device::BindGroupLayoutId MaterialLayout() const { return m_MaterialLayout; }
	device::BindGroupLayoutId DrawLayout() const { return m_DrawLayout; }

	// The pipeline for a variant (created on first use). With a debug
	// specialization (RFC 0014) that is not neutral, the same program with
	// the debug constants.
	foundation::Expected<device::PipelineId, SurfaceStatus> Pipeline(
	    const SurfaceVariant &variant, const shaderlib::DebugSpecialization &debug = {} );
	// The debug variant of a pipeline this program made (Pipeline with a
	// neutral specialization); kInvalidRequest when it did not make it. A
	// neutral specialization returns the pipeline itself.
	foundation::Expected<device::PipelineId, SurfaceStatus> DebugPipeline(
	    device::PipelineId shipped, const shaderlib::DebugSpecialization &debug );
	foundation::Expected<device::PipelineId, SurfaceStatus> StatePipeline(
	    device::PipelineId shipped, const SurfaceDrawState &state,
	    const shaderlib::DebugSpecialization &debug = {} );
	// State and lighting presence for one view, retaining the material features
	// of the shipped pipeline. Reappearing lights select another cached variant.
	foundation::Expected<device::PipelineId, SurfaceStatus> ViewPipeline(
	    device::PipelineId shipped, const SurfaceDrawState &state, std::uint32_t viewFeatures,
	    const shaderlib::DebugSpecialization &debug = {} );
	// Render-sequence diagnostic for the most recent refused pipeline. The
	// device's operation, status and native code survive the material boundary.
	const std::string &PipelineFailure() const { return m_PipelineFailure; }
	// A shipped pipeline's variant with terms added and removed (a pass's
	// variant: the prepass, the SSR targets); kInvalidRequest when this
	// program did not make it.
	foundation::Expected<device::PipelineId, SurfaceStatus> VariantPipeline(
	    device::PipelineId shipped, std::uint32_t add, std::uint32_t remove );
	// A single-sample D32 atlas pipeline, independent of the color target's
	// sample count and depth format. Refuses blended/transmitting points. The
	// caller retains the point's material/frame/draw groups and vertex layout.
	foundation::Expected<device::PipelineId, SurfaceStatus> ShadowPipeline(
	    device::PipelineId shipped );
	// A point as a MaterialPrograms request: the pipeline, the frame and draw
	// layouts, and the material group (the constants, with state.x set from
	// the variant, and the seven textures, one sampler description for all).
	foundation::Expected<ProgramRequest, SurfaceStatus> Request( const SurfaceVariant &variant,
	    const SurfaceConstants &constants, const SurfaceTextures &textures,
	    const device::SamplerDesc &sampler = {} );
	// The frame group: the terms, the split-sum table ('splitSumTable', a
	// TextureCache name of a texture holding SplitSumTable()) and the LTC
	// table ('ltcTable', one holding LtcTable()), each with a linear,
	// clamped sampler; a table no point of the frame reads is named empty.
	foundation::Expected<device::PipelineId, SurfaceStatus> TemporalPipeline(
	    device::PipelineId shipped, const SurfaceDrawState &state, std::uint32_t viewFeatures,
	    const shaderlib::DebugSpecialization &debug = {} );

	GroupRequest FrameGroup( const SurfaceFrame &frame, std::string splitSumTable = {},
	    std::string ltcTable = {}, const SurfaceMapTextures &map = {} ) const;
	// A view group: the view's parameters, its froxels' ranges (FroxelRange
	// records), its index list (ClusterIndexHeader then indices) and its
	// light records, as render.pass.lights lays them out.
	GroupRequest ViewGroup( const SurfaceViewGpu &view, std::span<const std::byte> froxels,
	    std::span<const std::byte> indices, std::span<const SurfaceLightGpu> lights,
	    const SurfaceShadows &shadows = {}, const SurfaceProjectors &projectors = {},
	    const SurfaceScreenInputs &screen = {} ) const;
	// The view group of a view with no clustered lights (and its screen
	// inputs, when a point reads one).
	GroupRequest NeutralViewGroup( const SurfaceScreenInputs &screen = {} ) const;
	// A draw group: the lightmap page ('page', a TextureCache name staged as
	// sRGB; empty for a mesh), the draw's model lighting and a directional
	// page's gradient page ('gradient', read under
	// kSurfaceDirectionalLightmap; empty otherwise).
	// 'indirect' names the bake's indirect layer of the same page
	// (kSurfaceAmbientOcclusion on a world surface; empty otherwise).
	GroupRequest DrawGroup( std::string page, const ModelLighting &lighting = {},
	    const device::SamplerDesc &sampler = {}, std::string gradient = {},
	    std::string indirect = {} ) const;

private:
	explicit SurfaceProgram( device::IRenderDevice2 &device ) : m_Device( device ) {}

	device::IRenderDevice2 &m_Device;
	device::Format m_ColorFormat = device::Format::kUnknown;
	device::Format m_DepthFormat = device::Format::kUnknown;
	std::uint32_t m_SampleCount = 1;
	device::BindGroupLayoutId m_FrameLayout;
	device::BindGroupLayoutId m_ViewLayout;
	device::BindGroupLayoutId m_MaterialLayout;
	device::BindGroupLayoutId m_DrawLayout;
	std::span<const std::uint32_t> m_FragmentModule;
	std::span<const std::uint32_t> m_ShadowFragmentModule;
	std::string m_PipelineFailure;
	std::map<std::pair<SurfaceVariant, shaderlib::DebugSpecialization>, device::PipelineId>
	    m_Pipelines;
	// The variant behind each shipped (neutral) pipeline, for DebugPipeline.
	std::map<std::uint64_t, SurfaceVariant> m_Shipped;
};

// A family's view of the program: the family's claims are drawn as the
// program's points. It owns its program, or borrows one that outlives it (a
// root that draws several families with one set of layouts).
class SurfaceFamily
{
public:
	explicit SurfaceFamily( std::unique_ptr<SurfaceProgram> program )
	    : m_Owned( std::move( program ) ), m_Program( m_Owned.get() )
	{
	}
	explicit SurfaceFamily( SurfaceProgram &program ) : m_Program( &program ) {}
	SurfaceFamily( const SurfaceFamily & ) = delete;
	SurfaceFamily &operator=( const SurfaceFamily & ) = delete;

	SurfaceProgram &Program() const { return *m_Program; }
	device::BindGroupLayoutId FrameLayout() const { return m_Program->FrameLayout(); }
	device::BindGroupLayoutId ViewLayout() const { return m_Program->ViewLayout(); }
	device::BindGroupLayoutId MaterialLayout() const { return m_Program->MaterialLayout(); }
	device::BindGroupLayoutId DrawLayout() const { return m_Program->DrawLayout(); }
	foundation::Expected<device::PipelineId, SurfaceStatus> DebugPipeline(
	    device::PipelineId shipped, const shaderlib::DebugSpecialization &debug ) const
	{
		return m_Program->DebugPipeline( shipped, debug );
	}
	GroupRequest FrameGroup( const SurfaceFrame &frame = {}, std::string splitSumTable = {},
	    std::string ltcTable = {} ) const
	{
		return m_Program->FrameGroup( frame, std::move( splitSumTable ), std::move( ltcTable ) );
	}
	// The view group of a view with no clustered lights.
	GroupRequest NeutralViewGroup() const { return m_Program->NeutralViewGroup(); }
	// A draw group of a mesh: no lightmap page, the draw's model lighting
	// (PackSourceModelLighting).
	GroupRequest LightingGroup( const ModelLighting &lighting ) const
	{
		return m_Program->DrawGroup( {}, lighting );
	}

private:
	std::unique_ptr<SurfaceProgram> m_Owned;
	SurfaceProgram *m_Program = nullptr;
};

// A family that owns a new program.
template <typename Family>
foundation::Expected<std::unique_ptr<Family>, SurfaceStatus> CreateSurfaceFamily(
    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
    std::uint32_t sampleCount = 1, std::span<const std::uint32_t> fragmentModule = {},
    std::span<const std::uint32_t> shadowFragmentModule = {} )
{
	auto program = SurfaceProgram::Create(
	    device, colorFormat, depthFormat, sampleCount, fragmentModule, shadowFragmentModule );
	if ( !program )
		return foundation::MakeUnexpected( program.Error() );
	return std::make_unique<Family>( std::move( program ).Value() );
}

} // namespace render::material

#endif // RENDER_MATERIAL_SURFACE_PROGRAM_H
