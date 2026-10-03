//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2, the backend-neutral device port (RFC 0016).
//			Portable render code records work through this interface only;
//			adapters (render.device.vulkan, render.device.gl, render.device.null)
//			implement it, and only render.composition and test fixtures name
//			an adapter (CAP011).
//
//			Obligations every adapter meets (contract:
//			unittests/rendertest/contracts/render.device.v2.md; shared suite
//			unittests/rendertest/core/device/device_conformance.h):
//
//			D1  Facts are immutable, report exactly kMaxBindGroups groups at
//			    most, and name the artifact format the device accepts.
//			D2  Invalid descriptions fail with kInvalidDescription, and a failed
//			    creation leaves no live resource behind.
//			D3  A layout list longer than kMaxBindGroups fails with
//			    kTooManyBindGroups.
//			D4  A pipeline whose artifacts reflect a binding its layouts lack
//			    fails with kLayoutMismatch; a foreign artifact format fails
//			    with kUnsupported.
//			D5  Release never frees before its token completes; Poll frees it
//			    after, and a released handle is never valid again.
//			D6  Submissions on one queue complete in submission order.
//			D7  After a device loss, Submit rejects waits on tokens of the old
//			    epoch with kStaleEpoch.
//			D8  Encoders run in Submit order; an encoder with a recorded error
//			    is rejected and none of that submission runs.
//			D9  Buffer writes, copies and texture clears land: bytes written
//			    and copied read back unchanged, and a cleared texture copies out
//			    as its clear color.
//			D10 Upload ranges are reused only after their token completes.
//
//=============================================================================//

#ifndef RENDER_DEVICE_DEVICE_H
#define RENDER_DEVICE_DEVICE_H

#include "foundation/expected.h"
#include "render/device/bind_group.h"
#include "render/device/completion.h"
#include "render/device/encoder.h"
#include "render/device/errors.h"
#include "render/device/external_images.h"
#include "render/device/facts.h"
#include "render/device/pipeline.h"
#include "render/device/resources.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace render::device
{

template <typename T> using DeviceResult = foundation::Expected<T, DeviceError>;

enum class DeviceState : std::uint8_t
{
	kAvailable,
	kLost, // every submission fails; Recover() starts a new epoch
	kRecovering,
	kFatal // recovery failed; the composition root tears the device down
};

class IRenderDevice2
{
public:
	virtual ~IRenderDevice2() = default;

	virtual const DeviceFacts &Facts() const = 0;
	virtual DeviceState State() const = 0;
	virtual std::uint32_t Epoch() const = 0;

	virtual DeviceResult<BufferId> CreateBuffer( const BufferDesc &desc ) = 0;
	virtual DeviceResult<TextureId> CreateTexture( const TextureDesc &desc ) = 0;
	virtual DeviceResult<SamplerId> CreateSampler( const SamplerDesc &desc ) = 0;
	virtual DeviceResult<BindGroupLayoutId> CreateBindGroupLayout(
	    const BindGroupLayoutDesc &desc ) = 0;
	virtual DeviceResult<BindGroupId> CreateBindGroup( const BindGroupDesc &desc ) = 0;
	virtual DeviceResult<PipelineId> CreatePipeline( const PipelineDesc &desc ) = 0;

	// Frees the resource once releaseAfter completes (at a later Poll).
	virtual DeviceResult<void> Release( ResourceId resource, CompletionToken releaseAfter ) = 0;

	virtual DeviceResult<CommandEncoder> BeginEncoder( QueueKind queue ) = 0;
	virtual DeviceResult<CompletionToken> Submit(
	    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits ) = 0;
	virtual bool IsComplete( CompletionToken token ) const = 0;
	// Frees released resources whose tokens completed; returns how many.
	virtual std::size_t Poll() = 0;

	// Reads a kReadback buffer. The caller has waited for the submission
	// that wrote it.
	virtual DeviceResult<void> ReadBuffer(
	    BufferId buffer, std::uint64_t offset, std::span<std::byte> out ) = 0;

	// Blocks until every submission completes. Reviewed callers only:
	// teardown, mode change and loss recovery (RFC 0016 K1), never a frame.
	virtual DeviceResult<void> WaitIdle() = 0;
	// After kLost: a new epoch with no live resources.
	virtual DeviceResult<void> Recover() = 0;

	virtual std::size_t LiveResourceCount() const = 0;

	// The exporter of textures to other APIs (clause D18): non-null exactly
	// when the facts claim kExternalImages. The device owns it.
	virtual IExternalImages *ExternalImages() { return nullptr; }

	// D25: creates an upload buffer initialized before this call returns, in
	// kCopySource usage. The source bytes may then be changed or destroyed.
	// Empty input fails. The buffer is immutable (copy-source usage only),
	// and Release must name its last GPU consumer's completion token.
	virtual DeviceResult<BufferId> CreateUploadBuffer( std::span<const std::byte> bytes ) = 0;
};

} // namespace render::device

#endif // RENDER_DEVICE_DEVICE_H
