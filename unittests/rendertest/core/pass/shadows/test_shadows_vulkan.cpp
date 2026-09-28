//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shadows.pixels (RFC 0016 K7 "Shadow oracles",
//			render.shadows.v1) on render.device.vulkan: the shadow atlas
//			planned by PlanShadowAtlas, caster depth drawn by
//			ShadowDepthRenderer into D32 atlas tiles, and a ground plane drawn
//			by ShadowReceiverRenderer through shadow_sample.glsl, all as
//			render.graph passes, with real pixels read back.
//
//			The oracle is a CPU ray test in double: a ground pixel is
//			shadowed when the segment from its ground point toward the light
//			crosses a caster's box. Only pixels away from edges are judged:
//			the class (outside the cone, shadowed by a caster, shadowed only
//			by a non-caster, lit) must agree at the point and over a disc
//			around it (four rings of 32 points) whose radius is two pixel
//			footprints plus three shadow texels projected onto the ground.
//
//			Spot light (atlas 2048, the spot's tile planned after two other
//			lights' tiles, which hold their own views):
//			P1 the atlas plan puts the spot's tile away from the atlas's
//			   origin, and the camera model the oracle uses reprojects to the
//			   pixel centers;
//			P2 depth lands only inside the rendered views' tile viewports:
//			   every other atlas texel keeps the clear depth, 1;
//			P3 a caster darkens its receiver: every judged pixel of a
//			   caster's shadow is dark;
//			P4 a non-caster does not: every judged pixel shadowed only by
//			   the non-caster is lit;
//			P5 lit pixels far from shadows stay lit, and pixels outside the
//			   cone stay dark;
//			P6 seeded defects are each detected by P3-P5: the depth compare
//			   reversed (shadow_defects_spv.h), the receiver reading a tile
//			   offset by one tile, and a caster left out of the depth pass.
//
//			Sun (four cascades from BuildCascades in four 1024 tiles):
//			C1 every cascade shades judged pixels, and the cascade each pixel
//			   used (blue channel) is the one whose split holds its view
//			   distance;
//			C2 the cascaded render and a single-cascade reference render
//			   (one 2048 cascade over the same distance) each match the
//			   oracle at every judged pixel, and agree with each other on at
//			   least 99.8 percent of the pixels judged for both
//			   (kCascadeAgreement);
//			C3 texel snapping: moving the camera 2.37 texels along the
//			   light's right axis shifts a cascade's depth image by whole
//			   texels, and the shifted images agree texel for texel; a
//			   cascade left unsnapped (its box centered on the sphere) is
//			   detected by the same comparison.
//
//			validation: with the Khronos validation layer (synchronization
//			validation included) the run reports no message; without the
//			layer the clause prints SKIP and certifies nothing.
//
//			RENDER_VK_ADAPTER=<n> picks the physical device. A missing Vulkan
//			device fails the run.
//
//=============================================================================//

