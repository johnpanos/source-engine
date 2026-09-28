//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.viewport (RFC 0002 hammer.adapters.render;
//			RFC 0016 "Editor viewports"; linux-native-vulkan-gpu): the
//			editor's ViewportRenderer on render.device.vulkan with real
//			pixels, judged by relations rather than goldens (RFC 0016 decision
//			"oracles"):
//
//			R1 3D: the pixel under each box's top-face center (projected by
//			   the view's own Camera3D) has that face's color as the geometry
//			   builds it: the selected box the shaded selection fill, the
//			   other its own fill;
//			R2 2D: the pixel on a box edge (projected by the Camera2D) has the
//			   edge color (selection orange for the selected box, the plain
//			   edge color otherwise); a grid line shows where no geometry is;
//			   the tool overlay's pending box lands where its corners project;
//			R3 a restage after the selection moves changes the old selected
//			   box's edges to the plain color; the same inputs give the same
//			   frame;
//			the Khronos validation layer reports no message.
//
//=============================================================================//

#include "hammer/adapters/render/viewport_renderer.h"
#include "render/device/vulkan/provider.h"
#include "testing/checks.h"
#include "viewport_fixture.h"

#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace
{

using namespace hammertest::viewport_render;
using hammer::render_adapter::BuildSceneGeometry;
using hammer::render_adapter::SceneGeometry;
using hammer::render_adapter::ViewportRenderer;
using hammer::render_adapter::ViewPixels;
using hammer::render_adapter::ViewRequest;
using hammer::viewport::ViewKind;

struct Rgb
{
	int r = 0;
	int g = 0;
	int b = 0;
	friend bool operator==( const Rgb &, const Rgb & ) = default;
};

Rgb At( const ViewPixels &pixels, int x, int y )
{
	if ( x < 0 || y < 0 || x >= int( pixels.width ) || y >= int( pixels.height ) ||
	     pixels.rgba.size() != std::size_t( pixels.width ) * pixels.height * 4 )
		return { -1, -1, -1 };
	const std::size_t i = ( std::size_t( y ) * pixels.width + std::size_t( x ) ) * 4;
	return { pixels.rgba[i], pixels.rgba[i + 1], pixels.rgba[i + 2] };
}

// Whether the pixel at 'p' or one of its 8 neighbors has 'color' (lines
// exactly on a pixel boundary may land on either side).
bool Near( const ViewPixels &pixels, double x, double y, Rgb color )
{
	for ( int dy = -1; dy <= 1; ++dy )
		for ( int dx = -1; dx <= 1; ++dx )
			if ( At( pixels, int( std::floor( x ) ) + dx, int( std::floor( y ) ) + dy ) == color )
				return true;
	return false;
}

Rgb ColorOf( std::uint32_t packed )
{
	return { int( packed & 0xFF ), int( ( packed >> 8 ) & 0xFF ), int( ( packed >> 16 ) & 0xFF ) };
}

std::optional<Rgb> TopFaceColor( const SceneGeometry &g, double z, double x0, double x1 )
{
	for ( std::size_t i = 0; i + 2 < g.faces.size(); i += 3 )
	{
		bool match = true;
		for ( std::size_t k = 0; k < 3; ++k )
		{
			const float *p = g.faces[i + k].position;
			match = match && std::fabs( p[2] - z ) < 1e-3 && p[0] >= x0 - 1e-3 && p[0] <= x1 + 1e-3;
		}
		if ( match )
			return ColorOf( g.faces[i].color );
	}
	return std::nullopt;
}

constexpr Rgb kSelectedEdge{ 255, 148, 38 };
constexpr Rgb kEdge{ 128, 133, 148 };
constexpr Rgb kPending{ 255, 224, 77 };

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
		std::unique_ptr<render::device::IRenderDevice2> device = std::move( created ).Value();
		Document d;
		Build( d );
		auto made = ViewportRenderer::Create( *device );
		if ( !checks.That( made.HasValue(), "setup.renderer" ) )
			return checks.Report();
		ViewportRenderer &renderer = *made.Value();
		const hammer::viewport::RenderSnapshot selected = Snapshot( d );
		const SceneGeometry geometry = BuildSceneGeometry( selected );
		checks.That( renderer.SetScene( selected, 1 ).HasValue(), "setup.scene" );

		// R1.
		const hammer::viewport::Camera3D eye = EyeCamera();
		ViewRequest request3D;
		request3D.kind = ViewKind::Camera3D;
		request3D.camera3D = &eye;
		request3D.pixelWidth = 256;
		request3D.pixelHeight = 192;
		auto frame3D = renderer.RenderAndWait( request3D );
		checks.That( frame3D.HasValue(), "R1.the-3d-view-renders" );
		if ( frame3D )
		{
			const auto leftCenter = eye.WorldToScreen( Vec3d( -64, 0, 128 ) );
			const auto rightCenter = eye.WorldToScreen( Vec3d( 128, 0, 64 ) );
			const auto leftColor = TopFaceColor( geometry, 128, -128, 0 );
			const auto rightColor = TopFaceColor( geometry, 64, 64, 192 );
			checks.That( leftCenter && leftColor &&
			                 At( frame3D.Value(), int( leftCenter->x ), int( leftCenter->y ) ) ==
			                     *leftColor,
			    "R1.the-selected-top-face-shows-its-selection-fill" );
			checks.That( rightCenter && rightColor &&
			                 At( frame3D.Value(), int( rightCenter->x ), int( rightCenter->y ) ) ==
			                     *rightColor,
			    "R1.the-other-top-face-shows-its-own-fill" );
		}

		// R2.
		const hammer::viewport::Camera2D top = TopCamera();
		ViewRequest request2D;
		request2D.kind = ViewKind::Top;
		request2D.camera2D = &top;
		request2D.grid = { { hammer::viewport::GridLineKind::Axis,
		    hammer::viewport::GridLineOrientation::Horizontal, 0.0, 170.0 } };
		request2D.overlay.Box(
		    Vec3d( 240, 40, 0 ), Vec3d( 256, 56, 16 ), hammer::tools::OverlayRole::Pending );
		request2D.pixelWidth = 256;
		request2D.pixelHeight = 192;
		auto frame2D = renderer.RenderAndWait( request2D );
		checks.That( frame2D.HasValue(), "R2.the-2d-view-renders" );
		if ( frame2D )
		{
			const hammer::viewport::ScreenPoint leftEdge = top.WorldToScreen( Vec3d( -128, 0, 0 ) );
			const hammer::viewport::ScreenPoint rightEdge = top.WorldToScreen( Vec3d( 192, 0, 0 ) );
			const hammer::viewport::ScreenPoint pending = top.WorldToScreen( Vec3d( 240, 48, 0 ) );
			checks.That( Near( frame2D.Value(), leftEdge.x, leftEdge.y, kSelectedEdge ),
			    "R2.the-selected-box-edge-is-orange" );
			checks.That( Near( frame2D.Value(), rightEdge.x, rightEdge.y, kEdge ),
			    "R2.the-other-box-edge-is-plain" );
			checks.That( Near( frame2D.Value(), 20, 170.5, Rgb{ 115, 51, 51 } ),
			    "R2.a-grid-line-shows-where-no-geometry-is" );
			checks.That( Near( frame2D.Value(), pending.x, pending.y, kPending ),
			    "R2.the-pending-box-lands-at-its-projected-corner-edge" );

			// R3.
			auto again = renderer.RenderAndWait( request2D );
			checks.That( again && again.Value().rgba == frame2D.Value().rgba,
			    "R3.the-same-inputs-give-the-same-frame" );
			checks.That( renderer.SetScene( Snapshot( d, false ), 2 ).HasValue(), "R3.restaged" );
			auto moved = renderer.RenderAndWait( request2D );
			checks.That( moved && Near( moved.Value(), leftEdge.x, leftEdge.y, kEdge ) &&
			                 !Near( moved.Value(), leftEdge.x, leftEdge.y, kSelectedEdge ),
			    "R3.a-restage-follows-the-selection" );
		}
		made.Value().reset();
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
