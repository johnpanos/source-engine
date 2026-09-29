//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.frame.v1, the frame port (RFC 0016). The engine drives
//			frames through IRenderer; the client marks the legacy stages in
//			between (through RenderStageMarkers001, the legacy frontend's C++11
//			face of MarkStage). EndFrame builds the frame's render graph from
//			the renderer's features, executes it on the device and reports
//			what ran.
//
//			Engine-facing: no dual-ABI library type (std::string, std::list)
//			appears here or in what it includes.
//
//=============================================================================//

#ifndef RENDER_FRAME_RENDERER_H
#define RENDER_FRAME_RENDERER_H

#include "foundation/expected.h"
#include "render/device/completion.h"
#include "render/device/errors.h"
#include "render/device/resources.h"
#include "render/device/usage.h"
#include "render/frame/debug_controls.h"
#include "render/frame/stage_hooks.h"
#include "render/frame/stages.h"

#include <cstdint>

namespace render::frame
{

struct FrameDesc
{
	std::uint64_t frame = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	// An imported target (a swapchain image). Invalid: the renderer uses a
	// transient target of width x height.
	device::TextureId target;
	device::TextureDesc targetDesc;
	device::ResourceUsage targetUsage = device::ResourceUsage::kUndefined;
	device::ResourceUsage targetFinal = device::ResourceUsage::kUndefined;
	// The frame's debug controls (RFC 0014). The renderer validates them at
	// BeginFrame; an invalid value is refused and the previous one kept, so
	// features always see the applied controls.
	DebugControls debug;
};

// One frame's result, and the renderer's running totals.
struct FrameStats
{
	std::uint64_t frames = 0;
	std::uint64_t failedFrames = 0;
	std::uint64_t stagesMarked = 0;
	std::uint64_t orderViolations = 0;
	std::uint64_t views = 0;
	std::uint64_t passes = 0;
	std::uint64_t culledPasses = 0;
	std::uint64_t transitions = 0;
	std::uint64_t transientsCreated = 0; // physical transients the frame created
	std::uint64_t transientsReused = 0;  // taken from the renderer's pool instead
	std::uint64_t debugRejected = 0;     // frames whose debug controls were refused
	device::CompletionToken lastToken;
};

enum class FrameStatus : std::uint8_t
{
	kNotInFrame = 1,
	kAlreadyInFrame,
	kInvalidFrame,
	kGraph, // the frame's graph did not compile
	kDevice // the device refused the frame
};

struct FrameError
{
	FrameStatus status = FrameStatus::kInvalidFrame;
	device::DeviceError device;
	std::uint32_t graphStatus = 0;
};

class IRenderer
{
public:
	virtual ~IRenderer() = default;

	virtual foundation::Expected<void, FrameError> BeginFrame( const FrameDesc &desc ) = 0;
	// False when the stage breaks the order rules (stages.h) or no frame is
	// open; the frame continues.
	virtual bool MarkStage( Stage stage ) = 0;
	virtual foundation::Expected<FrameStats, FrameError> EndFrame() = 0;
	virtual const FrameStats &Totals() const = 0;
	// The debug controls in effect (the last valid FrameDesc::debug), and why
	// the last refused one was refused (status 0 before any refusal).
	virtual const DebugControls &AppliedDebug() const = 0;
	virtual const DebugControlsError &LastDebugRejection() const = 0;
	// The programs cl_render_debug_view_program may name.
	virtual std::size_t DebugProgramCount() const = 0;
	virtual const char *DebugProgramName( std::size_t index ) const = 0;
	// cl_render_debug_term's names as term bits (frame::ParseDebugTerms).
	virtual bool ParseDebugTerms(
	    const char *names, std::uint32_t *bits, char *unknown, std::size_t unknownBytes ) const = 0;

	// Hooks outlive their registration.
	virtual void AddStageHooks( IRenderStageHooks *hooks ) = 0;
	virtual void RemoveStageHooks( IRenderStageHooks *hooks ) = 0;
};

} // namespace render::frame

#endif // RENDER_FRAME_RENDERER_H
