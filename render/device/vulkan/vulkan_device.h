//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan internals (RFC 0016 K1). Private to the
//			adapter: only files under render/device/vulkan/ include it.
//
//			Model (mirrors render.device.null): encoders record a command list;
//			Submit validates every encoder in order against the device's
//			per-resource usage state, then translates the whole submission into
//			one VkCommandBuffer on the submitting thread and signals the
//			graphics timeline semaphore with the submission's token value.
//			Resource state is tracked per resource, not per subresource, so a
//			transition always covers every mip and layer.
//
//			Threading: the device is used from one sequence. Encoders may be
//			recorded on other threads (one thread per encoder); the only
//			device state recording touches is the upload ring, which is locked,
//			and the completed-value cache, which is atomic.
//
//			Memory is suballocated by the pinned Vulkan Memory Allocator
//			(memory.cpp). Parallel native recording, transient aliasing and
//			async queues are not claimed.
//
//			Host mode (host_device.h): the legacy native Vulkan backend borrows
//			this device. The adapter then also enables the host's surface
//			extensions, device extensions and features, and a present queue.
//
//=============================================================================//

#ifndef RENDER_DEVICE_VULKAN_VULKAN_DEVICE_H
#define RENDER_DEVICE_VULKAN_VULKAN_DEVICE_H

#include "host_device.h"
#include "id_table.h"
#include "render/device/validation.h"
#include "render/device/vulkan/provider.h"

#include <vulkan/vulkan.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace render::device::vulkan
{

VkCompareOp Compare( CompareOp op );

inline foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, VkResult result = VK_SUCCESS )
{
	return foundation::MakeUnexpected(
	    DeviceError{ status, operation, static_cast<std::int32_t>( result ) } );
}

// The port status for a failed Vulkan call.
DeviceStatus StatusOf( VkResult result );

// transitions.cpp: the one table from port usages to synchronization2 scopes
// and image layouts, and the format and usage-flag mappings.
struct UsageScope
{
	VkPipelineStageFlags2 stages = VK_PIPELINE_STAGE_2_NONE;
	VkAccessFlags2 access = VK_ACCESS_2_NONE;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

const UsageScope &ScopeOf( ResourceUsage usage );
VkBufferUsageFlags BufferUsageFlags( UsageSet usages );
VkImageUsageFlags ImageUsageFlags( UsageSet usages );
VkFormat ToVkFormat( Format format );
// Every aspect of format (depth and stencil for a format with stencil).
VkImageAspectFlags BarrierAspects( Format format );
// The aspect copies address: depth only for depth formats.
VkImageAspectFlags CopyAspect( Format format );

// memory.cpp ---------------------------------------------------------------

// The adapter's one allocator (the pinned Vulkan Memory Allocator). All
// members are thread-safe.
class MemoryAllocator
{
public:
	MemoryAllocator() = default;
	MemoryAllocator( const MemoryAllocator & ) = delete;
	MemoryAllocator &operator=( const MemoryAllocator & ) = delete;
	~MemoryAllocator();

	VkResult Create( VkInstance instance, VkPhysicalDevice physical, VkDevice device,
	    std::uint32_t apiVersion, bool bufferDeviceAddress, bool memoryBudget );
	// Reports live allocations on stderr, then destroys the allocator.
	void Destroy();
	bool Valid() const { return m_Allocator != nullptr; }

	VkResult CreateBuffer( const VkBufferCreateInfo &info, const HostMemory &memory,
	    VkBuffer *buffer, HostAllocation *allocation, void **mapped, std::string_view label = {} );
	VkResult CreateImage( const VkImageCreateInfo &info, const HostMemory &memory, VkImage *image,
	    HostAllocation *allocation, std::string_view label = {} );
	void DestroyBuffer( VkBuffer buffer, HostAllocation allocation );
	void DestroyImage( VkImage image, HostAllocation allocation );
	void Free( HostAllocation allocation );
	VkResult Map( HostAllocation allocation, void **data );
	void Unmap( HostAllocation allocation );
	VkResult Invalidate( HostAllocation allocation, VkDeviceSize offset, VkDeviceSize size );
	VkMemoryPropertyFlags Properties( HostAllocation allocation ) const;
	std::uint64_t LiveAllocations() const { return m_Live.load(); }
	std::uint64_t LiveBytes() const { return m_Bytes.load(); }
	MemoryBudgetSnapshot ReadBudget( std::uint32_t epoch ) const;

