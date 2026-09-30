//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.panels (RFC 0016); see panels.h. Names no material
//			family: the panel's surface is a PBRMetalRough material resolved
//			by the one resolver.
//
//=============================================================================//

#include "render/pass/panels/panels.h"

#include "render/frame/debug_specialization.h"
#include "render/material/draw_program.h"
#include "render/material/material_programs.h"
#include "render/material/model_lighting.h"
#include "render/material/surface_program.h"
#include "render/material/vmt_import.h"
#include "render/math/matrix.h"
#include "render/resources/texture_cache.h"
#include "render/shaderlib/core_artifacts.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <deque>
#include <map>
#include <mutex>
#include <optional>
#include <span>
#include <tuple>
#include <utility>

namespace render::pass::panels
{

namespace
{

using namespace render::device;

constexpr const char *kRasterVertex = "render/pass/panels/panel_raster.vert";
constexpr const char *kRasterFragment = "render/pass/panels/panel_raster.frag";
constexpr const char *kCoatingFragment = "render/pass/panels/panel_coating.frag";
constexpr const char *kMipsCompute = "render/pass/panels/panel_mips.comp";
constexpr std::uint32_t kMipsGroup = 8; // the compute kernel's local size

// The panel material's constant texture (a TextureCache name, as the
// importer normalizes the "$mraotexture" value) and tables.
constexpr const char *kMraoName = "panel_mrao";
constexpr const char *kSplitSumName = "panel:split-sum";
constexpr const char *kLtcName = "panel:ltc";
// The panel surface's MRAO: dielectric, roughness 0.5 (a lit board behind a
// plastic face), no occlusion.
constexpr std::uint8_t kMrao[4] = { 0, 128, 255, 255 };

// The raster vertex (panel_raster.vert).
struct RasterVertex
{
	float position[2] = {};
	float uv[2] = {};
	std::uint8_t color[4] = { 255, 255, 255, 255 };
};
static_assert( sizeof( RasterVertex ) == 20 );

// The raster draw constants (both stages, 80 bytes).
struct RasterConstants
{
	float toClip[16] = {};
	float params[4] = {}; // x: texture alpha not read, y: premultiply
};
static_assert( sizeof( RasterConstants ) == 80 );

// The mips kernel's step (48 bytes).
struct MipsStep
{
	std::uint32_t level[4] = {}; // level, first texel, width, height
	std::uint32_t previous[4] =
	    {}; // first texel, width, height; the mode (0 emission, 1 albedo, 3 scatter)
	std::uint32_t scatter[4] =
	    {}; // the scatter grid's cells across and down, and the reach in cells
};
static_assert( sizeof( MipsStep ) == 48 );

struct Level
{
	std::uint32_t first = 0; // texels into the levels buffer
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

// One panel's images: the raster targets (the emissive composite, gamma, and
// the coatings, premultiplied linear), the level buffers, and the emission
// and albedo textures the surface samples.
struct Image
{
	Resolution resolution = {};
	std::vector<Level> levels;
	TextureId emissive;
	TextureId coating;
	bool rastersUsed = false; // in kSampled after their first frame
	BufferId emissionTexels;
	BufferId albedoTexels;
	// The coatings' scatter (render.world-panel.v1 ScatterField): the
	// emissive composite at the grid's cell centers, and the field.
	int scatterWide = 0;
	int scatterTall = 0;
	TextureId scatterRaster;
	BufferId scatterTexels;
	BindGroupId mipsGroup;
	TextureId emission;
	TextureId albedo;
	TextureDesc emissionDesc;          // and the albedo's (the same)
	bool filled = false;               // the textures in kSampled after their first frame
	std::uint64_t rasterizedFrame = 0; // the host frame the images hold
	bool rasterized = false;
};

// The surface's device objects for one target format.
struct Variant
{
	Format colorFormat = Format::kUnknown;
	Format depthFormat = Format::kUnknown;
	std::uint32_t samples = 1;
	bool encodeOutput = false;
	std::unique_ptr<resources::TextureCache> cache;
	std::unique_ptr<material::ProgramResolver> resolver;
	std::unique_ptr<material::MaterialPrograms> programs;
	std::unique_ptr<material::DrawGroups> draws;
	std::unique_ptr<material::GroupResidency> frames;
	std::optional<material::ResolvedProgram> program;
	// Each panel's material group (the emission and albedo textures and the
	// emission scale it names) and draw group (the ambient cube it holds).
	std::map<std::uint64_t, std::tuple<TextureId, TextureId, float>> boundMaterial;
	std::map<std::uint64_t, std::vector<std::byte>> boundLighting;
	std::vector<std::byte> frameConstants; // the frame group as last set
	std::string failure;                   // why the variant cannot draw
};

void Cross( const float a[3], const float b[3], float out[3] )
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

bool Normalize( float v[3] )
{
	const float length = std::sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
	if ( !( length > 0.0f ) || !std::isfinite( length ) )
		return false;
	for ( int k = 0; k < 3; ++k )
		v[k] /= length;
	return true;
}

std::vector<Level> LevelsFor( std::uint32_t width, std::uint32_t height )
{
	std::vector<Level> levels;
	std::uint32_t first = 0;
	for ( std::uint32_t w = width, h = height;; )
	{
		levels.push_back( { first, w, h } );
		first += w * h;
		if ( w == 1 && h == 1 )
			break;
		w = std::max( 1u, w / 2 );
		h = std::max( 1u, h / 2 );
	}
	return levels;
}

// The images' resident bytes: two mipped textures, three level buffers and
// the raster targets (8-bit emissive, half-float coating).
std::uint64_t ImageBytes( const Resolution &resolution )
{
	const std::vector<Level> levels = LevelsFor( resolution.width, resolution.height );
	const std::uint64_t chain = std::uint64_t( levels.back().first + 1 ) * 4;
	return chain * 5 + std::uint64_t( resolution.width ) * resolution.height * ( 4 + 8 );
}

} // namespace

// --- The pass -----------------------------------------------------------------

struct PanelPass::State
{
	// Main thread and render sequence, guarded by lock.
	mutable std::mutex lock;
	// Each panel's list per host frame (the frame's snapshot).
	std::map<std::pair<std::uint64_t, std::uint64_t>, std::shared_ptr<const Panel>> lists;
	std::uint32_t nextSerial = 1;
	std::map<std::uint32_t, PanelView> views; // queued, by serial
	// Views already recorded, by serial: the backend records a frame's
	// stream again for an on-demand capture (a screenshot), with the same
	// slots, which must draw the same views.
	std::deque<std::pair<std::uint32_t, PanelView>> recorded;
	std::vector<std::uint64_t> removed; // panels whose images go
	PanelStats stats;
	std::span<const std::uint32_t> mipsModule; // a suite's seeded kernel

