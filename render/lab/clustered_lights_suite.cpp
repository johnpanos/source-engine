//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite clustered-lights (RFC 0016 K11 "Model assembly",
//			the runtime-lights term): the surface program's pbr point lit by
//			render.light-set.v1 point and spot lights through the view's
//			cluster lists (render.pass.lights AssignLights, the serial path
//			the GPU kernel equals), judged against an oracle that shares no
//			code with the shader: every light of the set, not the lists, with
//			light_set.h's falloffs and cone and pbr_brdf.h's lobes.
//			- a mixed set (legacy and inverse-square falloffs, bounded and
//			  unbounded, points and spots) and a dense one (256 small lights),
//			  on three materials from an overhead and a grazing view;
//			- the term's neutral value (no lights, or the term off) is bitwise
//			  the frame of the program without the term.
//			Tolerance: each sampled pixel within 0.5 percent + 3e-4 of the
//			oracle (the target is half float), fixed before the first run,
//			widened by how far the oracle itself moves within 0.1 units of the
//			pixel's point (the rasterizer's position may differ from the
//			ray's by that much). The first run skipped such pixels, as the
//			area-light suite does, capped at 5 percent of a case; the lights'
//			steep gradients made most pixels skipped (none was outside the
//			band), so every pixel is now judged with its widened band.
//
//			Seeded programs (--sensitivity): the froxel's slice off by one,
//			each list's first light skipped, the inverse-square window
//			dropped, the spot cone's cosine to the axis dropped (vrad's rule,
//			light set v3; one mixed spot has exponent 2, the others 0).
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_receiver.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/light_set.h"
#include "render/material/pbr_family.h"
#include "render/pass/lights/clusters.h"
#include "render/pass/lights/map_lights.h"
#include "render/shaderlib/debug_view.h"
#include "spv/clustered_light_defects_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
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
constexpr float kReceiver = 700.0f;
constexpr int kSampleStep = 4;
constexpr std::uint32_t kTileSize = 16;

constexpr float kRelative = 0.005f;
constexpr float kAbsolute = 3e-4f;
constexpr float kConditioning = 0.1f; // world units of position uncertainty

light_set::RuntimeLight Point( float3 at, float r, float g, float b, float radius,
    light_set::LightFalloff falloff, float minLight = 0.0f )
{
	light_set::RuntimeLight light;
	light.id = 1;
	light.kind = light_set::LightKind::Dynamic;
	light.shape = light_set::LightShape::Point;
	std::memcpy( light.position, &at, sizeof( light.position ) );
	light.color[0] = r;
	light.color[1] = g;
	light.color[2] = b;
	light.radius = radius;
	light.falloff = falloff;
	light.minLight = minLight;
	light.sourceRadius = light_set::kInverseSquareSourceRadius;
	return light;
}

// A world light as vrad bakes it: intensity over ( c + l d + q d^2 ), here
// Portal 2's _fifty_percent_distance form ( d50^2, 0, 1 ), nearly flat to d50.
light_set::RuntimeLight Attenuated( light_set::RuntimeLight light, float fiftyPercent )
{
	light.falloff = light_set::LightFalloff::Attenuated;
	light.attenuation[0] = fiftyPercent * fiftyPercent;
	light.attenuation[1] = 0.0f;
	light.attenuation[2] = 1.0f;
	for ( float &c : light.color )
		c *= fiftyPercent * fiftyPercent;
	return light;
}

light_set::RuntimeLight Spot( light_set::RuntimeLight light, float3 direction,
    float innerDegrees, float outerDegrees, float exponent = 0.0f )
{
	light.spotExponent = exponent;
	light.shape = light_set::LightShape::Spot;
	const float3 unit = math::Normalize( direction );
	std::memcpy( light.direction, &unit, sizeof( light.direction ) );
	light.innerCos = std::cos( innerDegrees * 3.14159265f / 180.0f );
	light.outerCos = std::cos( outerDegrees * 3.14159265f / 180.0f );
	return light;
}

