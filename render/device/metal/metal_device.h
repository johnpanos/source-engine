//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.metal private: the Metal adapter of render.device.v2
//			(RFC 0025). See public/render/device/metal/provider.h. Every
//			translation unit including this header is Objective-C++ with ARC.
//
//			Model:
//			- Encoders record CPU command lists (any thread; uploads are copied
//			  into the adapter's shared-storage upload ring as they are
//			  recorded).
//			- Submit validates every encoder against the usage state the
//			  previous accepted submission left, then replays the lists in
//			  order into one MTLCommandBuffer and commits it: its token. The
//			  queue's command buffers complete in order; a completion handler
//			  advances the completed value.
//			- Usage transitions are no work: resources use Metal's automatic
//			  hazard tracking, which also covers the resources an argument
//			  buffer names once they are declared with useResource.
//			- Bind group g of a pipeline stage is the argument buffer at
//			  [[buffer(g)]], whose member [[id(n)]] is binding n. A group is
//			  encoded once per (pipeline, stage) through that stage function's
//			  argument encoder, and kept until the group is released.
//			- Vertex buffer slot s is buffer kVertexBufferBase + s; the draw
//			  constants (D16) are set bytes at kDrawConstantsSlot.
//
//			Resource state is tracked per resource, not per subresource.
//
//			Debt (RFC 0025 "Shared recording"): the command list, its
//			validation and the upload ring follow render.device.gl's line for
//			line; both move into one render.device helper when a third
//			list-replaying adapter (RFC 0024) needs them.
//
//=============================================================================//

#ifndef RENDER_DEVICE_METAL_METAL_DEVICE_H
#define RENDER_DEVICE_METAL_METAL_DEVICE_H

#if !defined( __OBJC__ ) || !__has_feature( objc_arc )
#error "render.device.metal is Objective-C++ with ARC"
#endif

#import <Metal/Metal.h>

#include "render/device/metal/provider.h"
#include "render/device/validation.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace render::device::metal
{

// The artifacts' contract (tools/render/shader_artifacts.py, METAL_*).
inline constexpr std::uint32_t kVertexBufferBase = 16;
inline constexpr std::uint32_t kDrawConstantsSlot = 30;
inline constexpr std::uint32_t kMaxVertexSlots = 8;
inline constexpr const char *kEntryPoint = "main0";
inline constexpr const char *kSpecializationLine = "// render.device.metal specialization";
inline constexpr const char *kThreadgroupLine = "// render.device.metal threadgroup";

// A port format in Metal; Invalid when the device has no such format.
MTLPixelFormat PixelFormatOf( Format format );
bool IsSrgb( Format format );

foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, std::int32_t nativeCode = 0 );

// Records -----------------------------------------------------------------------

struct BufferRecord
{
	BufferDesc desc;
	id<MTLBuffer> buffer = nil;
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;
};

struct TextureRecord
{
	TextureDesc desc;
	id<MTLTexture> texture = nil;
	std::uint32_t layers = 1; // array slices (a cube's six faces each)
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;

	std::uint32_t Width( std::uint32_t mip ) const { return std::max( 1u, desc.width >> mip ); }
	std::uint32_t Height( std::uint32_t mip ) const { return std::max( 1u, desc.height >> mip ); }
};

struct SamplerRecord
{
	id<MTLSamplerState> sampler = nil;
	bool released = false;
};

struct LayoutRecord
{
	BindGroupRole role = BindGroupRole::kFrame;
	std::vector<BindingDesc> bindings;
	bool released = false;
};

// Which pipeline stage's argument encoder an encoded group follows.
struct ArgumentKey
{
	std::uint64_t pipeline = 0;
	std::uint32_t stage = 0;
	friend bool operator<( const ArgumentKey &a, const ArgumentKey &b )
	{
		return a.pipeline != b.pipeline ? a.pipeline < b.pipeline : a.stage < b.stage;
	}
};

struct BindGroupRecord
{
	BindGroupLayoutId layout;
	std::uint32_t role = 0;
	std::vector<BindingDesc> bindings;
	std::vector<BindGroupEntry> entries;
	std::map<ArgumentKey, id<MTLBuffer>> encoded;
	bool released = false;
};

// One stage function's view of a group: its argument encoder and the ids
// (bindings) the stage declares.
struct StageGroup
{
	id<MTLArgumentEncoder> encoder = nil;
	std::vector<std::uint32_t> ids;
};

enum StageIndex : std::uint32_t
{
	kStageVertex = 0,
	kStageFragment = 1,
	kStageCompute = 2,
	kStageCount = 3
};

