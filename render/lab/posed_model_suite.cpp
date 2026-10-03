//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's posed-model fixture: the world pass draws one shared
//			mesh at two captured poses through the mesh PBR program, with a
//			live area light. Pixel coverage and light response are independent
//			checks on the pass that the game will use for animated doors.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/pass/world/world_pass.h"
#include "render/graph/scene_color.h"
#include "render/material/program_resolver.h"
#include "render/material/vmt_import.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace render::lab
{

namespace
{

using namespace render::device;
using namespace render::pass::world;

constexpr std::uint32_t kSize = 64;

class EmptyTextures final : public IWorldTextures
{
public:
	TextureId normalFixture;
	TextureId phongWarpFixture;
	TextureId refractNormalFixture;
	TextureId refractWarpFixture;
	TextureId twoTextureBase;
	TextureId twoTextureOverlay;
	TextureId cutoutFixture;
	TextureId Import( int handle, bool ) override
	{
		return handle == 1   ? normalFixture
		       : handle == 2 ? phongWarpFixture
		       : handle == 3 ? refractNormalFixture
		       : handle == 4 ? refractWarpFixture
		       : handle == 5 ? twoTextureBase
		       : handle == 6 ? twoTextureOverlay
		       : handle == 7 ? cutoutFixture
		                     : TextureId{};
	}
	SamplerDesc Sampler( int handle ) override
	{
		SamplerDesc sampler;
		if ( handle == 6 || handle == 7 )
		{
			sampler.address = AddressMode::kClampToEdge;
			sampler.minFilter = sampler.magFilter = Filter::kNearest;
		}
		return sampler;
	}
};

class LabSceneColorCapture final : public IWorldSceneColorCapture
{
public:
	explicit LabSceneColorCapture( IRenderDevice2 &device ) : m_Device( device ) {}
	~LabSceneColorCapture() override
	{
		(void)m_Device.WaitIdle();
		for ( graph::InlineGraphResources &capture : m_Captures )
			capture.Release( m_Device, CompletionToken() );
	}
	std::optional<WorldSceneColor> Capture( IRenderDevice2 &device, CommandEncoder &encoder,
	    TextureId source, const TextureDesc &sourceDesc, std::uint64_t ) override
	{
		if ( &device != &m_Device )
			return std::nullopt;
		auto captured = graph::RecordSceneColor( device, encoder, source, sourceDesc );
		if ( !captured )
			return std::nullopt;
		graph::RecordedSceneColor value = std::move( captured ).Value();
		WorldSceneColor result{ value.texture, value.desc };
		m_Captures.push_back( std::move( value.resources ) );
		return result;
	}

private:
	IRenderDevice2 &m_Device;
	std::vector<graph::InlineGraphResources> m_Captures;
};

WorldData MeshWorld( bool blended = false )
{
	WorldData world;
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = std::move( stage );
	WorldMaterial material;
	material.name = "posed-door-fixture";
	material.shader = "VertexLitGeneric";
	material.mesh = true;
	if ( blended )
	{
		material.variables.push_back( { "$translucent", "1" } );
		material.variables.push_back( { "$alpha", "0.5" } );
	}
	world.materials.push_back( std::move( material ) );
	WorldData::StaticMesh mesh;
	for ( const auto &xy : { std::pair{ -0.5f, -0.5f }, std::pair{ 0.5f, -0.5f },
	          std::pair{ 0.5f, 0.5f }, std::pair{ -0.5f, 0.5f } } )
	{
		material::SurfaceModelVertex vertex;
		vertex.position[0] = xy.first;
		vertex.position[1] = xy.second;
		vertex.position[2] = 0.5f;
		vertex.normal[2] = 1.0f;
		vertex.tangent[0] = vertex.tangent[3] = 1.0f;
		mesh.vertices.push_back( vertex );
	}
	mesh.indices = { 0, 1, 2, 0, 2, 3 };
	mesh.surfaces.push_back( { 0, 0, 0, 6 } );
	world.staticMeshes.push_back( std::move( mesh ) );
	return world;
}

WorldData MixedWorld()
{
	WorldData world = MeshWorld();
	WorldMaterial glass = world.materials.front();
	glass.name = "posed-glass-fixture";
	glass.translucent = true;
	glass.variables.push_back( { "$translucent", "1" } );
	glass.variables.push_back( { "$alpha", "0.5" } );
	world.materials.push_back( std::move( glass ) );
	world.staticMeshes[0].surfaces.push_back( { 1, 0, 0, 6 } );
	return world;
}

float Sum( const CanvasImage &image, std::uint32_t low, std::uint32_t high )
{
	float sum = 0.0f;
	for ( std::uint32_t y = 0; y < image.height; ++y )
		for ( std::uint32_t x = low; x < high; ++x )
			sum += image.At( x, y )[0];
	return sum;
}

std::optional<std::string> RunChecks(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( std::optional<std::string> why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	LabSceneColorCapture sceneColorCapture( *device );
	WorldData world = MeshWorld();
	const auto tubeGlass = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_backstage/vacum_pipe_glass" },
	        { "$bumpmap", "models/props_backstage/vacum_pipe_glass_normal" },
	        { "$translucent", "1" }, { "$envmap", "env_cubemap" },
	        { "$envmaptint", "[.85 .85 .85]" }, { "$basemapalphaenvmapmask", "1" },
	        { "$envmapfresnel", "1" }, { "$invertphongmask", "1" }, { "$halflambert", "1" },
	        { "$phong", "1" }, { "$phongexponent", "50" }, { "$phongboost", "1" },
	        { "$phongfresnelranges", "[.3 .6 4]" } },
	    {} );
	results.That(
	    tubeGlass && material::ClaimForMesh( tubeGlass.Value(), true ) &&
	        material::ClaimForMesh( tubeGlass.Value(), true ).Value() == BlendMode::kAlpha,
	    "posed-model.tube-glass-claims-native-probes" );
	results.That( tubeGlass && !material::ClaimForMesh( tubeGlass.Value(), false ),
	    "posed-model.tube-glass-requires-native-probes" );
	const auto fracturedGlass = material::MapVariables( "Refract_DX90",
	    { { "$model", "1" }, { "$normalmap", "models/props_destruction/glass_fracture_B_normal" },
	        { "$dudvmap", "models/props_destruction/glass_fracture_B_normal" },
	        { "$scale", "[1 1]" }, { "$refractamount", ".07" }, { "$bluramount", ".3" },
	        { "$refracttint", "{235 247 247}" }, { "$translucent", "1" },
	        { "$envmap", "env_cubemap" }, { "$envmapcontrast", "1" },
	        { "$envmapsaturation", "[1 1 1]" }, { "$envmaptint", "[.71 .79 .85]" } },
	    {} );
	results.That( fracturedGlass && !material::ClaimForMesh( fracturedGlass.Value(), true ),
	    "posed-model.refract-refuses-missing-scene-color" );
	results.That( fracturedGlass && !material::ClaimForMesh( fracturedGlass.Value(), false, true ),
	    "posed-model.refract-refuses-missing-native-probes" );
	results.That( fracturedGlass && material::ClaimForMesh( fracturedGlass.Value(), true, true ),
	    "posed-model.refract-authored-model-claims-with-scene-color" );
	auto refractResolver = material::ProgramResolver::Create(
	    *device, kCanvasColor, kCanvasDepth, 1, material::VertexLayout::kModel );
	bool refractPipeline = bool( refractResolver ) && bool( fracturedGlass );
	if ( refractPipeline )
	{
		refractResolver.Value()->SetWorldPbr( true, material::kSurfaceReflectionProbes );
		refractResolver.Value()->SetSceneColorAvailable( true );
		refractPipeline = bool( refractResolver.Value()->ResolveMesh( fracturedGlass.Value() ) );
	}
	results.That( refractPipeline, "posed-model.refract-builds-shared-surface-pipeline" );
	const auto unsupportedRefract = material::MapVariables( "Refract_DX90",
	    { { "$normalmap", "glass/normal" }, { "$normalmap2", "glass/other_normal" } }, {} );
	results.That(
	    unsupportedRefract && !material::ClaimForMesh( unsupportedRefract.Value(), true, true ),
	    "posed-model.refract-secondary-normal-stays-legacy" );
	const auto tubeBlades = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/elevator/elevator_blades" }, { "$phong", "1" },
	        { "$phongboost", "2" }, { "$phongdisablehalflambert", "1" }, { "$phongexponent", "12" },
	        { "$phongfresnelranges", "[.6 1 2]" }, { "$basemapalphaphongmask", "1" },
	        { "$rimlight", "1" }, { "$rimlightexponent", "0.2" }, { "$rimlightboost", "3" },
	        { "$phongalbedotint", "1" } },
	    {} );
	results.That(
	    tubeBlades && material::ClaimForMesh( tubeBlades.Value(), true ) &&
	        material::ClaimForMesh( tubeBlades.Value(), true ).Value() == BlendMode::kOpaque,
	    "posed-model.tube-blades-claim-shared-pbr-point" );
	const auto mappedExponent = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/door_02" }, { "$phong", "1" }, { "$phongexponent", "0" },
	        { "$phongexponenttexture", "models/props/door_02_exponent" },
	        { "$phongalbedotint", "1" } },
	    {} );
	results.That( mappedExponent && material::ClaimForMesh( mappedExponent.Value(), true ),
	    "posed-model.phong-exponent-map-claims-shared-pbr-point" );
	const auto doubleSidedProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/saucepan/saucepan" }, { "$envmap", "env_cubemap" },
	        { "$basealphaenvmapmask", "1" }, { "$normalmapalphaenvmapmask", "1" },
	        { "$nocull", "1" }, { "$bumpmap", "models/props/saucepan/saucepan_normal" } },
	    {} );
	results.That(
	    doubleSidedProbeMask && material::ClaimForMesh( doubleSidedProbeMask.Value(), true ),
	    "posed-model.double-sided-probe-mask-claims-native-probes" );
	const auto textureProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" }, { "$envmap", "env_cubemap" },
	        { "$envmapmask", "models/props_office/door_glass_mask" } },
	    {} );
	results.That( textureProbeMask && material::ClaimForMesh( textureProbeMask.Value(), true ) &&
	                  !material::ClaimForMesh( textureProbeMask.Value(), false ),
	    "posed-model.authored-rgb-mask-claims-native-rprb" );
	const auto conflictingProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" },
	        { "$envmapmask", "models/props_office/door_glass_mask" }, { "$phong", "1" },
	        { "$phongexponenttexture", "models/props/door_02_exponent" } },
	    {} );
	results.That(
	    conflictingProbeMask && !material::ClaimForMesh( conflictingProbeMask.Value(), true ),
	    "posed-model.probe-mask-and-phong-exponent-need-distinct-slots" );
	const auto bumpedProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" }, { "$envmap", "env_cubemap" },
	        { "$envmapmask", "models/props_office/door_glass_mask" },
	        { "$bumpmap", "models/props_office/door_glass_normal" } },
	    {} );
	results.That( bumpedProbeMask && material::ClaimForMesh( bumpedProbeMask.Value(), false ),
	    "posed-model.bumped-envmap-mask-clears-legacy-probe" );
	const auto normalAlphaProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" }, { "$envmap", "env_cubemap" },
	        { "$envmapmask", "models/props_office/door_glass_mask" },
	        { "$bumpmap", "models/props_office/door_glass_normal" },
	        { "$normalmapalphaenvmapmask", "1" } },
	    {} );
	results.That( normalAlphaProbeMask &&
	                  !material::ClaimForMesh( normalAlphaProbeMask.Value(), false ) &&
	                  material::ClaimForMesh( normalAlphaProbeMask.Value(), true ),
	    "posed-model.normal-alpha-mask-preserves-legacy-probe" );
	const auto bumpedBaseAlphaProbe = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" }, { "$envmap", "env_cubemap" },
	        { "$bumpmap", "models/props_office/door_glass_normal" },
	        { "$basealphaenvmapmask", "1" } },
	    {} );
	results.That(
	    bumpedBaseAlphaProbe && material::ClaimForMesh( bumpedBaseAlphaProbe.Value(), false ),
	    "posed-model.bumped-base-alpha-mask-clears-legacy-probe" );
	const auto paintedTurret = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/npcs/turret/turret_01" },
	        { "$detail", "models/npcs/turret/turret_paint_blue" }, { "$detailscale", "1" },
	        { "$detailblendmode", "2" }, { "$detailblendfactor", "0.5" } },
	    {} );
	results.That( paintedTurret && material::ClaimForMesh( paintedTurret.Value(), true ),
	    "posed-model.detail-paint-uses-shared-surface-combine" );
	const auto alphaTint = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_bts/aluminium_extrusion" }, { "$color", "[.5 .7 1]" },
	        { "$blendtintbybasealpha", "1" } },
	    {} );
	results.That( alphaTint && material::ClaimForMesh( alphaTint.Value(), true ),
	    "posed-model.base-alpha-tint-claims-shared-pbr-point" );
	const auto color2Tint = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/player/chell/chell_head_diffuse" },
	        { "$color2", "[.25 .5 1]" } },
	    {} );
	results.That( color2Tint && material::ClaimForMesh( color2Tint.Value(), true ),
	    "posed-model.color2-factor-claims-shared-pbr-point" );
	const auto editorArm = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_map_editor/arm_64x64" }, { "$glowcolor", "1" } }, {} );
	results.That( editorArm && material::ClaimForMesh( editorArm.Value(), true ),
	    "posed-model.vertexlit-ignores-unlit-only-glow-key" );
	const auto authoredProbeFresnel = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/player/coop_bots/assembly_machine_glass" },
	        { "$envmap", "env_cubemap" }, { "$envmapfresnel", "1" },
	        { "$envmapfresnelminmaxexp", "[.1 1 .7]" } },
	    {} );
	results.That(
	    authoredProbeFresnel && material::ClaimForMesh( authoredProbeFresnel.Value(), true ),
	    "posed-model.authored-probe-fresnel-uses-native-rprb" );
	const auto lightScaledProbe = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/a4_destruction/backpanels_color01b_vtx" },
	        { "$envmap", "env_cubemap" }, { "$envmaplightscale", ".6" } },
	    {} );
	results.That( lightScaledProbe && material::ClaimForMesh( lightScaledProbe.Value(), true ),
	    "posed-model.probe-light-scale-uses-native-irradiance" );
	const auto additiveGlass = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/reflecto_cube_glow" }, { "$additive", "1" } }, {} );
	results.That(
	    additiveGlass && material::ClaimForMesh( additiveGlass.Value(), true ) &&
	        material::ClaimForMesh( additiveGlass.Value(), true ).Value() == BlendMode::kAdditive,
	    "posed-model.additive-mesh-uses-shared-surface-blend" );
	const auto warpedLight = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/npcs/turret/turret_01" }, { "$phong", "1" },
	        { "$lightwarptexture", "models/npcs/turret/turret_lightwarp" } },
	    {} );
	results.That( warpedLight && material::ClaimForMesh( warpedLight.Value(), true ),
	    "posed-model.light-warp-uses-shared-direct-diffuse" );
	const auto warpedSpecular = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/reflecto_cube_glass" }, { "$phong", "1" },
	        { "$phongexponent", "2" },
	        { "$phongwarptexture", "models/props/reflecto_cube_iridescence" } },
	    {} );
	results.That( warpedSpecular && material::ClaimForMesh( warpedSpecular.Value(), true ),
	    "posed-model.phong-warp-claims-shared-pbr-specular" );
	const auto conflictingWarps = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/reflecto_cube_glass" }, { "$phong", "1" },
	        { "$phongwarptexture", "models/props/reflecto_cube_iridescence" },
	        { "$lightwarptexture", "models/npcs/turret/turret_lightwarp" } },
	    {} );
	results.That( conflictingWarps && !material::ClaimForMesh( conflictingWarps.Value(), true ),
	    "posed-model.two-warp-textures-require-distinct-slots" );
	const auto ssbump = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "anim_wp/framework/backpanels_color01" },
	        { "$bumpmap", "anim_wp/framework/backpanels_color01_height-ssbump" },
	        { "$ssbump", "1" }, { "$ssbumpmathfix", "1" } },
	    {} );
	results.That( ssbump && material::ClaimForMesh( ssbump.Value(), true ),
	    "posed-model.ssbump-basis-claims-shared-pbr-point" );
	const auto toxinNormal = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_destruction/toxintank" },
	        { "$bumpmap", "models/props_destruction/toxintank_normal" }, { "$bumpscale", "30" },
	        { "$phong", "1" } },
	    {} );
	results.That( toxinNormal && material::ClaimForMesh( toxinNormal.Value(), true ),
	    "posed-model.vertexlit-bumpscale-is-undeclared-metadata" );
	const auto unquotedTint = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "metal/black_wall_metal_002a" },
	        { "$envmap", "metal/black_wall_envmap_002a_hdr" }, { "$envmaptint", ".2" },
	        { ".2", ".2" } },
	    {} );
	results.That( unquotedTint && unquotedTint.Value().metadata.size() == 1 &&
	                  unquotedTint.Value().metadata[0].key == ".2" &&
	                  material::ClaimForMesh( unquotedTint.Value(), true ),
	    "posed-model.numeric-keyvalues-residue-is-audited-metadata" );
	const auto unknownControl = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "metal/black_wall_metal_002a" },
	        { "$unrecognized_render_control", "1" } },
	    {} );
	results.That( unknownControl && !material::ClaimForMesh( unknownControl.Value(), true ),
	    "posed-model.unknown-render-control-remains-a-gap" );
	const auto inactiveCloak = material::MapVariables( "VertexLitGeneric",
	    { { "$cloakfactor", "1" }, { "$cloakpassenabled", "0" } }, {} );
	const auto activeCloak = material::MapVariables( "VertexLitGeneric",
	    { { "$cloakfactor", "1" }, { "$cloakpassenabled", "1" } }, {} );
	results.That( inactiveCloak && material::ClaimForMesh( inactiveCloak.Value(), true ),
	    "posed-model.disabled-cloak-factor-is-inert" );
	results.That( activeCloak && !material::ClaimForMesh( activeCloak.Value(), true ),
	    "posed-model.enabled-cloak-still-requires-its-point" );
	const auto emissiveProbe = material::MapVariables( "UnlitGeneric",
	    { { "$basetexture", "models/props_map_editor/black_white_unlit" },
	        { "$envmap", "env_cubemap" } },
	    {} );
	results.That( emissiveProbe && material::ClaimForMesh( emissiveProbe.Value(), true ) &&
	                  !material::ClaimForMesh( emissiveProbe.Value(), false ),
	    "posed-model.unlit-reflection-requires-native-rprb" );
	const auto bindPose = world.staticMeshes[0].vertices;
	WorldPass pass;
	pass.SetWorld( std::move( world ) );
	results.That( pass.DrawsPosedModel( 0, 0 ), "posed-model.claims-mesh-program" );
	EmptyTextures empty;
	TextureDesc normalDesc;
	normalDesc.format = Format::kRGBA8Unorm;
	normalDesc.width = normalDesc.height = 1;
	normalDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	const std::array<std::byte, 4> normalPixel = {
	    std::byte{ 180 }, std::byte{ 80 }, std::byte{ 170 }, std::byte{ 255 } };
	auto normalTexture = textures.Stage( "ssbump-fixture", normalDesc, normalPixel );
	if ( !normalTexture )
		return "the model normal fixture could not be staged";
	empty.normalFixture = normalTexture.Value().texture;
	const std::array<std::byte, 4> warpPixel = {
	    std::byte{ 255 }, std::byte{ 10 }, std::byte{ 10 }, std::byte{ 255 } };
	auto warpTexture = textures.Stage( "phong-warp-fixture", normalDesc, warpPixel );
	if ( !warpTexture )
		return "the model specular warp fixture could not be staged";
	empty.phongWarpFixture = warpTexture.Value().texture;
	const std::array<std::byte, 4> refractNormalPixel = {
	    std::byte{ 128 }, std::byte{ 128 }, std::byte{ 255 }, std::byte{ 128 } };
	auto refractNormal = textures.Stage( "refract-normal-fixture", normalDesc, refractNormalPixel );
	if ( !refractNormal )
		return "the model Refract normal fixture could not be staged";
	empty.refractNormalFixture = refractNormal.Value().texture;
	const std::array<std::byte, 4> refractWarpPixel = {
	    std::byte{ 255 }, std::byte{ 128 }, std::byte{ 255 }, std::byte{ 255 } };
	auto refractWarp = textures.Stage( "refract-warp-fixture", normalDesc, refractWarpPixel );
	if ( !refractWarp )
		return "the model Refract warp fixture could not be staged";
	empty.refractWarpFixture = refractWarp.Value().texture;
	TextureDesc colorDesc = normalDesc;
	colorDesc.format = Format::kRGBA8Srgb;
	const std::array<std::byte, 4> basePixelFixture = {
	    std::byte{ 128 }, std::byte{ 192 }, std::byte{ 255 }, std::byte{ 64 } };
	const std::array<std::byte, 8> overlayPixels = { std::byte{ 255 }, std::byte{ 128 },
	    std::byte{ 64 }, std::byte{ 128 }, std::byte{ 64 }, std::byte{ 255 }, std::byte{ 128 },
	    std::byte{ 64 } };
	auto baseTexture = textures.Stage( "two-texture-base", colorDesc, basePixelFixture );
	colorDesc.width = 2;
	auto overlayTexture = textures.Stage( "two-texture-overlay", colorDesc, overlayPixels );
	if ( !baseTexture || !overlayTexture )
		return "the two-texture color fixtures could not be staged";
	empty.twoTextureBase = baseTexture.Value().texture;
	empty.twoTextureOverlay = overlayTexture.Value().texture;
	const std::array<std::byte, 8> cutoutPixels = { std::byte{ 255 }, std::byte{ 255 },
	    std::byte{ 255 }, std::byte{ 0 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 },
	    std::byte{ 255 } };
	auto cutoutTexture = textures.Stage( "model-depth-cutout", colorDesc, cutoutPixels );
	if ( !cutoutTexture )
		return "the model depth cutout fixture could not be staged";
	empty.cutoutFixture = cutoutTexture.Value().texture;
	auto render = [&]( WorldPass &active, float offset, bool lit, std::uint64_t frame,
	                  const ClearColor &clear, CanvasImage &image, bool twoLayers = false,
	                  RenderCoreDrawPhase phase = RenderCoreDrawPhase::kAll, bool copySource = true,
	                  WorldPass *under = nullptr, const WorldMaterial *dynamicMaterial = nullptr,
	                  bool invalidDynamicIndex = false, float underOffset = 0.1f,
	                  bool depthPrepass = false, bool staticModels = false,
	                  bool clipLeft = false ) -> std::optional<std::string>
	{
		WorldView view;
		for ( int i = 0; i < 4; ++i )
			view.toClip[i * 5] = 1.0f;
		view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
		view.hostFrame = frame;
		WorldView::PosedModel pose;
		pose.phase = phase;
		pose.vertices = bindPose;
		for ( auto &vertex : pose.vertices )
			vertex.position[0] += offset;
		if ( dynamicMaterial )
		{
			WorldView::DynamicDraw draw;
			draw.material = *dynamicMaterial;
			for ( const auto &vertex : pose.vertices )
			{
				WorldVertex out;
				std::copy_n( vertex.position, 3, out.position );
				std::copy_n( vertex.normal, 3, out.normal );
				std::copy_n( vertex.tangent, 3, out.tangentS );
				std::copy_n( vertex.uv, 2, out.uv );
				draw.vertices.push_back( out );
			}
			draw.indices = { 0, 1, 2, 0, 2, 3 };
			if ( invalidDynamicIndex )
				draw.indices.back() = 999;
			view.dynamicDraws.push_back( std::move( draw ) );
		}
		else if ( staticModels )
			view.staticInstances = { 0, 1 };
		else
			view.posedModels.push_back( std::move( pose ) );
		if ( twoLayers )
		{
			WorldView::PosedModel behind = view.posedModels.front();
			for ( auto &vertex : behind.vertices )
				vertex.position[2] += 0.1f;
			view.posedModels.push_back( std::move( behind ) );
		}
		if ( lit )
		{
			auto lights = std::make_shared<StageViewLights>();
			material::SurfaceAreaLight area;
			area.center[2] = 0.8f;
			area.center[3] = 1.0f;
			area.halfU[0] = 0.5f;
			area.halfU[3] = 10.0f;
			area.halfV[1] = -0.5f;
			area.radiance[0] = 8.0f;
			area.radiance[1] = 7.0f;
			area.radiance[2] = 6.0f;
			area.radiance[3] = -1.0f;
			lights->areas.push_back( area );
			view.lights = std::move( lights );
		}
		std::uint32_t underTag = 0;
		if ( under )
		{
			WorldView underView = view;
			for ( auto &pose : underView.posedModels )
				for ( auto &vertex : pose.vertices )
					vertex.position[2] += underOffset;
			underTag = under->QueueView( std::move( underView ) );
			if ( !underTag )
				return "the background model view queued nothing";
		}
		const std::uint32_t tag = active.QueueView( std::move( view ) );
		if ( !tag )
			return "the posed view queued nothing";
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId depth ) -> std::optional<std::string>
		{
			WorldTarget target;
			target.device = device.get();
			target.color = color;
			target.colorFormat = kCanvasColor;
			target.colorCopySource = copySource;
			target.sceneColorCapture = &sceneColorCapture;
			target.depth = depth;
			target.depthFormat = kCanvasDepth;
			target.width = target.height = kSize;
			target.textures = &empty;
			target.frame = frame;
			target.depthPrepass = depthPrepass;
			if ( clipLeft )
				target.clipPlanes[0][0] = 1.0f;
			target.eye[2] = 2.0f;
			if ( under )
				under->Record( underTag, encoder, target );
			active.Record( tag, encoder, target );
			return std::nullopt;
		};
		return canvas->Render( textures, groups, {}, clear, &image, post );
	};
	CanvasImage center, shifted, dark;
	const ClearColor black{ 0, 0, 0, 1 };
	if ( std::optional<std::string> why = render( pass, 0.0f, true, 1, black, center ) )
		return why;
	if ( std::optional<std::string> why = render( pass, 0.5f, true, 2, black, shifted ) )
		return why;
	if ( std::optional<std::string> why = render( pass, 0.0f, false, 3, black, dark ) )
		return why;
	if ( const char *directory = std::getenv( "RENDER_LAB_IMAGES" ) )
	{
		std::filesystem::create_directories( directory );
		for ( const auto &[name, image] : { std::pair{ "posed-lit", &center },
		          std::pair{ "posed-shifted", &shifted }, std::pair{ "posed-unlit", &dark } } )
		{
			if ( !WritePfm( std::filesystem::path( directory ) / ( std::string( name ) + ".pfm" ),
			         image->width, image->height, image->rgba ) )
				return "could not write posed-model image";
		}
	}
	results.That( pass.Stats().viewsFailed == 0 && pass.Stats().posedDrawsDrawn == 3,
	    "posed-model.three-views-recorded", pass.Stats().lastFailure );
	CanvasImage depthLit;
	if ( auto why = render( pass, 0.0f, true, 4, black, depthLit, false, RenderCoreDrawPhase::kAll,
	         true, nullptr, nullptr, false, 0.1f, true ) )
		return why;
	results.That( depthLit.rgba == center.rgba, "posed-model.depth-prepass-exact",
	    "the complete opaque depth prepass preserves the lit posed image" );
	CanvasImage layersControl, layersDepth;
	if ( auto why = render( pass, 0.0f, true, 4, black, layersControl, true ) )
		return why;
	if ( auto why = render( pass, 0.0f, true, 4, black, layersDepth, true,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, true ) )
		return why;
	results.That( layersDepth.rgba == layersControl.rgba,
	    "posed-model.overlapping-depth-prepass-exact",
	    "hidden posed surfaces preserve every lit pixel" );
	WorldData staticWorld = MeshWorld();
	WorldData::StaticInstance frontStatic;
	for ( int i = 0; i < 4; ++i )
		frontStatic.world[i * 5] = 1.0f;
	staticWorld.staticInstances.push_back( frontStatic );
	frontStatic.world[11] = 0.1f;
	staticWorld.staticInstances.push_back( frontStatic );
	WorldPass staticPass;
	staticPass.SetWorld( std::move( staticWorld ) );
	CanvasImage staticControl, staticDepth;
	if ( auto why = render( staticPass, 0.0f, true, 4, black, staticControl, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, false, true ) )
		return why;
	if ( auto why = render( staticPass, 0.0f, true, 4, black, staticDepth, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, true, true ) )
		return why;
	results.That( staticDepth.rgba == staticControl.rgba && Sum( staticDepth, 16, 32 ) > 0.1f,
	    "posed-model.static-overlapping-depth-prepass-exact",
	    "shared static geometry and instance transforms preserve every lit pixel" );
	staticPass.ReleaseDevice( *device );
	CanvasImage clippedControl, clippedDepth;
	if ( auto why = render( pass, 0.0f, true, 4, black, clippedControl, true,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, false, false, true ) )
		return why;
	if ( auto why = render( pass, 0.0f, true, 4, black, clippedDepth, true,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, true, false, true ) )
		return why;
	results.That( clippedDepth.rgba == clippedControl.rgba &&
	                  clippedDepth.rgba != layersControl.rgba &&
	                  Sum( clippedDepth, 32, 48 ) > 0.1f && Sum( clippedDepth, 16, 32 ) == 0.0f,
	    "posed-model.clipped-depth-prepass-exact",
	    "fragment clip planes preserve covered and discarded samples with overlapping models" );
	WorldData cutoutWorld = MeshWorld();
	cutoutWorld.materials.push_back( cutoutWorld.materials[0] );
	cutoutWorld.materials[0].variables.push_back( { "$alphatest", "1" } );
	cutoutWorld.materials[0].variables.push_back( { "$basetexture", "model-depth-cutout" } );
	cutoutWorld.materials[0].textures.push_back( { "$basetexture", 7 } );
	for ( auto &vertex : cutoutWorld.staticMeshes[0].vertices )
	{
		vertex.uv[0] = vertex.position[0] + 0.5f;
		vertex.uv[1] = vertex.position[1] + 0.5f;
	}
	cutoutWorld.staticMeshes.push_back( cutoutWorld.staticMeshes[0] );
	cutoutWorld.staticMeshes[1].surfaces[0].material = 1;
	frontStatic.world[11] = 0.0f;
	cutoutWorld.staticInstances.push_back( frontStatic );
	frontStatic.world[11] = 0.1f;
	frontStatic.mesh = 1;
	cutoutWorld.staticInstances.push_back( frontStatic );
	WorldPass cutoutPass;
	cutoutPass.SetWorld( std::move( cutoutWorld ) );
	CanvasImage cutoutControl, cutoutPrepassed;
	if ( auto why = render( cutoutPass, 0.0f, true, 4, black, cutoutControl, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, false, true ) )
		return why;
	if ( auto why = render( cutoutPass, 0.0f, true, 4, black, cutoutPrepassed, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, true, true ) )
		return why;
	results.That( cutoutPrepassed.rgba == cutoutControl.rgba &&
	                  cutoutPrepassed.rgba != staticControl.rgba &&
	                  Sum( cutoutPrepassed, 16, 32 ) > 0.1f,
	    "posed-model.cutout-depth-prepass-exact",
	    "alpha-tested front geometry reveals the opaque model behind it without changing any "
	    "pixel" );
	cutoutPass.ReleaseDevice( *device );
	const float centralLight = Sum( center, 16, 32 );
	results.That( centralLight > 0.1f && centralLight > Sum( dark, 16, 32 ) * 1.5f,
	    "posed-model.lit-by-frame-light",
	    "lit " + std::to_string( centralLight ) + ", dark " +
	        std::to_string( Sum( dark, 16, 32 ) ) );
	results.That( Sum( shifted, 48, 64 ) > Sum( center, 48, 64 ) + 10.0f &&
	                  Sum( shifted, 16, 32 ) < Sum( center, 16, 32 ) * 0.5f,
	    "posed-model.current-pose-moves-pixels",
	    "right " + std::to_string( Sum( center, 48, 64 ) ) + " -> " +
	        std::to_string( Sum( shifted, 48, 64 ) ) + ", left " +
	        std::to_string( Sum( center, 16, 32 ) ) + " -> " +
	        std::to_string( Sum( shifted, 16, 32 ) ) );
	CanvasImage reused;
	if ( std::optional<std::string> why = render( pass, 0.0f, true, 4, black, reused ) )
		return why;
	results.That( reused.rgba == center.rgba && pass.Stats().viewsFailed == 0,
	    "posed-model.reused-view-resources-preserve-every-pixel" );
	WorldData color2World = MeshWorld();
	color2World.materials[0].variables.push_back( { "$color2", "[.25 .5 1]" } );
	WorldPass color2Pass;
	color2Pass.SetWorld( std::move( color2World ) );
	CanvasImage color2Image;
	if ( std::optional<std::string> why = render( color2Pass, 0.0f, true, 18, black, color2Image ) )
		return why;
	const float *basePixel = center.At( 32, 32 );
	const float *tintPixel = color2Image.At( 32, 32 );
	results.That( color2Pass.Stats().viewsFailed == 0 && color2Pass.Stats().posedDrawsDrawn == 1 &&
	                  tintPixel[0] < basePixel[0] * 0.55f && tintPixel[1] < basePixel[1] * 0.75f &&
	                  std::abs( tintPixel[2] - basePixel[2] ) < 0.005f,
	    "posed-model.color2-weights-pbr-albedo-before-lighting", color2Pass.Stats().lastFailure );
	WorldPass glass;
	glass.SetWorld( MeshWorld( true ) );
	results.That( glass.DrawsPosedModel( 0, 0 ), "posed-model.claims-alpha-blended-mesh" );
	const ClearColor background{ 0.12f, 0.24f, 0.36f, 1.0f };
	const float backgroundRgb[3] = { background.r, background.g, background.b };
	CanvasImage opaqueOverBackground, blended;
	if ( std::optional<std::string> why =
	         render( pass, 0.0f, true, 4, background, opaqueOverBackground ) )
		return why;
	if ( std::optional<std::string> why = render( glass, 0.0f, true, 5, background, blended ) )
		return why;
	bool halfOverBackground = true;
	for ( int channel = 0; channel < 3; ++channel )
	{
		const float expected =
		    0.5f * opaqueOverBackground.At( 32, 32 )[channel] + 0.5f * backgroundRgb[channel];
		halfOverBackground &= std::abs( blended.At( 32, 32 )[channel] - expected ) < 0.01f;
	}
	results.That( halfOverBackground && std::abs( blended.At( 32, 32 )[3] - 1.0f ) < 0.001f,
	    "posed-model.alpha-blends-without-alpha-write" );
	results.That( glass.Stats().viewsFailed == 0 && glass.Stats().posedDrawsDrawn == 1,
	    "posed-model.blended-view-recorded", glass.Stats().lastFailure );
	WorldData refractWorld = MeshWorld();
	refractWorld.materials[0].shader = "Refract_DX90";
	refractWorld.materials[0].variables = { { "$model", "1" },
	    { "$normalmap", "refract-normal-fixture" }, { "$refractamount", "0.07" },
	    { "$refracttint", "[1 .25 .5]" } };
	refractWorld.materials[0].textures.push_back( { "$normalmap", 3 } );
	WorldPass refract;
	refract.SetWorld( std::move( refractWorld ) );
	results.That( refract.DrawsPosedModel( 0, 0 ), "posed-model.refract-claims-scene-color-model" );
	CanvasImage refracted;
	if ( std::optional<std::string> why =
	         render( refract, 0.0f, false, 19, background, refracted ) )
		return why;
	results.That( refract.Stats().viewsFailed == 0 && refract.Stats().posedDrawsDrawn == 1,
	    "posed-model.refract-captures-scene-color", refract.Stats().lastFailure );
	results.That( std::abs( refracted.At( 32, 32 )[0] - background.r ) < 0.01f &&
	                  refracted.At( 32, 32 )[1] > background.g * 0.48f &&
	                  refracted.At( 32, 32 )[1] < background.g * 0.6f &&
	                  refracted.At( 32, 32 )[2] > background.b * 0.56f &&
	                  refracted.At( 32, 32 )[2] < background.b * 0.68f,
	    "posed-model.refract-tints-and-alpha-blends-captured-background",
	    "pixel " + std::to_string( refracted.At( 32, 32 )[0] ) + ", " +
	        std::to_string( refracted.At( 32, 32 )[1] ) + ", " +
	        std::to_string( refracted.At( 32, 32 )[2] ) );
	CanvasImage missingCapture;
	if ( std::optional<std::string> why = render( refract, 0.0f, false, 20, background,
	         missingCapture, false, RenderCoreDrawPhase::kAll, false ) )
		return why;
	results.That( refract.Stats().viewsFailed == 1 &&
	                  std::abs( missingCapture.At( 32, 32 )[0] - background.r ) < 0.01f,
	    "posed-model.refract-refuses-view-without-capture-source" );
	WorldData underWorld = MeshWorld();
	underWorld.materials[0].shader = "UnlitGeneric";
	WorldPass under;
	under.SetWorld( std::move( underWorld ) );
	WorldData warpWorld = MeshWorld();
	warpWorld.materials[0].shader = "Refract_DX90";
	warpWorld.materials[0].variables = {
	    { "$model", "1" }, { "$normalmap", "refract-warp-fixture" }, { "$refractamount", "0.3" } };
	warpWorld.materials[0].textures.push_back( { "$normalmap", 4 } );
	WorldData zeroWorld = warpWorld;
	zeroWorld.materials[0].variables.back().second = "0";
	WorldPass zeroWarp;
	WorldPass displaced;
	zeroWarp.SetWorld( std::move( zeroWorld ) );
	displaced.SetWorld( std::move( warpWorld ) );
	CanvasImage noWarp, withWarp;
	if ( std::optional<std::string> why = render( zeroWarp, 0.0f, false, 21, black, noWarp, false,
	         RenderCoreDrawPhase::kAll, true, &under ) )
		return why;
	if ( std::optional<std::string> why = render( displaced, 0.0f, false, 22, black, withWarp,
	         false, RenderCoreDrawPhase::kAll, true, &under ) )
		return why;
	results.That( zeroWarp.Stats().viewsFailed == 0 && displaced.Stats().viewsFailed == 0 &&
	                  noWarp.At( 40, 32 )[0] > withWarp.At( 40, 32 )[0] + 0.15f,
	    "posed-model.refract-displaces-authored-scene-edge",
	    "no warp " + std::to_string( noWarp.At( 40, 32 )[0] ) + ", warp " +
	        std::to_string( withWarp.At( 40, 32 )[0] ) );
	CanvasImage singlePane, twoPanes;
	if ( std::optional<std::string> why = render( glass, 0.0f, true, 6, black, singlePane ) )
		return why;
	if ( std::optional<std::string> why = render( glass, 0.0f, true, 7, black, twoPanes, true ) )
		return why;
	results.That( twoPanes.At( 32, 32 )[0] > singlePane.At( 32, 32 )[0] * 1.1f,
	    "posed-model.blended-front-pane-does-not-write-depth" );
	WorldPass mixed;
	mixed.SetWorld( MixedWorld() );
	results.That( mixed.DrawsPosedModel( 0, 0, RenderCoreDrawPhase::kOpaque ) &&
	                  mixed.DrawsPosedModel( 0, 0, RenderCoreDrawPhase::kBlended ),
	    "posed-model.claims-both-studio-phases" );
	CanvasImage opaquePhase, blendedPhase;
	if ( std::optional<std::string> why = render(
	         mixed, 0.0f, true, 8, background, opaquePhase, false, RenderCoreDrawPhase::kOpaque ) )
		return why;
	if ( std::optional<std::string> why = render( mixed, 0.0f, true, 9, background, blendedPhase,
	         false, RenderCoreDrawPhase::kBlended ) )
		return why;
	bool phasesMatch = true;
	for ( int channel = 0; channel < 3; ++channel )
	{
		phasesMatch &= std::abs( opaquePhase.At( 32, 32 )[channel] -
		                         opaqueOverBackground.At( 32, 32 )[channel] ) < 0.01f;
		phasesMatch &=
		    std::abs( blendedPhase.At( 32, 32 )[channel] - blended.At( 32, 32 )[channel] ) < 0.01f;
	}
	results.That( phasesMatch && mixed.Stats().viewsFailed == 0,
	    "posed-model.two-pass-surfaces-stay-in-their-phases", mixed.Stats().lastFailure );
	WorldData floorWorld = MeshWorld();
	floorWorld.materials[0].variables.push_back( { "$selfillum", "1" } );
	floorWorld.materials[0].variables.push_back( { "$color", "[.5 .5 .5]" } );
	floorWorld.materials[0].variables.push_back( { "$selfillumtint", "[.7 .85 1]" } );
	WorldPass floor;
	floor.SetWorld( std::move( floorWorld ) );
	results.That( floor.DrawsPosedModel( 0, 0 ), "posed-model.claims-self-lit-elevator-floor" );
	CanvasImage selfLit, selfLitWithLights;
	if ( std::optional<std::string> why = render( floor, 0.0f, false, 10, black, selfLit ) )
		return why;
	if ( std::optional<std::string> why =
	         render( floor, 0.0f, true, 11, black, selfLitWithLights ) )
		return why;
	bool maskedReplacement = true;
	for ( int channel = 0; channel < 3; ++channel )
		maskedReplacement &= std::abs( selfLit.At( 32, 32 )[channel] -
		                               selfLitWithLights.At( 32, 32 )[channel] ) < 0.005f;
	results.That( maskedReplacement && selfLit.At( 32, 32 )[0] > 0.05f &&
	                  std::abs( selfLit.At( 32, 32 )[3] - 1.0f ) < 0.001f,
	    "posed-model.self-lit-mask-replaces-lighting-without-alpha-cutout" );
	results.That( selfLit.At( 32, 32 )[0] < selfLit.At( 32, 32 )[1] &&
	                  selfLit.At( 32, 32 )[1] < selfLit.At( 32, 32 )[2],
	    "posed-model.self-illumination-tint-is-authored" );
	WorldData bumpWorld = MeshWorld();
	bumpWorld.materials[0].variables.push_back( { "$bumpmap", "ssbump-fixture" } );
	bumpWorld.materials[0].textures.push_back( { "$bumpmap", 1 } );
	WorldData ssbumpWorld = bumpWorld;
	ssbumpWorld.materials[0].variables.push_back( { "$ssbump", "1" } );
	WorldPass bump;
	WorldPass ssbumpPass;
	bump.SetWorld( std::move( bumpWorld ) );
	ssbumpPass.SetWorld( std::move( ssbumpWorld ) );
	CanvasImage regularNormal, ssbumpNormal;
	if ( std::optional<std::string> why = render( bump, 0.0f, true, 12, black, regularNormal ) )
		return why;
	if ( std::optional<std::string> why =
	         render( ssbumpPass, 0.0f, true, 13, black, ssbumpNormal ) )
		return why;
	results.That(
	    bump.Stats().viewsFailed == 0 && ssbumpPass.Stats().viewsFailed == 0 &&
	        bump.Stats().posedDrawsDrawn == 1 && ssbumpPass.Stats().posedDrawsDrawn == 1 &&
	        std::abs( regularNormal.At( 32, 32 )[0] - ssbumpNormal.At( 32, 32 )[0] ) > 0.005f,
	    "posed-model.ssbump-basis-changes-pbr-lighting", ssbumpPass.Stats().lastFailure );
	WorldData plainSpecWorld = MeshWorld();
	plainSpecWorld.materials[0].variables.push_back( { "$phong", "1" } );
	plainSpecWorld.materials[0].variables.push_back( { "$phongexponent", "2" } );
	plainSpecWorld.materials[0].variables.push_back( { "$phongboost", "6" } );
	WorldData warpedSpecWorld = plainSpecWorld;
	warpedSpecWorld.materials[0].variables.push_back(
	    { "$phongwarptexture", "phong-warp-fixture" } );
	warpedSpecWorld.materials[0].textures.push_back( { "$phongwarptexture", 2 } );
	WorldPass plainSpec;
	WorldPass warpedSpec;
	plainSpec.SetWorld( std::move( plainSpecWorld ) );
	warpedSpec.SetWorld( std::move( warpedSpecWorld ) );
	CanvasImage plainSpecImage, warpedSpecImage;
	if ( std::optional<std::string> why =
	         render( plainSpec, 0.0f, true, 14, black, plainSpecImage ) )
		return why;
	if ( std::optional<std::string> why =
	         render( warpedSpec, 0.0f, true, 15, black, warpedSpecImage ) )
		return why;
	results.That( plainSpec.Stats().viewsFailed == 0 && warpedSpec.Stats().viewsFailed == 0 &&
	                  warpedSpec.Stats().posedDrawsDrawn == 1 &&
	                  warpedSpecImage.At( 32, 32 )[1] < plainSpecImage.At( 32, 32 )[1] * 0.9f,
	    "posed-model.phong-warp-colors-direct-pbr-specular", warpedSpec.Stats().lastFailure );
	WorldData unlitWorld = MeshWorld();
	unlitWorld.materials[0].shader = "UnlitGeneric";
	WorldPass unlit;
	unlit.SetWorld( std::move( unlitWorld ) );
	results.That( unlit.DrawsPosedModel( 0, 0 ), "posed-model.claims-emissive-unlit-mesh" );
	CanvasImage unlitDark, unlitBright;
	if ( std::optional<std::string> why = render( unlit, 0.0f, false, 16, black, unlitDark ) )
		return why;
	if ( std::optional<std::string> why = render( unlit, 0.0f, true, 17, black, unlitBright ) )
		return why;
	bool independentOfLight = true;
	for ( int channel = 0; channel < 3; ++channel )
		independentOfLight &= std::abs( unlitDark.At( 32, 32 )[channel] -
		                                unlitBright.At( 32, 32 )[channel] ) < 0.005f;
	results.That( independentOfLight && unlitDark.At( 32, 32 )[0] > 0.05f,
	    "posed-model.emissive-mesh-ignores-direct-light" );
	WorldData depthWorld = MeshWorld();
	depthWorld.materials[0].shader = "WriteZ_DX9";
	WorldPass depthOnly;
	depthOnly.SetWorld( std::move( depthWorld ) );
	CanvasImage occluded, depthBehind;
	if ( auto why = render( unlit, 0.0f, false, 30, background, occluded, false,
	         RenderCoreDrawPhase::kAll, true, &depthOnly, nullptr, false, -0.1f ) )
		return why;
	if ( auto why = render( unlit, 0.0f, false, 31, background, depthBehind, false,
	         RenderCoreDrawPhase::kAll, true, &depthOnly ) )
		return why;
	bool untouchedColor = true;
	for ( int channel = 0; channel < 3; ++channel )
		untouchedColor &=
		    std::abs( occluded.At( 32, 32 )[channel] - backgroundRgb[channel] ) < 0.005f;
	results.That( depthOnly.Failures() == 0 && depthOnly.Stats().posedDrawsDrawn == 2 &&
	                  untouchedColor && depthBehind.At( 32, 32 )[0] > background.r + 0.1f,
	    "posed-model.writez-occludes-only-behind-it-without-writing-color",
	    depthOnly.Stats().lastFailure );
	// The legacy WriteZ point has no texture sampling or alpha-test shader.
	const auto cutoutDepth = material::MapVariables( "WriteZ_DX9", { { "$alphatest", "1" } }, {} );
	results.That( cutoutDepth && !material::ClaimForMesh( cutoutDepth.Value(), false ),
	    "posed-model.writez-does-not-claim-an-unimplemented-alpha-mask" );
	depthOnly.ReleaseDevice( *device );
	WorldData twoTextureWorld = MeshWorld();
	twoTextureWorld.materials[0].shader = "UnlitTwoTexture_DX9";
	twoTextureWorld.materials[0].variables = {
	    { "$basetexture", "two-texture-base" }, { "$texture2", "two-texture-overlay" } };
	twoTextureWorld.materials[0].textures = { { "$basetexture", 5 }, { "$texture2", 6 } };
	WorldData transformedWorld = twoTextureWorld;
	transformedWorld.materials[0].variables.push_back( { "$texture2transform",
	    "center .5 .5 scale 1 1 rotate 0 translate .75 0" } );
	WorldPass twoTexture, transformed;
	twoTexture.SetWorld( std::move( twoTextureWorld ) );
	transformed.SetWorld( std::move( transformedWorld ) );
	CanvasImage multiplied, shiftedOverlay;
	if ( auto why = render( twoTexture, 0.0f, false, 23, black, multiplied ) )
		return why;
	if ( auto why = render( transformed, 0.0f, false, 24, black, shiftedOverlay ) )
		return why;
	// IEC sRGB decode of the two authored byte images, multiplied in linear
	// light. Their alpha values must not change UnlitTwoTexture's alpha of 1.
	results.That( twoTexture.Stats().viewsFailed == 0 &&
	                  std::abs( multiplied.At( 32, 32 )[0] - 0.21586f ) < 0.003f &&
	                  std::abs( multiplied.At( 32, 32 )[1] - 0.11378f ) < 0.003f &&
	                  std::abs( multiplied.At( 32, 32 )[2] - 0.05127f ) < 0.003f &&
	                  std::abs( multiplied.At( 32, 32 )[3] - 1.0f ) < 0.003f,
	    "posed-model.two-texture-linear-product-and-opaque-alpha",
	    twoTexture.Stats().lastFailure + " claims " + std::to_string( twoTexture.Stats().claimedMaterials ) +
	        " rgb " + std::to_string( multiplied.At( 32, 32 )[0] ) + " " +
	        std::to_string( multiplied.At( 32, 32 )[1] ) + " " +
	        std::to_string( multiplied.At( 32, 32 )[2] ) );
	results.That( transformed.Stats().viewsFailed == 0 &&
	                  shiftedOverlay.At( 32, 32 )[0] < multiplied.At( 32, 32 )[0] * 0.1f &&
	                  shiftedOverlay.At( 32, 32 )[1] > multiplied.At( 32, 32 )[1] * 3.0f,
	    "posed-model.two-texture-independent-transform-negative-control",
	    std::to_string( shiftedOverlay.At( 32, 32 )[0] ) + " " +
	        std::to_string( shiftedOverlay.At( 32, 32 )[1] ) );
	WorldPass dynamic;
	dynamic.SetWorld( MeshWorld() );
	WorldMaterial dynamicMaterial;
	dynamicMaterial.name = "dynamic-two-texture-fixture";
	dynamicMaterial.shader = "UnlitTwoTexture_DX9";
	dynamicMaterial.variables = {
	    { "$basetexture", "two-texture-base" }, { "$texture2", "two-texture-overlay" } };
	dynamicMaterial.textures = { { "$basetexture", 5 }, { "$texture2", 6 } };
	CanvasImage dynamicImage, proxyImage, invalidImage;
	if ( auto why = render( dynamic, 0.0f, false, 25, black, dynamicImage, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, &dynamicMaterial ) )
		return why;
	dynamicMaterial.variables.push_back(
	    { "$texture2transform", "center .5 .5 scale 1 1 rotate 0 translate .75 0" } );
	if ( auto why = render( dynamic, 0.0f, false, 26, black, proxyImage, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, &dynamicMaterial ) )
		return why;
	results.That(
	    dynamic.Stats().dynamicDrawsDrawn == 2 && dynamic.Failures() == 0 &&
	        std::abs( dynamicImage.At( 32, 32 )[0] - multiplied.At( 32, 32 )[0] ) < .003f &&
	        std::abs( proxyImage.At( 32, 32 )[0] - shiftedOverlay.At( 32, 32 )[0] ) < .003f,
	    "posed-model.dynamic-material-proxy-uses-current-snapshot", dynamic.Stats().lastFailure );
	if ( auto why = render( dynamic, 0.0f, false, 27, black, invalidImage, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, &dynamicMaterial, true ) )
		return why;
	results.That( dynamic.Failures() == 1 && dynamic.Stats().dynamicDrawsDrawn == 2 &&
	                  invalidImage.At( 32, 32 )[0] == 0,
	    "posed-model.dynamic-invalid-index-fails-before-draw", dynamic.Stats().lastFailure );
	dynamic.ReleaseDevice( *device );
	WorldPass refused;
	refused.SetWorld( MeshWorld() );
	WorldView unsupported;
	WorldView::DynamicDraw unsupportedDraw;
	unsupportedDraw.material.name = "post-material-negative-control";
	unsupportedDraw.material.shader = "MotionBlur_dx9";
	unsupported.dynamicDraws.push_back( std::move( unsupportedDraw ) );
	results.That( refused.QueueView( std::move( unsupported ) ) == 0 && refused.Failures() == 0 &&
	                  refused.Stats().viewsQueued == 0 &&
	                  refused.Stats().dynamicDrawsRefused == 1 &&
	                  refused.Stats().lastRefusal.find( "post-material-negative-control" ) !=
	                      std::string::npos,
	    "posed-model.unsupported-dynamic-material-is-refused-before-claiming-a-slot",
	    refused.Stats().lastRefusal );
	WorldData forcedWorld = MeshWorld();
	forcedWorld.materials[0].variables = { { "$phong", "0" }, { "$forcephong", "1" } };
	WorldData phongWorld = MeshWorld();
	phongWorld.materials[0].variables = { { "$phong", "1" } };
	WorldPass forced, phong;
	forced.SetWorld( std::move( forcedWorld ) );
	phong.SetWorld( std::move( phongWorld ) );
	CanvasImage forcedImage, phongImage;
	if ( auto why = render( forced, 0.0f, true, 28, black, forcedImage ) )
		return why;
	if ( auto why = render( phong, 0.0f, true, 29, black, phongImage ) )
		return why;
	results.That( forced.Failures() == 0 && forced.Stats().posedDrawsDrawn == 1 &&
	                  forcedImage.rgba == phongImage.rgba,
	    "posed-model.forcephong-selects-the-authored-phong-point" );
	results.That( forcedImage.rgba != center.rgba,
	    "posed-model.forcephong-response-differs-from-the-neutral-mesh-point" );
	forced.ReleaseDevice( *device );
	phong.ReleaseDevice( *device );

	twoTexture.ReleaseDevice( *device );
	transformed.ReleaseDevice( *device );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	glass.ReleaseDevice( *device );
	mixed.ReleaseDevice( *device );
	floor.ReleaseDevice( *device );
	bump.ReleaseDevice( *device );
	ssbumpPass.ReleaseDevice( *device );
	plainSpec.ReleaseDevice( *device );
	warpedSpec.ReleaseDevice( *device );
	unlit.ReleaseDevice( *device );
	color2Pass.ReleaseDevice( *device );
	messages = counter.load();
	return std::nullopt;
}

} // namespace

int RunPosedModelSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "posed-model", std::span<const Seeded>(), RunChecks );
}

} // namespace render::lab