	// Diagnostic memory report (SOURCE_VK_MEMORY_REPORT=<seconds>): live
	// allocations grouped by owner label, or by image/buffer shape where no
	// label was given, with the driver's per-heap usage and budget. Reported on
	// stderr after an allocation failure and, at most once per interval, when
	// the live total has moved by 32 MiB. Off: no tracking cost.
	void Label( HostAllocation allocation, std::string_view label );
	void Report( const char *reason );

private:
	struct Tracked
	{
		std::string key;
		std::uint64_t size = 0;
	};
	void Count( HostAllocation allocation, bool add );
	void Track( HostAllocation allocation, std::string key );
	void Untrack( HostAllocation allocation );
	void ReportFailure(
	    std::string_view label, const char *what, VkResult result, std::uint64_t bytes );
	void MaybeReport();

	void *m_Allocator = nullptr; // VmaAllocator
	bool m_MemoryBudgetEnabled = false;
	std::atomic<std::uint64_t> m_Live{ 0 };
	std::atomic<std::uint64_t> m_Bytes{ 0 };
	double m_ReportSeconds = 0.0; // 0: tracking off
	std::mutex m_TrackMutex;
	std::unordered_map<HostAllocation, Tracked> m_Tracked;
	std::uint64_t m_ReportedBytes = 0;
	double m_LastReport = 0.0;
	bool m_FailureReported = false;
};

// instance.cpp -------------------------------------------------------------

// Owns the VkInstance and its debug messenger. It outlives the logical
// device, which Recover() rebuilds.
struct InstanceHandle
{
	InstanceHandle() = default;
	InstanceHandle( const InstanceHandle & ) = delete;
	InstanceHandle &operator=( const InstanceHandle & ) = delete;
	~InstanceHandle();

	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
	bool validation = false; // the Khronos layer is enabled
	std::atomic<std::uint64_t> messages{ 0 };
	std::atomic<std::uint64_t> *sink = nullptr;
	PFN_vkCmdBeginDebugUtilsLabelEXT beginLabel = nullptr;
	PFN_vkCmdEndDebugUtilsLabelEXT endLabel = nullptr;
	PFN_vkSetDebugUtilsObjectNameEXT setObjectName = nullptr;
	std::uint32_t apiVersion = 0; // the version the instance was created with
	bool debugUtils = false;      // VK_EXT_debug_utils is enabled
};

DeviceResult<std::unique_ptr<InstanceHandle>> CreateInstance(
    bool validation, std::atomic<std::uint64_t> *sink );
// The instance of a host device: the host's extensions and messenger, the
// highest API version the loader offers up to 1.3.
DeviceResult<std::unique_ptr<InstanceHandle>> CreateHostInstance(
    const HostDeviceRequest &request, std::uint32_t *apiVersion );

struct AdapterChoice
{
	VkPhysicalDevice physical = VK_NULL_HANDLE;
	std::uint32_t queueFamily = 0;
	std::uint32_t presentFamily = 0; // host mode: the family that presents
	// A compute family without graphics (RFC 0016 S8: the async compute
	// queue, Capability::kAsyncCompute), or UINT32_MAX. Port mode only.
	std::uint32_t computeFamily = UINT32_MAX;
	bool core12 = false;             // else Vulkan 1.1 with VK_KHR_timeline_semaphore
	bool core13 = false;             // else the synchronization2 extension
	// Dynamic rendering: always for the port's own device; a host device may
	// lack it (a Vulkan 1.1 driver), and then the port refuses rendering
	// passes and graphics pipelines. RFC 0016 "Devices without dynamic
	// rendering" makes the core serve them before cutover.
	bool dynamicRendering = false;
	bool anisotropy = false;
	bool textureCompressionBC = false; // the kBC* formats (D19)
	bool multiDrawIndirect = false;    // D30: drawCount above 1
	bool imageCubeArray = false;       // D32: cube-array image views
	bool fillModeNonSolid = false;     // D38: VK_POLYGON_MODE_LINE
	bool occlusionQueryPrecise = false; // D43: exact occlusion sample counts
	bool drawIndirectCount = false;    // D31: Vulkan 1.2's drawIndirectCount
	bool drawIndirectFirstInstance = false; // D30: nonzero firstInstance in records
	bool memoryBudget = false; // VK_EXT_memory_budget was enabled for VMA
	// Without dynamic rendering, read-only depth stores nothing through
	// VK_EXT_load_store_op_none or VK_QCOM_render_pass_store_ops.
	bool storeOpNone = false;
	// dmabuf export of LINEAR images (external memory fd, dma_buf, DRM
	// format modifiers): the adapter claims kExternalImages.
	bool externalImages = false;
	std::vector<const char *> extensions; // device extensions to enable
};

DeviceResult<AdapterChoice> SelectAdapter( VkInstance instance, int adapterIndex );
// Host mode: the device that presents to surface (when there is one) from a
// graphics family, has the swapchain extension then, and meets the port's
// requirements; discrete first, as the legacy backend chose. reason names
// the first requirement no device met.
DeviceResult<AdapterChoice> SelectHostAdapter( VkInstance instance, VkSurfaceKHR surface,
    bool requireDiscrete, int adapterIndex, const char **reason );

// upload_ring.cpp ----------------------------------------------------------

// A host-visible, coherent, persistently mapped buffer.
struct HostBuffer
{
	VkBuffer buffer = VK_NULL_HANDLE;
	HostAllocation memory = nullptr;
	std::byte *mapped = nullptr;
};

// The upload ring. Allocations retire in order once the timeline reaches
// their submission's value, or at once when their encoder was destroyed
// without submitting. All members lock, so encoders on other threads may
// stage uploads.
class UploadRing
{
public:
	void Attach( HostBuffer buffer, std::uint64_t capacity );
	// Detaches the buffer for destruction and forgets every allocation.
	HostBuffer Detach();