	// Render sequence only.
	IRenderDevice2 *device = nullptr;
	bool made = false;
	std::string madeFailure;
	BindGroupLayoutId rasterLayout;
	PipelineId rasterPipelines[3]; // by world_panel::Blend
	PipelineId coatingPipeline;    // the coatings: premultiplied linear over nothing
	BindGroupLayoutId mipsLayout;
	PipelineId mipsPipeline;
	SamplerId pointSampler;
	TextureId white;
	BufferId whiteStaging;
	bool whiteFilled = false;
	std::vector<std::pair<SamplerDesc, SamplerId>> samplers;
	std::map<std::uint64_t, Image> images;
	std::vector<Variant> variants;
	std::uint64_t currentFrame = ~0ull;
	std::vector<std::pair<std::uint64_t, ResourceId>> retired; // with the frame that last used them

	void Fail( const std::string &why )
	{
		std::lock_guard<std::mutex> guard( lock );
		++stats.viewsFailed;
		stats.lastFailure = why;
	}

	void Retire( std::uint64_t frame, ResourceId resource )
	{
		if ( resource.value != 0 )
			retired.emplace_back( frame, resource );
	}

	void RetireImage( std::uint64_t frame, Image &image )
	{
		for ( ResourceId resource : { ResourceId( image.emissive ), ResourceId( image.coating ),
		          ResourceId( image.emissionTexels ), ResourceId( image.albedoTexels ),
		          ResourceId( image.scatterRaster ), ResourceId( image.scatterTexels ),
		          ResourceId( image.mipsGroup ), ResourceId( image.emission ),
		          ResourceId( image.albedo ) } )
			Retire( frame, resource );
		image = Image();
	}

	std::optional<SamplerId> SamplerFor( const SamplerDesc &desc )
	{
		for ( const auto &[known, sampler] : samplers )
		{
			if ( known.minFilter == desc.minFilter && known.magFilter == desc.magFilter &&
			     known.mipFilter == desc.mipFilter && known.address == desc.address &&
			     known.maxAnisotropy == desc.maxAnisotropy )
				return sampler;
		}
		auto sampler = device->CreateSampler( desc );
		if ( !sampler )
			return std::nullopt;
		samplers.emplace_back( desc, sampler.Value() );
		return sampler.Value();
	}

	std::optional<std::string> Make();
	Variant *VariantFor( const PanelTarget &target );
	std::optional<std::string> EnsureImage(
	    Image &image, const Resolution &resolution, float unitsWide, float unitsTall );
	std::optional<std::string> Rasterize(
	    CommandEncoder &encoder, const PanelTarget &target, const Panel &panel, Image &image );
	void ReleaseAll( CompletionToken after );
};

std::optional<std::string> PanelPass::State::Make()
{
	if ( made )
		return madeFailure.empty() ? std::nullopt : std::optional<std::string>( madeFailure );
	made = true;
	auto failed = [&]( std::string why )
	{
		madeFailure = std::move( why );
		return std::optional<std::string>( madeFailure );
	};
	if ( !device->Facts().capabilities.Has( Capability::kCompute ) )
		return failed( "the device has no compute (the panel image's mip chain)" );

	const ShaderStageSet fragment{ ShaderStage::kFragment };
	const BindingDesc rasterBindings[] = { { 0, BindingKind::kSampledTexture, 1, fragment },
	    { 1, BindingKind::kSampler, 1, fragment } };
	auto rasterLayoutMade =
	    device->CreateBindGroupLayout( { BindGroupRole::kMaterial, rasterBindings } );
	if ( !rasterLayoutMade )
		return failed( "the raster bind group layout was refused" );
	rasterLayout = rasterLayoutMade.Value();
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat2, 0, 0 },
	    { 1, VertexFormat::kFloat2, 8, 0 }, { 2, VertexFormat::kUnorm8x4, 16, 0 } };
	const VertexBufferLayout buffers[] = { { sizeof( RasterVertex ), false } };
	const BlendMode blends[3] = { BlendMode::kOpaque, BlendMode::kAlpha, BlendMode::kAdditive };
	for ( int b = 0; b < 3; ++b )
	{
		shaderlib::PipelineRecipe recipe =
		    shaderlib::CoreRecipe( { kRasterVertex, kRasterFragment } );
		recipe.raster.cull = CullMode::kNone;
		recipe.colorFormats = { Format::kRGBA8Unorm };
		recipe.blends = { blends[b] };
		recipe.layouts = { {}, {}, rasterLayout, {} };
		recipe.debugName = "render.pass.panels raster";
		auto resolved = shaderlib::Resolve(
		    recipe, shaderlib::CoreArtifacts(), device->Facts().artifactFormat );
		if ( !resolved )
			return failed( "the raster program has no artifact for the device" );
		PipelineDesc desc = resolved.Value().Desc();
		desc.drawConstantBytes = sizeof( RasterConstants );
		desc.vertex = { attributes, buffers };
		auto pipeline = device->CreatePipeline( desc );
		if ( !pipeline )
			return failed( "a raster pipeline was refused" );
		rasterPipelines[b] = pipeline.Value();
	}
	// The coatings: premultiplied linear color over nothing, into a
	// half-float target.
	{
		shaderlib::PipelineRecipe recipe =
		    shaderlib::CoreRecipe( { kRasterVertex, kCoatingFragment } );
		recipe.raster.cull = CullMode::kNone;
		recipe.colorFormats = { Format::kRGBA16Float };
		recipe.blends = { BlendMode::kPremultiplied };
		recipe.layouts = { {}, {}, rasterLayout, {} };
		recipe.debugName = "render.pass.panels coating";
		auto resolved = shaderlib::Resolve(
		    recipe, shaderlib::CoreArtifacts(), device->Facts().artifactFormat );
		if ( !resolved )
			return failed( "the coating program has no artifact for the device" );
		PipelineDesc desc = resolved.Value().Desc();
		desc.drawConstantBytes = sizeof( RasterConstants );
		desc.vertex = { attributes, buffers };
		auto pipeline = device->CreatePipeline( desc );
		if ( !pipeline )
			return failed( "a coating pipeline was refused" );
		coatingPipeline = pipeline.Value();
	}

