//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's presenting host (RFC 0016 K11, render.lab.app): the
//			lab's scenes shown on a real display through render.presentation.v1,
//			so the output term (render.output.v1: exposure, tone map, output
//			encoding) is proven on the display it is for before any product
//			wiring. The headless render_lab links no window system; this host
//			adds one: an SDL3 window, the SDL3-Vulkan presentation bridge and
//			the render.backend.v1 provider over the Vulkan adapter's host device,
//			whose Port() the core draws with. One Vulkan device serves both.
//
//			Each frame the core's render.pass.output reads the scene (a linear
//			RGBA16F image in multiples of SDR white) and writes the
//			presentation's back buffer, imported as a port texture whose home
//			usage is kExternal (the bridge reads it next). The headroom comes
//			from the presentation every frame.
//
//			An extended-linear presentation (kRGBA16Float) is asked for first;
//			when the surface cannot show it, a standard one (kRGBA8Unorm), and
//			Range() says which. Nothing else falls back.
//
//			The scene is the HDR chart (ChartTexel): a gray ramp from 1/64 to
//			16 times white, gray and warm patches from 0.18 to 16, and six
//			saturated patches at 4. Its values are stored as half floats, the
//			lab canvas's format.
//
//			Private to render.lab.app; the device suite
//			(unittests/rendertest/lab/test_lab_hdr_device.cpp) and the
//			program's main (lab_app_main.cpp) drive it.
//
//=============================================================================//

#ifndef RENDER_LAB_APP_LAB_APP_H
#define RENDER_LAB_APP_LAB_APP_H

#include "render/render_presentation.h"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace render::lab::app
{

// The chart's peak: its brightest value, in multiples of SDR white.
inline constexpr float kChartPeak = 16.0f;

// The linear RGB of chart texel (x, y) of a width x height chart, as the
// scene stores it (rounded to half floats); row 0 at the top.
std::array<float, 3> ChartTexel(
    std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height );

// A uniform patch of the chart: its center as fractions of the chart's width
// and height, and its value.
struct ChartPatch
{
	const char *name;
	float u = 0.0f;
	float v = 0.0f;
	std::array<float, 3> value{};
};
std::vector<ChartPatch> ChartPatches();

struct LabAppOptions
{
	// Ask for the extended-linear range first (false: standard only).
	bool extended = true;
	// The scene's width at most; the height follows the drawable's aspect,
	// and the bridge scales the back buffer to the drawable.
	std::uint32_t maxWidth = 1920;
	bool fullscreen = true;
};

struct LabFrame
{
	float exposure = 1.0f;
	float scenePeak = kChartPeak;
	bool toneMap = true;  // false: a debug view, the encoding alone
	bool capture = false; // keep the presented swapchain image for ReadCapture
};

struct LabCapture
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<float> linear;       // extended-linear presentations: RGBA floats
	std::vector<std::uint8_t> rgba8; // standard presentations: RGBA bytes
	// Linear RGB (or the bytes, as 0 to 255) at fractions (u, v) of the image.
	std::array<float, 3> At( float u, float v ) const;
};

// tvOS's display mode request (render.presentation.v1 on the Apple bridge).
struct LabDisplayMode
{
	bool known = false; // false where there is none (not tvOS)
	bool matchingEnabled = false;
	bool switching = false;
	bool askedForHdr = false;
	unsigned hdrModes = 0; // the display's HDR modes: hlg 1, hdr10 2, dolbyVision 4
	bool eligibleForHdr = false;
};

class LabApp
{
public:
	static std::unique_ptr<LabApp> Create( const LabAppOptions &options, std::string *error );
	~LabApp();

	LabApp( const LabApp & ) = delete;
	LabApp &operator=( const LabApp & ) = delete;

	RenderDynamicRange Range() const;
	RenderDynamicRangeState DynamicRange() const;
	std::uint32_t SceneWidth() const;
	std::uint32_t SceneHeight() const;

	// Presents one frame of the scene through the output pass with the
	// presentation's current headroom (returned in *usedHeadroom). False,
	// with the reason, when the frame could not be drawn or presented.
	bool Frame( const LabFrame &frame, float *usedHeadroom, std::string *error );
	// The last frame presented with capture set.
	bool ReadCapture( LabCapture &out );
	// What the platform layer reports (Apple): high range, extended linear sRGB.
	bool ReadLayer( bool *extended, bool *extendedLinear );
	LabDisplayMode ReadDisplayMode();
	// Handles window events; false once the window is asked to close.
	bool Pump();

	struct State;

private:
	explicit LabApp( std::unique_ptr<State> state );
	std::unique_ptr<State> m_State;
};

} // namespace render::lab::app

#endif // RENDER_LAB_APP_LAB_APP_H