#include "render/device/vulkan/provider.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/pass/shadows/atlas.h"
#include "render/pass/shadows/shadow_passes.h"
#include "render/pass/shadows/shadow_views.h"
#include "spv/shadow_defects_spv.h"
#include "testing/checks.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace
{

using namespace render;
using namespace render::device;
using namespace render::pass::shadows;
using math::float3;
using math::float4x4;

constexpr std::uint32_t kSize = 256; // receiver frame
constexpr double kPi = 3.14159265358979323846;
constexpr int kDark = 70; // red at or below: unlit (ambient 0.2 is 51)
constexpr int kLit = 240; // red at or above: lit
constexpr float kBias = 1.0e-4f;
// C2: the share of pixels judged for both renders on which they may
// disagree. Measured at introduction (2026-09-28): see the progress record.
constexpr double kCascadeAgreement = 0.998;
// C3: texels of a snapped cascade's shifted depth image that may differ
// from the unshifted one (float rounding of the translated vertices).
constexpr double kSnapTolerance = 1.0e-4; // of the compared texels

// ---------------------------------------------------------------------------
// Scene geometry in double

struct D3
{
	double x = 0.0, y = 0.0, z = 0.0;
};

D3 operator+( const D3 &a, const D3 &b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}
D3 operator-( const D3 &a, const D3 &b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}
D3 operator*( const D3 &a, double s )
{
	return { a.x * s, a.y * s, a.z * s };
}
double Dot( const D3 &a, const D3 &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
D3 Cross( const D3 &a, const D3 &b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
double Len( const D3 &a )
{
	return std::sqrt( Dot( a, a ) );
}
D3 Unit( const D3 &a )
{
	return a * ( 1.0 / Len( a ) );
}
float3 F( const D3 &a )
{
	return { float( a.x ), float( a.y ), float( a.z ) };
}

// A box: the unit cube scaled by size, turned about +Z, moved to center.
struct Box
{
	D3 center;
	D3 size;
	double angle = 0.0; // radians about +Z

	float4x4 World() const
	{
		float4x4 turn;
		const float c = float( std::cos( angle ) );
		const float s = float( std::sin( angle ) );
		turn.rows[0] = { c, -s, 0.0f, 0.0f };
		turn.rows[1] = { s, c, 0.0f, 0.0f };
		return math::Multiply(
		    math::Translation( F( center ) ), math::Multiply( turn, math::Scale( F( size ) ) ) );
	}

	// Whether the segment a-b crosses the box (slab test in its frame).
	bool Hits( const D3 &a, const D3 &b ) const
	{
		const double c = std::cos( angle );
		const double s = std::sin( angle );
		const auto local = [&]( const D3 &p )
		{
			const D3 d = p - center;
			return D3{ c * d.x + s * d.y, -s * d.x + c * d.y, d.z };
		};
		const D3 p = local( a );
		const D3 q = local( b );
		const double from[3] = { p.x, p.y, p.z };
		const double to[3] = { q.x, q.y, q.z };
		const double half[3] = { size.x * 0.5, size.y * 0.5, size.z * 0.5 };
		double t0 = 0.0;
		double t1 = 1.0;
		for ( int i = 0; i < 3; ++i )
		{
			const double d = to[i] - from[i];
			if ( std::fabs( d ) < 1e-12 )
			{
				if ( std::fabs( from[i] ) > half[i] )
					return false;
				continue;
			}
			double ta = ( -half[i] - from[i] ) / d;
			double tb = ( half[i] - from[i] ) / d;
			if ( ta > tb )
				std::swap( ta, tb );
			t0 = std::max( t0, ta );
			t1 = std::min( t1, tb );
			if ( t0 > t1 )
				return false;
		}
		return true;
	}
};

struct Camera
{
	D3 eye;
	D3 target;
	double fov = 1.0;
	float nearZ = 0.5f;
	float farZ = 200.0f;

	float4x4 View() const { return math::LookAt( F( eye ), F( target ), { 0.0f, 0.0f, 1.0f } ); }
	float4x4 ViewProjection() const
	{
		return math::Multiply( math::Perspective( float( fov ), 1.0f, nearZ, farZ ), View() );
	}
	// The ray through a pixel position (x right, y down, in pixels).
	D3 Ray( double px, double py ) const
	{
		const D3 forward = Unit( target - eye );
		const D3 right = Unit( Cross( forward, { 0.0, 0.0, 1.0 } ) );
		const D3 up = Cross( right, forward );
		const double t = std::tan( fov * 0.5 );
		const double nx = 2.0 * px / kSize - 1.0;
		const double ny = 1.0 - 2.0 * py / kSize;
		return forward + right * ( nx * t ) + up * ( ny * t );
	}
	// Where a pixel position's ray meets the ground (z = 0) within `extent`.
	std::optional<D3> Ground( double px, double py, double extent ) const
	{
		const D3 d = Ray( px, py );
		if ( !( d.z < -1e-6 ) )
			return std::nullopt;
		const D3 g = eye + d * ( -eye.z / d.z );
		if ( std::fabs( g.x ) > extent || std::fabs( g.y ) > extent )
			return std::nullopt;
		return g;
	}
	double ViewDistance( const D3 &p ) const { return Dot( p - eye, Unit( target - eye ) ); }
};

enum Class
{
	kOutside,   // outside the spot's cone or range
	kShadowed,  // a caster blocks the light
	kNonCaster, // only a non-caster blocks it
	kLitClass,
};

struct Scene
{
	std::vector<Box> casters;
	std::vector<Box> nonCasters;

	bool Blocked( const std::vector<Box> &boxes, const D3 &a, const D3 &b ) const
	{
		for ( const Box &box : boxes )
		{
			if ( box.Hits( a, b ) )
				return true;
		}
		return false;
	}
	Class Classify( const D3 &point, const D3 &toward ) const
	{
		if ( Blocked( casters, point, toward ) )
			return kShadowed;
		return Blocked( nonCasters, point, toward ) ? kNonCaster : kLitClass;
	}
};

struct Spot
{
	D3 position;
	D3 axis;
	double halfAngle = 0.0;
	double range = 0.0;

	bool Holds( const D3 &p ) const
	{
		const D3 v = p - position;
		const double d = Len( v );
		return d <= range && Dot( v, axis ) >= std::cos( halfAngle ) * d;
	}
};

// ---------------------------------------------------------------------------
// Device runs

bool Wait( IRenderDevice2 &device, CompletionToken token )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	while ( !device.IsComplete( token ) )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		(void)device.Poll();
		std::this_thread::yield();
	}
	(void)device.Poll();
	return true;
}

struct Meshes
{
	resources::MeshEntry cube;
	resources::MeshEntry ground; // +-80 units at z = 0
};

std::optional<Meshes> StageMeshes( IRenderDevice2 &device, resources::MeshCache &cache )
{
	std::vector<float> cube;
	for ( int corner = 0; corner < 8; ++corner )
	{
		cube.push_back( ( corner & 1 ) ? 0.5f : -0.5f );
		cube.push_back( ( corner & 2 ) ? 0.5f : -0.5f );
		cube.push_back( ( corner & 4 ) ? 0.5f : -0.5f );
	}
	const std::vector<std::uint16_t> cubeIndices = { 0, 1, 3, 0, 3, 2, 4, 6, 7, 4, 7, 5, 0, 4, 5, 0,
	    5, 1, 2, 3, 7, 2, 7, 6, 0, 2, 6, 0, 6, 4, 1, 5, 7, 1, 7, 3 };
	const std::vector<float> ground = { -80, -80, 0, 80, -80, 0, 80, 80, 0, -80, 80, 0 };
	const std::vector<std::uint16_t> groundIndices = { 0, 1, 2, 0, 2, 3 };
	const auto data = []( const std::vector<float> &v, const std::vector<std::uint16_t> &i )
	{
		resources::MeshData mesh;
		mesh.vertices = std::as_bytes( std::span<const float>( v ) );
		mesh.vertexStride = 12;
		mesh.indices = std::as_bytes( std::span<const std::uint16_t>( i ) );
		mesh.indexFormat = IndexFormat::kUint16;
		return mesh;
	};
	auto cubeEntry = cache.Stage( "cube", data( cube, cubeIndices ) );
	auto groundEntry = cache.Stage( "ground", data( ground, groundIndices ) );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !cubeEntry || !groundEntry || !encoder )
		return std::nullopt;
	cache.RecordUploads( encoder.Value() );
	CommandEncoder encoders[] = { std::move( encoder ).Value() };
	auto token = device.Submit( QueueKind::kGraphics, encoders, {} );
	if ( !token || !Wait( device, token.Value() ) )
		return std::nullopt;
	cache.Retire( token.Value() );
	return Meshes{ cubeEntry.Value(), groundEntry.Value() };
}

struct Frame
{
	bool ok = false;
	std::vector<std::uint8_t> rgba; // kSize x kSize, row 0 at the top
	std::vector<float> atlas;       // atlasSize^2 when read
};