	// Returns (offset, allocation id), or nothing when the ring is full.
	std::optional<std::pair<std::uint64_t, std::uint64_t>> Allocate(
	    std::uint64_t size, std::uint64_t completedValue );
	void Submit( std::uint64_t id, std::uint64_t value );
	void Abandon( std::uint64_t id );
	void Retire( std::uint64_t completedValue );

	std::byte *Data( std::uint64_t offset ) const { return m_Buffer.mapped + offset; }
	VkBuffer Buffer() const { return m_Buffer.buffer; }

private:
	struct Allocation
	{
		std::uint64_t offset = 0;
		std::uint64_t size = 0;
		std::uint64_t id = 0;
		std::uint64_t value = 0;
		bool submitted = false;
		bool abandoned = false;
	};

	Allocation *FindLocked( std::uint64_t id ); // caller holds m_Mutex
	void RetireLocked( std::uint64_t completedValue );

	mutable std::mutex m_Mutex;
	HostBuffer m_Buffer;
	std::uint64_t m_Capacity = 0;
	std::deque<Allocation> m_Live;
	std::uint64_t m_Head = 0;
	std::uint64_t m_NextId = 0;
};

// Resources ----------------------------------------------------------------

// The usage a resource is in, and whether it was written in that usage
// since its last barrier (so the next access needs one: the port runs
// commands as if one after another, including two writes in one usage).
struct Track
{
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool dirty = false;
};

struct BufferRecord
{
	BufferDesc desc; // debugName cleared
	VkBuffer buffer = VK_NULL_HANDLE;
	HostAllocation memory = nullptr;
	std::byte *mapped = nullptr;
	bool coherent = true;
	Track track;
	bool released = false;
};

struct TextureRecord
{
	TextureDesc desc; // debugName cleared
	std::uint32_t layers = 1;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkImage image = VK_NULL_HANDLE;
	HostAllocation memory = nullptr;
	VkImageView view = VK_NULL_HANDLE;           // the whole texture, for bindings
	VkImageView attachmentView = VK_NULL_HANDLE; // mip 0, layer 0, for rendering
	// An exported image's own dedicated memory (else the allocator's).
	VkDeviceMemory exported = VK_NULL_HANDLE;
	// A host image (IHostDevice::ImportImage): the adapter owns only the
	// views. home is the usage whose layout the host keeps it in.
	bool imported = false;
	ResourceUsage home = ResourceUsage::kUndefined;
	Track track;
	bool released = false;

	std::uint32_t Width( std::uint32_t mip ) const;
	std::uint32_t Height( std::uint32_t mip ) const;
};

struct SamplerRecord
{
	VkSampler sampler = VK_NULL_HANDLE;
	bool released = false;
};

// A descriptor set layout shared by its bind-group layout record and by the
// bind groups and pipelines made from it, so releasing the layout never
// invalidates them. Destroyed when the last holder drops it, always before
// the device (DestroyLogical clears every holder first).
struct SetLayout
{
	SetLayout( VkDevice device, VkDescriptorSetLayout layout ) : device( device ), layout( layout )
	{
	}
	SetLayout( const SetLayout & ) = delete;
	SetLayout &operator=( const SetLayout & ) = delete;
	~SetLayout();