std::map<std::string, std::vector<light_set::RuntimeLight>> LightSets()
{
	using light_set::LightFalloff;
	std::map<std::string, std::vector<light_set::RuntimeLight>> sets;
	sets["mixed"] = { Point( { 0, 0, 60 }, 1.0f, 0.8f, 0.6f, 220, LightFalloff::Legacy, 0.02f ),
	    Point( { 80, 40, 50 }, 0.3f, 0.35f, 0.5f, 400, LightFalloff::InverseSquare ),
	    Spot( Point( { -100, -50, 120 }, 0.5f, 0.45f, 0.3f, 600, LightFalloff::InverseSquare ),
	        { 0.2f, 0.1f, -1.0f }, 20, 35 ),
	    Spot( Point( { 150, -120, 80 }, 0.9f, 0.5f, 0.4f, 300, LightFalloff::Legacy, 0.05f ),
	        { 0.3f, 0.2f, -1.0f }, 15, 30, 2.0f ),
	    Point( { 0, 300, 200 }, 1.0f, 1.0f, 1.0f, 0, LightFalloff::InverseSquare ) };
	std::vector<light_set::RuntimeLight> dense;
	for ( int i = 0; i < 16; ++i )
	{
		for ( int j = 0; j < 16; ++j )
		{
			const float hue = float( ( i * 16 + j ) % 7 ) / 7.0f;
			dense.push_back(
			    Point( { -375.0f + 50.0f * float( i ), -375.0f + 50.0f * float( j ), 20 },
			        0.4f + 0.6f * hue, 0.9f - 0.5f * hue, 0.5f, 60, LightFalloff::Legacy, 0.1f ) );
		}
	}
	sets["dense"] = dense;
	// World lights as vrad bakes them (LightFalloff::Attenuated): Portal 2's
	// fifty-percent form, one with a hard radius.
	sets["attenuated"] = {
	    Attenuated( Point( { -60, 20, 90 }, 0.6f, 0.5f, 0.4f, 0, LightFalloff::Legacy ), 120.0f ),
	    Attenuated( Spot( Point( { 90, -40, 110 }, 0.5f, 0.6f, 0.7f, 260, LightFalloff::Legacy ),
	                    { -0.2f, 0.1f, -1.0f }, 25, 40, 1.0f ),
	        80.0f ) };
	return sets;
}

// The oracle: every light of the set at p, both lobes.
float Oracle( const std::vector<light_set::RuntimeLight> &lights, const ReceiverMaterial &material,
    float3 p, float3 v )
{
	double total = 0.0;
	for ( const light_set::RuntimeLight &light : lights )
		total += double( RuntimeLightOracle( light, material, p, v ) );
	return float( total );
}

struct Lab
{
	std::unique_ptr<LabClusterLists> clusters;
	IRenderDevice2 &device;
	resources::TextureCache textures;
	material::GroupResidency groups;
	std::unique_ptr<Canvas> canvas;
	std::unique_ptr<material::PbrFamily> family;
	std::map<std::string, std::uint64_t> materialGroups;
	std::uint64_t frameGroup = 0;
	std::uint64_t drawGroup = 0;
	std::uint64_t nextGroup = 1;
	std::vector<std::byte> receiver;

	explicit Lab( IRenderDevice2 &d ) : device( d ), textures( d ), groups( d, textures ) {}
};

std::optional<std::string> Prepare( Lab &lab, std::span<const std::uint32_t> module )
{
	if ( std::optional<std::string> why = Canvas::Create( lab.device, kSize, kSize, lab.canvas ) )
		return why;
	auto family = material::CreateSurfaceFamily<material::PbrFamily>(
	    lab.device, kCanvasColor, kCanvasDepth, 1, module );
	if ( !family )
		return std::string( "the surface program was refused" );
	lab.family = std::move( family ).Value();
	bool staged = StageConstant(
	    lab.textures, "cl/base", Format::kRGBA8Srgb, ByteTexel( 255, 255, 255, 255 ) );
	for ( const ReceiverMaterial &m : kReceiverMaterials )
		staged = staged && StageConstant( lab.textures, std::string( "cl/mrao-" ) + m.name,
		                       Format::kRGBA8Unorm, ByteTexel( m.metal, m.roughness, 255, 255 ) );
	const material::PbrSplitSumTable table = material::SplitSumTable();
	TextureDesc desc;
	desc.format = table.format;
	desc.width = table.width;
	desc.height = table.height;
	staged = staged &&
	         lab.textures.Stage( "cl/splitsum", desc, std::as_bytes( std::span( table.texels ) ) )
	             .HasValue();
	if ( !staged )
		return std::string( "a fixture texture was refused" );
	for ( const ReceiverMaterial &m : kReceiverMaterials )
	{
		material::PbrClaim claim;
		claim.claimed = true;
		material::SurfaceTextures textures;
		textures.base = "cl/base";
		textures.mrao = std::string( "cl/mrao-" ) + m.name;
		auto request = lab.family->Request( claim, textures );
		if ( !request )
			return std::string( "the pbr point was refused" );
		const std::uint64_t id = lab.nextGroup++;
		if ( !lab.groups.Set( id, request.Value().material ) )
			return std::string( "a material group was refused" );
		lab.materialGroups[m.name] = id;
	}
	lab.drawGroup = lab.nextGroup++;
	if ( !lab.groups.Set( lab.drawGroup, lab.family->LightingGroup( material::ModelLighting() ) ) )
		return std::string( "the draw group was refused" );
	lab.receiver = ReceiverMesh( kReceiver );
	return std::nullopt;
}

