//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.webgpu private: the WebGPU adapter of
//			render.device.v2 (RFC 0029). See public/render/device/webgpu/
//			provider.h. The only render target that sees webgpu.h.
//
//			Model:
//			- Encoders record CPU command lists (any thread; render/device/
//			  recording.h). WriteBuffer bytes travel in their commands.
//			- Submit validates every encoder against the usage state the
//			  previous accepted submission left, then replays the lists in
//			  order into one WGPUCommandBuffer. A submission's uploads and draw
//			  constants go in buffers of its own, written by the queue before
//			  the command buffer runs and freed with its token.
//			- Usage transitions are no work: WebGPU tracks hazards itself.
//			- WebGPU layouts need binding types the port's layouts do not
//			  carry, so a pipeline's layouts come from its artifacts' binding
//			  lines (shader_artifacts.py wgsl_compile), shared between
//			  pipelines through a cache keyed by their entries. A port bind
//			  group becomes a WebGPU bind group per layout it is used with.
//			- The draw constants are a uniform buffer at group 3, binding
//			  kDrawConstantsBinding, bound with a dynamic offset into the
//			  submission's constants buffer; a group-3 bind group of a pipeline
//			  with draw constants is built per submission.
//			- A readback buffer is a GPU buffer and a MapRead copy: after each
//			  submission that names it, the GPU buffer is copied and mapped,
//			  and its bytes kept on the CPU. The token completes when the work
//			  is done and those maps have landed, so ReadBuffer never waits.
//			- WebGPU's row pitch for texture copies is a multiple of 256
//			  bytes; a tightly packed region whose rows are not is copied
//			  through a padded buffer of the submission, row by row.
//
//			Resource state is tracked per resource, not per subresource.
//
//=============================================================================//

#ifndef RENDER_DEVICE_WEBGPU_WEBGPU_DEVICE_H
#define RENDER_DEVICE_WEBGPU_WEBGPU_DEVICE_H

#include <webgpu/webgpu.h>

#include "render/device/recording.h"
#include "render/device/validation.h"
#include "render/device/webgpu/provider.h"

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
#include <string_view>
#include <unordered_map>
#include <vector>

namespace render::device::webgpu
{

// The artifacts' contract (tools/render/shader_artifacts.py, WEBGPU_*).
inline constexpr std::uint32_t kDrawGroup = 3;
inline constexpr std::uint32_t kDrawConstantsBinding = 255;
inline constexpr std::string_view kArtifactLine = "// render.device.webgpu ";
inline constexpr const char *kEntryPoint = "main";
inline constexpr std::uint32_t kMaxVertexSlots = 8;
inline constexpr std::uint64_t kCopyRowAlignment = 256; // WebGPU's bytesPerRow rule

WGPUTextureFormat TextureFormatOf( Format format );
WGPUStringView View( std::string_view text );
std::string_view Text( WGPUStringView view );
foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, std::int32_t nativeCode = 0 );

// A WebGPU handle released on destruction (the release function of its type).
template <typename T, void ( *ReleaseFn )( T )> class Handle
{
public:
	Handle() = default;
	explicit Handle( T handle ) : m_Handle( handle ) {}
	Handle( Handle &&other ) noexcept : m_Handle( other.m_Handle ) { other.m_Handle = nullptr; }
	Handle &operator=( Handle &&other ) noexcept
	{
		if ( this != &other )
		{
			Reset();
			m_Handle = other.m_Handle;
			other.m_Handle = nullptr;
		}
		return *this;
	}
	Handle( const Handle & ) = delete;
	Handle &operator=( const Handle & ) = delete;
	~Handle() { Reset(); }
	void Reset( T handle = nullptr )
	{
		if ( m_Handle )
			ReleaseFn( m_Handle );
		m_Handle = handle;
	}
	T Get() const { return m_Handle; }
	explicit operator bool() const { return m_Handle != nullptr; }

private:
	T m_Handle = nullptr;
};

