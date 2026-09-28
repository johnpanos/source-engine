//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.lines (RFC 0016, linux-native-vulkan-gpu): render.pass.lines
//			on render.device.vulkan with real pixels. The view is the identity
//			box (world x, y in [-1, 1] across the 64 x 64 target, y up; world
//			z is depth), so every expected pixel follows from the item.
//
//			P1 a screen-space quad covers its pixel rectangle (origin top-left,
//			   y down) and nothing outside it;
//			P2 a world-space line lands on the row its y projects to;
//			P3 depth: a depth-tested line behind a depth-tested triangle is
//			   hidden; an untested line draws over it;
//			P4 the depth bias (a clip-space z offset) pulls depth-tested
//			   lines, not triangles, toward the eye: a line just behind a face
//			   shows with the bias and is hidden without it; a triangle just
//			   behind stays hidden;
//			P5 screen filled items draw after screen lines (a handle covers
//			   the line through it);
//			P6 straight alpha blends: a half-transparent white quad over black
//			   reads half white;
//			P7 a resident batch draws, and the same inputs give identical
//			   frames;
//			P8 colors are display values: on an sRGB target the pass decodes
//			   them, so they read back as written (without the decode 128
//			   would read 188);
//			the Khronos validation layer reports no message.
//
//=============================================================================//

#include "lines_fixtures.h"
#include "render/device/vulkan/provider.h"
#include "testing/checks.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{

using namespace rendertest::lines;

struct Rgb
{
	int r = 0;
	int g = 0;
	int b = 0;
	friend bool operator==( const Rgb &, const Rgb & ) = default;
};

Rgb At( const FrameResult &frame, int x, int y )
{
	if ( frame.rgba.size() != kSize * kSize * 4 || x < 0 || y < 0 || x >= int( kSize ) ||
	     y >= int( kSize ) )
		return { -1, -1, -1 };
	const std::size_t i = ( std::size_t( y ) * kSize + std::size_t( x ) ) * 4;
	return { frame.rgba[i], frame.rgba[i + 1], frame.rgba[i + 2] };
}

// The world y whose projection is the center of pixel row 'row'.
float RowY( int row )
{
	return 1.0f - 2.0f * ( float( row ) + 0.5f ) / float( kSize );
}

constexpr Rgba8 kRed{ 255, 0, 0, 255 };
constexpr Rgba8 kGreen{ 0, 255, 0, 255 };
constexpr Rgba8 kBlue{ 0, 0, 255, 255 };
constexpr Rgba8 kYellow{ 255, 255, 0, 255 };
constexpr Rgba8 kMagenta{ 255, 0, 255, 255 };
constexpr Rgb kBlack{ 0, 0, 0 };

// A world triangle covering the whole target at depth z.
void Floor( LineList &list, float z, Rgba8 color )
{
	list.Triangle(
	    { Space::kWorld, true }, { -3, -3, z }, { 3, -3, z }, { 0, 3, z }, color, color, color );
}

} // namespace

