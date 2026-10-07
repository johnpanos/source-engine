//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pbr-specular-aa.v1's image oracles (RFC 0012 A2): the
//			surface program's pbr point on a bumpy low-roughness metal plane
//			under one point light, seen at a grazing angle while the camera
//			strafes by fractions of a pixel. A 4x4 supersampled render of the
//			same poses, box-downsampled, is the reference. Per frame the error
//			is the display-referred image (Reinhard per sample) minus the
//			reference; shimmer is its per-pixel
//			temporal standard deviation over the steps, and the reference
//			error its mean absolute value. Declared before measuring:
//			shimmer with the filter <= 0.9 x without, reference error with
//			<= without, mean energy within 3 %; a flat (unmapped) plane is
//			bit for bit the same with and without. Seeded: disabled (fails
//			shimmer), biased without derivatives (fails flat identity).
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_receiver.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "spv/specular_aa_defects_spv.h"

#include "render/light_set.h"
#include "render/material/pbr_family.h"
#include "render/pass/lights/clusters.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{
namespace
{
using namespace render::device;
using math::float3;

constexpr std::uint32_t kSize = 128;
constexpr std::uint32_t kSuper = 4; // reference samples per axis
constexpr std::uint32_t kTileSize = 16;
constexpr float kExtent = 700.0f;
constexpr float kUvScale = 1.0f / 256.0f; // world units to UV: one texel per unit
constexpr std::uint32_t kNormalSize = 256;
constexpr int kSteps = 16;
constexpr float kStepUnits = 0.5f;

constexpr float kShimmerLimit = 0.9f;
constexpr float kEnergyLimit = 0.03f;

// Bumps of an 8-texel period at about 25 degrees' largest tilt, as a Source
// normal map stores them (RG = tangent-space XY * 0.5 + 0.5), and their mips
// box-filtered as stored bytes (no variance-aware filtering: RFC 0012 A4).
std::vector<std::vector<std::byte>> NormalMips()
{
	std::vector<std::vector<std::byte>> levels;
	std::vector<float> xyz( kNormalSize * kNormalSize * 3 );
	for ( std::uint32_t y = 0; y < kNormalSize; ++y )
		for ( std::uint32_t x = 0; x < kNormalSize; ++x )
		{
			const float k = 2.0f * 3.14159265f / 8.0f;
			const float sx = 0.45f * std::cos( k * float( x ) ) * std::sin( k * 0.5f * float( y ) );
			const float sy = 0.45f * std::sin( k * float( y ) ) * std::cos( k * 0.5f * float( x ) );
			const float n = std::sqrt( sx * sx + sy * sy + 1.0f );
			float *t = &xyz[( std::size_t( y ) * kNormalSize + x ) * 3];
			t[0] = -sx / n;
			t[1] = -sy / n;
			t[2] = 1.0f / n;
		}
	std::uint32_t size = kNormalSize;
	for ( ;; )
	{
		std::vector<std::byte> bytes( std::size_t( size ) * size * 4 );
		for ( std::size_t i = 0; i < std::size_t( size ) * size; ++i )
		{
			for ( int c = 0; c < 3; ++c )
				bytes[i * 4 + c] = std::byte( std::clamp(
				    int( std::lround( ( xyz[i * 3 + c] * 0.5f + 0.5f ) * 255.0f ) ), 0, 255 ) );
			bytes[i * 4 + 3] = std::byte( 255 );
		}
		levels.push_back( std::move( bytes ) );
		if ( size == 1 )
			break;
		const std::uint32_t half = size / 2;
		std::vector<float> next( std::size_t( half ) * half * 3 );
		for ( std::uint32_t y = 0; y < half; ++y )
			for ( std::uint32_t x = 0; x < half; ++x )
				for ( int c = 0; c < 3; ++c )
					next[( std::size_t( y ) * half + x ) * 3 + c] =
					    0.25f * ( xyz[( std::size_t( 2 * y ) * size + 2 * x ) * 3 + c] +
					                xyz[( std::size_t( 2 * y ) * size + 2 * x + 1 ) * 3 + c] +
					                xyz[( std::size_t( 2 * y + 1 ) * size + 2 * x ) * 3 + c] +
					                xyz[( std::size_t( 2 * y + 1 ) * size + 2 * x + 1 ) * 3 + c] );
		xyz = std::move( next );
		size = half;
	}
	return levels;
}

// The plane z = 0 with tiled UVs (lab_receiver's mesh has none).
std::vector<std::byte> PlaneMesh()
{
	std::vector<std::byte> bytes;
	const float corners[4][2] = { { -kExtent, -kExtent }, { kExtent, -kExtent },
	    { kExtent, kExtent }, { -kExtent, kExtent } };
	for ( int corner : { 0, 1, 2, 0, 2, 3 } )
	{
		material::SurfaceModelVertex vertex;
		vertex.position[0] = corners[corner][0];
		vertex.position[1] = corners[corner][1];
		vertex.normal[2] = 1.0f;
		vertex.tangent[0] = 1.0f;
		vertex.tangent[3] = 1.0f;
		vertex.uv[0] = corners[corner][0] * kUvScale;
		vertex.uv[1] = corners[corner][1] * kUvScale;
		const auto *raw = reinterpret_cast<const std::byte *>( &vertex );
		bytes.insert( bytes.end(), raw, raw + sizeof( vertex ) );
	}
	return bytes;
}

light_set::RuntimeLight Light()
{
	light_set::RuntimeLight light;
	light.id = 1;
	light.kind = light_set::LightKind::Dynamic;
	light.shape = light_set::LightShape::Point;
	const float3 at{ 0, 420, 70 };
	std::memcpy( light.position, &at, sizeof( light.position ) );
	light.color[0] = light.color[1] = light.color[2] = 2000.0f;
	light.falloff = light_set::LightFalloff::InverseSquare;
	light.sourceRadius = light_set::kInverseSquareSourceRadius;
	return light;
}

struct Lab
{
	IRenderDevice2 &device;
	resources::TextureCache textures;
	material::GroupResidency groups;
	std::unique_ptr<Canvas> canvas;    // kSize
	std::unique_ptr<Canvas> reference; // kSize * kSuper
	std::unique_ptr<material::PbrFamily> family;
	std::unique_ptr<LabClusterLists> clusters;
	std::map<std::string, std::uint64_t> materialGroups;
	std::uint64_t drawGroup = 0;
	std::uint64_t nextGroup = 1;
	std::vector<std::byte> plane;
	std::vector<light_set::RuntimeLight> lights{ Light() };