using Buffer = Handle<WGPUBuffer, wgpuBufferRelease>;
using Texture = Handle<WGPUTexture, wgpuTextureRelease>;
using TextureViewHandle = Handle<WGPUTextureView, wgpuTextureViewRelease>;
using Sampler = Handle<WGPUSampler, wgpuSamplerRelease>;
using BindGroupLayout = Handle<WGPUBindGroupLayout, wgpuBindGroupLayoutRelease>;
using BindGroupHandle = Handle<WGPUBindGroup, wgpuBindGroupRelease>;
using PipelineLayout = Handle<WGPUPipelineLayout, wgpuPipelineLayoutRelease>;
using RenderPipeline = Handle<WGPURenderPipeline, wgpuRenderPipelineRelease>;
using ComputePipeline = Handle<WGPUComputePipeline, wgpuComputePipelineRelease>;
using ShaderModule = Handle<WGPUShaderModule, wgpuShaderModuleRelease>;
using CommandBuffer = Handle<WGPUCommandBuffer, wgpuCommandBufferRelease>;

// One binding an artifact's stage uses (its "binding" line).
struct BindingLine
{
	enum class Kind : std::uint8_t
	{
		kUniform,
		kStorage,
		kReadOnlyStorage,
		kTexture,
		kSampler,
		kStorageTexture
	};
	std::uint32_t group = 0;
	std::uint32_t binding = 0;
	Kind kind = Kind::kUniform;
	WGPUTextureSampleType sampleType = WGPUTextureSampleType_Float;
	WGPUTextureViewDimension dimension = WGPUTextureViewDimension_2D;
	bool multisampled = false;
	WGPUSamplerBindingType sampler = WGPUSamplerBindingType_Filtering;
	WGPUStorageTextureAccess access = WGPUStorageTextureAccess_WriteOnly;
	WGPUTextureFormat storageFormat = WGPUTextureFormat_Undefined;
	WGPUShaderStage visibility = WGPUShaderStage_None;
};

// A specialization constant a stage declares (its "override" line).
struct OverrideLine
{
	enum class Type : std::uint8_t
	{
		kBool,
		kInt,
		kUint,
		kFloat
	};
	std::uint32_t id = 0;
	Type type = Type::kInt;
};

// What an artifact's header lines say.
struct ArtifactHeader
{
	std::vector<BindingLine> bindings;
	std::uint32_t drawConstantBytes = 0; // 0: the stage reads none
	std::vector<OverrideLine> overrides;
};
// False when a line is malformed.
bool ReadArtifactHeader( std::string_view text, ShaderStage stage, ArtifactHeader &header );

// A WebGPU bind group layout of the cache, shared by every pipeline whose
// group has the same entries.
struct GroupLayout
{
	std::string key;
	BindGroupLayout layout;
	std::vector<BindingLine> entries; // sorted by binding; the draw constants excluded
	bool drawConstants = false;
};

// Records ---------------------------------------------------------------------------

struct BufferRecord
{
	BufferDesc desc;
	Buffer buffer;
	std::uint64_t allocated = 0; // desc.size rounded up to 4
	// kReadback: the MapRead copy, and the bytes of the last completed copy.
	Buffer readback;
	std::vector<std::byte> shadow;
	bool mapPending = false;
	WGPUFuture mapFuture{}; // the pending map's, waited for before the copy is reused
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;
};

struct TextureRecord
{
	TextureDesc desc;
	Texture texture;
	std::uint32_t layers = 1; // array slices (a cube's six faces each)
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;
	bool padded = false; // a block-compressed texture made whole blocks larger
	// Views by (kind, dimension, aspect, mip, layer).
	std::map<std::uint64_t, TextureViewHandle> views;
	std::uint32_t Width( std::uint32_t mip ) const { return std::max( 1u, desc.width >> mip ); }
	std::uint32_t Height( std::uint32_t mip ) const { return std::max( 1u, desc.height >> mip ); }
};

struct SamplerRecord
{
	Sampler sampler;
	bool comparison = false;
	bool filtering = false;
	bool released = false;
};

struct LayoutRecord
{
	BindGroupRole role = BindGroupRole::kFrame;
	std::vector<BindingDesc> bindings;
	bool released = false;
};

struct BindGroupRecord
{
	BindGroupLayoutId layout;
	std::vector<BindingDesc> bindings;
	std::vector<BindGroupEntry> entries;
	// The WebGPU group per layout key it has been bound with.
	std::map<std::string, BindGroupHandle> built;
	bool released = false;
};

