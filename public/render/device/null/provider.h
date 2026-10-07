//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.null, the recording adapter of render.device.v2
//			(RFC 0016). It runs no GPU, but it models one: submitted work runs
//			when its token completes, buffers and textures hold real bytes, and
//			every executed command is recorded. Headless products compose it,
//			and the shared suites use it as their reference adapter.
//
//			Completion: kOnPoll completes everything submitted at the next
//			Poll(), then frees what was released behind it (products). kManual
//			completes only through INullDeviceControl (tests), so release and
//			reuse ordering can be observed.
//
//=============================================================================//

#ifndef RENDER_DEVICE_NULL_PROVIDER_H
#define RENDER_DEVICE_NULL_PROVIDER_H

#include "render/device/provider.h"

#include <cstdint>
#include <memory>
#include <span>

namespace render::device::null
{

enum class CompletionMode : std::uint8_t
{
	kOnPoll,
	kManual
};

struct NullOptions
{
	// Everything but external images: the null device has no memory another
	// API could import (clause D18).
	CapabilitySet capabilities = CapabilitySet::All().Remove( Capability::kExternalImages );
	ArtifactFormat artifactFormat = ArtifactFormat::kSpirv;
	CompletionMode completion = CompletionMode::kOnPoll;
	std::uint64_t uploadRingBytes = 1u << 20;
	// Sensitivity fixture only (render.device.v2.sensitivity): retire upload
	// ranges at submission instead of completion, the defect clause D10
	// must catch. Never set by a product.
	bool unsafeUploadReuse = false;
	// Keep every executed command for Recorded() (the port suites read it).
	// A product composes the null device without the log: it grows by every
	// command of every frame and is never read there.
	bool recordCommands = true;
};

enum class RecordedOp : std::uint8_t
{
	kTransitionTexture,
	kTransitionBuffer,
	kClearTexture,
	kWriteBuffer,
	kCopyBuffer,
	kCopyTextureToBuffer,
	kCopyBufferToTexture,
	kCopyTexture,
	kBeginRendering,
	kEndRendering,
	kSetPipeline,
	kSetBindGroup,
	kSetVertexBuffer,
	kSetIndexBuffer,
	kSetViewport,
	kDraw,
	kDrawIndexed,
	kDispatch,
	kBeginLabel,
	kEndLabel,
	kSetDrawConstants,        // count: the bytes written
	kWriteTimestamp,          // D23: count is the tick written
	kDrawIndexedIndirect,     // D30: count is the draw count
	kDrawIndexedIndirectCount // D31: count is the maximum draw count
};

// One executed command. resource is the command's main handle value;
// before and after are set for transitions; count is the vertex, index or
// group count, or the byte count of a write or copy.
struct RecordedCommand
{
	RecordedOp op = RecordedOp::kDraw;
	std::uint64_t resource = 0;
	ResourceUsage before = ResourceUsage::kUndefined;
	ResourceUsage after = ResourceUsage::kUndefined;
	std::uint64_t count = 0;
	std::uint64_t submission = 0; // token value of the submission that ran it

	friend constexpr bool operator==( const RecordedCommand &, const RecordedCommand & ) = default;
};

// Test and evidence hooks of a device made by this adapter.
class INullDeviceControl
{
public:
	virtual ~INullDeviceControl() = default;

	// Runs and completes submissions of queue up to value.
	virtual void CompleteThrough( QueueKind queue, std::uint64_t value ) = 0;
	virtual void CompleteAll() = 0;
	// The device reports kLost until Recover().
	virtual void LoseDevice() = 0;
	virtual std::span<const RecordedCommand> Recorded() const = 0;
	virtual void ClearRecorded() = 0;
	// Uploads that found the ring full and took a deferred block instead.
	virtual std::uint64_t DeferredUploads() const = 0;
};

const DeviceProviderDescriptor &Describe();
DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const NullOptions &options );
// nullptr unless device was made by this adapter.
INullDeviceControl *Control( IRenderDevice2 &device );

} // namespace render::device::null

#endif // RENDER_DEVICE_NULL_PROVIDER_H