struct PipelineRecord
{
	PipelineKind kind = PipelineKind::kGraphics;
	id<MTLRenderPipelineState> render = nil;
	id<MTLComputePipelineState> compute = nil;
	id<MTLDepthStencilState> depthStencil = nil;
	MTLPrimitiveType primitive = MTLPrimitiveTypeTriangle;
	MTLCullMode cull = MTLCullModeBack;
	MTLWinding winding = MTLWindingCounterClockwise;
	float depthBiasConstant = 0.0f;
	float depthBiasSlope = 0.0f;
	std::uint32_t stencilReference = 0;
	MTLSize threadgroup = MTLSizeMake( 1, 1, 1 );
	std::array<std::array<StageGroup, kMaxBindGroups>, kStageCount> groups{};
	std::array<bool, kStageCount> stages{};
	std::vector<Format> colorFormats;
	Format depthFormat = Format::kUnknown;
	std::uint32_t sampleCount = 1;
	std::uint32_t vertexBuffers = 0;
	std::uint32_t drawConstantBytes = 0;
	std::array<BindGroupLayoutId, kMaxBindGroups> layouts{};
	std::array<bool, kMaxBindGroups> layoutHasBindings{};
	bool released = false;
};

// Commands ----------------------------------------------------------------------

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
	kWriteTimestamp,
	kDrawIndexedIndirect,
	kDrawIndexedIndirectCount
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
	TextureBufferCopy textureCopy;
	std::uint64_t offset = 0; // a vertex, index or draw-constant offset
	std::uint32_t slot = 0;   // a vertex slot or bind-group role
	std::uint32_t count = 0;  // vertices, indices or x groups
	std::uint32_t instances = 1;
	std::uint32_t first = 0; // first vertex or index; y groups; an indirect stride
	std::int32_t vertexOffset = 0;
	std::uint32_t firstInstance = 0; // z groups of a dispatch
	IndexFormat indexFormat = IndexFormat::kUint16;
	Viewport viewport;
	std::vector<ColorAttachment> colors;
	std::optional<DepthAttachment> depth;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	// An upload: in the ring at ringOffset, else in bytes (a full ring); the
	// bytes of draw constants; a label's text.
	std::uint64_t ringOffset = 0;
	bool fromRing = false;
	std::vector<std::byte> bytes;
};

class MetalDevice;
struct ValidationState;

class MetalEncoder final : public IEncoderBackend
{
public:
	MetalEncoder( MetalDevice &device, QueueKind queue ) : m_Device( device ), m_Queue( queue ) {}
	~MetalEncoder() override;

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
	bool HasError() const override { return m_Error; }

	bool Complete() const { return !m_Error && !m_Rendering && m_Labels == 0; }
	MetalDevice &Device() const { return m_Device; }
	QueueKind Queue() const { return m_Queue; }
	const std::vector<Command> &Commands() const { return m_Commands; }
	const std::vector<std::uint64_t> &RingAllocations() const { return m_RingAllocations; }
	void AddRingAllocation( std::uint64_t id ) { m_RingAllocations.push_back( id ); }
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

	MetalDevice &m_Device;
	QueueKind m_Queue;
	std::vector<Command> m_Commands;
	std::vector<std::uint64_t> m_RingAllocations;
	bool m_Rendering = false;
	bool m_Error = false;
	bool m_Submitted = false;
	std::uint32_t m_Labels = 0;
};

// The upload ring: one shared-storage MTLBuffer. Ranges retire in order once
// their token completes, or when their encoder is destroyed unsubmitted.
class UploadRing
{
public:
	void Reset( std::uint64_t capacity )
	{
		m_Capacity = capacity;
		m_Live.clear();
		m_Head = 0;
	}
	std::optional<std::uint64_t> Allocate( std::uint64_t size, std::uint64_t id );
	void Submit( std::uint64_t id, CompletionToken token );
	void Abandon( std::uint64_t id );
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

// Device -------------------------------------------------------------------------

class MetalDevice final : public IRenderDevice2
{
public:
	explicit MetalDevice( const MetalAdapterOptions &options );
	~MetalDevice() override;

	DeviceResult<void> Initialize();

	const DeviceFacts &Facts() const override { return m_Facts; }
	DeviceState State() const override { return m_State.load(); }
	std::uint32_t Epoch() const override { return m_Epoch; }
	MemoryBudgetSnapshot ReadMemoryBudget() const override;

	DeviceResult<BufferId> CreateBuffer( const BufferDesc &desc ) override;
	DeviceResult<BufferId> CreateUploadBuffer( std::span<const std::byte> bytes ) override;
	DeviceResult<TextureId> CreateTexture( const TextureDesc &desc ) override;
	DeviceResult<SamplerId> CreateSampler( const SamplerDesc &desc ) override;
	DeviceResult<BindGroupLayoutId> CreateBindGroupLayout(
	    const BindGroupLayoutDesc &desc ) override;
	DeviceResult<BindGroupId> CreateBindGroup( const BindGroupDesc &desc ) override;
	DeviceResult<PipelineId> CreatePipeline( const PipelineDesc &desc ) override;
	DeviceResult<void> Release( ResourceId resource, CompletionToken releaseAfter ) override;
	DeviceResult<CommandEncoder> BeginEncoder( QueueKind queue ) override;
	DeviceResult<CompletionToken> Submit(
	    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits ) override;
	bool IsComplete( CompletionToken token ) const override;
	std::size_t Poll() override;
	DeviceResult<void> ReadBuffer(
	    BufferId buffer, std::uint64_t offset, std::span<std::byte> out ) override;
	DeviceResult<void> WaitIdle() override;
	DeviceResult<void> Recover() override;
	std::size_t LiveResourceCount() const override;