struct PipelineRecord
{
	PipelineKind kind = PipelineKind::kGraphics;
	RenderPipeline render;
	ComputePipeline compute;
	// The pipeline layout's groups, 0..groupCount-1; an unused group in
	// between has the empty layout.
	std::array<const GroupLayout *, kMaxBindGroups> groups{};
	std::uint32_t groupCount = 0;
	std::uint32_t stencilReference = 0;
	std::vector<Format> colorFormats;
	Format depthFormat = Format::kUnknown;
	std::uint32_t sampleCount = 1;
	std::uint32_t vertexBuffers = 0;
	std::uint32_t drawConstantBytes = 0;
	std::array<BindGroupLayoutId, kMaxBindGroups> layouts{};
	std::array<bool, kMaxBindGroups> layoutHasBindings{};
	bool released = false;
};

// One submission in flight: its token and the memory freed with it.
struct Submission
{
	std::uint64_t value = 0;
	std::vector<Buffer> buffers;
	std::vector<BindGroupHandle> groups;
	std::vector<std::uint64_t> readbacks; // buffer ids mapped after the work
	std::uint32_t pendingMaps = 0;
	bool workDone = false;
	bool failed = false;
};

using recording::Command;
using recording::Op;
using recording::RecordingEncoder;

// Device ---------------------------------------------------------------------------