	const ShaderStageSet compute{ ShaderStage::kCompute };
	const BindingDesc mipsBindings[] = { { 0, BindingKind::kSampledTexture, 1, compute },
	    { 1, BindingKind::kSampledTexture, 1, compute },
	    { 2, BindingKind::kSampledTexture, 1, compute }, { 3, BindingKind::kSampler, 1, compute },
	    { 4, BindingKind::kStorageBuffer, 1, compute },
	    { 5, BindingKind::kStorageBuffer, 1, compute },
	    { 6, BindingKind::kStorageBuffer, 1, compute } };
	auto mipsLayoutMade = device->CreateBindGroupLayout( { BindGroupRole::kDraw, mipsBindings } );
	if ( !mipsLayoutMade )
		return failed( "the mips bind group layout was refused" );
	mipsLayout = mipsLayoutMade.Value();
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !mipsModule.empty() &&
	     !artifacts.ReplaceSpirv( kMipsCompute, mipsModule, device->Facts().artifactFormat ) )
		return failed( "the seeded mips kernel does not replace the core one" );
	shaderlib::PipelineRecipe recipe =
	    shaderlib::CoreRecipe( { kMipsCompute }, PipelineKind::kCompute );
	recipe.layouts = { {}, {}, {}, mipsLayout };
	recipe.debugName = "render.pass.panels mips";
	auto resolved = shaderlib::Resolve( recipe, artifacts, device->Facts().artifactFormat );
	if ( !resolved )
		return failed( "the mips program has no artifact for the device" );
	PipelineDesc desc = resolved.Value().Desc();
	desc.drawConstantBytes = sizeof( MipsStep );
	auto pipeline = device->CreatePipeline( desc );
	if ( !pipeline )
		return failed( "the mips pipeline was refused" );
	mipsPipeline = pipeline.Value();

	SamplerDesc point;
	point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
	point.address = AddressMode::kClampToEdge;
	auto pointMade = device->CreateSampler( point );
	if ( !pointMade )
		return failed( "the point sampler was refused" );
	pointSampler = pointMade.Value();

	TextureDesc whiteDesc;
	whiteDesc.format = Format::kRGBA8Unorm;
	whiteDesc.width = whiteDesc.height = 1;
	whiteDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	whiteDesc.debugName = "render.pass.panels white";
	auto whiteMade = device->CreateTexture( whiteDesc );
	BufferDesc stagingDesc;
	stagingDesc.size = 4;
	stagingDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	auto stagingMade = device->CreateBuffer( stagingDesc );
	if ( !whiteMade || !stagingMade )
		return failed( "the white texture was refused" );
	white = whiteMade.Value();
	whiteStaging = stagingMade.Value();
	return std::nullopt;
}

Variant *PanelPass::State::VariantFor( const PanelTarget &target )
{
	for ( Variant &v : variants )
	{
		if ( v.colorFormat == target.colorFormat && v.depthFormat == target.depthFormat &&
		     v.samples == target.samples && v.encodeOutput == target.terms.encodeOutput )
			return &v;
	}
	Variant &v = variants.emplace_back();
	v.colorFormat = target.colorFormat;
	v.depthFormat = target.depthFormat;
	v.samples = target.samples;
	v.encodeOutput = target.terms.encodeOutput;
	v.cache = std::make_unique<resources::TextureCache>( *device );
	auto resolver = material::ProgramResolver::Create( *device, target.colorFormat,
	    target.depthFormat, target.samples, material::VertexLayout::kSurface );
	if ( !resolver )
	{
		v.failure = "the panel's resolver: " + resolver.Error();
		return &v;
	}
	v.resolver = std::move( resolver ).Value();
	v.resolver->SetWorldPbr( true, 0 );
	v.programs = std::make_unique<material::MaterialPrograms>( *device, *v.cache );
	v.draws = std::make_unique<material::DrawGroups>( *device, *v.cache );
	v.frames = std::make_unique<material::GroupResidency>( *device, *v.cache );

	// The panel material: PBRMetalRough with the albedo image as its base, a
	// dielectric MRAO and the emission image (both the pass's own, bound for
	// each panel; the names here only give the program its terms).
	std::vector<material::VmtPair> variables = { { "$basetexture", "panel_albedo" },
	    { "$mraotexture", kMraoName }, { "$emissiontexture", "panel_emission" },
	    { "$emissionscale", "1" } };
	auto mapped = material::MapVariables( "PBRMetalRough", std::move( variables ), {} );
	if ( !mapped )
	{
		v.failure = "the panel material does not map: " + mapped.Error().detail;
		return &v;
	}
	auto program = v.resolver->ResolveMesh( mapped.Value() );
	if ( !program )
	{
		v.failure = "the panel material has no program: " + program.Error();
		return &v;
	}
	v.program = std::move( program ).Value();

	// Its constant textures and the pbr point's tables.
	auto stage = [&]( const std::string &name, Format format, std::uint32_t width,
	                 std::uint32_t height, std::span<const std::byte> texels )
	{
		TextureDesc desc;
		desc.format = format;
		desc.width = width;
		desc.height = height;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		return bool( v.cache->Stage( name, desc, texels ) );
	};
	std::string mraoName;
	for ( const material::ProgramTexture &texture : v.program->request.material.textures )
	{
		// The importer's name for the constant texture.
		if ( texture.name.find( kMraoName ) != std::string::npos )
			mraoName = texture.name;
	}
	const material::PbrSplitSumTable splitSum = material::SplitSumTable();
	const material::PbrSplitSumTable ltc = material::LtcTable();
	if ( mraoName.empty() ||
	     !stage( mraoName, Format::kRGBA8Unorm, 1, 1, std::as_bytes( std::span( kMrao ) ) ) ||
	     !stage( kSplitSumName, splitSum.format, splitSum.width, splitSum.height,
	         std::as_bytes( std::span( splitSum.texels ) ) ) ||
	     !stage( kLtcName, ltc.format, ltc.width, ltc.height,
	         std::as_bytes( std::span( ltc.texels ) ) ) )
	{
		v.failure = "the panel material's constant textures were refused";
		return &v;
	}
	return &v;
}

