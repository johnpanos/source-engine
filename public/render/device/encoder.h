//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 command encoders (RFC 0016).
//
//			- One encoder is recorded by one thread at a time. The encoder binds
//			  to the first thread that records it; a call from another thread is
//			  counted in SequenceViolations() (the diagnostic every adapter gets,
//			  since the check lives here, in the port).
//			- Misuse (a draw outside rendering, an unknown handle, a transition
//			  from a usage the resource is not in) is recorded, not thrown, and
//			  Submit rejects the encoder with kInvalidState.
//			- Transitions name abstract usages. The adapter turns each into its
//			  API's barriers, layout transitions or nothing.
//			- Encoders are submitted in the order given to Submit. A submitted
//			  or destroyed encoder records nothing more.
//
//=============================================================================//

#ifndef RENDER_DEVICE_ENCODER_H
#define RENDER_DEVICE_ENCODER_H

#include "render/device/bind_group.h"
#include "render/device/completion.h"
#include "render/device/resources.h"
#include "render/device/usage.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <thread>

namespace render::device
{

enum class LoadOp : std::uint8_t
{
	kLoad,
	kClear,
	kDiscard
};

enum class StoreOp : std::uint8_t
{
	kStore,
	kDiscard
};

struct ClearColor
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;
};

struct ColorAttachment
{
	TextureId texture;
	LoadOp load = LoadOp::kClear;
	StoreOp store = StoreOp::kStore;
	ClearColor clear;
	TextureId resolve; // optional multisample resolve target
};

struct DepthAttachment
{
	TextureId texture;
	LoadOp load = LoadOp::kClear;
	StoreOp store = StoreOp::kStore;
	float clearDepth = 1.0f;
};

struct RenderingDesc
{
	std::span<const ColorAttachment> colors;
	std::optional<DepthAttachment> depth;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

struct BufferCopy
{
	std::uint64_t sourceOffset = 0;
	std::uint64_t destinationOffset = 0;
	std::uint64_t size = 0;
};

// A tightly packed image region of one mip and layer, at texel (x, y) (a
// block-compressed region starts on a block).
struct TextureBufferCopy
{
	std::uint64_t bufferOffset = 0;
	std::uint32_t mip = 0;
	std::uint32_t layer = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t x = 0;
	std::uint32_t y = 0;
};

// D37: one mip/layer region copied between two single-sample textures of the
// same format, at the same texel (x, y) of both (a block-compressed region
// starts on a block). The source is in kCopySource, the destination in
// kCopyDestination; no buffer is involved.
struct TextureCopy
{
	std::uint32_t mip = 0;
	std::uint32_t layer = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t x = 0;
	std::uint32_t y = 0;
};

enum class IndexFormat : std::uint8_t
{
	kUint16,
	kUint32
};

struct Viewport
{
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	float minDepth = 0.0f;
	float maxDepth = 1.0f;
};

// One indexed draw's parameters in an indirect buffer (clauses D30, D31):
// the layout of VkDrawIndexedIndirectCommand and GL's
// DrawElementsIndirectCommand, tightly packed.
struct DrawIndexedIndirectCommand
{
	std::uint32_t indexCount = 0;
	std::uint32_t instanceCount = 0;
	std::uint32_t firstIndex = 0;
	std::int32_t vertexOffset = 0;
	std::uint32_t firstInstance = 0;
};
static_assert( sizeof( DrawIndexedIndirectCommand ) == 20 );

// The argument rules of D30/D31 that do not depend on device state: offset
// and stride multiples of 4, stride at least 20, and `drawCount` records
// from `offset` inside a buffer of `bufferSize` bytes. Every adapter's
// validation uses this one rule.
bool IndirectRecordsFit(
    std::uint64_t bufferSize, std::uint64_t offset, std::uint32_t drawCount, std::uint32_t stride );

// What an adapter records. Only the port's CommandEncoder calls it.
class IEncoderBackend
{
public:
	virtual ~IEncoderBackend() = default;