int main()
{
	testing::Checks checks;
	namespace vulkan = render::device::vulkan;
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
		auto made = LinesRenderer::Create(
		    *device, device::Format::kRGBA8Unorm, device::Format::kD32Float );
		if ( !checks.That( made.HasValue(), "setup.renderer" ) )
			return checks.Report();
		LinesRenderer &renderer = *made.Value();

		// P1, P2.
		LineList basic;
		basic.Quad( { Space::kScreen, false }, 10, 10, 20, 20, kRed );
		basic.Line( { Space::kWorld, false }, { -1, RowY( 40 ), 0 }, { 1, RowY( 40 ), 0 }, kGreen );
		const FrameResult frame = Draw( *device, renderer, basic );
		checks.That( frame.ok, "P1.the-frame-runs" );
		checks.That(
		    At( frame, 11, 11 ) == Rgb{ 255, 0, 0 } && At( frame, 18, 18 ) == Rgb{ 255, 0, 0 },
		    "P1.the-quad-covers-its-pixels" );
		checks.That( At( frame, 8, 15 ) == kBlack && At( frame, 22, 15 ) == kBlack &&
		                 At( frame, 15, 8 ) == kBlack && At( frame, 15, 22 ) == kBlack,
		    "P1.nothing-outside-the-quad" );
		checks.That( At( frame, 32, 40 ) == Rgb{ 0, 255, 0 } && At( frame, 32, 38 ) == kBlack,
		    "P2.a-world-line-lands-on-its-projected-row" );

		// P3.
		LineList depth;
		Floor( depth, 0.25f, kBlue );
		depth.Line(
		    { Space::kWorld, true }, { -1, RowY( 20 ), 0.75f }, { 1, RowY( 20 ), 0.75f }, kRed );
		depth.Line( { Space::kWorld, false }, { -1, RowY( 30 ), 0.75f }, { 1, RowY( 30 ), 0.75f },
		    kMagenta );
		const FrameResult tested = Draw( *device, renderer, depth );
		checks.That(
		    At( tested, 32, 20 ) == Rgb{ 0, 0, 255 }, "P3.a-tested-line-behind-a-face-is-hidden" );
		checks.That(
		    At( tested, 32, 30 ) == Rgb{ 255, 0, 255 }, "P3.an-untested-line-draws-over-it" );

		// P4.
		auto behind = [&]( float bias )
		{
			LineList list;
			Floor( list, 0.25f, kBlue );
			list.Line( { Space::kWorld, true }, { -1, RowY( 20 ), 0.252f },
			    { 1, RowY( 20 ), 0.252f }, kYellow );
			list.Triangle( { Space::kWorld, true }, { -1, RowY( 44 ), 0.252f },
			    { 1, RowY( 44 ), 0.252f }, { 0, RowY( 60 ), 0.252f }, kRed, kRed, kRed );
			return Draw( *device, renderer, list, {}, BoxView( bias ) );
		};
		const FrameResult biased = behind( 0.01f );
		const FrameResult unbiased = behind( 0.0f );
		checks.That(
		    At( biased, 32, 20 ) == Rgb{ 255, 255, 0 }, "P4.the-bias-shows-a-line-just-behind" );
		checks.That(
		    At( unbiased, 32, 20 ) == Rgb{ 0, 0, 255 }, "P4.without-the-bias-it-is-hidden" );
		checks.That(
		    At( biased, 32, 50 ) == Rgb{ 0, 0, 255 }, "P4.the-bias-does-not-move-triangles" );

		// P5.
		LineList handles;
		handles.Line( { Space::kScreen, false }, { 0, 32.5f, 0 }, { 64, 32.5f, 0 }, kRed );
		handles.Disc( { Space::kScreen, false }, 32.0f, 32.5f, 4.0f, kGreen );
		const FrameResult handled = Draw( *device, renderer, handles );
		checks.That(
		    At( handled, 32, 32 ) == Rgb{ 0, 255, 0 } && At( handled, 10, 32 ) == Rgb{ 255, 0, 0 },
		    "P5.screen-filled-items-draw-over-screen-lines" );

		// P8.
		{
			auto srgb = LinesRenderer::Create(
			    *device, device::Format::kRGBA8Srgb, device::Format::kD32Float );
			const Rgba8 colors[] = { { 128, 64, 200, 255 }, { 10, 20, 30, 255 },
			    { 250, 5, 90, 255 }, { 255, 255, 255, 255 } };
			LineList quads;
			for ( int i = 0; i < 4; ++i )
				quads.Quad( { Space::kScreen, false }, float( i * 16 ), 0, float( i * 16 + 16 ), 16,
				    colors[i] );
			const FrameResult frame = srgb ? Draw( *device, *srgb.Value(), quads, {}, BoxView(),
			                                     true, device::Format::kRGBA8Srgb )
			                               : FrameResult();
			bool exact = frame.ok;
			for ( int i = 0; i < 4 && exact; ++i )
			{
				const Rgb got = At( frame, i * 16 + 8, 8 );
				exact = std::abs( got.r - colors[i].r ) <= 1 &&
				        std::abs( got.g - colors[i].g ) <= 1 &&
				        std::abs( got.b - colors[i].b ) <= 1;
			}
			checks.That( exact, "P8.display-colors-read-back-on-an-srgb-target" );
		}

		// P6.
		LineList alpha;
		alpha.Quad( { Space::kScreen, false }, 0, 0, 64, 64, { 255, 255, 255, 128 } );
		const FrameResult blended = Draw( *device, renderer, alpha );
		const Rgb half = At( blended, 32, 32 );
		checks.That( half.r >= 126 && half.r <= 130 && half.r == half.g && half.g == half.b,
		    "P6.straight-alpha-blends" );

		// P7.
		resources::MeshCache cache( *device );
		std::vector<render::pass::lines::LineVertex> vertices( 2 );
		vertices[0] = { { -1.0f, RowY( 50 ), 0.0f }, kYellow.Packed() };
		vertices[1] = { { 1.0f, RowY( 50 ), 0.0f }, kYellow.Packed() };
		auto mesh = StageMesh( *device, cache, "resident", vertices );
		checks.That( mesh.has_value(), "P7.the-batch-is-staged" );
		if ( mesh )
		{
			const MeshBatch batches[] = { { *mesh, Topology::kLines, { Space::kWorld, false } } };
			const FrameResult resident = Draw( *device, renderer, LineList(), batches );
			checks.That(
			    At( resident, 32, 50 ) == Rgb{ 255, 255, 0 }, "P7.a-resident-batch-draws" );
		}
		const FrameResult again = Draw( *device, renderer, basic );
		checks.That(
		    again.ok && again.rgba == frame.rgba, "P7.the-same-inputs-give-the-same-frame" );
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
