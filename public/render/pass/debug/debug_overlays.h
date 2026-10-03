//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.debug (RFC 0014): what the debug controls draw that
//			no program draws. Today: the not-applicable hatch over a whole
//			target, recorded at the start of a frame under a pixel view (or
//			cl_render_debug_legacy 2), so every pixel the core does not draw
//			shows it (RFC 0014 "Legacy stream passes under a view").
//
//			And a flat color blended over a whole target: cl_render_debug_legacy
//			1's magenta, which the core then draws its own surfaces over again
//			(render.composition), so what stays tinted is what the core does
//			not draw.
//
//			Render sequence. Pipelines are made on first use, one per target
//			format and encoding, and released by ReleaseDevice or with the
//			overlays.
//
//=============================================================================//

#ifndef RENDER_PASS_DEBUG_DEBUG_OVERLAYS_H
#define RENDER_PASS_DEBUG_DEBUG_OVERLAYS_H

#include "render/device/device.h"

#include <cstdint>
#include <map>
#include <span>
#include <tuple>
#include <string_view>

namespace render::pass::debug
{

struct HatchTarget
{
	device::TextureId color; // home kColorAttachment, left there
	device::Format format = device::Format::kUnknown;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t samples = 1;
	// The target has no sRGB view: the program encodes the hatch itself.
	bool encodeOutput = false;
};

// Inclusive labeled sections, measured on the CPU recording sequence and GPU.
// CPU and GPU overlap; neither columns nor nested rows are additive.
struct CostRow
{
	std::string_view name;
	std::uint32_t depth = 0;
	double cpuMilliseconds = 0;
	double gpuMilliseconds = 0;
};

struct CostOverlay
{
	std::span<const CostRow> rows;
	std::uint64_t frame = 0;
	std::uint64_t currentFrame = 0;
	std::uint32_t dropped = 0;
	bool supported = true;
};

class DebugOverlays
{
public:
	DebugOverlays() = default;
	~DebugOverlays();
	DebugOverlays( const DebugOverlays & ) = delete;
	DebugOverlays &operator=( const DebugOverlays & ) = delete;

	// Records the hatch over the whole target, outside rendering. False when
	// the pipeline is refused (nothing is recorded).
	bool RecordHatch( device::IRenderDevice2 &device, device::CommandEncoder &encoder,
	    const HatchTarget &target );
	// Records `rgba` (linear color, a the blend weight) blended over the
	// whole target, outside rendering. False when the pipeline is refused.
	bool RecordTint( device::IRenderDevice2 &device, device::CommandEncoder &encoder,
	    const HatchTarget &target, const float rgba[4] );
	// Draw a bounded, labeled CPU/GPU cost panel over the existing image.
	// No resource uploads/readbacks; the caller supplies a completed sample.
	bool RecordCosts( device::IRenderDevice2 &device, device::CommandEncoder &encoder,
	    const HatchTarget &target, const CostOverlay &costs );
	// The device is about to go: its pipelines are released now.
	void ReleaseDevice( device::IRenderDevice2 &device );

	std::uint64_t HatchesRecorded() const { return m_Hatches; }

private:
	device::IRenderDevice2 *m_Device = nullptr;
	// By (program: 0 hatch, 1 tint, 2 costs; format, samples, encode).
	std::map<std::tuple<int, device::Format, std::uint32_t, bool>, device::PipelineId> m_Pipelines;
	std::uint64_t m_Tints = 0;

	device::PipelineId PipelineFor(
	    device::IRenderDevice2 &device, int program, const HatchTarget &target );
	void RecordFullTarget( device::CommandEncoder &encoder, const HatchTarget &target,
	    device::PipelineId pipeline, std::span<const std::byte> constants, const char *label );
	std::uint64_t m_Hatches = 0;
};

} // namespace render::pass::debug

#endif // RENDER_PASS_DEBUG_DEBUG_OVERLAYS_H