	explicit Lab( IRenderDevice2 &d ) : device( d ), textures( d ), groups( d, textures ) {}
};

struct Surface
{
	const char *name;
	int roughness; // MRAO byte
	bool bumpy;
};
constexpr Surface kSurfaces[] = {
    { "bumpy-r10", 26, true }, { "bumpy-r25", 64, true }, { "flat-r10", 26, false } };

std::optional<std::string> Prepare( Lab &lab, std::span<const std::uint32_t> module )
{
	if ( auto why = Canvas::Create( lab.device, kSize, kSize, lab.canvas ) )
		return why;
	if ( auto why = Canvas::Create( lab.device, kSize * kSuper, kSize * kSuper, lab.reference ) )
		return why;
	auto family = material::CreateSurfaceFamily<material::PbrFamily>(
	    lab.device, kCanvasColor, kCanvasDepth, 1, module );
	if ( !family )
		return std::string( "the surface program was refused" );
	lab.family = std::move( family ).Value();
	bool staged = StageConstant(
	    lab.textures, "sa/base", Format::kRGBA8Srgb, ByteTexel( 255, 255, 255, 255 ) );
	staged = staged && StageConstant( lab.textures, "sa/flat-normal", Format::kRGBA8Unorm,
	                       ByteTexel( 128, 128, 255, 255 ) );
	for ( const Surface &s : kSurfaces )
		staged = staged && StageConstant( lab.textures, std::string( "sa/mrao-" ) + s.name,
		                       Format::kRGBA8Unorm, ByteTexel( 255, s.roughness, 255, 255 ) );
	const auto mips = NormalMips();
	std::vector<std::span<const std::byte>> levels;
	for ( const auto &level : mips )
		levels.emplace_back( level );
	TextureDesc normal;
	normal.format = Format::kRGBA8Unorm;
	normal.width = normal.height = kNormalSize;
	normal.mipLevels = std::uint32_t( mips.size() );
	staged = staged && lab.textures.StageMips( "sa/bumps", normal, levels ).HasValue();
	const material::PbrSplitSumTable table = material::SplitSumTable();
	TextureDesc desc;
	desc.format = table.format;
	desc.width = table.width;
	desc.height = table.height;
	staged = staged &&
	         lab.textures.Stage( "sa/splitsum", desc, std::as_bytes( std::span( table.texels ) ) )
	             .HasValue();
	if ( !staged )
		return std::string( "a fixture texture was refused" );
	for ( const Surface &s : kSurfaces )
	{
		material::PbrClaim claim;
		claim.claimed = true;
		claim.normalMap = s.bumpy;
		material::SurfaceTextures textures;
		textures.base = "sa/base";
		textures.mrao = std::string( "sa/mrao-" ) + s.name;
		textures.bump = s.bumpy ? "sa/bumps" : "sa/flat-normal";
		auto request = lab.family->Request( claim, textures );
		if ( !request )
			return std::string( "the pbr point was refused" );
		const std::uint64_t id = lab.nextGroup++;
		if ( !lab.groups.Set( id, request.Value().material ) )
			return std::string( "a material group was refused" );
		lab.materialGroups[s.name] = id;
	}
	lab.drawGroup = lab.nextGroup++;
	if ( !lab.groups.Set( lab.drawGroup, lab.family->LightingGroup( material::ModelLighting() ) ) )
		return std::string( "the draw group was refused" );
	lab.plane = PlaneMesh();
	return std::nullopt;
}

std::optional<material::GroupRequest> ViewGroup( Lab &lab, const ReceiverView &view )
{
	pass::lights::ClusterLimits limits = pass::lights::DesktopClusterLimits();
	limits.tileSizePixels = kTileSize;
	pass::lights::ClusterViewDesc desc;
	desc.view = view.view;
	desc.projection = view.projection;
	desc.widthPixels = desc.heightPixels = view.size;
	desc.nearZ = view.nearZ;
	desc.farZ = view.farZ;
	auto grid = pass::lights::CreateClusterGrid( desc, limits );
	if ( !grid )
		return std::nullopt;
	lab.clusters = LabClusterLists::Create( lab.device, grid.Value(), lab.lights );
	if ( !lab.clusters )
		return std::nullopt;
	material::SurfaceViewGpu gpu;
	gpu.grid[0] = grid.Value().tilesX;
	gpu.grid[1] = grid.Value().tilesY;
	gpu.grid[2] = grid.Value().slices;
	gpu.grid[3] = kTileSize;
	gpu.slices[0] = grid.Value().sliceScale;
	gpu.slices[1] = grid.Value().sliceBias;
	gpu.slices[2] = grid.Value().nearZ;
	const math::float4 &z = view.view.rows[2];
	gpu.viewDistance[0] = -z.x;
	gpu.viewDistance[1] = -z.y;
	gpu.viewDistance[2] = -z.z;
	gpu.viewDistance[3] = -z.w;
	std::vector<material::SurfaceLightGpu> records;
	for ( const light_set::RuntimeLight &light : lab.lights )
		records.push_back( material::PackSurfaceLight( light ) );
	auto request = lab.family->Program().ViewGroup( gpu, {}, {}, records );
	lab.clusters->Bind( request );
	return request;
}

// The red channel of one frame (the scene is white, so channels agree).
std::optional<std::string> Render( Lab &lab, const Surface &surface, bool filter, float shift,
    bool supersampled, std::vector<float> &out )
{
	Canvas &canvas = supersampled ? *lab.reference : *lab.canvas;
	const std::uint32_t size = canvas.Width();
	const ReceiverView view = MakeReceiverView( { shift, -500, 80 }, { shift, 60, 0 }, size );
	material::SurfaceFrame terms;
	terms.eye[0] = view.eye.x;
	terms.eye[1] = view.eye.y;
	terms.eye[2] = view.eye.z;
	const std::uint64_t frameGroup = lab.nextGroup++;
	const std::uint64_t viewGroup = lab.nextGroup++;
	const std::optional<material::GroupRequest> viewRequest = ViewGroup( lab, view );
	if ( !viewRequest )
		return std::string( "the cluster lists were not built" );
	if ( !lab.groups.Set( frameGroup, lab.family->FrameGroup( terms, "sa/splitsum" ) ) ||
	     !lab.groups.Set( viewGroup, *viewRequest ) )
		return std::string( "a frame or view group was refused" );
	if ( auto why = canvas.Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr ) )
		return why;
	material::PbrClaim claim;
	claim.claimed = true;
	claim.normalMap = surface.bumpy;
	material::SurfaceVariant variant = claim.Variant();
	variant.terms |= material::kSurfaceClustered;
	variant.viewFeatures =
	    filter ? material::kSurfaceAllViewFeatures
	           : material::kSurfaceAllViewFeatures & ~material::kSurfaceViewSpecularAa;
	auto pipeline = lab.family->Program().Pipeline( variant, {} );
	if ( !pipeline )
		return std::string( "no pipeline" );
	auto group = [&]( std::uint64_t id ) -> BindGroupId
	{
		const material::ResidentGroup *resident = lab.groups.Group( id );
		return resident ? resident->group : BindGroupId();
	};
	CanvasDraw draw;
	draw.pipeline = pipeline.Value();
	draw.groups[std::size_t( BindGroupRole::kFrame )] = group( frameGroup );
	draw.groups[std::size_t( BindGroupRole::kView )] = group( viewGroup );
	draw.groups[std::size_t( BindGroupRole::kMaterial )] =
	    group( lab.materialGroups.at( surface.name ) );
	draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.drawGroup );
	draw.vertices = canvas.Vertices( lab.plane );
	draw.vertexCount = 6;
	material::FamilyDrawConstants constants;
	std::memcpy( constants.toClip, &view.toClip, sizeof( constants.toClip ) );
	constants.world[0] = constants.world[5] = constants.world[10] = constants.world[15] = 1.0f;
	const auto *bytes = reinterpret_cast<const std::byte *>( &constants );
	draw.constants.assign( bytes, bytes + sizeof( constants ) );
	const CanvasDraw draws[] = { draw };
	CanvasImage image;
	std::optional<std::string> why =
	    canvas.Render( lab.textures, lab.groups, draws, { 0, 0, 0, 1 }, &image );
	lab.groups.Remove( frameGroup );
	lab.groups.Remove( viewGroup );
	if ( why )
		return why;
	// Display-referred (Reinhard x / (1 + x) per sample, so HDR peaks do not
	// dominate), then box-downsampled to kSize.
	const std::uint32_t factor = size / kSize;
	out.assign( std::size_t( kSize ) * kSize, 0.0f );
	for ( std::uint32_t y = 0; y < size; ++y )
		for ( std::uint32_t x = 0; x < size; ++x )
		{
			const float value = std::max( image.At( x, y )[0], 0.0f );
			out[std::size_t( y / factor ) * kSize + x / factor] +=
			    value / ( 1.0f + value ) / float( factor * factor );
		}
	return std::nullopt;
}