struct Frame
{
	const std::vector<light_set::RuntimeLight> *lights = nullptr;
	const ReceiverMaterial *material = &kReceiverMaterials[0];
	const ReceiverView *view = nullptr;
	bool clustered = true; // the variant with kSurfaceClustered
	shaderlib::DebugSpecialization debug;
};

// The view group of a frame: the cluster grid of its view and the serial
// assignment of its lights.
std::optional<material::GroupRequest> ViewGroup( Lab &lab, const Frame &frame )
{
	const std::vector<light_set::RuntimeLight> none;
	const std::vector<light_set::RuntimeLight> &lights = frame.lights ? *frame.lights : none;
	pass::lights::ClusterLimits limits = pass::lights::DesktopClusterLimits();
	limits.tileSizePixels = kTileSize;
	pass::lights::ClusterViewDesc desc;
	desc.view = frame.view->view;
	desc.projection = frame.view->projection;
	desc.widthPixels = desc.heightPixels = kSize;
	desc.nearZ = frame.view->nearZ;
	desc.farZ = frame.view->farZ;
	auto grid = pass::lights::CreateClusterGrid( desc, limits );
	if ( !grid )
		return std::nullopt;
	lab.clusters = LabClusterLists::Create( lab.device, grid.Value(), lights );
	if ( !lab.clusters )
		return std::nullopt;
	material::SurfaceViewGpu view;
	view.grid[0] = grid.Value().tilesX;
	view.grid[1] = grid.Value().tilesY;
	view.grid[2] = grid.Value().slices;
	view.grid[3] = kTileSize;
	view.slices[0] = grid.Value().sliceScale;
	view.slices[1] = grid.Value().sliceBias;
	view.slices[2] = grid.Value().nearZ;
	const math::float4 &z = frame.view->view.rows[2];
	view.viewDistance[0] = -z.x;
	view.viewDistance[1] = -z.y;
	view.viewDistance[2] = -z.z;
	view.viewDistance[3] = -z.w;
	std::vector<material::SurfaceLightGpu> records;
	for ( const light_set::RuntimeLight &light : lights )
		records.push_back( material::PackSurfaceLight( light ) );
	if ( records.empty() )
		records.emplace_back();
	auto request = lab.family->Program().ViewGroup( view, {}, {}, records );
	lab.clusters->Bind( request );
	return request;
}

