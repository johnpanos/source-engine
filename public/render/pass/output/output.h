//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.output (RFC 0016 "Output", render.output.v1): the last
//			frame term. It takes the linear scene image (multiples of SDR
//			reference white) to the values a presentation shows:
//
//			1. exposure: the scene times OutputParams::exposure (the legacy
//			   chain computes it; this pass applies it);
//			2. the tone map (render/shaders/common/tone_map.glsl): each channel
//			   clipped to the scene's peak, then max(R, G, B) mapped from
//			   [0, scenePeak] onto [0, headroom] by the ITU-R BT.2390 EETF with
//			   one scale for the three channels. It is the clip alone when
//			   headroom >= scenePeak, so at SDR (peak 1, headroom 1) it is
//			   exactly the legacy clip;
//			3. the output encoding (render/shaders/common/color_encoding.glsl),
//			   chosen by the target's format:
//			   - kRGBA8Unorm, kBGRA8Unorm: the sRGB curve in the shader;
//			   - kRGBA8Srgb, kBGRA8Srgb: linear values the attachment encodes;
//			   - kRGBA16Float: linear values (extended linear sRGB, scRGB), for
//			     a render.presentation.v1 kExtendedLinear back buffer.
//			   The 8-bit targets show the standard range: they take headroom 1
//			   only.
//
//			The headroom is the presentation's current headroom
//			(render.presentation.v1's RenderDynamicRangeState), read every
//			frame.
//			A debug view (RFC 0014: PixelViewActive(frame.debug)) sets
//			toneMap false: exposure and the tone map are bypassed and the
//			encoding alone is applied, so a view's linear values reach the
//			target untouched.
//
//			The pass reads the scene texel under each target pixel: the scene
//			and the target have the same extent (scaling to the drawable is
//			presentation's). Nothing falls back: invalid targets or
//			parameters fail before any pass is added.
//
//=============================================================================//

#ifndef RENDER_PASS_OUTPUT_OUTPUT_H
#define RENDER_PASS_OUTPUT_OUTPUT_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/graph/graph_builder.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace render::pass::output
{

// SDR reference white in the PQ domain the tone map works in (ITU-R BT.2408).
inline constexpr float kReferenceWhiteNits = 203.0f;
// The brightest scene peak: PQ's 10000 cd/m^2.
inline constexpr float kMaxScenePeak = 10000.0f / kReferenceWhiteNits;

enum class OutputStatus : std::uint8_t
{
	kInvalidTarget, // a target format the pass cannot encode for
	kInvalidParams, // exposure, scene peak or headroom out of range
	kSizeMismatch,  // scene and target extents differ
	kDevice,        // the device refused a layout or pipeline
};

// How the pass encodes for a target (color_encoding.glsl's kOutputEncoding*).
enum class OutputEncoding : std::uint8_t
{
	kSrgb = 0,     // the sRGB curve in the shader, into an 8-bit UNORM target
	kHardware = 1, // linear into an 8-bit sRGB target, which encodes
	kLinear = 2,   // linear into a half-float target (extended linear range)
};

struct OutputParams
{
	float exposure = 1.0f;  // finite, >= 0
	float scenePeak = 1.0f; // in (0, kMaxScenePeak]
	float headroom = 1.0f;  // finite, >= 1; exactly 1 on an 8-bit target
	bool toneMap = true;    // false for a debug view: the encoding alone
};

struct OutputTargets
{
	graph::ResourceRef scene;  // a float texture the pass samples
	graph::ResourceRef target; // written whole as a color attachment
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

// The encoding for a target format, or kInvalidTarget.
foundation::Expected<OutputEncoding, OutputStatus> EncodingFor( device::Format target );

class OutputRenderer
{
public:
	static foundation::Expected<std::unique_ptr<OutputRenderer>, OutputStatus> Create(
	    device::IRenderDevice2 &device, device::Format targetFormat );
	// Test endpoint: the same pass with another fragment program (a suite's
	// seeded defects), which must declare the same interface.
	static foundation::Expected<std::unique_ptr<OutputRenderer>, OutputStatus> CreateWithFragment(
	    device::IRenderDevice2 &device, device::Format targetFormat,
	    std::span<const std::uint32_t> fragmentSpirv );
	~OutputRenderer();

	OutputRenderer( const OutputRenderer & ) = delete;
	OutputRenderer &operator=( const OutputRenderer & ) = delete;

	OutputEncoding Encoding() const { return m_Encoding; }

	// Adds the output pass: it reads targets.scene (kSampled) and writes all of
	// targets.target (kColorAttachment). The renderer must outlive the
	// graph's execution.
	foundation::Expected<void, OutputStatus> AddPass(
	    graph::GraphBuilder &builder, const OutputTargets &targets, const OutputParams &params );

	// Releases the bind groups of executions up to `token` once it completes.
	void Collect( device::CompletionToken token );
	// Record-time failures (a bind group the device refused); 0 when every
	// recorded pass drew.
	std::uint32_t RecordFailures() const;

private:
	OutputRenderer( device::IRenderDevice2 &device ) : m_Device( device ) {}

	device::IRenderDevice2 &m_Device;
	device::Format m_TargetFormat = device::Format::kUnknown;
	OutputEncoding m_Encoding = OutputEncoding::kSrgb;
	device::BindGroupLayoutId m_Layout;
	device::SamplerId m_Sampler;
	device::PipelineId m_Pipeline;
	mutable std::mutex m_PendingLock;
	std::vector<device::BindGroupId> m_Pending;
	std::uint32_t m_RecordFailures = 0;
	device::CompletionToken m_LastToken;
};

} // namespace render::pass::output

#endif // RENDER_PASS_OUTPUT_OUTPUT_H
