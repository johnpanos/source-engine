//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Recorded command lists for adapters that replay them (RFC 0025
//			decision 9): render.device.gl records every encoder as a CPU command list,
//			validate it at Submit against the usage state the previous
//			accepted submission left, and replay it on their API. This header
//			owns the parts they share: the command list, the recording
//			encoder, the upload ring and the validation rules. Each adapter
//			keeps its own replay and answers validation's questions about its
//			resources through IRecordedResources.
//
//			Adapter facing only: portable modules record through
//			CommandEncoder (encoder.h) and never see a Command.
//
//=============================================================================//

#ifndef RENDER_DEVICE_RECORDING_H
#define RENDER_DEVICE_RECORDING_H

#include "render/device/encoder.h"
#include "render/device/errors.h"
#include "render/device/facts.h"
#include "render/device/pipeline.h"
#include "render/device/resources.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

namespace render::device::recording
{

enum class Op : std::uint8_t
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
	kSetDrawConstants,
	kBeginLabel,
	kEndLabel,
	kWriteTimestamp,      // D23: buffer a, at offset
	kBeginOcclusionQuery, // D43: buffer a, at offset
	kEndOcclusionQuery,
	// D30/D31: records in buffer a at offset, count = draw count (maximum),
	// first = stride; the count's buffer b at copy.destinationOffset.
	kDrawIndexedIndirect,
	kDrawIndexedIndirectCount,
	kClearRegion // D44: region
};

struct Command
{
	Op op = Op::kDraw;
	std::uint64_t a = 0; // main handle
	std::uint64_t b = 0; // second handle
	ResourceUsage before = ResourceUsage::kUndefined;
	ResourceUsage after = ResourceUsage::kUndefined;
	SubresourceRange range;
	ClearColor color;
	BufferCopy copy;
	TextureBufferCopy textureCopy; // D37's TextureCopy too (bufferOffset 0)
	std::uint64_t offset = 0; // a vertex, index or draw-constant offset
	std::uint32_t slot = 0;   // a vertex slot or bind-group role
	std::uint32_t count = 0;  // vertices, indices or x groups
	std::uint32_t instances = 1;
	std::uint32_t first = 0; // first vertex or index; y groups of a dispatch
	std::int32_t vertexOffset = 0;
	std::uint32_t firstInstance = 0; // z groups of a dispatch
	IndexFormat indexFormat = IndexFormat::kUint16;
	Viewport viewport;
	ClearRegion region; // D44
	std::vector<ColorAttachment> colors;
	std::optional<DepthAttachment> depth;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	// An upload: in the adapter's ring at ringOffset, else in bytes (a full
	// ring); the bytes of draw constants; a label's text.
	std::uint64_t ringOffset = 0;
	bool fromRing = false;
	std::vector<std::byte> bytes;
};

class RecordingEncoder;

// Where an adapter stages WriteBuffer uploads, from any recording thread.
class IUploadStager
{
public:
	// Copies bytes into the adapter's ring (setting ringOffset and fromRing,
	// and calling AddRingAllocation) or, when the ring is full, into
	// command.bytes.
	virtual void StageUpload(
	    RecordingEncoder &encoder, Command &command, std::span<const std::byte> bytes ) = 0;
	// The ring ranges of an encoder destroyed without being submitted.
	virtual void AbandonUploads( const std::vector<std::uint64_t> &allocations ) = 0;

protected:
	~IUploadStager() = default;
};

// The encoder backend of every list-replaying adapter. `owner` identifies the
// device that made it, so Submit can refuse another device's encoders.
class RecordingEncoder final : public IEncoderBackend
{
public:
	RecordingEncoder( IUploadStager &stager, const void *owner, QueueKind queue )
	    : m_Stager( stager ), m_Owner( owner ), m_Queue( queue )
	{
	}
	~RecordingEncoder() override;