std::optional<std::string> Render( Lab &lab, const Frame &frame, CanvasImage &image )
{
	material::SurfaceFrame terms;
	terms.eye[0] = frame.view->eye.x;
	terms.eye[1] = frame.view->eye.y;
	terms.eye[2] = frame.view->eye.z;
	const std::uint64_t frameGroup = lab.nextGroup++;
	const std::uint64_t viewGroup = lab.nextGroup++;
	const std::optional<material::GroupRequest> view =
	    frame.clustered ? ViewGroup( lab, frame ) : lab.family->Program().NeutralViewGroup();
	if ( !view )
		return std::string( "the cluster lists were not built" );
	if ( !lab.groups.Set( frameGroup, lab.family->FrameGroup( terms, "cl/splitsum" ) ) ||
	     !lab.groups.Set( viewGroup, *view ) )
		return std::string( "a frame or view group was refused" );
	if ( std::optional<std::string> why =
	         lab.canvas->Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr ) )
		return why;
	material::PbrClaim claim;
	claim.claimed = true;
	material::SurfaceVariant variant = claim.Variant();
	if ( frame.clustered )
		variant.terms |= material::kSurfaceClustered;
	auto pipeline = lab.family->Program().Pipeline( variant, frame.debug );
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
	    group( lab.materialGroups.at( frame.material->name ) );
	draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.drawGroup );
	draw.vertices = lab.canvas->Vertices( lab.receiver );
	draw.vertexCount = 6;
	material::FamilyDrawConstants constants;
	std::memcpy( constants.toClip, &frame.view->toClip, sizeof( constants.toClip ) );
	constants.world[0] = constants.world[5] = constants.world[10] = constants.world[15] = 1.0f;
	const auto *bytes = reinterpret_cast<const std::byte *>( &constants );
	draw.constants.assign( bytes, bytes + sizeof( constants ) );
	const CanvasDraw draws[] = { draw };
	std::optional<std::string> why =
	    lab.canvas->Render( lab.textures, lab.groups, draws, { 0, 0, 0, 1 }, &image );
	lab.groups.Remove( frameGroup );
	lab.groups.Remove( viewGroup );
	return why;
}

bool SameImage( const CanvasImage &a, const CanvasImage &b )
{
	return a.rgba.size() == b.rgba.size() &&
	       std::memcmp( a.rgba.data(), b.rgba.data(), a.rgba.size() * sizeof( float ) ) == 0;
}

std::string Format3( const char *format, double a, double b = 0, double c = 0 )
{
	char text[160];
	std::snprintf( text, sizeof( text ), format, a, b, c );
	return text;
}

void MapLightChecks( Results &results )
{
	const std::vector<pass::lights::Entity> entities = {
	    { { "classname", "light" }, { "origin", "0 0 0" }, { "_light", "255 255 255 255" },
	        { "_constant_attn", "1500" }, { "_quadratic_attn", "1" } },
	    { { "classname", "light" }, { "origin", "20 0 0" }, { "_light", "255 255 255 255" },
	        { "_quadratic_attn", "1" }, { "style", "32" } },
	    { { "classname", "light" }, { "origin", "40 0 0" }, { "_light", "255 255 255 255" },
	        { "_fifty_percent_distance", "100" } } };
	const pass::lights::MapLights map = pass::lights::MapLightsFromEntities( entities );
	results.That(
	    map.lights.size() == 2 && map.unsupported == 1, "map-lights.unsupported-falloff-refused" );
	if ( map.lights.size() != 2 )
		return;
	const light_set::RuntimeLight &lamp = map.lights[0];
	results.That( lamp.falloff == light_set::LightFalloff::Attenuated &&
	                  std::fabs( lamp.color[0] * light_set::AttenuatedFalloff(
	                                                 10000.0f, lamp.radius, lamp.attenuation ) -
	                             1.0f ) < 1e-5f,
	    "map-lights.vrad-attenuation-at-reference-distance" );
	light_set::RuntimeLight switched = map.lights[1];
	switched.id = 100;
	const auto merged = pass::lights::MergeMapLights( { switched }, map );
	results.That( merged.size() == 2 && merged[1].style == 0 && merged[1].id != switched.id,
	    "map-lights.one-switchable-light-does-not-hide-baked-lamps" );
	const auto present = pass::lights::MergeMapLights( { lamp, switched }, map );
	results.That( present.size() == 2, "map-lights.compiled-world-light-is-not-doubled" );
}