	virtual void TransitionTexture( TextureId texture, ResourceUsage before, ResourceUsage after,
	    const SubresourceRange &range ) = 0;
	virtual void TransitionBuffer( BufferId buffer, ResourceUsage before, ResourceUsage after ) = 0;
	virtual void ClearTexture(
	    TextureId texture, const ClearColor &color, const SubresourceRange &range ) = 0;
	// Through the adapter's upload ring; the destination is in kCopyDestination.
	virtual void WriteBuffer(
	    BufferId buffer, std::uint64_t offset, std::span<const std::byte> bytes ) = 0;
	virtual void CopyBuffer( BufferId source, BufferId destination, const BufferCopy &copy ) = 0;
	virtual void CopyTextureToBuffer(
	    TextureId source, BufferId destination, const TextureBufferCopy &copy ) = 0;
	virtual void CopyBufferToTexture(
	    BufferId source, TextureId destination, const TextureBufferCopy &copy ) = 0;
	virtual void CopyTexture(
	    TextureId source, TextureId destination, const TextureCopy &copy ) = 0;
	virtual void BeginRendering( const RenderingDesc &desc ) = 0;
	virtual void EndRendering() = 0;
	virtual void SetPipeline( PipelineId pipeline ) = 0;
	virtual void SetBindGroup( BindGroupRole role, BindGroupId group ) = 0;
	virtual void SetVertexBuffer( std::uint32_t slot, BufferId buffer, std::uint64_t offset ) = 0;
	virtual void SetIndexBuffer( BufferId buffer, std::uint64_t offset, IndexFormat format ) = 0;
	virtual void SetViewport( const Viewport &viewport ) = 0;
	virtual void Draw( std::uint32_t vertexCount, std::uint32_t instanceCount,
	    std::uint32_t firstVertex, std::uint32_t firstInstance ) = 0;
	virtual void DrawIndexed( std::uint32_t indexCount, std::uint32_t instanceCount,
	    std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance ) = 0;
	virtual void DrawIndexedIndirect(
	    BufferId buffer, std::uint64_t offset, std::uint32_t drawCount, std::uint32_t stride ) = 0;
	virtual void DrawIndexedIndirectCount( BufferId buffer, std::uint64_t offset,
	    BufferId countBuffer, std::uint64_t countOffset, std::uint32_t maxDrawCount,
	    std::uint32_t stride ) = 0;
	virtual void Dispatch( std::uint32_t x, std::uint32_t y, std::uint32_t z ) = 0;
	// D16: bytes of the bound pipeline's draw-constant block, at `offset`.
	virtual void SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes ) = 0;
	virtual void BeginLabel( std::string_view label ) = 0;
	virtual void EndLabel() = 0;
	virtual void WriteTimestamp( BufferId buffer, std::uint64_t offset ) = 0;

	// True once any recorded call was invalid; Submit then fails.
	virtual bool HasError() const = 0;
};

class CommandEncoder;

// Sees an encoder's labels as they record, on the recording thread: RFC 0014
// D4's GPU timers write a timestamp after each label opens and before it
// closes. It may record into the encoder (not labels).
class ILabelObserver
{
public:
	virtual void OnBeginLabel( CommandEncoder &encoder, std::string_view label ) = 0;
	virtual void OnEndLabel( CommandEncoder &encoder ) = 0;

protected:
	~ILabelObserver() = default;
};

class CommandEncoder
{
public:
	CommandEncoder() = default;
	CommandEncoder( QueueKind queue, std::unique_ptr<IEncoderBackend> backend );
	CommandEncoder( CommandEncoder && ) noexcept;
	CommandEncoder &operator=( CommandEncoder && ) noexcept;
	CommandEncoder( const CommandEncoder & ) = delete;
	CommandEncoder &operator=( const CommandEncoder & ) = delete;
	~CommandEncoder();

	bool IsOpen() const { return m_Backend != nullptr; }
	QueueKind Queue() const { return m_Queue; }
	// Calls made from a thread other than the one that first recorded.
	std::uint32_t SequenceViolations() const { return m_Violations; }