	void TransitionTexture( TextureId texture, ResourceUsage before, ResourceUsage after,
	    const SubresourceRange &range ) override;
	void TransitionBuffer( BufferId buffer, ResourceUsage before, ResourceUsage after ) override;
	void ClearTexture(
	    TextureId texture, const ClearColor &color, const SubresourceRange &range ) override;
	void WriteBuffer(
	    BufferId buffer, std::uint64_t offset, std::span<const std::byte> bytes ) override;
	void CopyBuffer( BufferId source, BufferId destination, const BufferCopy &copy ) override;
	void CopyTextureToBuffer(
	    TextureId source, BufferId destination, const TextureBufferCopy &copy ) override;
	void CopyBufferToTexture(
	    BufferId source, TextureId destination, const TextureBufferCopy &copy ) override;
	void CopyTexture( TextureId source, TextureId destination, const TextureCopy &copy ) override;
	void BeginRendering( const RenderingDesc &desc ) override;
	void EndRendering() override;
	void ClearRegion( const render::device::ClearRegion &region ) override;
	void SetPipeline( PipelineId pipeline ) override;
	void SetBindGroup( BindGroupRole role, BindGroupId group ) override;
	void SetVertexBuffer( std::uint32_t slot, BufferId buffer, std::uint64_t offset ) override;
	void SetIndexBuffer( BufferId buffer, std::uint64_t offset, IndexFormat format ) override;
	void SetViewport( const Viewport &viewport ) override;
	void DrawIndexedIndirect( BufferId buffer, std::uint64_t offset, std::uint32_t drawCount,
	    std::uint32_t stride ) override;
	void DrawIndexedIndirectCount( BufferId buffer, std::uint64_t offset, BufferId countBuffer,
	    std::uint64_t countOffset, std::uint32_t maxDrawCount, std::uint32_t stride ) override;
	void Draw( std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t firstVertex,
	    std::uint32_t firstInstance ) override;
	void DrawIndexed( std::uint32_t indexCount, std::uint32_t instanceCount,
	    std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance ) override;
	void Dispatch( std::uint32_t x, std::uint32_t y, std::uint32_t z ) override;
	void SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes ) override;
	void BeginLabel( std::string_view label ) override;
	void EndLabel() override;
	void WriteTimestamp( BufferId buffer, std::uint64_t offset ) override;
	void BeginOcclusionQuery( BufferId buffer, std::uint64_t offset ) override;
	void EndOcclusionQuery() override;
	bool HasError() const override { return m_Error; }

	// Recorded without error, outside rendering, every label closed.
	bool Complete() const { return !m_Error && !m_Rendering && m_Labels == 0; }
	const void *Owner() const { return m_Owner; }
	QueueKind Queue() const { return m_Queue; }
	const std::vector<Command> &Commands() const { return m_Commands; }
	const std::vector<std::uint64_t> &RingAllocations() const { return m_RingAllocations; }
	void AddRingAllocation( std::uint64_t id ) { m_RingAllocations.push_back( id ); }
	// Its ring ranges now follow the submission's token, not the encoder.
	void MarkSubmitted() { m_Submitted = true; }

private:
	void Push( Command command ) { m_Commands.push_back( std::move( command ) ); }
	void NotRendering()
	{
		if ( m_Rendering )
			m_Error = true;
	}
	void Drawing()
	{
		if ( !m_Rendering )
			m_Error = true;
	}

	IUploadStager &m_Stager;
	const void *m_Owner;
	QueueKind m_Queue;
	std::vector<Command> m_Commands;
	std::vector<std::uint64_t> m_RingAllocations;
	bool m_Rendering = false;
	bool m_Error = false;
	bool m_Submitted = false;
	std::uint32_t m_Labels = 0;
};