std::optional<std::string> PanelPass::State::EnsureImage(
    Image &image, const Resolution &resolution, float unitsWide, float unitsTall )
{
	if ( image.emissive.IsValid() && image.resolution == resolution )
		return std::nullopt;
	if ( image.emissive.IsValid() )
		RetireImage( currentFrame, image );
	image.resolution = resolution;
	image.levels = LevelsFor( resolution.width, resolution.height );
	TextureDesc raster;
	raster.format = Format::kRGBA8Unorm;
	raster.width = resolution.width;
	raster.height = resolution.height;
	raster.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kSampled };
	raster.debugName = "render.pass.panels raster";
	TextureDesc sampled;
	sampled.format = Format::kRGBA8Srgb;
	sampled.width = resolution.width;
	sampled.height = resolution.height;
	sampled.mipLevels = std::uint32_t( image.levels.size() );
	sampled.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	sampled.debugName = "render.pass.panels image";
	BufferDesc texels;
	texels.size = std::uint64_t( image.levels.back().first + 1 ) * 4;
	texels.usages = {
	    ResourceUsage::kStorageRead, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource };
	texels.debugName = "render.pass.panels levels";
	bool complete = true;
	auto texture = [&]( const TextureDesc &desc, TextureId &out )
	{
		auto made = device->CreateTexture( desc );
		if ( made )
			out = made.Value();
		complete = complete && made;
	};
	auto buffer = [&]( BufferId &out )
	{
		auto made = device->CreateBuffer( texels );
		if ( made )
			out = made.Value();
		complete = complete && made;
	};
	texture( raster, image.emissive );
	TextureDesc coating = raster;
	coating.format = Format::kRGBA16Float;
	coating.debugName = "render.pass.panels coating";
	texture( coating, image.coating );
	texture( sampled, image.emission );
	texture( sampled, image.albedo );
	buffer( image.emissionTexels );
	buffer( image.albedoTexels );
	// The scatter grid: its own size, whatever the image's resolution.
	world_panel::ScatterGridSize( unitsWide, unitsTall, image.scatterWide, image.scatterTall );
	TextureDesc scatterRaster = raster;
	scatterRaster.width = std::uint32_t( image.scatterWide );
	scatterRaster.height = std::uint32_t( image.scatterTall );
	scatterRaster.debugName = "render.pass.panels scatter";
	texture( scatterRaster, image.scatterRaster );
	{
		BufferDesc field = texels;
		field.size = std::uint64_t( image.scatterWide ) * image.scatterTall * 4;
		field.debugName = "render.pass.panels scatter field";
		auto made = device->CreateBuffer( field );
		if ( made )
			image.scatterTexels = made.Value();
		complete = complete && made;
	}
	if ( !complete )
	{
		RetireImage( currentFrame, image );
		return "the panel's images (" + std::to_string( resolution.width ) + " x " +
		       std::to_string( resolution.height ) + ") were refused";
	}
	sampled.debugName = {};
	image.emissionDesc = sampled;
	const BindGroupEntry entries[] = { { 0, {}, 0, 0, image.emissive, {} },
	    { 1, {}, 0, 0, image.coating, {} }, { 2, {}, 0, 0, image.scatterRaster, {} },
	    { 3, {}, 0, 0, {}, pointSampler }, { 4, image.emissionTexels, 0, 0, {}, {} },
	    { 5, image.albedoTexels, 0, 0, {}, {} }, { 6, image.scatterTexels, 0, 0, {}, {} } };
	auto group = device->CreateBindGroup( { mipsLayout, entries } );
	if ( !group )
	{
		RetireImage( currentFrame, image );
		return "the panel's mips bind group was refused";
	}
	image.mipsGroup = group.Value();
	return std::nullopt;
}