	VkDevice device;
	VkDescriptorSetLayout layout;
};

struct LayoutRecord
{
	BindGroupRole role = BindGroupRole::kFrame;
	std::vector<BindingDesc> bindings;
	std::shared_ptr<SetLayout> handle;
	bool released = false;
};

struct BindGroupRecord
{
	BindGroupLayoutId layout;
	std::shared_ptr<SetLayout> setLayout;
	VkDescriptorPool pool = VK_NULL_HANDLE;
	VkDescriptorSet set = VK_NULL_HANDLE;
	std::vector<ResourceId> resources; // buffers, textures and samplers it names
	bool released = false;
};

// The stages a pipeline's draw constants (push constants) are visible to.
inline VkShaderStageFlags DrawConstantStages( PipelineKind kind )
{
	return kind == PipelineKind::kCompute
	           ? VK_SHADER_STAGE_COMPUTE_BIT
	           : VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
}

struct PipelineRecord
{
	PipelineKind kind = PipelineKind::kGraphics;
	std::array<BindGroupLayoutId, kMaxBindGroups> layouts{};
	std::array<bool, kMaxBindGroups> layoutHasBindings{};
	std::array<std::shared_ptr<SetLayout>, kMaxBindGroups> setLayouts{};
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::vector<Format> colorFormats;
	Format depthFormat = Format::kUnknown;
	std::uint32_t sampleCount = 1;
	std::uint32_t vertexBuffers = 0;
	std::uint32_t drawConstantBytes = 0; // D16: a push-constant range of this size
	bool released = false;
};

// Encoders -----------------------------------------------------------------

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
	kDrawIndexedIndirect,      // D30: buffer a at offset; params[0] draws, params[1] stride
	kDrawIndexedIndirectCount, // D31: plus count buffer b at copy.destinationOffset
	kDispatch,
	kBeginLabel,
	kEndLabel,
	kSetDrawConstants,
	kWriteTimestamp, // D23: buffer a, at offset
	kBeginOcclusionQuery, // D43: buffer a, at offset
	kEndOcclusionQuery,
	kClearRegion,    // D44: region
	kComputeInterop, // private compute bridge with declared texture accesses
	kNative,         // host work (host_device.h RecordNative)
	kSectionBegin,   // port commands host work runs (host_device.h BeginSection)
	kSectionEnd
};

// Private adapter bridge. A submission retains payloads until its completion
// token retires; providers may release their own reference immediately. All
// methods execute on the device sequence. Aborted marks CPU history invalid.
struct ComputeInterop
{
	struct Image
	{
		TextureId id;
		ResourceUsage usage;
	};
	std::vector<Image> images;
	virtual ~ComputeInterop() = default;
	virtual bool Record( VkCommandBuffer cmd ) = 0;
	virtual void Submitted( CompletionToken token ) = 0;
	virtual void Aborted() = 0;
	virtual void DeviceDestroyed() = 0;
};

// A begin-rendering command's attachments, kept by its encoder.
struct RenderingPayload
{
	std::vector<ColorAttachment> colors;
	std::optional<DepthAttachment> depth;
};

// One recorded command: trivially copyable, so an encoder's list grows and
// clears without per-command construction. Payloads that are rare or
// variable-sized live in the encoder (VulkanEncoder's payload storage) and
// are referenced here for the encoder's lifetime.
struct Command
{
	Op op = Op::kDraw;
	ComputeInterop *compute = nullptr; // kComputeInterop; owned by the encoder
	std::uint64_t a = 0; // main handle (source for copies)
	std::uint64_t b = 0; // destination handle for copies
	ResourceUsage before = ResourceUsage::kUndefined;
	ResourceUsage after = ResourceUsage::kUndefined;
	SubresourceRange range;
	ClearColor color;
	BufferCopy copy;
	TextureBufferCopy textureCopy;
	// Uploads: from the ring at ringOffset, or from a dedicated staging buffer.
	std::uint64_t ringOffset = 0;
	VkBuffer staging = VK_NULL_HANDLE;
	// Rendering: the attachments (kBeginRendering).
	const RenderingPayload *rendering = nullptr;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	// Bindings, viewport and draws.
	std::uint32_t slot = 0;
	std::uint64_t offset = 0;
	IndexFormat indexFormat = IndexFormat::kUint16;
	Viewport viewport;
	ClearRegion region; // kClearRegion
	std::uint32_t params[4] = {};
	std::int32_t vertexOffset = 0;
	const std::string *label = nullptr; // kBeginLabel
	std::span<const std::byte> bytes;   // draw constants (D16), written at `offset`
	void ( *native )( void *user, VkCommandBuffer cmd ) = nullptr;
	void *nativeUser = nullptr;
};
static_assert( std::is_trivially_copyable_v<Command> );

class VulkanDevice;