class WebGpuDevice final : public IRenderDevice2,
                           private recording::IUploadStager,
                           private recording::IRecordedResources
{
public:
	explicit WebGpuDevice( const WebGpuAdapterOptions &options );
	~WebGpuDevice() override;

	DeviceResult<void> Initialize();

	const DeviceFacts &Facts() const override { return m_Facts; }
	DeviceState State() const override { return m_State.load(); }
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

	std::uint64_t ValidationErrors() const { return m_ValidationErrors.load(); }
	void SimulateLoss() { m_State.store( DeviceState::kLost ); }
	void Hold( bool held );

	// For the replay (execute.cpp).
	WGPUDevice Native() const { return m_Device; }
	const WGPULimits &NativeLimits() const { return m_Limits; }
	BufferRecord *ExistingBuffer( std::uint64_t id );
	TextureRecord *ExistingTexture( std::uint64_t id );
	const PipelineRecord *ExistingPipeline( std::uint64_t id ) const;
	BindGroupRecord *ExistingBindGroup( std::uint64_t id );
	const SamplerRecord *ExistingSampler( std::uint64_t id ) const;
	WGPUBindGroup EmptyGroup() const { return m_EmptyGroup.Get(); }
	// A view of one texture: kind 0 sampled (dimension, depth aspect), 1 one
	// attachment subresource (mip, layer).
	WGPUTextureView SampledView(
	    TextureRecord &texture, WGPUTextureViewDimension dimension, bool depthAspect );
	WGPUTextureView AttachmentView(
	    TextureRecord &texture, std::uint32_t mip, std::uint32_t layer );
	// The WebGPU group of a port group for a cached layout; nullptr if an
	// entry the layout needs is missing or of the wrong kind.
	WGPUBindGroup BuiltGroup( BindGroupRecord &group, const GroupLayout &layout );
	// A group-3 group with the submission's draw constants (and, when the
	// port bound one, its draw group's entries); kept by the submission.
	WGPUBindGroup ConstantsGroup( BindGroupRecord *group, const GroupLayout &layout,
	    WGPUBuffer constants, Submission &submission );
	void CountError( std::string_view message );
	// The internal pipeline that writes a depth texture's region from a
	// buffer of 32-bit floats (WebGPU copies no buffer into depth), and its
	// group layout: (storage buffer, region uniform).
	struct DepthUpload
	{
		BindGroupLayout layout;
		RenderPipeline pipeline;
	};
	const DepthUpload *DepthUploadFor( WGPUTextureFormat format );

private:
	// recording::IUploadStager: uploads travel in their commands.
	void StageUpload(
	    RecordingEncoder &encoder, Command &command, std::span<const std::byte> bytes ) override;
	void AbandonUploads( const std::vector<std::uint64_t> & ) override {}

	// recording::IRecordedResources (the device lock is held).
	std::optional<recording::TextureView> Texture( std::uint64_t id ) const override;
	std::optional<recording::BufferView> Buffer( std::uint64_t id ) const override;
	std::optional<recording::PipelineView> Pipeline( std::uint64_t id ) const override;
	std::optional<BindGroupLayoutId> BindGroup( std::uint64_t id ) const override;
	std::uint32_t MaxColorAttachments() const override
	{
		return m_Facts.limits.maxColorAttachments;
	}
	std::uint32_t MaxVertexSlots() const override { return kMaxVertexSlots; }
	// Clears are render passes: the format must be renderable.
	bool CanClear( const recording::TextureView &texture ) const override;
	// WebGPU copies no buffer into a depth texture.
	bool CanCopyWithBuffer( const recording::TextureView &texture ) const override;

	struct PendingRelease
	{
		ResourceId resource;
		CompletionToken token;
	};

	DeviceResult<void> OpenDevice();
	void QueryFacts();
	void ProcessEvents() const;
	bool WaitFor( WGPUFuture future ) const;
	std::size_t Collect();
	void Erase( ResourceId resource );
	void OnWorkDone( std::uint32_t epoch, std::uint64_t value, bool failed );
	void OnMapped( std::uint32_t epoch, std::uint64_t value, std::uint64_t buffer, bool ok );
	void Advance();
	void SubmitNative( std::unique_ptr<Submission> submission, CommandBuffer commands );
	BufferRecord *LiveBuffer( std::uint64_t id );
	TextureRecord *LiveTexture( std::uint64_t id );
	std::optional<LayoutView> FindLayout( BindGroupLayoutId id ) const;
	bool *ReleasedFlag( ResourceId resource );
	void ClearResources();

	// pipelines.cpp
	const GroupLayout *CachedLayout( std::vector<BindingLine> entries, bool drawConstants );
	DeviceResult<void> BuildShader( const ShaderArtifactView &artifact, const PipelineDesc &desc,
	    ShaderModule &module, ArtifactHeader &header, std::deque<std::string> &keys,
	    std::vector<WGPUConstantEntry> &constants );

	// execute.cpp: the submission's command buffer, or nullptr (an error the
	// replay could not express, named on stderr).
	CommandBuffer Execute( std::vector<RecordingEncoder *> &encoders, Submission &submission );

	WebGpuAdapterOptions m_Options;
	WGPUInstance m_Instance = nullptr;
	WGPUAdapter m_Adapter = nullptr;
	WGPUDevice m_Device = nullptr;
	WGPUQueue m_Queue = nullptr;
	WGPULimits m_Limits = WGPU_LIMITS_INIT;
	std::vector<WGPUFeatureName> m_Features;
	std::string m_AdapterName;
	DeviceFacts m_Facts;
	std::atomic<DeviceState> m_State{ DeviceState::kAvailable };
	std::uint32_t m_Epoch = 1;
	std::uint64_t m_NextId = 0;
	std::atomic<std::uint64_t> m_ValidationErrors{ 0 };

	std::unordered_map<std::uint64_t, BufferRecord> m_Buffers;
	std::unordered_map<std::uint64_t, TextureRecord> m_Textures;
	std::unordered_map<std::uint64_t, SamplerRecord> m_Samplers;
	std::unordered_map<std::uint64_t, LayoutRecord> m_Layouts;
	std::unordered_map<std::uint64_t, BindGroupRecord> m_BindGroups;
	std::unordered_map<std::uint64_t, PipelineRecord> m_Pipelines;
	std::map<std::string, GroupLayout> m_GroupLayouts;
	const GroupLayout *m_EmptyLayout = nullptr;
	BindGroupHandle m_EmptyGroup;
	std::map<WGPUTextureFormat, DepthUpload> m_DepthUploads;
	std::vector<PendingRelease> m_Releases;

	std::uint64_t m_Submitted = 0;
	std::atomic<std::uint64_t> m_Completed{ 0 };
	std::deque<std::unique_ptr<Submission>> m_InFlight;
	bool m_Held = false;
	std::vector<std::pair<std::unique_ptr<Submission>, CommandBuffer>> m_HeldSubmissions;
	mutable std::recursive_mutex m_Lock;
};

} // namespace render::device::webgpu

#endif // RENDER_DEVICE_WEBGPU_WEBGPU_DEVICE_H