std::optional<std::string> PanelPass::State::Rasterize(
    CommandEncoder &encoder, const PanelTarget &target, const Panel &panel, Image &image )
{
	// Each texture's group, and the quads as vertices: the emissive quads in
	// paint order, then the coatings.
	std::vector<BindGroupId> groups( panel.textures.size() );
	BindGroupId whiteGroup;
	auto groupOf = [&]( std::uint32_t texture ) -> std::optional<BindGroupId>
	{
		BindGroupId *slot = texture == world_panel::kWhite ? &whiteGroup : &groups[texture];
		if ( slot->IsValid() )
			return *slot;
		TextureId id = white;
		SamplerDesc samplerDesc;
		if ( texture != world_panel::kWhite )
		{
			if ( !target.textures )
				return std::nullopt;
			id = target.textures->Import( panel.textures[texture] );
			if ( !id.IsValid() )
				return std::nullopt;
			samplerDesc = target.textures->Sampler( panel.textures[texture] );
		}
		const std::optional<SamplerId> sampler = SamplerFor( samplerDesc );
		if ( !sampler )
			return std::nullopt;
		const BindGroupEntry entries[] = { { 0, {}, 0, 0, id, {} }, { 1, {}, 0, 0, {}, *sampler } };
		auto group = device->CreateBindGroup( { rasterLayout, entries } );
		if ( !group )
			return std::nullopt;
		Retire( currentFrame, group.Value() );
		*slot = group.Value();
		return *slot;
	};
	struct Run
	{
		std::uint32_t texture = world_panel::kWhite;
		std::uint8_t blend = world_panel::kBlendAlpha;
		std::uint32_t first = 0;
		std::uint32_t count = 0;
	};
	std::vector<Run> runs[2]; // by world_panel::Layer
	std::vector<RasterVertex> vertices;
	vertices.reserve( panel.quads.size() * 6 );
	for ( const std::uint8_t layer : { world_panel::kLayerEmissive, world_panel::kLayerCoating } )
	{
		for ( const Quad &quad : panel.quads )
		{
			if ( quad.layer > world_panel::kLayerCoating )
				return "a quad's layer is " + std::to_string( quad.layer );
			if ( quad.layer != layer )
				continue;
			if ( quad.texture != world_panel::kWhite && quad.texture >= panel.textures.size() )
				return "a quad names texture " + std::to_string( quad.texture ) + " of " +
				       std::to_string( panel.textures.size() );
			if ( quad.blend > world_panel::kBlendAdditive )
				return "a quad's blend is " + std::to_string( quad.blend );
			if ( !groupOf( quad.texture ) )
				return "texture " +
				       ( quad.texture == world_panel::kWhite
				               ? std::string( "white" )
				               : std::to_string( panel.textures[quad.texture] ) ) +
				       " of the list does not import";
			std::vector<Run> &list = runs[layer];
			const std::uint8_t blend = layer == world_panel::kLayerCoating
			                               ? std::uint8_t( world_panel::kBlendAlpha )
			                               : quad.blend;
			if ( list.empty() || list.back().texture != quad.texture || list.back().blend != blend )
				list.push_back( { quad.texture, blend, std::uint32_t( vertices.size() ), 0 } );
			RasterVertex corners[4];
			const float xs[4] = { quad.x0, quad.x1, quad.x1, quad.x0 };
			const float ys[4] = { quad.y0, quad.y0, quad.y1, quad.y1 };
			const float ss[4] = { quad.s0, quad.s1, quad.s1, quad.s0 };
			const float ts[4] = { quad.t0, quad.t0, quad.t1, quad.t1 };
			for ( int c = 0; c < 4; ++c )
			{
				corners[c].position[0] = xs[c];
				corners[c].position[1] = ys[c];
				corners[c].uv[0] = ss[c];
				corners[c].uv[1] = ts[c];
				std::memcpy( corners[c].color, quad.color, 4 );
			}
			for ( int index : { 0, 1, 2, 0, 2, 3 } )
				vertices.push_back( corners[index] );
			list.back().count += 6;
		}
	}

	BufferId vertexBuffer;
	if ( !vertices.empty() )
	{
		BufferDesc desc;
		desc.size = vertices.size() * sizeof( RasterVertex );
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
		desc.debugName = "render.pass.panels quads";
		auto made = device->CreateBuffer( desc );
		if ( !made )
			return std::string( "the list's vertex buffer was refused" );
		vertexBuffer = made.Value();
		Retire( currentFrame, vertexBuffer );
		encoder.TransitionBuffer(
		    vertexBuffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( vertexBuffer, 0, std::as_bytes( std::span( vertices ) ) );
		encoder.TransitionBuffer(
		    vertexBuffer, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
	}
	if ( !whiteFilled )
	{
		const std::uint8_t texel[4] = { 255, 255, 255, 255 };
		encoder.TransitionBuffer(
		    whiteStaging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( whiteStaging, 0, std::as_bytes( std::span( texel ) ) );
		encoder.TransitionBuffer(
		    whiteStaging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		encoder.TransitionTexture(
		    white, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		TextureBufferCopy copy;
		copy.width = copy.height = 1;
		encoder.CopyBufferToTexture( whiteStaging, white, copy );
		encoder.TransitionTexture(
		    white, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
		whiteFilled = true;
	}

	RasterConstants constants;
	const math::float4x4 toClip = math::PixelToClip( panel.unitsWide, panel.unitsTall );
	for ( int r = 0; r < 4; ++r )
	{
		constants.toClip[r * 4 + 0] = toClip.rows[r].x;
		constants.toClip[r * 4 + 1] = toClip.rows[r].y;
		constants.toClip[r * 4 + 2] = toClip.rows[r].z;
		constants.toClip[r * 4 + 3] = toClip.rows[r].w;
	}
	Viewport viewport;
	viewport.width = float( image.resolution.width );
	viewport.height = float( image.resolution.height );
	const ResourceUsage rest =
	    image.rastersUsed ? ResourceUsage::kSampled : ResourceUsage::kUndefined;
	image.rastersUsed = true;
	auto draw = [&]( const std::vector<Run> &list, bool coating )
	{
		if ( vertexBuffer.IsValid() )
			encoder.SetVertexBuffer( 0, vertexBuffer, 0 );
		for ( const Run &run : list )
		{
			encoder.SetPipeline( coating ? coatingPipeline : rasterPipelines[int( run.blend )] );
			encoder.SetBindGroup( BindGroupRole::kMaterial, *groupOf( run.texture ) );
			constants.params[0] = !coating && run.blend == world_panel::kBlendOpaque ? 1.0f : 0.0f;
			constants.params[1] =
			    !coating && run.blend == world_panel::kBlendAdditive ? 1.0f : 0.0f;
			encoder.SetDrawConstants( 0, std::as_bytes( std::span( &constants, 1 ) ) );
			encoder.Draw( run.count, 1, run.first, 0 );
		}
	};

	// The emissive quads over opaque black (what shows where the panel paints
	// nothing: no emission).
	encoder.BeginLabel( "panel raster" );
	encoder.TransitionTexture( image.emissive, rest, ResourceUsage::kColorAttachment );
	{
		const ColorAttachment colors[] = {
		    { image.emissive, LoadOp::kClear, StoreOp::kStore, { 0.0f, 0.0f, 0.0f, 1.0f }, {} } };
		RenderingDesc rendering;
		rendering.colors = colors;
		rendering.width = image.resolution.width;
		rendering.height = image.resolution.height;
		encoder.BeginRendering( rendering );
		encoder.SetViewport( viewport );
		draw( runs[world_panel::kLayerEmissive], false );
		encoder.EndRendering();
	}
	encoder.TransitionTexture(
	    image.emissive, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	// The same quads at the scatter grid's cell centers (its own raster).
	encoder.TransitionTexture( image.scatterRaster, rest, ResourceUsage::kColorAttachment );
	{
		const ColorAttachment colors[] = { { image.scatterRaster, LoadOp::kClear, StoreOp::kStore,
		    { 0.0f, 0.0f, 0.0f, 1.0f }, {} } };
		RenderingDesc rendering;
		rendering.colors = colors;
		rendering.width = std::uint32_t( image.scatterWide );
		rendering.height = std::uint32_t( image.scatterTall );
		encoder.BeginRendering( rendering );
		Viewport grid;
		grid.width = float( image.scatterWide );
		grid.height = float( image.scatterTall );
		encoder.SetViewport( grid );
		draw( runs[world_panel::kLayerEmissive], false );
		encoder.EndRendering();
	}
	encoder.TransitionTexture(
	    image.scatterRaster, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	// The coatings over nothing.
	encoder.TransitionTexture( image.coating, rest, ResourceUsage::kColorAttachment );
	{
		const ColorAttachment colors[] = {
		    { image.coating, LoadOp::kClear, StoreOp::kStore, { 0.0f, 0.0f, 0.0f, 0.0f }, {} } };
		RenderingDesc rendering;
		rendering.colors = colors;
		rendering.width = image.resolution.width;
		rendering.height = image.resolution.height;
		encoder.BeginRendering( rendering );
		encoder.SetViewport( viewport );
		draw( runs[world_panel::kLayerCoating], true );
		encoder.EndRendering();
	}
	encoder.TransitionTexture(
	    image.coating, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	encoder.EndLabel();

	// Both mip chains in linear light, then into the emission and albedo
	// textures.
	encoder.BeginLabel( "panel mips" );
	for ( BufferId texels : { image.emissionTexels, image.albedoTexels, image.scatterTexels } )
		encoder.TransitionBuffer( texels, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
	encoder.SetPipeline( mipsPipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, image.mipsGroup );
	MipsStep step;
	step.scatter[0] = std::uint32_t( image.scatterWide );
	step.scatter[1] = std::uint32_t( image.scatterTall );
	step.scatter[2] = std::uint32_t( world_panel::kCoatingScatterReach );
	// The scatter field first (the emission reads it).
	step.previous[3] = 3;
	encoder.SetDrawConstants( 0, std::as_bytes( std::span( &step, 1 ) ) );
	encoder.Dispatch( ( std::uint32_t( image.scatterWide ) + kMipsGroup - 1 ) / kMipsGroup,
	    ( std::uint32_t( image.scatterTall ) + kMipsGroup - 1 ) / kMipsGroup );
	encoder.TransitionBuffer(
	    image.scatterTexels, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
	for ( const std::uint32_t mode : { 0u, 1u } )
	{
		const BufferId texels = mode == 0 ? image.emissionTexels : image.albedoTexels;
		for ( std::uint32_t level = 0; level < image.levels.size(); ++level )
		{
			const Level &l = image.levels[level];
			step.level[0] = level;
			step.level[1] = l.first;
			step.level[2] = l.width;
			step.level[3] = l.height;
			step.previous[0] = step.previous[1] = step.previous[2] = 0;
			step.previous[3] = mode;
			if ( level > 0 )
			{
				const Level &p = image.levels[level - 1];
				step.previous[0] = p.first;
				step.previous[1] = p.width;
				step.previous[2] = p.height;
			}
			encoder.SetDrawConstants( 0, std::as_bytes( std::span( &step, 1 ) ) );
			encoder.Dispatch( ( l.width + kMipsGroup - 1 ) / kMipsGroup,
			    ( l.height + kMipsGroup - 1 ) / kMipsGroup );
			encoder.TransitionBuffer(
			    texels, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
		}
	}
	const SubresourceRange all{ 0, std::uint32_t( image.levels.size() ), 0, 1 };
	for ( int chain = 0; chain < 2; ++chain )
	{
		const BufferId texels = chain == 0 ? image.emissionTexels : image.albedoTexels;
		const TextureId texture = chain == 0 ? image.emission : image.albedo;
		encoder.TransitionBuffer(
		    texels, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
		encoder.TransitionTexture( texture,
		    image.filled ? ResourceUsage::kSampled : ResourceUsage::kUndefined,
		    ResourceUsage::kCopyDestination, all );
		for ( std::uint32_t level = 0; level < image.levels.size(); ++level )
		{
			TextureBufferCopy copy;
			copy.bufferOffset = std::uint64_t( image.levels[level].first ) * 4;
			copy.mip = level;
			copy.width = image.levels[level].width;
			copy.height = image.levels[level].height;
			encoder.CopyBufferToTexture( texels, texture, copy );
		}
		encoder.TransitionTexture(
		    texture, ResourceUsage::kCopyDestination, ResourceUsage::kSampled, all );
	}
	image.filled = true;
	encoder.EndLabel();
	return std::nullopt;
}

void PanelPass::State::ReleaseAll( CompletionToken after )
{
	if ( !device )
		return;
	for ( auto &[frame, resource] : retired )
		(void)device->Release( resource, after );
	retired.clear();
	for ( auto &[id, image] : images )
	{
		for ( ResourceId resource : { ResourceId( image.emissive ), ResourceId( image.coating ),
		          ResourceId( image.emissionTexels ), ResourceId( image.albedoTexels ),
		          ResourceId( image.scatterRaster ), ResourceId( image.scatterTexels ),
		          ResourceId( image.mipsGroup ), ResourceId( image.emission ),
		          ResourceId( image.albedo ) } )
		{
			if ( resource.value != 0 )
				(void)device->Release( resource, after );
		}
	}
	images.clear();
	for ( Variant &v : variants )
	{
		v.programs.reset();
		v.draws.reset();
		v.frames.reset();
		v.resolver.reset();
		v.cache.reset();
	}
	variants.clear();
	for ( auto &[desc, sampler] : samplers )
		(void)device->Release( sampler, after );
	samplers.clear();
	for ( ResourceId resource :
	    { ResourceId( rasterPipelines[0] ), ResourceId( rasterPipelines[1] ),
	        ResourceId( rasterPipelines[2] ), ResourceId( coatingPipeline ),
	        ResourceId( rasterLayout ), ResourceId( mipsPipeline ), ResourceId( mipsLayout ),
	        ResourceId( pointSampler ), ResourceId( white ), ResourceId( whiteStaging ) } )
	{
		if ( resource.value != 0 )
			(void)device->Release( resource, after );
	}
	rasterPipelines[0] = rasterPipelines[1] = rasterPipelines[2] = PipelineId();
	coatingPipeline = PipelineId();
	rasterLayout = BindGroupLayoutId();
	mipsPipeline = PipelineId();
	mipsLayout = BindGroupLayoutId();
	pointSampler = SamplerId();
	white = TextureId();
	whiteStaging = BufferId();
	whiteFilled = false;
	made = false;
	madeFailure.clear();
	currentFrame = ~0ull;
	device = nullptr;
}

PanelPass::PanelPass( std::span<const std::uint32_t> mipsModule )
    : m_State( std::make_unique<State>() )
{
	m_State->mipsModule = mipsModule;
}

// Without ReleaseDevice the device may be gone: the handles are dropped.
PanelPass::~PanelPass() = default;

bool PanelPass::Submit( std::uint64_t hostFrame, Panel panel )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	auto refuse = [&]( std::string why )
	{
		++s.stats.refused;
		s.stats.lastFailure = std::move( why );
		return false;
	};
	if ( !world_panel::ResolutionFits( panel.resolution, panel.unitsWide, panel.unitsTall ) )
		return refuse( "panel " + std::to_string( panel.id ) +
		               ": its resolution is not world_panel::ChooseResolution's for its units" );
	float normal[3];
	Cross( panel.placement.down, panel.placement.right, normal );
	if ( !Normalize( normal ) )
		return refuse( "panel " + std::to_string( panel.id ) + ": its placement spans no area" );
	const auto key = std::make_pair( hostFrame, panel.id );
	if ( s.lists.count( key ) )
		return refuse( "panel " + std::to_string( panel.id ) + " already has frame " +
		               std::to_string( hostFrame ) + "'s list" );
	s.lists.emplace( key, std::make_shared<const Panel>( std::move( panel ) ) );
	++s.stats.submitted;
	// Frames long past are done.
	while ( !s.lists.empty() && s.lists.begin()->first.first + 16 < hostFrame )
		s.lists.erase( s.lists.begin() );
	return true;
}

bool PanelPass::Submitted( std::uint64_t hostFrame, std::uint64_t id ) const
{
	std::lock_guard<std::mutex> guard( m_State->lock );
	return m_State->lists.count( std::make_pair( hostFrame, id ) ) != 0;
}

std::uint32_t PanelPass::QueueView( PanelView view )
{
	if ( view.panels.empty() )
		return 0;
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	const std::uint32_t serial = s.nextSerial;
	s.nextSerial = ( s.nextSerial + 1 ) & kPanelSerialMask;
	if ( s.nextSerial == 0 )
		s.nextSerial = 1;
	s.views[serial] = std::move( view );
	++s.stats.viewsQueued;
	// A backlog this deep never records (no backend slots): drop the oldest.
	while ( s.views.size() > 256 )
		s.views.erase( s.views.begin() );
	return kPanelTag | serial;
}

void PanelPass::Remove( std::uint64_t id )
{
	std::lock_guard<std::mutex> guard( m_State->lock );
	m_State->removed.push_back( id );
}

PanelStats PanelPass::Stats() const
{
	std::lock_guard<std::mutex> guard( m_State->lock );
	return m_State->stats;
}

std::uint64_t PanelPass::Failures() const
{
	std::lock_guard<std::mutex> guard( m_State->lock );
	return m_State->stats.viewsFailed;
}

void PanelPass::ReleaseDevice( IRenderDevice2 &device )
{
	State &s = *m_State;
	if ( s.device != &device )
		return;
	s.ReleaseAll( CompletionToken() );
}

void PanelPass::Record( std::uint32_t tag, CommandEncoder &encoder, const PanelTarget &target )
{
	State &s = *m_State;
	if ( !IsPanelTag( tag ) )
	{
		s.Fail( "a slot's tag is not a panel tag" );
		return;
	}
	const std::uint32_t serial = tag & kPanelSerialMask;
	PanelView view;
	std::vector<std::shared_ptr<const Panel>> panels;
	std::vector<std::uint64_t> removed;
	{
		std::lock_guard<std::mutex> guard( s.lock );
		auto found = s.views.find( serial );
		if ( found != s.views.end() )
		{
			view = std::move( found->second );
			s.views.erase( found );
			s.recorded.emplace_back( serial, view );
			while ( s.recorded.size() > 64 )
				s.recorded.pop_front();
		}
		else
		{
			const auto again = std::find_if( s.recorded.begin(), s.recorded.end(),
			    [&]( const auto &entry )
			    {
				    return entry.first == serial;
			    } );
			if ( again == s.recorded.end() )
			{
				++s.stats.viewsFailed;
				s.stats.lastFailure = "a panel slot names no queued view";
				return;
			}
			view = again->second;
		}
		for ( std::uint64_t id : view.panels )
		{
			const auto list = s.lists.find( std::make_pair( view.hostFrame, id ) );
			if ( list == s.lists.end() )
			{
				++s.stats.viewsFailed;
				s.stats.lastFailure = "panel " + std::to_string( id ) + " has no list for frame " +
				                      std::to_string( view.hostFrame );
				return;
			}
			panels.push_back( list->second );
		}
		removed.swap( s.removed );
	}

	if ( !target.device || !target.color.IsValid() || !target.depth.IsValid() )
	{
		s.Fail( "a panel slot's target is incomplete" );
		return;
	}
	if ( s.device && s.device != target.device )
		s.ReleaseAll( CompletionToken() ); // a device without ReleaseDevice: its handles drop
	s.device = target.device;
	if ( std::optional<std::string> why = s.Make() )
	{
		s.Fail( *why );
		return;
	}

	// A new frame: what earlier frames used goes behind their submissions.
	if ( target.frame != s.currentFrame )
	{
		if ( target.frame != 0 )
		{
			std::erase_if( s.retired,
			    [&]( const auto &entry )
			    {
				    if ( entry.first >= target.frame )
					    return false;
				    (void)s.device->Release( entry.second, target.submitted );
				    return true;
			    } );
			for ( Variant &v : s.variants )
			{
				if ( v.cache )
					v.cache->Retire( target.submitted );
				if ( v.programs )
					v.programs->Retire( target.submitted );
				if ( v.draws )
					v.draws->Retire( target.submitted );
				if ( v.frames )
					v.frames->Retire( target.submitted );
			}
		}
		s.currentFrame = target.frame;
	}
	for ( std::uint64_t id : removed )
	{
		const auto image = s.images.find( id );
		if ( image != s.images.end() )
		{
			s.RetireImage( s.currentFrame, image->second );
			s.images.erase( image );
		}
		for ( Variant &v : s.variants )
		{
			if ( v.programs )
				v.programs->Remove( id );
			if ( v.draws )
				v.draws->Remove( id );
			v.boundMaterial.erase( id );
			v.boundLighting.erase( id );
		}
	}

	Variant *variant = s.VariantFor( target );
	if ( !variant->failure.empty() )
	{
		s.Fail( variant->failure );
		return;
	}
	Variant &v = *variant;

	// The panels facing the eye; each one's image for this frame.
	float eye[3];
	std::copy( target.terms.eye, target.terms.eye + 3, eye );
	std::vector<const Panel *> drawn;
	for ( const std::shared_ptr<const Panel> &panel : panels )
	{
		float normal[3];
		Cross( panel->placement.down, panel->placement.right, normal );
		float toEye[3];
		for ( int k = 0; k < 3; ++k )
			toEye[k] = eye[k] - panel->placement.origin[k];
		if ( normal[0] * toEye[0] + normal[1] * toEye[1] + normal[2] * toEye[2] <= 0.0f )
			continue; // seen from behind: the legacy screen is not drawn either
		Image &image = s.images[panel->id];
		if ( std::optional<std::string> why =
		         s.EnsureImage( image, panel->resolution, panel->unitsWide, panel->unitsTall ) )
		{
			s.Fail( "panel " + std::to_string( panel->id ) + ": " + *why );
			return;
		}
		if ( !image.rasterized || image.rasterizedFrame != view.hostFrame )
		{
			if ( std::optional<std::string> why = s.Rasterize( encoder, target, *panel, image ) )
			{
				s.Fail( "panel " + std::to_string( panel->id ) + ": " + *why );
				return;
			}
			image.rasterized = true;
			image.rasterizedFrame = view.hostFrame;
			std::lock_guard<std::mutex> guard( s.lock );
			++s.stats.rasterized;
			s.stats.lastResolution = image.resolution;
		}
		const std::tuple<TextureId, TextureId, float> bindings(
		    image.emission, image.albedo, panel->emissionScale );
		const auto bound = v.boundMaterial.find( panel->id );
		if ( bound == v.boundMaterial.end() || bound->second != bindings )
		{
			material::ProgramRequest request = v.program->request;
			// The panel's emission scale in the material constants
			// (SurfaceConstants::emission.x, the pbr point's $emissionscale).
			material::SurfaceConstants constants;
			if ( request.material.constants.size() != sizeof( constants ) )
			{
				s.Fail( "the panel material's constants are not SurfaceConstants" );
				return;
			}
			std::memcpy( &constants, request.material.constants.data(), sizeof( constants ) );
			constants.emission[0] = panel->emissionScale;
			std::memcpy( request.material.constants.data(), &constants, sizeof( constants ) );
			// The images: the surface program's base (1) and emission (13),
			// clamped at the panel's edges, bilinear within the nearest mip:
			// the image holds 1 to 1.41 texels per screen pixel where the panel
			// is densest (world_panel::ChooseResolution), so that is mip 0, and
			// text keeps its raster's edges (a trilinear blend with mip 1
			// would soften them).
			SamplerDesc imageSampler;
			imageSampler.mipFilter = Filter::kNearest;
			imageSampler.address = AddressMode::kClampToEdge;
			int boundTextures = 0;
			for ( material::ProgramTexture &texture : request.material.textures )
			{
				if ( texture.binding == 1 || texture.binding == 13 )
				{
					texture.name.clear();
					texture.external = texture.binding == 1 ? image.albedo : image.emission;
					texture.externalDesc = image.emissionDesc;
					texture.sampler = imageSampler;
					++boundTextures;
				}
			}
			if ( boundTextures != 2 || !v.programs->Set( panel->id, request ) )
			{
				s.Fail(
				    "panel " + std::to_string( panel->id ) + ": its material group was refused" );
				return;
			}
			v.boundMaterial[panel->id] = bindings;
		}
		// The scene's light at the panel, which the coatings' albedo reflects:
		// its ambient cube, as model lighting in the draw group.
		{
			const material::ModelLighting lighting =
			    material::PackSourceModelLighting( target.terms.eye, panel->ambientCube, {} );
			const material::GroupRequest draw = v.resolver->Program().DrawGroup( {}, lighting );
			std::vector<std::byte> &last = v.boundLighting[panel->id];
			if ( draw.constants != last || !v.draws->Group( panel->id ) )
			{
				if ( !v.draws->Set( panel->id, draw ) )
				{
					s.Fail(
					    "panel " + std::to_string( panel->id ) + ": its draw group was refused" );
					return;
				}
				last = draw.constants;
			}
		}
		drawn.push_back( panel.get() );
	}
	if ( drawn.empty() )
	{
		std::lock_guard<std::mutex> guard( s.lock );
		++s.stats.viewsDrawn;
		return;
	}

	// The frame group for the slot's terms (the same terms as the world's
	// surfaces), set again only when they change.
	material::FrameTerms terms = target.terms;
	terms.splitSumTable = kSplitSumName;
	terms.ltcTable = kLtcName;
	const std::optional<material::GroupRequest> frameRequest =
	    v.resolver->FrameGroup( *v.program, terms );
	const std::uint64_t frameId = v.program->request.frameLayout.value;
	if ( frameRequest && frameRequest->constants != v.frameConstants )
	{
		if ( !v.frames->Set( frameId, *frameRequest ) )
		{
			s.Fail( "the panels' frame group was refused" );
			return;
		}
		v.frameConstants = frameRequest->constants;
	}
	v.cache->RecordUploads( encoder );
	v.programs->RecordUploads( encoder );
	v.draws->RecordUploads( encoder );
	v.frames->RecordUploads( encoder );
	const material::ResidentGroup *frameGroup = frameRequest ? v.frames->Group( frameId ) : nullptr;
	if ( frameRequest && !frameGroup )
	{
		s.Fail( "the panels' frame group is not resident" );
		return;
	}

	// The quads, two triangles each, in world space.
	std::vector<material::SurfaceWorldVertex> vertices;
	for ( const Panel *panel : drawn )
	{
		const Placement &p = panel->placement;
		material::SurfaceWorldVertex corners[4];
		const float us[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
		const float vs[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		float normal[3], tangentS[3], tangentT[3];
		Cross( p.down, p.right, normal );
		std::copy( p.right, p.right + 3, tangentS );
		std::copy( p.down, p.down + 3, tangentT );
		(void)Normalize( normal );
		(void)Normalize( tangentS );
		(void)Normalize( tangentT );
		for ( int c = 0; c < 4; ++c )
		{
			for ( int k = 0; k < 3; ++k )
			{
				corners[c].position[k] = p.origin[k] + p.right[k] * us[c] + p.down[k] * vs[c];
				corners[c].normal[k] = normal[k];
				corners[c].tangentS[k] = tangentS[k];
				corners[c].tangentT[k] = tangentT[k];
			}
			corners[c].uv[0] = us[c];
			corners[c].uv[1] = vs[c];
		}
		for ( int index : { 0, 1, 2, 0, 2, 3 } )
			vertices.push_back( corners[index] );
	}
	BufferDesc desc;
	desc.size = vertices.size() * sizeof( material::SurfaceWorldVertex );
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
	desc.debugName = "render.pass.panels surfaces";
	auto vertexBuffer = s.device->CreateBuffer( desc );
	if ( !vertexBuffer )
	{
		s.Fail( "the panels' vertex buffer was refused" );
		return;
	}
	s.Retire( s.currentFrame, vertexBuffer.Value() );
	encoder.TransitionBuffer(
	    vertexBuffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( vertexBuffer.Value(), 0, std::as_bytes( std::span( vertices ) ) );
	encoder.TransitionBuffer(
	    vertexBuffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kVertex );

	// The program under the frame's debug controls (RFC 0014).
	PipelineId pipeline = v.program->request.pipeline;
	if ( !frame::DebugControlsNeutral( view.debug ) )
	{
		auto debug = v.resolver->DebugPipeline(
		    *v.program, frame::DebugSpecializationFor( view.debug, v.program->name ) );
		if ( !debug )
		{
			s.Fail( "debug view: " + debug.Error() );
			return;
		}
		pipeline = debug.Value();
	}
	material::FamilyDrawConstants constants;
	std::memcpy( constants.toClip, view.toClip, sizeof( constants.toClip ) );
	for ( int i = 0; i < 4; ++i )
		constants.world[i * 5] = 1.0f;
	const auto constantBytes = std::as_bytes( std::span( &constants, 1 ) );

	const ColorAttachment colors[] = { { target.color, LoadOp::kLoad, StoreOp::kStore, {}, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.depth = DepthAttachment{ target.depth, LoadOp::kLoad, StoreOp::kStore, 1.0f };
	rendering.width = target.width;
	rendering.height = target.height;
	bool complete = true;
	std::string failure;
	encoder.BeginLabel( "core panels" );
	encoder.BeginRendering( rendering );
	encoder.SetViewport( view.viewport );
	encoder.SetPipeline( pipeline );
	encoder.SetVertexBuffer( 0, vertexBuffer.Value(), 0 );
	if ( frameGroup )
		encoder.SetBindGroup( BindGroupRole::kFrame, frameGroup->group );
	std::uint32_t first = 0;
	std::uint32_t drawnCount = 0;
	for ( const Panel *panel : drawn )
	{
		const material::DrawProgram *program = v.programs->Program( panel->id );
		const material::DrawGroup *draw = v.draws->Group( panel->id );
		if ( !program || !program->material.group.IsValid() ||
		     ( program->drawLayout.IsValid() && !draw ) ||
		     ( program->viewLayout.IsValid() && !program->hasNeutralView ) )
		{
			complete = false;
			failure = "panel " + std::to_string( panel->id ) + ": its groups are not resident";
			first += 6;
			continue;
		}
		if ( program->viewLayout.IsValid() )
			encoder.SetBindGroup( BindGroupRole::kView, program->neutralView.group );
		encoder.SetBindGroup( BindGroupRole::kMaterial, program->material.group );
		if ( draw )
			encoder.SetBindGroup( BindGroupRole::kDraw, draw->resident.group );
		encoder.SetDrawConstants( 0, constantBytes.first( program->drawConstantBytes ) );
		encoder.Draw( 6, 1, first, 0 );
		first += 6;
		++drawnCount;
	}
	encoder.EndRendering();
	encoder.EndLabel();

	std::lock_guard<std::mutex> guard( s.lock );
	++s.stats.viewsDrawn;
	s.stats.panelsDrawn += drawnCount;
	std::uint64_t bytes = 0;
	for ( const auto &[id, image] : s.images )
		bytes += image.emissive.IsValid() ? ImageBytes( image.resolution ) : 0;
	s.stats.textureBytes = bytes;
	if ( !complete )
	{
		++s.stats.viewsFailed;
		s.stats.lastFailure = failure;
	}
}

} // namespace render::pass::panels
