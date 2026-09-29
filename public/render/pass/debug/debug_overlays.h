//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.debug (RFC 0014): what the debug controls draw that
//			no program draws. Today: the not-applicable hatch over a whole
//			target, recorded at the start of a frame under a pixel view (or
//			cl_render_debug_legacy 2), so every pixel the core does not draw
//			shows it (RFC 0014 "Legacy stream passes under a view").
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
#include <tuple>

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
	// The device is about to go: its pipelines are released now.
	void ReleaseDevice( device::IRenderDevice2 &device );

	std::uint64_t HatchesRecorded() const { return m_Hatches; }

private:
	device::IRenderDevice2 *m_Device = nullptr;
	// By (format, samples, encode).
	std::map<std::tuple<device::Format, std::uint32_t, bool>, device::PipelineId> m_Pipelines;
	std::uint64_t m_Hatches = 0;
};

} // namespace render::pass::debug

#endif // RENDER_PASS_DEBUG_DEBUG_OVERLAYS_H