class VulkanEncoder final : public IEncoderBackend
{
public:
	VulkanEncoder( VulkanDevice &device, QueueKind queue );
	~VulkanEncoder() override;

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
	void Draw( std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t firstVertex,
	    std::uint32_t firstInstance ) override;
	void DrawIndexed( std::uint32_t indexCount, std::uint32_t instanceCount,
	    std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance ) override;
	void DrawIndexedIndirect( BufferId buffer, std::uint64_t offset, std::uint32_t drawCount,
	    std::uint32_t stride ) override;
	void DrawIndexedIndirectCount( BufferId buffer, std::uint64_t offset, BufferId countBuffer,
	    std::uint64_t countOffset, std::uint32_t maxDrawCount, std::uint32_t stride ) override;
	void Dispatch( std::uint32_t x, std::uint32_t y, std::uint32_t z ) override;
	void SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes ) override;
	void BeginLabel( std::string_view label ) override;
	void EndLabel() override;
	void WriteTimestamp( BufferId buffer, std::uint64_t offset ) override;
	void BeginOcclusionQuery( BufferId buffer, std::uint64_t offset ) override;
	void EndOcclusionQuery() override;
	bool HasError() const override { return m_Error; }

	bool Complete() const { return !m_Error && !m_Rendering && m_Labels == 0 && !m_InSection; }
	VulkanDevice &Device() const { return m_Device; }
	QueueKind Queue() const { return m_Queue; }
	const std::vector<Command> &Commands() const { return m_Commands; }
	// The compute payloads of the kComputeInterop commands, in command order.
	const std::vector<std::shared_ptr<ComputeInterop>> &Computes() const { return m_Computes; }
	std::vector<std::uint64_t> &RingAllocations() { return m_RingAllocations; }
	std::vector<HostBuffer> &Staging() { return m_Staging; }
	void MarkSubmitted() { m_Submitted = true; }
	void SetError() { m_Error = true; }
	// Host interop (host_device.h).
	void Compute( std::shared_ptr<ComputeInterop> payload );
	void Native( void ( *record )( void *, VkCommandBuffer ), void *user );
	void BeginSection();
	void EndSection();
	void AddWait( VkSemaphore semaphore, VkPipelineStageFlags2 stage );
	void AddSignal( VkSemaphore semaphore );
	const std::vector<VkSemaphoreSubmitInfo> &Waits() const { return m_Waits; }
	const std::vector<VkSemaphoreSubmitInfo> &Signals() const { return m_Signals; }

private:
	void Push( const Command &command ) { m_Commands.push_back( command ); }
	// Stable storage for a command's draw-constant bytes.
	std::span<const std::byte> KeepBytes( std::span<const std::byte> bytes );
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

	VulkanDevice &m_Device;
	QueueKind m_Queue;
	std::vector<Command> m_Commands; // from the device's pool, returned on destruction
	// Payload storage the commands reference (deques keep their addresses).
	std::vector<std::shared_ptr<ComputeInterop>> m_Computes;
	std::deque<RenderingPayload> m_Renderings;
	std::deque<std::string> m_LabelText;
	std::vector<std::vector<std::byte>> m_ByteChunks; // each filled within its capacity
	std::vector<std::uint64_t> m_RingAllocations;
	std::vector<HostBuffer> m_Staging;            // uploads that found the ring full
	std::vector<VkSemaphoreSubmitInfo> m_Waits;   // binary, host interop
	std::vector<VkSemaphoreSubmitInfo> m_Signals; // binary, host interop
	bool m_Rendering = false;
	bool m_Error = false;
	bool m_Submitted = false;
	std::uint32_t m_Labels = 0;
	bool m_InSection = false;
	std::uint32_t m_SectionLabels = 0; // labels open when the section began
};

// The device ---------------------------------------------------------------

struct DeviceDispatch
{
	PFN_vkCmdPipelineBarrier2 cmdPipelineBarrier2 = nullptr;
	PFN_vkCmdDrawIndexedIndirectCount cmdDrawIndexedIndirectCount = nullptr;
	PFN_vkQueueSubmit2 queueSubmit2 = nullptr;
	PFN_vkCmdBeginRendering cmdBeginRendering = nullptr;
	PFN_vkGetMemoryFdKHR getMemoryFd = nullptr; // with externalImages
	PFN_vkCmdEndRendering cmdEndRendering = nullptr;
	PFN_vkGetSemaphoreCounterValue getSemaphoreCounterValue = nullptr;
	PFN_vkWaitSemaphores waitSemaphores = nullptr;
	PFN_vkSignalSemaphore signalSemaphore = nullptr; // tests only (Hold)
};

class Translator;

class VulkanDevice final : public IRenderDevice2, public IExternalImages
{
public:
	// host is set for a device a legacy host borrows (host_device.h); it is
	// read only during Initialize.
	explicit VulkanDevice( const VulkanAdapterOptions &options,
	    const HostDeviceRequest *host = nullptr, std::shared_ptr<InstanceHandle> instance = {} );
	~VulkanDevice() override;

	DeviceResult<void> Initialize();
	// Why Initialize failed, when a host requirement named it (else null).
	const char *FailureReason() const { return m_FailureReason; }

	// IRenderDevice2 ---------------------------------------------------------

	const DeviceFacts &Facts() const override { return m_Facts; }
	DeviceState State() const override { return m_State; }
	std::uint32_t Epoch() const override { return m_Epoch; }

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
	ResourceActivity ReadResourceActivity() const override;
	MemoryBudgetSnapshot ReadMemoryBudget() const override
	{
		return m_Memory.ReadBudget( Epoch() );
	}
	IExternalImages *ExternalImages() override;

	// IExternalImages ----------------------------------------------------------

	foundation::Expected<ExternalImage, DeviceError> CreateExported(
	    const TextureDesc &desc ) override;
	void CloseHandle( std::int64_t handle ) override;

