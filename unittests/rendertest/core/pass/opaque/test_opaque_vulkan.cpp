//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.opaque (RFC 0016 K5, K4 families) on render.device.vulkan:
//			scene draw lists drawn by render.pass.opaque through render.graph
//			and the unlit family (flat $color materials over a white texture),
//			with real pixels.
//
//			- O1 scene A: the near red cube covers the center of the far blue
//			  cube although blue draws later (the depth test), and the blue
//			  cube shows around it; the background is the clear color;
//			- O2 two scenes: A's frame has nothing of B's green cube and B's
//			  frame nothing of A's cubes;
//			- O3 destroying scene A (the scene and its snapshots) leaves B's
//			  frame byte-identical;
//			- O4 draws whose mesh or material does not resolve, whose texture is
//			  not resident, whose stride is not the program's or whose draw group
//			  is missing or of another layout are counted and not drawn: A with
//			  seven such draws (one reading a view group the frame lacks) gives
//			  A's bytes;
//			- O6 scene C, the other families with their groups: a lightmapped
//			  cube reads its lightmap page from its draw group (white times the
//			  page times the lightmap scale), and a pbr cube reads the frame's
//			  split-sum table and the view's model lighting (a white rough
//			  dielectric under a uniform ambient cube returns the cube), and a
//			  vertexlit cube reads its lighting from its draw group (white under
//			  an ambient cube lit on +z shows that face's light); all three draw
//			  and none is unresolved. Without the view group the pbr draw is
//			  counted and not drawn;
//			- validation: no message from the Khronos validation layer
//			  (synchronization validation included) when it is installed.
//
//=============================================================================//

#include "opaque_fixtures.h"
#include "render/device/vulkan/provider.h"
#include "testing/checks.h"