std::optional<std::string> ClusteredChecks( Lab &lab, Results &results )
{
	const ReceiverView overhead = MakeReceiverView( { 0, -60, 300 }, { 0, 0, 0 }, kSize );
	const ReceiverView grazing = MakeReceiverView( { 0, -500, 80 }, { 0, 60, 0 }, kSize );
	const std::pair<const char *, const ReceiverView *> views[] = {
	    { "overhead", &overhead }, { "grazing", &grazing } };
	const auto sets = LightSets();
	const ReceiverMaterial *materials[] = {
	    &kReceiverMaterials[0], &kReceiverMaterials[1], &kReceiverMaterials[2] };

	for ( const auto &[setName, lights] : sets )
	{
		for ( const ReceiverMaterial *material : materials )
		{
			for ( const auto &[viewName, view] : views )
			{
				Frame frame{ &lights, material, view, true, {} };
				CanvasImage image;
				if ( std::optional<std::string> why = Render( lab, frame, image ) )
					return why;
				int pixels = 0, bad = 0;
				float worst = 0.0f, brightest = 0.0f;
				for ( std::uint32_t y = 0; y < kSize; y += kSampleStep )
				{
					for ( std::uint32_t x = 0; x < kSize; x += kSampleStep )
					{
						const std::optional<float3> p = ReceiverHit( *view, x, y, kReceiver );
						if ( !p )
							continue;
						const float expected = Oracle( lights, *material, *p, ToEye( *view, *p ) );
						const float band = kRelative * expected + kAbsolute;
						++pixels;
						brightest = std::max( brightest, expected );
						float moved = 0.0f;
						for ( const float3 offset :
						    { float3{ kConditioning, 0, 0 }, float3{ -kConditioning, 0, 0 },
						        float3{ 0, kConditioning, 0 }, float3{ 0, -kConditioning, 0 } } )
						{
							const float3 q{ p->x + offset.x, p->y + offset.y, 0.0f };
							moved = std::max( moved,
							    std::fabs( Oracle( lights, *material, q, ToEye( *view, q ) ) -
							               expected ) );
						}
						const float error = std::fabs( image.At( x, y )[0] - expected );
						worst = std::max( worst, error / std::max( expected, 1e-3f ) );
						bad += error > band + moved ? 1 : 0;
					}
				}
				results.That( pixels > 100 && bad == 0 && brightest > 0.05f,
				    "clustered." + setName + "." + material->name + "." + viewName,
				    Format3( "%.0f of %.0f outside the tolerance, worst relative %.3g", bad, pixels,
				        worst ) );
			}
		}
	}

	// The neutral value: no lights, and the term off, are the frame of the
	// program without the term, bitwise; lights change it.
	{
		const std::vector<light_set::RuntimeLight> none;
		CanvasImage without, empty, off, lit;
		Frame withoutTerm{ nullptr, &kReceiverMaterials[2], &overhead, false, {} };
		Frame emptySet{ &none, &kReceiverMaterials[2], &overhead, true, {} };
		Frame termOff{ &sets.at( "mixed" ), &kReceiverMaterials[2], &overhead, true, {} };
		termOff.debug.termsOff = shaderlib::kDebugTermClustered;
		Frame litFrame{ &sets.at( "mixed" ), &kReceiverMaterials[2], &overhead, true, {} };
		for ( auto [frame, image] :
		    { std::pair{ &withoutTerm, &without }, std::pair{ &emptySet, &empty },
		        std::pair{ &termOff, &off }, std::pair{ &litFrame, &lit } } )
		{
			if ( std::optional<std::string> why = Render( lab, *frame, *image ) )
				return why;
		}
		results.That(
		    SameImage( without, empty ), "neutral.no-lights-is-the-frame-without-the-term" );
		results.That( SameImage( without, off ), "neutral.term-off-is-the-frame-without-the-term" );
		results.That( !SameImage( without, lit ), "neutral.a-lit-frame-differs" );
	}
	return std::nullopt;
}

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		Lab lab( *device );
		MapLightChecks( results );
		if ( std::optional<std::string> why = Prepare( lab, module ) )
			return why;
		if ( std::optional<std::string> why = ClusteredChecks( lab, results ) )
			return why;
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kClusteredSeeded[] = {
    { "slice-off-by-one", spirv::kSurfaceClusterSliceOffByOne, "clustered.dense" },
    { "skips-first", spirv::kSurfaceClusterSkipsFirst, "clustered." },
    { "falloff-unwindowed", spirv::kSurfaceRuntimeFalloffUnwindowed, "clustered.mixed" },
    { "spot-no-cosine", spirv::kSurfaceSpotNoCosine, "clustered.mixed" } };

} // namespace

int RunClusteredLightsSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "clustered-lights", kClusteredSeeded, RunOnce );
}

} // namespace render::lab