	// Diagnostics ------------------------------------------------------------

	std::uint64_t ValidationMessageCount() const;
	std::uint64_t DeferredUploadCount() const { return m_DeferredUploads.load(); }
	// Tests only (vulkan::HoldSubmissions): while held, each submission also
	// waits on the hold semaphore's next value, which releasing signals from
	// the host. False if the hold semaphore cannot be made.
	bool Hold( bool held );

	// For VulkanEncoder (any recording thread) --------------------------------

	// Copies bytes into the ring, or a dedicated staging buffer when it is full.
	void StageUpload( VulkanEncoder &encoder, Command &command, std::span<const std::byte> bytes );
	// Returns an unsubmitted encoder's ring ranges and staging buffers.
	void AbandonUploads( VulkanEncoder &encoder );

	// For the host interop (host_device.cpp) ---------------------------------

	bool FsrEnabled() const { return m_Options.fsr411; }
	// FSR's mixed float dot extension was enabled (otherwise its portable prepass).
	bool FsrMixedFloatDot() const { return m_FsrMixedFloatDot; }
	const HostDeviceInfo &HostInfo() const { return m_HostInfo; }
	void SetPipelineCache( VkPipelineCache cache )
	{
		m_PipelineCache.store( cache, std::memory_order_release );
	}
	MemoryAllocator &Memory() { return m_Memory; }
	VkSemaphore TimelineSemaphore() const { return m_Timeline; }
	std::uint64_t ReserveValue();
	void AbandonValue( std::uint64_t value );
	std::uint64_t SubmittedValue() const { return m_Submitted; }
	std::uint64_t Completed() const { return CompletedValue(); }
	VkResult WaitValue( std::uint64_t value, std::uint64_t timeoutNs );
	struct HostRelease
	{
		std::uint64_t value = 0;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkImage image = VK_NULL_HANDLE;
		HostAllocation allocation = nullptr;
		void ( *destroy )( void * ) = nullptr;
		void *object = nullptr;
	};
	void ReleaseHostAfter( const HostRelease &release );
	// A host image as a texture (host_device.h ImportImage).
	DeviceResult<TextureId> ImportImage(
	    VkImage image, const TextureDesc &desc, ResourceUsage home );
	// The imported textures, for the host-work boundaries of a submission.
	const std::unordered_set<std::uint64_t> &Imported() const { return m_Imported; }
	// The imports some usage of which writes (attachment, storage, copy
	// destination). The rest (sampled-only material textures) are never
	// written by the port, and the host orders its own writes to them
	// (ImportImage), so no host-work boundary needs a barrier for them.
	const std::unordered_set<std::uint64_t> &WritableImports() const { return m_WritableImports; }
	// Runs section `index` of the host work being translated (host_device.h
	// RunSection); false outside host work or past its last section.
	bool RunSection( VkCommandBuffer cmd, std::uint32_t index );
	std::size_t CollectHost();
	std::size_t PendingHostReleases() const;
	std::mutex &QueueMutex() { return m_QueueMutex; }
	// Encoders' command lists, recycled with their capacity (any thread).
	std::vector<Command> TakeCommandList();
	void ReturnCommandList( std::vector<Command> commands );

private:
	friend class Translator;
	friend class FsrProvider;

	std::mutex m_CommandListLock;
	std::vector<std::vector<Command>> m_CommandLists;

	struct PendingRelease
	{
		ResourceId resource;
		CompletionToken token;
	};

	// One command pool and buffer per submission, recycled once its value
	// completes, with the staging buffers the submission read from.
	struct CommandContext
	{
		VkCommandPool pool = VK_NULL_HANDLE;
		VkCommandBuffer buffer = VK_NULL_HANDLE;
		std::uint64_t value = 0;
		std::vector<HostBuffer> staging;
		// Render-pass fallback: the submission's framebuffers.
		std::vector<VkFramebuffer> framebuffers;
		std::vector<std::shared_ptr<ComputeInterop>> compute;
		// D23: the submission's timestamps, reset at its start.
		VkQueryPool queries = VK_NULL_HANDLE;
		std::uint32_t queryCapacity = 0;
		// D43: the submission's occlusion queries, reset at its start.
		VkQueryPool occlusion = VK_NULL_HANDLE;
		std::uint32_t occlusionCapacity = 0;
	};