// An upload ring's bookkeeping (the adapter owns the memory). Ranges retire
// in order once their token completes, or when their encoder is destroyed
// without submitting. Not thread-safe: the adapter locks it.
class UploadRing
{
public:
	void Reset( std::uint64_t capacity )
	{
		m_Capacity = capacity;
		m_Live.clear();
		m_Head = 0;
	}
	// A 16-byte aligned offset of `size` bytes, or nullopt when full.
	std::optional<std::uint64_t> Allocate( std::uint64_t size, std::uint64_t id );
	void Submit( std::uint64_t id, CompletionToken token );
	void Abandon( std::uint64_t id );
	// Retires ranges whose tokens have values up to completed (one queue,
	// the current epoch), and every range of an earlier epoch.
	void Retire( std::uint32_t epoch, std::uint64_t completed );

private:
	struct Allocation
	{
		std::uint64_t offset;
		std::uint64_t size;
		std::uint64_t id;
		CompletionToken token;
		bool submitted;
		bool abandoned;
	};
	std::uint64_t m_Capacity = 0;
	std::deque<Allocation> m_Live;
	std::uint64_t m_Head = 0;
};

// What validation needs to know of a resource. Views are copies taken for
// one Validate call; the device's lock is held throughout.
struct TextureView
{
	TextureDesc desc;
	std::uint32_t layers = 1; // array layers (1 for a 3D texture)
	ResourceUsage usage = ResourceUsage::kUndefined;

	std::uint32_t Width( std::uint32_t mip ) const;
	std::uint32_t Height( std::uint32_t mip ) const;
};

struct BufferView
{
	BufferDesc desc;
	ResourceUsage usage = ResourceUsage::kUndefined;
};

struct PipelineView
{
	PipelineKind kind = PipelineKind::kGraphics;
	std::span<const Format> colorFormats; // the adapter's record outlives the call
	Format depthFormat = Format::kUnknown;
	std::uint32_t sampleCount = 1;
	std::uint32_t vertexBuffers = 0;
	std::uint32_t drawConstantBytes = 0;
	std::array<BindGroupLayoutId, kMaxBindGroups> layouts{};
	std::array<bool, kMaxBindGroups> layoutHasBindings{};
};

class IRecordedResources
{
public:
	// Live (not released) resources only; nullopt otherwise.
	virtual std::optional<TextureView> Texture( std::uint64_t id ) const = 0;
	virtual std::optional<BufferView> Buffer( std::uint64_t id ) const = 0;
	virtual std::optional<PipelineView> Pipeline( std::uint64_t id ) const = 0;
	// The layout of a live bind group whose buffers, textures and samplers
	// are all live (a group naming a released resource would read freed
	// memory); nullopt otherwise.
	virtual std::optional<BindGroupLayoutId> BindGroup( std::uint64_t id ) const = 0;
	virtual std::uint32_t MaxColorAttachments() const = 0;
	virtual std::uint32_t MaxVertexSlots() const = 0;
	// Adapter limits on top of the port's rules, each named in the adapter.
	virtual bool CanClear( const TextureView & ) const { return true; }
	virtual bool CanCopyWithBuffer( const TextureView & ) const { return true; }
	virtual bool CanCopyTexture( const TextureView & ) const { return true; }

protected:
	~IRecordedResources() = default;
};

// One encoder's list against the port's rules. states holds each touched
// resource's usage after the encoders already validated in this submission
// (absent: the resource's committed usage), and is updated; on success the
// adapter commits it once the whole submission is accepted.
bool Validate( const std::vector<Command> &commands,
    std::unordered_map<std::uint64_t, ResourceUsage> &states, const IRecordedResources &resources );

// The submission-wide rules after every list validated: indirect draws need
// their capabilities (D30/D31), timestamps need kTimestamps (D23) and
// occlusion queries kOcclusionQueries (D43), and both end the submission
// with their buffer in kCopyDestination. kUnsupported or
// kInvalidState, or nullopt when the submission may run.
std::optional<DeviceStatus> CheckSubmission( std::span<const RecordingEncoder *const> encoders,
    CapabilitySet capabilities, const std::unordered_map<std::uint64_t, ResourceUsage> &states );

} // namespace render::device::recording

#endif // RENDER_DEVICE_RECORDING_H