#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace
{

using namespace rendertest::opaque;
namespace vulkan = render::device::vulkan;

struct Rgb
{
	int r, g, b;
	friend bool operator==( Rgb, Rgb ) = default;
};

Rgb At( const FrameResult &frame, std::pair<int, int> pixel )
{
	const auto [x, y] = pixel;
	if ( !frame.ok || x < 0 || y < 0 || x >= int( kSize ) || y >= int( kSize ) )
		return { -1, -1, -1 };
	const std::uint8_t *p = &frame.rgba[( std::size_t( y ) * kSize + std::size_t( x ) ) * 4];
	return { p[0], p[1], p[2] };
}

bool Near( Rgb a, Rgb b )
{
	return std::abs( a.r - b.r ) <= 2 && std::abs( a.g - b.g ) <= 2 && std::abs( a.b - b.b ) <= 2;
}

int Unorm( float linear )
{
	return int( std::lround( std::fmin( std::fmax( linear, 0.0f ), 1.0f ) * 255.0f ) );
}

float SrgbToLinear( std::uint8_t value )
{
	const float c = value / 255.0f;
	return c <= 0.04045f ? c / 12.92f : std::pow( ( c + 0.055f ) / 1.055f, 2.4f );
}

bool Contains( const FrameResult &frame, Rgb color )
{
	for ( std::size_t i = 0; frame.ok && i + 3 < frame.rgba.size(); i += 4 )
	{
		if ( Rgb{ frame.rgba[i], frame.rgba[i + 1], frame.rgba[i + 2] } == color )
			return true;
	}
	return false;
}

constexpr Rgb kRed{ 255, 0, 0 };
constexpr Rgb kBlue{ 0, 0, 255 };
constexpr Rgb kGreen{ 0, 255, 0 };
constexpr Rgb kClear{ 0, 0, 0 };

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
		std::unique_ptr<device::IRenderDevice2> device = std::move( created ).Value();
		resources::MeshCache cache( *device );
		std::optional<Meshes> meshes = StageCube( *device, cache );
		std::unique_ptr<Materials> materials =
		    StageMaterials( *device, device::Format::kRGBA8Unorm, device::Format::kD32Float );
		if ( !checks.That( meshes && materials, "setup.meshes-and-materials" ) )
			return checks.Report();
		const Materials &programs = *materials;

		const scene::SceneView view = View();
		auto a = SceneA();
		auto b = SceneB();
		const FrameResult frameA = DrawScene( *device, programs, *meshes, *a->Snapshot() );
		const FrameResult frameB = DrawScene( *device, programs, *meshes, *b->Snapshot() );
		checks.That( frameA.ok && frameB.ok, "O1.both-frames-run" );
		checks.That(
		    frameA.stats.drawn == 2 && frameA.stats.unresolved == 0 && frameB.stats.drawn == 1,
		    "O1.every-draw-resolves" );
		checks.That( At( frameA, Pixel( view, { -2.0f, 0.0f, -5.6f } ) ) == kRed,
		    "O1.the-near-cube-wins-the-depth-test" );
		checks.That( At( frameA, Pixel( view, { -2.0f, 1.2f, -8.5f } ) ) == kBlue,
		    "O1.the-far-cube-shows-around-it" );
		checks.That(
		    At( frameA, { 2, 2 } ) == kClear && At( frameA, { int( kSize ) - 3, 2 } ) == kClear,
		    "O1.the-background-is-the-clear-color" );
		checks.That( At( frameB, Pixel( view, { 2.5f, 0.0f, -7.0f } ) ) == kGreen,
		    "O2.scene-b-draws-its-cube" );
		checks.That(
		    !Contains( frameA, kGreen ) && !Contains( frameB, kRed ) && !Contains( frameB, kBlue ),
		    "O2.scenes-do-not-share-content" );

		a.reset();
		const FrameResult again = DrawScene( *device, programs, *meshes, *b->Snapshot() );
		checks.That( again.ok && again.rgba == frameB.rgba, "O3.destroying-a-leaves-b-identical" );

		auto withMissing = SceneA( true );
		const FrameResult missing =
		    DrawScene( *device, programs, *meshes, *withMissing->Snapshot() );
		checks.That( missing.stats.drawn == 2 && missing.stats.unresolved == 7,
		    "O4.unresolved-draws-are-counted" );
		checks.That(
		    missing.ok && missing.rgba == frameA.rgba, "O4.unresolved-draws-are-not-drawn" );
		auto c = SceneC();
		const material::DrawGroup *frameGroup = materials->drawGroups.Group( kPbrFrameGroup );
		const material::DrawGroup *viewGroup = materials->drawGroups.Group( kPbrViewGroup );
		// The lightmapped family's frame terms are another frame layout.
		const material::DrawGroup *const lightmappedFrame[] = {
		    materials->drawGroups.Group( kLightmappedFrameGroup ) };
		const FrameResult frameC = DrawScene(
		    *device, programs, *meshes, *c->Snapshot(), frameGroup, viewGroup, lightmappedFrame );
		if ( !checks.That( frameGroup && viewGroup && frameC.ok && frameC.stats.drawn == 3 &&
		                       frameC.stats.unresolved == 0,
		         "O6.lightmapped-and-pbr-draws-resolve" ) )
			std::printf( "O6: ok %d drawn %u unresolved %u lightmapped frame group %s\n",
			    int( frameC.ok ), frameC.stats.drawn, frameC.stats.unresolved,
			    lightmappedFrame[0] ? "set" : "missing" );
		const Rgb page{ Unorm( SrgbToLinear( kPageTexel[0] ) * material::kLightmapScaleLinear ),
		    Unorm( SrgbToLinear( kPageTexel[1] ) * material::kLightmapScaleLinear ),
		    Unorm( SrgbToLinear( kPageTexel[2] ) * material::kLightmapScaleLinear ) };
		checks.That( Near( At( frameC, Pixel( view, { -1.5f, 0.0f, -5.25f } ) ), page ),
		    "O6.the-lightmapped-cube-shows-its-page" );
		const Rgb ambient{ Unorm( kAmbient[0] ), Unorm( kAmbient[1] ), Unorm( kAmbient[2] ) };
		checks.That( Near( At( frameC, Pixel( view, { 1.5f, 0.0f, -5.25f } ) ), ambient ),
		    "O6.the-pbr-cube-returns-the-view-s-ambient-cube" );
		const Rgb vertexLit{ Unorm( kVertexLitAmbient[0] ), Unorm( kVertexLitAmbient[1] ),
		    Unorm( kVertexLitAmbient[2] ) };
		checks.That( Near( At( frameC, Pixel( view, { 0.0f, 1.6f, -5.5f } ) ), vertexLit ),
		    "O6.the-vertexlit-cube-shows-its-draw-group-s-lighting" );
		const FrameResult noView = DrawScene(
		    *device, programs, *meshes, *c->Snapshot(), frameGroup, nullptr, lightmappedFrame );
		checks.That( noView.ok && noView.stats.drawn == 2 && noView.stats.unresolved == 1 &&
		                 At( noView, Pixel( view, { 1.5f, 0.0f, -5.25f } ) ) == kClear,
		    "O6.without-the-view-group-the-pbr-draw-is-unresolved" );
		checks.Equal( materials->drawGroups.GroupFailures(), 0u, "O6.draw-groups-without-failure" );
		checks.Equal( materials->programs.GroupFailures(), 0u, "O1.groups-without-failure" );
		(void)device->WaitIdle();
		materials.reset();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
