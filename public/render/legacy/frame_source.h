//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The legacy frame in the core's frame graph (RFC 0016 K3).
//
//			A legacy backend (the native Vulkan shader API) no longer records
//			and submits its frame itself. At present it hands the frontend's
//			frame executor an ILegacyFrameSource, and the executor runs the
//			frame as a render.graph execution on the source's device. The
//			source prepares the frame (acquires its target), records each of
//			its stages into a graph pass of its own (LegacyFrameStage, in
//			order), and finishes it with the submission's token (present,
//			readback). The stages share the execution's encoder, so they
//			reach the device as one submission in stage order.
//
//			ABI-facing: the backend builds with the engine's standard-library
//			ABI and the frontend with the core's, so only interfaces and port
//			types without library strings or containers cross here.
//
//=============================================================================//

#ifndef RENDER_LEGACY_FRAME_SOURCE_H
#define RENDER_LEGACY_FRAME_SOURCE_H

#include "render/device/completion.h"
#include "render/device/device.h"
#include "render/device/encoder.h"

namespace render::legacy
{

// A legacy frame's stages, in recording order; each is a pass of the frame
// graph (named "legacy-" plus LegacyFrameStageName).
enum class LegacyFrameStage : unsigned int
{
	kComputeAndUploads = 0, // compute queued since the last frame, deferred uploads
	kScene,                 // the recorded legacy stream into the back buffer
	kResolve,               // the multisampled back buffer resolved
	kCapture,               // the back buffer copied for a requested readback
	kPresent,               // the present blit or gamma pass into the swapchain image
	kCount
};

inline const char *LegacyFrameStageName( LegacyFrameStage stage )
{
	switch ( stage )
	{
	case LegacyFrameStage::kComputeAndUploads:
		return "compute-and-uploads";
	case LegacyFrameStage::kScene:
		return "scene";
	case LegacyFrameStage::kResolve:
		return "resolve";
	case LegacyFrameStage::kCapture:
		return "capture";
	case LegacyFrameStage::kPresent:
		return "present";
	case LegacyFrameStage::kCount:
		break;
	}
	return "?";
}

class ILegacyFrameSource
{
public:
	// The device the frame runs on (the backend's host device's port).
	virtual device::IRenderDevice2 &Device() = 0;
	// Acquires the frame's target. False fails the frame; *skip means there
	// is nothing to render this time (minimized, surface lost), not a failure.
	virtual bool Prepare( bool *skip ) = 0;
	// Whether the prepared frame has this stage (resolve only when
	// multisampled, capture only when a readback of the back buffer was
	// requested); the others always run. Valid after Prepare.
	virtual bool HasStage( LegacyFrameStage stage ) = 0;
	// Records one stage into its pass's encoder, in LegacyFrameStage order.
	virtual bool RecordStage( LegacyFrameStage stage, device::CommandEncoder &encoder ) = 0;
	// After the submission (`submitted`, with its token) or its failure.
	virtual bool Finish( const device::CompletionToken &token, bool submitted ) = 0;

protected:
	~ILegacyFrameSource() = default;
};

class ILegacyFrameExecutor
{
public:
	// Runs one frame: Prepare, the graph with one pass per stage, Finish.
	virtual bool RunFrame( ILegacyFrameSource &source ) = 0;
	// Frames run so far, and the last frame's graph passes (evidence).
	virtual unsigned long long Frames() const = 0;
	virtual unsigned int LastFramePasses() const = 0;

protected:
	~ILegacyFrameExecutor() = default;
};

// The frontend's executor (process lifetime).
ILegacyFrameExecutor &LegacyFrameExecutor();

} // namespace render::legacy

// The composition root binds the executor into the native Vulkan backend
// (next to NativeVulkanShaderBackend_BindDeviceFactory). Without one the
// backend runs each frame on an encoder of its own.
extern "C" void NativeVulkanShaderBackend_BindFrameExecutor(
    render::legacy::ILegacyFrameExecutor *executor );

#endif // RENDER_LEGACY_FRAME_SOURCE_H