	void TransitionTexture( TextureId texture, ResourceUsage before, ResourceUsage after,
	    const SubresourceRange &range = {} );
	void TransitionBuffer( BufferId buffer, ResourceUsage before, ResourceUsage after );
	void ClearTexture(
	    TextureId texture, const ClearColor &color, const SubresourceRange &range = {} );
	void WriteBuffer( BufferId buffer, std::uint64_t offset, std::span<const std::byte> bytes );
	void CopyBuffer( BufferId source, BufferId destination, const BufferCopy &copy );
	void CopyTextureToBuffer(
	    TextureId source, BufferId destination, const TextureBufferCopy &copy );
	void CopyBufferToTexture(
	    BufferId source, TextureId destination, const TextureBufferCopy &copy );
	void CopyTexture( TextureId source, TextureId destination, const TextureCopy &copy );
	void BeginRendering( const RenderingDesc &desc );
	void EndRendering();
	void SetPipeline( PipelineId pipeline );
	void SetBindGroup( BindGroupRole role, BindGroupId group );
	void SetVertexBuffer( std::uint32_t slot, BufferId buffer, std::uint64_t offset = 0 );
	void SetIndexBuffer( BufferId buffer, std::uint64_t offset, IndexFormat format );
	void SetViewport( const Viewport &viewport );
	void Draw( std::uint32_t vertexCount, std::uint32_t instanceCount = 1,
	    std::uint32_t firstVertex = 0, std::uint32_t firstInstance = 0 );
	void DrawIndexed( std::uint32_t indexCount, std::uint32_t instanceCount = 1,
	    std::uint32_t firstIndex = 0, std::int32_t vertexOffset = 0,
	    std::uint32_t firstInstance = 0 );
	// D30 (Capability::kMultiDrawIndirect): `drawCount` indexed draws whose
	// DrawIndexedIndirectCommand records are read by the GPU from `buffer`,
	// the first at `offset` and each next `stride` bytes on. Everything an
	// indexed draw needs is bound as for DrawIndexed. The buffer is in
	// kIndirect; offset and stride are multiples of 4, the stride at least
	// 20, and every record lies inside the buffer. A draw count of zero
	// draws nothing. Without the capability the submission fails
	// (kUnsupported); an invalid call fails it (kInvalidState). A record's
	// firstInstance must be 0 unless the device claims
	// Capability::kIndirectFirstInstance, which makes it offset the instance
	// index and instance-rate attributes as DrawIndexed's does (the GPU reads
	// it, so the port cannot check it).
	void DrawIndexedIndirect(
	    BufferId buffer, std::uint64_t offset, std::uint32_t drawCount, std::uint32_t stride );
	// D31 (Capability::kDrawIndirectCount): as D30, with the draw count read
	// by the GPU as a 32-bit value at `countOffset` of `countBuffer` (also in
	// kIndirect; it may be `buffer`), clamped to `maxDrawCount`, whose
	// records must all lie inside the buffer. Without the capability the
	// submission fails (kUnsupported).
	void DrawIndexedIndirectCount( BufferId buffer, std::uint64_t offset, BufferId countBuffer,
	    std::uint64_t countOffset, std::uint32_t maxDrawCount, std::uint32_t stride );
	void Dispatch( std::uint32_t x, std::uint32_t y = 1, std::uint32_t z = 1 );
	// D16: writes the bound pipeline's draw constants. Binding a pipeline
	// leaves them undefined, so they are set after SetPipeline; a draw or
	// dispatch with any word of the block unset since then fails the
	// submission (kInvalidState), as does a write outside the block.
	void SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes );
	void BeginLabel( std::string_view label );
	void EndLabel();
	// D23 (Capability::kTimestamps): the GPU's time when the commands recorded
	// before it have run, in ticks (DeviceFacts::timestampPeriodNs), as a
	// 64-bit value at `offset` of `buffer` once the submission completes.
	// Allowed inside rendering. The buffer is kReadback memory in
	// kCopyDestination here and at the end of the submission; the offset is
	// a multiple of 8. Timestamps of one queue do not decrease in submission
	// order. Without the capability the submission fails (kUnsupported).
	void WriteTimestamp( BufferId buffer, std::uint64_t offset );

	// The observer of this encoder's labels (null: none); not owned.
	void SetLabelObserver( ILabelObserver *observer ) { m_LabelObserver = observer; }
	ILabelObserver *LabelObserver() const { return m_LabelObserver; }

	// For adapters: takes the recorded backend at submission and closes the
	// encoder. Returns nullptr for a closed encoder.
	std::unique_ptr<IEncoderBackend> TakeBackend();
	// For adapters: the backend while the encoder is open, else nullptr.
	IEncoderBackend *Backend() const { return m_Backend.get(); }

private:
	IEncoderBackend *Enter();

	QueueKind m_Queue = QueueKind::kGraphics;
	std::unique_ptr<IEncoderBackend> m_Backend;
	std::thread::id m_Owner;
	std::uint32_t m_Violations = 0;
	ILabelObserver *m_LabelObserver = nullptr;
};

} // namespace render::device

#endif // RENDER_DEVICE_ENCODER_H
