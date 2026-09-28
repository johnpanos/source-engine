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
//			- validation: no message from the Khronos validation layer
//			  (synchronization validation included) when it is installed.
//
//=============================================================================//

#include "opaque_fixtures.h"
#include "render/device/vulkan/provider.h"
#include "testing/checks.h"

#include <atomic>
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