	std::uint64_t CommandBufferErrors() const { return m_Errors.load(); }
	void SimulateLoss() { m_State.store( DeviceState::kLost ); }

	// For MetalEncoder: copies an upload into the ring, or into the command
	// when the ring is full. Any recording thread.
	void StageUpload( MetalEncoder &encoder, Command &command, std::span<const std::byte> bytes );
	void AbandonUploads( const std::vector<std::uint64_t> &allocations );

	// For the replay (execute.mm).
	id<MTLDevice> Native() const { return m_Device; }
	BufferRecord *ExistingBuffer( std::uint64_t id );
	TextureRecord *ExistingTexture( std::uint64_t id );
	const PipelineRecord *ExistingPipeline( std::uint64_t id ) const;
	BindGroupRecord *ExistingBindGroup( std::uint64_t id );
	id<MTLSamplerState> Sampler( std::uint64_t id ) const;
	id<MTLBuffer> Ring() const { return m_RingBuffer; }
	// The argument buffer of group for one pipeline stage, encoded on first use.
	id<MTLBuffer> ArgumentBuffer( BindGroupRecord &group, std::uint64_t pipeline,
	    std::uint32_t stage, const StageGroup &view );

private:
	struct PendingRelease
	{
		ResourceId resource;
		CompletionToken token;
	};

	DeviceResult<void> CreateQueueObjects();
	void QueryFacts();
	std::size_t Collect();
	void Erase( ResourceId resource );
	void OnCompleted( std::uint32_t epoch, std::uint64_t value, bool failed, bool lost );

	BufferRecord *LiveBuffer( std::uint64_t id );
	TextureRecord *LiveTexture( std::uint64_t id );
	std::optional<LayoutView> FindLayout( BindGroupLayoutId id ) const;
	bool *ReleasedFlag( ResourceId resource );

	// pipelines.mm
	DeviceResult<void> BuildStage( const ShaderArtifactView &artifact, const PipelineDesc &desc,
	    PipelineRecord &record, id<MTLFunction> __strong &function, std::int32_t &nativeCode );

	// execute.mm
	bool Validate( const std::vector<Command> &commands,
	    std::unordered_map<std::uint64_t, ResourceUsage> &states );
	bool ValidateDraw( const ValidationState &state, bool indexed ) const;
	bool GroupsMatch( const ValidationState &state ) const;
	void Execute( id<MTLCommandBuffer> commands, std::vector<MetalEncoder *> &encoders );

	MetalAdapterOptions m_Options;
	id<MTLDevice> m_Device = nil;
	id<MTLCommandQueue> m_Queue = nil;
	id<MTLBuffer> m_RingBuffer = nil;
	std::string m_AdapterName;
	DeviceFacts m_Facts;
	std::atomic<DeviceState> m_State{ DeviceState::kAvailable };
	std::uint32_t m_Epoch = 1;
	std::uint64_t m_NextId = 0;
	std::unordered_map<std::uint64_t, BufferRecord> m_Buffers;
	std::unordered_map<std::uint64_t, TextureRecord> m_Textures;
	std::unordered_map<std::uint64_t, SamplerRecord> m_Samplers;
	std::unordered_map<std::uint64_t, LayoutRecord> m_Layouts;
	std::unordered_map<std::uint64_t, BindGroupRecord> m_BindGroups;
	std::unordered_map<std::uint64_t, PipelineRecord> m_Pipelines;
	std::vector<PendingRelease> m_Releases;
	std::uint64_t m_Submitted = 0;
	// The newest completed submission of the current epoch, and the ones
	// that completed out of order before it (completion handlers run on
	// Metal's threads).
	std::atomic<std::uint64_t> m_Completed{ 0 };
	std::mutex m_CompletionLock;
	std::unordered_set<std::uint64_t> m_CompletedEarly;
	std::atomic<std::uint64_t> m_Errors{ 0 };
	UploadRing m_Ring;
	mutable std::mutex m_RingLock; // guards m_Ring, m_NextAllocation
	std::uint64_t m_NextAllocation = 0;
	id<MTLCommandBuffer> m_LastCommitted = nil;
	mutable std::recursive_mutex m_Lock;
};

} // namespace render::device::metal

#endif // RENDER_DEVICE_METAL_METAL_DEVICE_H