	// Per-encoder validation state (Submit rejects before anything records).
	struct ValidationState
	{
		const PipelineRecord *pipeline = nullptr;
		std::array<std::uint64_t, kMaxBindGroups> groups{};
		std::uint64_t vertexSlots = 0;
		bool index = false;
		bool rendering = false;
		bool occlusionOpen = false; // D43: a query open in this rendering scope
		std::vector<Format> colors;
		Format depth = Format::kUnknown;
		std::uint32_t samples = 0;
		DrawConstantCoverage constants; // D16
	};

public:
	// One subpass's attachments for the render-pass fallback. A resolve's
	// format is VK_FORMAT_UNDEFINED where its color has none.
	struct RenderPassAttachment
	{
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentLoadOp load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkAttachmentStoreOp store = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};
	struct RenderPassDesc
	{
		std::vector<RenderPassAttachment> colors;
		std::vector<RenderPassAttachment> resolves; // empty, or one per color
		std::optional<RenderPassAttachment> depth;
		bool stencil = false; // the depth format has stencil
	};
	// The cached render pass for desc, or VK_NULL_HANDLE (any thread).
	VkRenderPass RenderPassFor( const RenderPassDesc &desc );

private:
	// Logical device lifetime (rebuilt by Recover).
	void ForceRenderPasses(); // VulkanAdapterOptions::renderPasses
	DeviceResult<void> SelectHost();
	DeviceResult<void> CreateLogical();
	void DestroyLogical();
	void MarkLost() { m_State = DeviceState::kLost; }

	// Memory (resources.cpp).
	static HostMemory MemoryFor( MemoryKind kind );
	DeviceResult<HostBuffer> CreateHostBuffer(
	    std::uint64_t size, VkBufferUsageFlags usage, DeviceOperation op );
	void DestroyHostBuffer( HostBuffer &buffer );
	void Name( VkObjectType type, std::uint64_t handle, std::string_view name );

	// Completion (device.cpp).
	std::uint64_t CompletedValue() const;
	void RecycleCompleted();
	std::size_t CollectReleases();
	void Erase( ResourceId resource );
	// Destroys a texture's views, image and memory (the allocator's or its
	// exported dedicated memory); an imported texture's views only.
	void DestroyTexture( TextureRecord &record );
	// The views a texture's usages need (resources.cpp).
	VkResult CreateTextureViews( TextureRecord &record, VkImageUsageFlags usage );
	bool *ReleasedFlag( ResourceId resource );

	// Descriptors (pipelines.cpp).
	DeviceResult<std::pair<VkDescriptorPool, VkDescriptorSet>> AllocateSet(
	    VkDescriptorSetLayout layout );
	DeviceResult<VkDescriptorPool> CreateDescriptorPool();

	// Submission (encoder.cpp).
	bool Validate( const std::vector<Command> &commands,
	    std::unordered_map<std::uint64_t, ResourceUsage> &states );
	// Whether every imported texture the states name is in its home usage.
	bool ImportsAtHome( const std::unordered_map<std::uint64_t, ResourceUsage> &states );
	bool ValidateDraw( const ValidationState &state, bool indexed ) const;
	bool GroupsMatch( const ValidationState &state ) const;
	DeviceResult<CommandContext> AcquireContext( QueueKind queue = QueueKind::kGraphics );

	BufferRecord *LiveBuffer( std::uint64_t id );
	TextureRecord *LiveTexture( std::uint64_t id );
	std::optional<LayoutView> FindLayout( BindGroupLayoutId id ) const;
	template <typename Map> static bool Live( const Map &map, std::uint64_t id )
	{
		const auto found = map.find( id );
		return found != map.end() && !found->second.released;
	}

	std::vector<std::weak_ptr<ComputeInterop>> m_ComputeInterop;
	VulkanAdapterOptions m_Options;
	const HostDeviceRequest *m_HostRequest = nullptr; // during Initialize only
	bool m_HostMode = false;
	const char *m_FailureReason = nullptr;
	bool m_FsrMixedFloatDot = false;
	// Diagnostic (SOURCE_VK_PIPELINE_STATS=1): VK_KHR_pipeline_executable_
	// properties enabled, every pipeline compiled with statistics captured, and
	// each executable's statistics (registers, spills, ...) logged to stderr as
	// one "[pipeline-stats]" line. Compiler diagnostics, not GPU timings.
	bool m_PipelineStats = false;
	PFN_vkGetPipelineExecutablePropertiesKHR m_GetExecutableProperties = nullptr;
	PFN_vkGetPipelineExecutableStatisticsKHR m_GetExecutableStatistics = nullptr;
	void ReportPipelineStatistics( VkPipeline pipeline, std::string_view name,
	    const std::vector<std::uint32_t> &fragmentSpecIds,
	    const std::vector<std::uint32_t> &fragmentSpecValues ) const;
	HostDeviceInfo m_HostInfo;
	std::shared_ptr<InstanceHandle> m_Instance; // destroyed last (and shared by a host instance)
	AdapterChoice m_Adapter;
	VkPhysicalDeviceProperties m_Properties{};
	VkPhysicalDeviceMemoryProperties m_MemoryProperties{};
	std::string m_AdapterName;
	DeviceFacts m_Facts;