struct Receivers
{
	ShadowReceiverRenderer *renderer = nullptr;
	ShadowReceiverView view;
	ShadowReceiverLight light;
	std::vector<ShadowReceiver> receivers;
};

// Draws the depth views into a fresh atlas and, when given, the receivers
// into a fresh color target; reads back the color and, when asked, the
// atlas.
Frame Run( IRenderDevice2 &device, ShadowDepthRenderer &depth, std::uint32_t atlasSize,
    std::uint32_t guard, std::span<const ShadowDepthView> views, const Receivers *receivers,
    bool readAtlas )
{
	Frame frame;
	TextureDesc atlasDesc;
	atlasDesc.format = Format::kD32Float;
	atlasDesc.width = atlasDesc.height = atlasSize;
	atlasDesc.usages = {
	    ResourceUsage::kDepthWrite, ResourceUsage::kSampled, ResourceUsage::kCopySource };
	TextureDesc colorDesc;
	colorDesc.format = Format::kRGBA8Unorm;
	colorDesc.width = colorDesc.height = kSize;
	colorDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	BufferDesc colorReadDesc;
	colorReadDesc.size = kSize * kSize * 4;
	colorReadDesc.usages = { ResourceUsage::kCopyDestination };
	colorReadDesc.memory = MemoryKind::kReadback;
	BufferDesc atlasReadDesc = colorReadDesc;
	atlasReadDesc.size = std::uint64_t( atlasSize ) * atlasSize * 4;

	auto atlas = device.CreateTexture( atlasDesc );
	auto color = device.CreateTexture( colorDesc );
	auto colorRead = device.CreateBuffer( colorReadDesc );
	std::optional<DeviceResult<BufferId>> atlasRead;
	if ( readAtlas )
		atlasRead = device.CreateBuffer( atlasReadDesc );
	bool ok = atlas && color && colorRead && ( !readAtlas || *atlasRead );
	if ( ok )
	{
		graph::GraphBuilder builder;
		const graph::ResourceRef atlasRef = builder.ImportTexture(
		    "atlas", atlas.Value(), atlasDesc, ResourceUsage::kUndefined, ResourceUsage::kSampled );
		auto stats = depth.AddPasses( builder, { atlasRef, atlasSize, guard }, views );
		ok = stats.HasValue();
		if ( ok && receivers )
		{
			const graph::ResourceRef colorRef = builder.ImportTexture( "color", color.Value(),
			    colorDesc, ResourceUsage::kUndefined, ResourceUsage::kCopySource );
			const graph::ResourceRef colorCopy =
			    builder.ImportBuffer( "color-readback", colorRead.Value(), colorReadDesc,
			        ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			ok = receivers->renderer
			         ->AddPasses( builder, atlasRef, receivers->view, receivers->light,
			             receivers->receivers, { colorRef, kSize, kSize, { 0, 0, 0, 1 } } )
			         .HasValue();
			builder.AddPass( "color-readback", graph::PassKind::kCopy )
			    .Read( colorRef, ResourceUsage::kCopySource )
			    .Write( colorCopy, ResourceUsage::kCopyDestination )
			    .SideEffect()
			    .Execute(
			        [colorRef, colorCopy]( graph::RecordContext &context )
			        {
				        context.Encoder().CopyTextureToBuffer( context.Texture( colorRef ),
				            context.Buffer( colorCopy ), { 0, 0, 0, kSize, kSize } );
			        } );
		}
		if ( ok && readAtlas )
		{
			const graph::ResourceRef atlasCopy =
			    builder.ImportBuffer( "atlas-readback", atlasRead->Value(), atlasReadDesc,
			        ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			builder.AddPass( "atlas-readback", graph::PassKind::kCopy )
			    .Read( atlasRef, ResourceUsage::kCopySource )
			    .Write( atlasCopy, ResourceUsage::kCopyDestination )
			    .SideEffect()
			    .Execute(
			        [atlasRef, atlasCopy, atlasSize]( graph::RecordContext &context )
			        {
				        context.Encoder().CopyTextureToBuffer( context.Texture( atlasRef ),
				            context.Buffer( atlasCopy ), { 0, 0, 0, atlasSize, atlasSize } );
			        } );
		}
		if ( ok )
		{
			auto compiled = graph::CompileGraph( std::move( builder ) );
			ok = compiled.HasValue();
			if ( ok )
			{
				graph::SerialGraphExecutor executor;
				auto executed = executor.Execute( compiled.Value(), device );
				ok = executed.HasValue() && Wait( device, executed.Value().token );
				if ( executed )
				{
					depth.Collect( executed.Value().token );
					if ( receivers )
						receivers->renderer->Collect( executed.Value().token );
				}
			}
		}
	}
	if ( ok && receivers )
	{
		frame.rgba.resize( kSize * kSize * 4 );
		ok = device
		         .ReadBuffer( colorRead.Value(), 0,
		             std::as_writable_bytes( std::span<std::uint8_t>( frame.rgba ) ) )
		         .HasValue();
	}
	if ( ok && readAtlas )
	{
		frame.atlas.resize( std::size_t( atlasSize ) * atlasSize );
		ok = device
		         .ReadBuffer( atlasRead->Value(), 0,
		             std::as_writable_bytes( std::span<float>( frame.atlas ) ) )
		         .HasValue();
	}
	frame.ok = ok;
	if ( atlas )
		(void)device.Release( atlas.Value(), CompletionToken() );
	if ( color )
		(void)device.Release( color.Value(), CompletionToken() );
	if ( colorRead )
		(void)device.Release( colorRead.Value(), CompletionToken() );
	if ( atlasRead && *atlasRead )
		(void)device.Release( atlasRead->Value(), CompletionToken() );
	(void)device.Poll();
	return frame;
}

int Red( const Frame &frame, std::uint32_t x, std::uint32_t y )
{
	return frame.rgba[( std::size_t( y ) * kSize + x ) * 4];
}
int Green( const Frame &frame, std::uint32_t x, std::uint32_t y )
{
	return frame.rgba[( std::size_t( y ) * kSize + x ) * 4 + 1];
}
int Blue( const Frame &frame, std::uint32_t x, std::uint32_t y )
{
	return frame.rgba[( std::size_t( y ) * kSize + x ) * 4 + 2];
}

std::vector<ShadowCaster> Casters( const Meshes &meshes, const std::vector<Box> &boxes )
{
	std::vector<ShadowCaster> casters;
	for ( const Box &box : boxes )
		casters.push_back( { meshes.cube, box.World() } );
	return casters;
}

// A judged pixel: its expected class, or nothing near an edge.
struct Judged
{
	std::uint32_t x = 0;
	std::uint32_t y = 0;
	Class expected = kOutside;
	D3 ground;
};

// The pixels whose class holds at the point and over the disc around it of
// radius `margin( point, footprint )` on the ground (four rings of 32 points).
template <typename ClassAt, typename Margin>
std::vector<Judged> JudgePixels( const Camera &camera, ClassAt classAt, Margin margin )
{
	std::vector<Judged> judged;
	for ( std::uint32_t y = 0; y < kSize; ++y )
	{
		for ( std::uint32_t x = 0; x < kSize; ++x )
		{
			const auto g = camera.Ground( x + 0.5, y + 0.5, 79.0 );
			const auto gx = camera.Ground( x + 1.5, y + 0.5, 79.0 );
			const auto gy = camera.Ground( x + 0.5, y + 1.5, 79.0 );
			if ( !g || !gx || !gy )
				continue;
			const double footprint = std::max( Len( *gx - *g ), Len( *gy - *g ) );
			const double m = margin( *g, footprint );
			const Class c = classAt( *g );
			bool stable = true;
			for ( int ring = 1; ring <= 4 && stable; ++ring )
			{
				const double r = m * ring / 4.0;
				for ( int k = 0; k < 32 && stable; ++k )
				{
					const double a = k * kPi / 16.0;
					stable = classAt( *g + D3{ std::cos( a ) * r, std::sin( a ) * r, 0.0 } ) == c;
				}
			}
			if ( stable )
				judged.push_back( { x, y, c, *g } );
		}
	}
	return judged;
}

struct Tally
{
	std::uint64_t counts[4] = {};
	std::uint64_t wrong[4] = {};
	std::uint64_t notReceiver = 0;

	std::uint64_t Wrong() const { return wrong[0] + wrong[1] + wrong[2] + wrong[3] + notReceiver; }
};

bool Dark( int red )
{
	return red <= kDark;
}
bool Bright( int red )
{
	return red >= kLit;
}

Tally Score( const Frame &frame, const std::vector<Judged> &judged )
{
	Tally tally;
	if ( !frame.ok )
	{
		tally.notReceiver = 1;
		return tally;
	}
	for ( const Judged &j : judged )
	{
		const int red = Red( frame, j.x, j.y );
		++tally.counts[j.expected];
		if ( Green( frame, j.x, j.y ) != 255 )
			++tally.notReceiver;
		const bool lit = j.expected == kNonCaster || j.expected == kLitClass;
		if ( lit ? !Bright( red ) : !Dark( red ) )
			++tally.wrong[j.expected];
	}
	return tally;
}

void Print( const char *what, const Tally &t )
{
	std::printf(
	    "INFO %s: outside %llu (%llu wrong), shadowed %llu (%llu), non-caster %llu (%llu), "
	    "lit %llu (%llu), not receiver %llu\n",
	    what, static_cast<unsigned long long>( t.counts[0] ),
	    static_cast<unsigned long long>( t.wrong[0] ),
	    static_cast<unsigned long long>( t.counts[1] ),
	    static_cast<unsigned long long>( t.wrong[1] ),
	    static_cast<unsigned long long>( t.counts[2] ),
	    static_cast<unsigned long long>( t.wrong[2] ),
	    static_cast<unsigned long long>( t.counts[3] ),
	    static_cast<unsigned long long>( t.wrong[3] ),
	    static_cast<unsigned long long>( t.notReceiver ) );
}

// ---------------------------------------------------------------------------
// The spot scene

Scene SpotScene()
{
	Scene scene;
	scene.casters = { { { 0.5, 1.5, 3.5 }, { 2.2, 1.4, 1.0 }, 30.0 * kPi / 180.0 },
	    { { -3.0, 3.5, 2.0 }, { 1.2, 1.2, 1.2 }, -20.0 * kPi / 180.0 } };
	scene.nonCasters = { { { 3.0, -2.0, 2.5 }, { 1.4, 1.0, 1.0 }, 10.0 * kPi / 180.0 } };
	return scene;
}

void CheckSpot( testing::Checks &checks, IRenderDevice2 &device, ShadowDepthRenderer &depth,
    ShadowReceiverRenderer &receiver, ShadowReceiverRenderer &reversed, const Meshes &meshes )
{
	const Scene scene = SpotScene();
	const Camera camera{ { 0.0, -16.0, 13.0 }, { 0.0, 1.0, 0.0 }, 1.0 };
	const Spot spot{ { 1.0, 0.5, 11.0 }, Unit( { 0.05, 0.1, -1.0 } ), 38.0 * kPi / 180.0, 40.0 };

	// Two other lights take their tiles first.
	ShadowAtlasLimits limits;
	limits.atlasSize = 2048;
	limits.minTileSize = 128;
	limits.maxTileSize = 1024;
	limits.casterBudget = 8;
	limits.guardTexels = 2;
	const ShadowRequest requests[] = { { 1, 5.0f, 0.5f }, { 2, 4.0f, 0.5f }, { 3, 3.0f, 1.0f } };
	auto plan = PlanShadowAtlas( limits, requests );
	auto spotView = BuildSpotShadowView( { F( spot.position ), F( spot.axis ),
	    float( std::cos( spot.halfAngle ) ), 0.5f, float( spot.range ) } );
	auto otherA = BuildSpotShadowView(
	    { { -6.0f, -6.0f, 9.0f }, { 0.6f, 0.6f, -1.0f }, std::cos( 0.6f ), 0.5f, 40.0f } );
	auto otherB = BuildSpotShadowView(
	    { { 7.0f, 5.0f, 8.0f }, { -0.7f, -0.5f, -1.0f }, std::cos( 0.5f ), 0.5f, 40.0f } );
	if ( !checks.That(
	         plan && spotView && otherA && otherB && plan.Value().allocations[0].HasTile() &&
	             plan.Value().allocations[1].HasTile() && plan.Value().allocations[2].HasTile(),
	         "P1.plan-and-views" ) )
		return;
	const ShadowTile tile = plan.Value().allocations[2].tile;
	std::printf( "INFO spot tile at (%u, %u), %u texels\n", tile.x, tile.y, tile.size );
	checks.That( tile.x != 0 || tile.y != 0, "P1.the-spot-tile-is-away-from-the-atlas-origin" );

	// The oracle's camera model against the renderer's matrices.
	double reprojection = 0.0;
	const float4x4 viewProjection = camera.ViewProjection();
	for ( std::uint32_t y = 7; y < kSize; y += 31 )
	{
		for ( std::uint32_t x = 5; x < kSize; x += 29 )
		{
			const auto g = camera.Ground( x + 0.5, y + 0.5, 1e9 );
			if ( !g )
				continue;
			const math::float4 h = math::Transform(
			    viewProjection, { float( g->x ), float( g->y ), float( g->z ), 1 } );
			const double px = ( h.x / h.w * 0.5 + 0.5 ) * kSize;
			const double py = ( 0.5 - h.y / h.w * 0.5 ) * kSize;
			reprojection = std::max( reprojection,
			    std::max( std::fabs( px - ( x + 0.5 ) ), std::fabs( py - ( y + 0.5 ) ) ) );
		}
	}
	std::printf( "INFO camera model reprojects within %.2g pixels\n", reprojection );
	checks.That( reprojection < 0.01, "P1.the-oracle-camera-matches-the-projection" );

	const std::vector<ShadowCaster> casters = Casters( meshes, scene.casters );
	std::vector<Box> everything = scene.casters;
	everything.insert( everything.end(), scene.nonCasters.begin(), scene.nonCasters.end() );
	const std::vector<ShadowCaster> all = Casters( meshes, everything );
	const std::vector<ShadowCaster> withoutFirst( casters.begin() + 1, casters.end() );
	const ShadowDepthView views[] = {
	    { otherA.Value().viewProjection, plan.Value().allocations[0].tile, all },
	    { otherB.Value().viewProjection, plan.Value().allocations[1].tile, all },
	    { spotView.Value().viewProjection, tile, casters } };
	const ShadowDepthView missingCaster[] = {
	    views[0], views[1], { spotView.Value().viewProjection, tile, withoutFirst } };

	const auto receiversFor = [&]( ShadowReceiverRenderer &renderer, const ShadowTile &sampled )
	{
		Receivers r;
		r.renderer = &renderer;
		r.view = { camera.View(), viewProjection };
		r.light.kind = ShadowReceiverLight::Kind::kSpot;
		r.light.position = F( spot.position );
		r.light.axis = F( spot.axis );
		r.light.outerCos = float( std::cos( spot.halfAngle ) );
		r.light.range = float( spot.range );
		r.light.tileCount = 1;
		r.light.tiles[0] = PackShadowTile( MakeTileProjection( spotView.Value().viewProjection,
		                                       sampled, 2048, limits.guardTexels ),
		    2048, kBias );
		r.receivers = { { meshes.ground, float4x4::Identity() } };
		return r;
	};

	const Frame frame = Run( device, depth, 2048, limits.guardTexels, views, nullptr, true );
	checks.That( frame.ok, "P2.the-depth-pass-runs" );
	if ( frame.ok )
	{
		// Texels outside the three viewports keep the clear depth.
		std::uint64_t outside = 0;
		std::uint64_t written = 0;
		std::uint64_t spotWritten = 0;
		const ShadowViewport viewports[] = { TileViewport( views[0].tile, limits.guardTexels ),
		    TileViewport( views[1].tile, limits.guardTexels ),
		    TileViewport( tile, limits.guardTexels ) };
		for ( std::uint32_t y = 0; y < 2048; ++y )
		{
			for ( std::uint32_t x = 0; x < 2048; ++x )
			{
				const float d = frame.atlas[std::size_t( y ) * 2048 + x];
				int inside = -1;
				for ( int v = 0; v < 3; ++v )
				{
					const ShadowViewport &p = viewports[v];
					if ( x >= p.x && x < p.x + p.size && y >= p.y && y < p.y + p.size )
						inside = v;
				}
				if ( inside < 0 && d != 1.0f )
					++outside;
				if ( inside >= 0 && d < 1.0f )
					++written;
				if ( inside == 2 && d < 1.0f )
					++spotWritten;
			}
		}
		std::printf( "INFO atlas: %llu texels written in the viewports (%llu in the spot's), %llu "
		             "outside\n",
		    static_cast<unsigned long long>( written ),
		    static_cast<unsigned long long>( spotWritten ),
		    static_cast<unsigned long long>( outside ) );
		checks.That( spotWritten > 1000 && outside == 0, "P2.depth-lands-only-inside-its-tiles" );
	}

	const double texelAngle =
	    1.5 * 2.0 * std::tan( spot.halfAngle ) / double( tile.size - 2 * limits.guardTexels );
	const auto classAt = [&]( const D3 &p )
	{
		return spot.Holds( p ) ? scene.Classify( p, spot.position ) : kOutside;
	};
	const auto margin = [&]( const D3 &p, double footprint )
	{
		const D3 toLight = spot.position - p;
		const double cosIncidence = std::max( 0.2, std::fabs( toLight.z ) / Len( toLight ) );
		return 2.0 * footprint + 3.0 * texelAngle * Len( toLight ) / cosIncidence;
	};
	const std::vector<Judged> judged = JudgePixels( camera, classAt, margin );

	const Receivers good = receiversFor( receiver, tile );
	const Frame lit = Run( device, depth, 2048, limits.guardTexels, views, &good, false );
	const Tally tally = Score( lit, judged );
	Print( "spot", tally );
	checks.That( lit.ok && tally.notReceiver == 0, "P3.the-receiver-frame-runs" );
	checks.That( tally.counts[kShadowed] >= 200 && tally.wrong[kShadowed] == 0,
	    "P3.a-caster-darkens-its-receiver" );
	checks.That( tally.counts[kNonCaster] >= 50 && tally.wrong[kNonCaster] == 0,
	    "P4.a-non-caster-does-not" );
	checks.That( tally.counts[kLitClass] >= 2000 && tally.wrong[kLitClass] == 0,
	    "P5.lit-pixels-far-from-shadows-stay-lit" );
	checks.That( tally.counts[kOutside] >= 200 && tally.wrong[kOutside] == 0,
	    "P5.pixels-outside-the-cone-stay-dark" );

	// Seeded defects.
	const Receivers flipped = receiversFor( reversed, tile );
	const Tally reversedTally =
	    Score( Run( device, depth, 2048, limits.guardTexels, views, &flipped, false ), judged );
	Print( "seeded depth-reversed", reversedTally );
	checks.That( reversedTally.wrong[kShadowed] + reversedTally.wrong[kLitClass] +
	                     reversedTally.wrong[kNonCaster] >
	                 0,
	    "P6.detects-depth-reversed" );

	ShadowTile shifted = tile;
	shifted.x = ( tile.x + tile.size ) % 2048;
	shifted.y = shifted.x == 0 ? ( tile.y + tile.size ) % 2048 : tile.y;
	const Receivers offset = receiversFor( receiver, shifted );
	const Tally offsetTally =
	    Score( Run( device, depth, 2048, limits.guardTexels, views, &offset, false ), judged );
	Print( "seeded tile-offset", offsetTally );
	checks.That( offsetTally.wrong[kShadowed] + offsetTally.wrong[kLitClass] +
	                     offsetTally.wrong[kNonCaster] >
	                 0,
	    "P6.detects-a-wrong-tile-offset" );

	const Tally missingTally = Score(
	    Run( device, depth, 2048, limits.guardTexels, missingCaster, &good, false ), judged );
	Print( "seeded caster-left-out", missingTally );
	checks.That( missingTally.wrong[kShadowed] > 0, "P6.detects-a-caster-left-out" );
}

// ---------------------------------------------------------------------------
// The sun

Scene SunScene()
{
	Scene scene;
	const double deg = kPi / 180.0;
	scene.casters = { { { -0.3, -16.2, 0.5 }, { 0.6, 0.5, 0.3 }, 20 * deg },
	    { { -1.5, -14.0, 1.5 }, { 1.0, 1.4, 0.8 }, 25 * deg },
	    { { 2.0, -9.0, 2.0 }, { 1.6, 1.0, 1.0 }, -35 * deg },
	    { { -3.0, 0.0, 2.5 }, { 2.4, 1.8, 1.2 }, 15 * deg },
	    { { 4.0, 10.0, 3.0 }, { 3.0, 2.2, 1.5 }, 40 * deg },
	    { { -6.0, 24.0, 4.0 }, { 4.5, 3.0, 2.0 }, -25 * deg },
	    { { 5.0, 36.0, 4.0 }, { 5.0, 4.0, 2.0 }, 30 * deg } };
	scene.nonCasters = { { { 1.5, -11.5, 1.5 }, { 1.6, 1.6, 1.0 }, 0.0 } };
	return scene;
}

std::optional<CascadeSet> Cascades(
    const Camera &camera, const D3 &sun, std::uint32_t count, std::uint32_t resolution )
{
	CascadeDesc desc;
	desc.cameraView = camera.View();
	desc.verticalFovRadians = float( camera.fov );
	desc.aspect = 1.0f;
	desc.nearZ = camera.nearZ;
	desc.shadowDistance = 60.0f;
	desc.lambda = 0.8f;
	desc.cascadeCount = count;
	desc.resolution = resolution;
	desc.lightDirection = F( sun );
	desc.casterDistance = 40.0f;
	auto set = BuildCascades( desc );
	if ( !set )
		return std::nullopt;
	return set.Value();
}

// C3: the depth images of one cascade drawn for two cameras 2.37 texels
// apart along the light's right axis, compared after the whole-texel shift
// their boxes differ by. Returns the share of compared texels that differ.
double ShiftDifference( IRenderDevice2 &device, ShadowDepthRenderer &depth, const Meshes &meshes,
    const Scene &scene, const Camera &camera, const D3 &sun, bool snapped, double &shift )
{
	const std::uint32_t resolution = 1020;
	auto first = Cascades( camera, sun, 1, resolution );
	if ( !first )
		return 1.0;
	const Cascade &a = first->cascades[0];
	const D3 right{ a.view.view.rows[0].x, a.view.view.rows[0].y, a.view.view.rows[0].z };
	Camera moved = camera;
	moved.eye = camera.eye + right * ( 2.37 * a.texelSize );
	moved.target = camera.target + right * ( 2.37 * a.texelSize );
	auto second = Cascades( moved, sun, 1, resolution );
	if ( !second )
		return 1.0;
	const Cascade &b = second->cascades[0];
	// The unsnapped defect: each box centered on its sphere.
	const auto viewOf = [snapped]( const Cascade &c )
	{
		float4x4 view = c.view.view;
		if ( !snapped )
		{
			const float3 center = c.bounds.center;
			view.rows[0].w =
			    -math::Dot( { view.rows[0].x, view.rows[0].y, view.rows[0].z }, center );
			view.rows[1].w =
			    -math::Dot( { view.rows[1].x, view.rows[1].y, view.rows[1].z }, center );
		}
		return view;
	};
	const float4x4 viewA = viewOf( a );
	const float4x4 viewB = viewOf( b );
	const std::vector<ShadowCaster> casters = Casters( meshes, scene.casters );
	const ShadowTile tileA{ 0, 0, 1024 };
	const ShadowTile tileB{ 1024, 0, 1024 };
	const ShadowDepthView views[] = {
	    { math::Multiply( a.view.projection, viewA ), tileA, casters },
	    { math::Multiply( b.view.projection, viewB ), tileB, casters } };
	const Frame frame = Run( device, depth, 2048, 2, views, nullptr, true );
	if ( !frame.ok )
		return 1.0;
	// A view's row 0 is ( right, -x ): texel column ( right . p - x ) / texel
	// from the box's left edge, so a world point at column j of A sits at
	// column j - s of B, s = ( xB - xA ) / texel.
	shift = ( double( viewA.rows[0].w ) - double( viewB.rows[0].w ) ) / a.texelSize;
	const long s = std::lround( shift );
	std::uint64_t compared = 0;
	std::uint64_t differ = 0;
	for ( std::uint32_t y = 2; y < 2 + resolution; ++y )
	{
		for ( std::uint32_t x = 2; x < 2 + resolution; ++x )
		{
			const long xb = long( x ) - s;
			if ( xb < 2 || xb >= long( 2 + resolution ) )
				continue;
			const float da = frame.atlas[std::size_t( y ) * 2048 + x];
			const float db = frame.atlas[std::size_t( y ) * 2048 + 1024 + std::size_t( xb )];
			if ( da == 1.0f && db == 1.0f )
				continue;
			++compared;
			differ += std::fabs( da - db ) > 1.0e-5f;
		}
	}
	return compared ? double( differ ) / double( compared ) : 1.0;
}

void CheckSun( testing::Checks &checks, IRenderDevice2 &device, ShadowDepthRenderer &depth,
    ShadowReceiverRenderer &receiver, const Meshes &meshes )
{
	const Scene scene = SunScene();
	const Camera camera{ { 0.0, -20.0, 2.0 }, { 0.0, 20.0, -4.0 }, 1.0 };
	const D3 sun = Unit( { 0.4, 0.5, -1.0 } );
	auto cascades = Cascades( camera, sun, 4, 1020 );
	auto reference = Cascades( camera, sun, 1, 2044 );
	ShadowAtlasLimits limits;
	limits.atlasSize = 2048;
	limits.minTileSize = 256;
	limits.maxTileSize = 1024;
	limits.casterBudget = 4;
	limits.guardTexels = 2;
	const ShadowRequest requests[] = {
	    { 10, 4.0f, 1.0f }, { 11, 3.0f, 1.0f }, { 12, 2.0f, 1.0f }, { 13, 1.0f, 1.0f } };
	auto plan = PlanShadowAtlas( limits, requests );
	if ( !checks.That( cascades && reference && plan && plan.Value().allocated == 4,
	         "C1.cascades-and-plan" ) )
		return;

	const std::vector<ShadowCaster> casters = Casters( meshes, scene.casters );
	std::vector<ShadowDepthView> views;
	Receivers cascaded;
	cascaded.renderer = &receiver;
	cascaded.view = { camera.View(), camera.ViewProjection() };
	cascaded.light.kind = ShadowReceiverLight::Kind::kSun;
	cascaded.light.tileCount = 4;
	cascaded.receivers = { { meshes.ground, float4x4::Identity() } };
	for ( std::uint32_t c = 0; c < 4; ++c )
	{
		const Cascade &cascade = cascades->cascades[c];
		const ShadowTile tile = plan.Value().allocations[c].tile;
		views.push_back( { cascade.view.viewProjection, tile, casters } );
		cascaded.light.tiles[c] = PackShadowTile(
		    MakeTileProjection( cascade.view.viewProjection, tile, 2048, 2 ), 2048, kBias );
		cascaded.light.splitFar[c] = cascade.splitFar;
	}
	const Frame cascadedFrame = Run( device, depth, 2048, 2, views, &cascaded, false );

	const Cascade &single = reference->cascades[0];
	const ShadowTile whole{ 0, 0, 2048 };
	const ShadowDepthView referenceViews[] = { { single.view.viewProjection, whole, casters } };
	Receivers referenced = cascaded;
	referenced.light.tileCount = 1;
	referenced.light.tiles[0] = PackShadowTile(
	    MakeTileProjection( single.view.viewProjection, whole, 2048, 2 ), 2048, kBias );
	referenced.light.splitFar[0] = single.splitFar;
	const Frame referenceFrame = Run( device, depth, 2048, 2, referenceViews, &referenced, false );
	checks.That( cascadedFrame.ok && referenceFrame.ok, "C1.the-sun-frames-run" );
	if ( !cascadedFrame.ok || !referenceFrame.ok )
		return;

	const D3 towardSun = sun * -200.0;
	const auto classAt = [&]( const D3 &p )
	{
		return scene.Classify( p, p + towardSun );
	};
	const double cosIncidence = std::fabs( sun.z );
	const auto cascadeOf = [&]( const D3 &p )
	{
		const double d = camera.ViewDistance( p );
		std::uint32_t c = 0;
		while ( c < 4 && d > cascades->cascades[c].splitFar )
			++c;
		return c;
	};
	const auto margin = [&]( const D3 &p, double footprint )
	{
		const std::uint32_t c = cascadeOf( p );
		const double texel = std::max(
		    c < 4 ? double( cascades->cascades[c].texelSize ) : 0.0, double( single.texelSize ) );
		return 2.0 * footprint + 3.0 * texel / cosIncidence;
	};
	std::vector<Judged> judged = JudgePixels( camera, classAt, margin );
	// Beyond the shadow distance both renders light everything.
	std::erase_if( judged,
	    [&]( const Judged &j )
	    {
		    return camera.ViewDistance( j.ground ) > 0.98 * 60.0;
	    } );

	std::uint64_t perCascade[5] = {};
	std::uint64_t selectionErrors = 0;
	std::uint64_t selectionJudged = 0;
	for ( const Judged &j : judged )
	{
		const std::uint32_t c = cascadeOf( j.ground );
		++perCascade[c];
		// Away from split boundaries, the cascade used is the CPU's.
		const double d = camera.ViewDistance( j.ground );
		bool nearSplit = false;
		for ( std::uint32_t k = 0; k < 4; ++k )
			nearSplit = nearSplit || std::fabs( d - cascades->cascades[k].splitFar ) < 0.01 * d;
		if ( nearSplit )
			continue;
		++selectionJudged;
		selectionErrors += Blue( cascadedFrame, j.x, j.y ) != int( std::lround( c * 63.75 ) );
	}
	std::printf( "INFO sun: judged pixels per cascade %llu %llu %llu %llu; selection %llu of %llu "
	             "wrong; splits %.2f %.2f %.2f %.2f; texels %.4f %.4f %.4f %.4f, reference %.4f\n",
	    static_cast<unsigned long long>( perCascade[0] ),
	    static_cast<unsigned long long>( perCascade[1] ),
	    static_cast<unsigned long long>( perCascade[2] ),
	    static_cast<unsigned long long>( perCascade[3] ),
	    static_cast<unsigned long long>( selectionErrors ),
	    static_cast<unsigned long long>( selectionJudged ), cascades->cascades[0].splitFar,
	    cascades->cascades[1].splitFar, cascades->cascades[2].splitFar,
	    cascades->cascades[3].splitFar, cascades->cascades[0].texelSize,
	    cascades->cascades[1].texelSize, cascades->cascades[2].texelSize,
	    cascades->cascades[3].texelSize, single.texelSize );
	checks.That(
	    perCascade[0] >= 50 && perCascade[1] >= 50 && perCascade[2] >= 50 && perCascade[3] >= 50,
	    "C1.every-cascade-shades-judged-pixels" );
	checks.That( selectionJudged > 1000 && selectionErrors == 0,
	    "C1.each-pixel-uses-the-cascade-its-split-holds" );

	const Tally cascadedTally = Score( cascadedFrame, judged );
	const Tally referenceTally = Score( referenceFrame, judged );
	Print( "sun cascaded", cascadedTally );
	Print( "sun reference", referenceTally );
	checks.That( cascadedTally.counts[kShadowed] >= 200 &&
	                 cascadedTally.counts[kLitClass] >= 2000 && cascadedTally.Wrong() == 0,
	    "C2.cascades-match-the-oracle" );
	checks.That(
	    referenceTally.Wrong() == 0, "C2.the-single-cascade-reference-matches-the-oracle" );
	std::uint64_t agree = 0;
	for ( const Judged &j : judged )
	{
		const bool a = Bright( Red( cascadedFrame, j.x, j.y ) );
		const bool b = Bright( Red( referenceFrame, j.x, j.y ) );
		agree += a == b;
	}
	const double agreement = judged.empty() ? 0.0 : double( agree ) / double( judged.size() );
	std::printf( "INFO sun: cascaded and reference agree on %llu of %zu judged pixels (%.5f)\n",
	    static_cast<unsigned long long>( agree ), judged.size(), agreement );
	checks.That(
	    agreement >= kCascadeAgreement, "C2.cascades-agree-with-the-single-cascade-reference" );

	double shift = 0.0;
	const double snapped =
	    ShiftDifference( device, depth, meshes, scene, camera, sun, true, shift );
	std::printf(
	    "INFO snapping: shift %.4f texels, %.6f of compared texels differ\n", shift, snapped );
	checks.That( std::fabs( shift - std::round( shift ) ) < 1e-3 && std::lround( shift ) != 0 &&
	                 snapped <= kSnapTolerance,
	    "C3.a-snapped-cascade-moves-by-whole-texels" );
	double unsnappedShift = 0.0;
	const double unsnapped =
	    ShiftDifference( device, depth, meshes, scene, camera, sun, false, unsnappedShift );
	std::printf( "INFO seeded unsnapped: shift %.4f texels, %.6f of compared texels differ\n",
	    unsnappedShift, unsnapped );
	checks.That( unsnapped > 10.0 * kSnapTolerance, "C3.detects-an-unsnapped-cascade" );
}

} // namespace

int main()
{
	testing::Checks checks;
	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<IRenderDevice2> device = std::move( created ).Value();
		const std::string_view name = device->Facts().adapterName;
		std::printf( "INFO shadows adapter: %.*s\n", static_cast<int>( name.size() ), name.data() );
		resources::MeshCache cache( *device );
		const std::optional<Meshes> meshes = StageMeshes( *device, cache );
		auto depth = ShadowDepthRenderer::Create( *device );
		auto receiver = ShadowReceiverRenderer::Create( *device, Format::kRGBA8Unorm );
		auto reversed = ShadowReceiverRenderer::Create( *device, Format::kRGBA8Unorm,
		    rendertest::shadows::spirv::kShadowReceiverDepthReversed );
		if ( !checks.That( meshes && depth && receiver && reversed, "setup.meshes-and-renderers" ) )
			return checks.Report();

		CheckSpot( checks, *device, *depth.Value(), *receiver.Value(), *reversed.Value(), *meshes );
		CheckSun( checks, *device, *depth.Value(), *receiver.Value(), *meshes );
		checks.That( depth.Value()->RecordFailures() == 0 &&
		                 receiver.Value()->RecordFailures() == 0 &&
		                 reversed.Value()->RecordFailures() == 0,
		    "setup.records-without-failure" );
		depth.Value().reset();
		receiver.Value().reset();
		reversed.Value().reset();
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