struct Sequence
{
	std::vector<std::vector<float>> frames;
};

std::optional<std::string> Capture(
    Lab &lab, const Surface &surface, bool filter, bool supersampled, Sequence &out )
{
	out.frames.resize( kSteps );
	for ( int k = 0; k < kSteps; ++k )
		if ( auto why = Render( lab, surface, filter, kStepUnits * float( k ), supersampled,
		         out.frames[std::size_t( k )] ) )
			return why;
	return std::nullopt;
}

struct Errors
{
	double shimmer = 0; // mean per-pixel temporal standard deviation of (frame - reference)
	double l1 = 0;      // mean |frame - reference|
	double energy = 0;  // mean value
};

Errors Measure( const Sequence &test, const Sequence &reference )
{
	Errors e;
	const std::size_t pixels = std::size_t( kSize ) * kSize;
	for ( std::size_t i = 0; i < pixels; ++i )
	{
		double sum = 0, squares = 0;
		for ( int k = 0; k < kSteps; ++k )
		{
			const double error = double( test.frames[k][i] ) - reference.frames[k][i];
			sum += error;
			squares += error * error;
			e.l1 += std::abs( error );
			e.energy += test.frames[k][i];
		}
		const double mean = sum / kSteps;
		e.shimmer += std::sqrt( std::max( squares / kSteps - mean * mean, 0.0 ) );
	}
	e.shimmer /= double( pixels );
	e.l1 /= double( pixels * kSteps );
	e.energy /= double( pixels * kSteps );
	return e;
}