	// Everything below belongs to the logical device.
	VkDevice m_Device = VK_NULL_HANDLE;
	VkQueue m_Queue = VK_NULL_HANDLE;
	// A host's pipeline cache (IHostDevice::SetPipelineCache), which every
	// pipeline creation uses while set; the host owns, persists and destroys it.
	std::atomic<VkPipelineCache> m_PipelineCache{ VK_NULL_HANDLE };
	std::mutex m_QueueMutex; // the graphics queue's external synchronization
	// Progressive enhancement for devices without dynamic rendering (Vulkan
	// 1.1, Adreno 730): rendering passes and graphics pipelines use render
	// passes, one per attachment description, kept for the device's life.
	// Pipelines take a compatible one (formats and samples; a single
	// subpass ignores resolves); passes the one with their load/store ops.
	std::mutex m_RenderPassMutex;
	std::map<std::vector<std::uint32_t>, VkRenderPass> m_RenderPasses;
	MemoryAllocator m_Memory;
	VkSemaphore m_Timeline = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_EmptySetLayout = VK_NULL_HANDLE;
	DeviceDispatch m_Vk;
	UploadRing m_Ring;
	std::vector<VkDescriptorPool> m_DescriptorPools;
	std::vector<CommandContext> m_FreeContexts;
	std::deque<CommandContext> m_InFlight;

	mutable std::atomic<DeviceState> m_State{ DeviceState::kAvailable };
	std::uint32_t m_Epoch = 1;
	std::uint64_t m_Submitted = 0; // the last signaled timeline value
	// Tests only (Hold): a timeline semaphore of the logical device, made on
	// the first hold. While m_Holding, submissions wait on m_HoldValue + 1.
	VkSemaphore m_Hold = VK_NULL_HANDLE;
	std::uint64_t m_HoldValue = 0;
	bool m_Holding = false;
	// Signals the pending hold value, so held work runs (or, on a lost
	// device, is dropped with it). Before every idle wait and teardown.
	void ReleaseHold();
	mutable std::atomic<std::uint64_t> m_Completed{ 0 };
	// The async compute queue (Capability::kAsyncCompute; RFC 0016 S8): its
	// own family, timeline and command contexts; tokens name it by
	// QueueKind::kCompute. Resources are created with concurrent sharing
	// between the two families, so work moves between queues by semaphore
	// waits alone (no queue-family ownership transfers).
	struct ComputeQueue
	{
		VkQueue queue = VK_NULL_HANDLE;
		VkSemaphore timeline = VK_NULL_HANDLE;
		std::uint64_t submitted = 0;
		mutable std::atomic<std::uint64_t> completed{ 0 };
		std::vector<CommandContext> free;
		std::deque<CommandContext> inFlight;
	} m_Compute;
	bool AsyncCompute() const { return m_Compute.queue != VK_NULL_HANDLE; }
	std::uint64_t CompletedValue( QueueKind queue ) const;
	std::uint64_t SubmittedValue( QueueKind queue ) const
	{
		return queue == QueueKind::kCompute ? m_Compute.submitted : m_Submitted;
	}
	// Concurrent sharing between the graphics and compute families.
	template <typename Info> void ApplySharing( Info &info ) const
	{
		if ( !AsyncCompute() )
			return;
		info.sharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = m_SharedFamilies;
	}
	std::uint32_t m_SharedFamilies[2] = {};
	std::atomic<std::uint64_t> m_DeferredUploads{ 0 };
	// Label observers may sample from recording workers. Counters are diagnostic
	// only; they never publish resource contents or control retirement.
	struct ActivityCounters
	{
		std::array<std::atomic<std::uint64_t>, ResourceActivity::kKinds> created{};
		std::array<std::atomic<std::uint64_t>, ResourceActivity::kKinds> destroyed{};
		std::atomic<std::uint64_t> releaseRequests{ 0 };
		std::atomic<std::uint64_t> bufferBytes{ 0 };
		std::atomic<std::uint64_t> pending{ 0 };
	} m_ResourceActivity;
	IdTable<BufferRecord> m_Buffers{ 1 };
	IdTable<TextureRecord> m_Textures{ 2 };
	std::unordered_set<std::uint64_t> m_Imported; // host images among m_Textures
	std::unordered_set<std::uint64_t> m_WritableImports; // those with a write usage
	IdTable<SamplerRecord> m_Samplers{ 3 };
	IdTable<LayoutRecord> m_Layouts{ 4 };
	IdTable<BindGroupRecord> m_BindGroups{ 5 };
	IdTable<PipelineRecord> m_Pipelines{ 6 };
	std::vector<PendingRelease> m_Releases;
	Translator *m_Translating = nullptr; // during Submit's translation only
	mutable std::mutex m_HostReleaseMutex;
	std::deque<HostRelease> m_HostReleases;
};

} // namespace render::device::vulkan

#endif // RENDER_DEVICE_VULKAN_VULKAN_DEVICE_H