std::string Numbers( const Errors &on, const Errors &off, const Errors &ref )
{
	char text[256];
	std::snprintf( text, sizeof( text ),
	    "shimmer %.5f/%.5f (%.3f), L1 %.5f/%.5f (%.3f), energy %.4f/%.4f, reference %.4f",
	    on.shimmer, off.shimmer, off.shimmer > 0 ? on.shimmer / off.shimmer : 0.0, on.l1, off.l1,
	    off.l1 > 0 ? on.l1 / off.l1 : 0.0, on.energy, off.energy, ref.energy );
	return text;
}

std::optional<std::string> Checks( Lab &lab, Results &results )
{
	for ( const Surface &surface : kSurfaces )
	{
		Sequence on, off, ref;
		if ( auto why = Capture( lab, surface, true, false, on ) )
			return why;
		if ( auto why = Capture( lab, surface, false, false, off ) )
			return why;
		const std::string prefix = std::string( "specular-aa.image." ) + surface.name;
		if ( !surface.bumpy )
		{
			bool same = true;
			for ( int k = 0; k < kSteps; ++k )
				same = same && std::memcmp( on.frames[k].data(), off.frames[k].data(),
				                   on.frames[k].size() * sizeof( float ) ) == 0;
			results.That( same, prefix + ".identity" );
			continue;
		}
		if ( auto why = Capture( lab, surface, false, true, ref ) )
			return why;
		const Errors a = Measure( on, ref ), b = Measure( off, ref ), r = Measure( ref, ref );
		const std::string numbers = Numbers( a, b, r );
		std::fprintf( stderr, "%s: %s\n", prefix.c_str(), numbers.c_str() );
		results.That( r.energy > 0.02, prefix + ".highlight-visible", numbers );
		results.That( a.shimmer <= kShimmerLimit * b.shimmer, prefix + ".shimmer", numbers );
		results.That( a.l1 <= b.l1, prefix + ".reference", numbers );
		results.That(
		    std::abs( a.energy / b.energy - 1.0 ) <= kEnergyLimit, prefix + ".energy", numbers );
	}
	return std::nullopt;
}

std::optional<std::string> Run( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		Lab lab( *device );
		if ( auto why = Prepare( lab, module ) )
			return why;
		if ( auto why = Checks( lab, results ) )
			return why;
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}
} // namespace

int RunSpecularAaImageSuite( int argc, char **argv )
{
	const Seeded seeded[] = {
	    { "disabled", spirv::kSurfaceSpecularAaDisabled, "specular-aa.image.bumpy" },
	    { "biased", spirv::kSurfaceSpecularAaBiased, "specular-aa.image.flat-r10.identity" } };
	return RunSeededSuite( argc, argv, "specular-aa-image", seeded, Run );
}
} // namespace render::lab
