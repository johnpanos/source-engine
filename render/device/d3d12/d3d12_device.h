//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.d3d12 private: the Direct3D 12 adapter of
//			render.device.v2 (RFC 0024). See public/render/device/d3d12/provider.h.
//
//			Model (the GL adapter's, render/device/gl/gl_device.h):
//			- Encoders record CPU command lists; uploads are copied into the
//			  adapter's persistently mapped upload-heap ring as they are
//			  recorded, or kept in the command when the ring is full.
//			- Submit validates every encoder against the usage state the
//			  previous accepted submission left, then replays the lists into
//			  one ID3D12GraphicsCommandList and signals the fence with the
//			  submission's token value.
//			- Each port usage maps to one D3D12 resource state; the replay
//			  tracks the actual state per resource and issues whole-resource
//			  transition barriers. Upload-heap buffers stay GENERIC_READ and
//			  readback-heap buffers COPY_DEST, as D3D12 requires.
//			- Copies between textures and tightly packed port buffers go
//			  through a 256-byte-pitch scratch buffer when the port's layout
//			  does not meet D3D12's footprint alignment.
//
//			The command list, its encoder, the upload ring's bookkeeping and the
//			validator are render.device's shared recording helper
//			(public/render/device/recording.h); this adapter supplies the upload
//			memory and the resource views, and owns the replay.
//
//=============================================================================//

#ifndef RENDER_DEVICE_D3D12_D3D12_DEVICE_H
#define RENDER_DEVICE_D3D12_D3D12_DEVICE_H

#include "render/device/d3d12/host_device.h"
#include "render/device/d3d12/provider.h"
#include "render/device/recording.h"
#include "render/device/validation.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace render::device::d3d12
{

// A COM reference, released on destruction.
template <typename T> class Com
{
public:
	Com() = default;
	Com( const Com & ) = delete;
	Com &operator=( const Com & ) = delete;
	Com( Com &&other ) noexcept : m_P( other.m_P ) { other.m_P = nullptr; }
	Com &operator=( Com &&other ) noexcept
	{
		if ( this != &other )
		{
			Reset();
			m_P = other.m_P;
			other.m_P = nullptr;
		}
		return *this;
	}
	~Com() { Reset(); }
	void Reset()
	{
		if ( m_P )
			m_P->Release();
		m_P = nullptr;
	}
	T *Get() const { return m_P; }
	T *operator->() const { return m_P; }
	explicit operator bool() const { return m_P != nullptr; }
	T **Put()
	{
		Reset();
		return &m_P;
	}
	void **PutVoid() { return reinterpret_cast<void **>( Put() ); }

private:
	T *m_P = nullptr;
};

inline constexpr std::uint32_t kMaxVertexSlots = 16;

foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, std::int32_t nativeCode = 0 );

// A port format in D3D12: the resource's format (typeless for depth, so it
// can be both attached and sampled), its attachment view's, its shader view's,
// and the format of its plane-0 copy footprint.
struct DxFormat
{
	DXGI_FORMAT resource = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT attachment = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT view = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT copy = DXGI_FORMAT_UNKNOWN;
};
DxFormat FormatOf( Format format );
void SetName( ID3D12Object *object, std::string_view name );
D3D12_RESOURCE_STATES StateOf( ResourceUsage usage, bool depth );

// Records -----------------------------------------------------------------------

struct BufferRecord
{
	BufferDesc desc;
	Com<ID3D12Resource> resource;
	// The heap's fixed state (upload GENERIC_READ, readback COPY_DEST) or the
	// replay's current state.
	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
	bool fixedState = false;
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;
};

struct TextureRecord
{
	TextureDesc desc;
	Com<ID3D12Resource> resource;
	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
	std::uint32_t layers = 1;
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;

	std::uint32_t Width( std::uint32_t mip ) const { return std::max( 1u, desc.width >> mip ); }
	std::uint32_t Height( std::uint32_t mip ) const { return std::max( 1u, desc.height >> mip ); }
	std::uint32_t Subresource( std::uint32_t mip, std::uint32_t layer ) const
	{
		return mip + layer * desc.mipLevels;
	}
};

struct SamplerRecord
{
	SamplerDesc desc;
	bool released = false;
};

struct LayoutRecord
{
	BindGroupRole role = BindGroupRole::kFrame;
	std::vector<BindingDesc> bindings;
	bool released = false;
};

// A bind group's descriptors: one range of the shader-visible view heap
// (its CBV, SRV and UAV bindings in layout order, each binding's count
// contiguous) and one of the sampler heap. Written once at creation.
struct BindGroupRecord
{
	BindGroupLayoutId layout;
	std::vector<BindingDesc> bindings;
	std::vector<BindGroupEntry> entries;
	std::uint32_t viewBase = 0;
	std::uint32_t viewCount = 0;
	std::uint32_t samplerBase = 0;
	std::uint32_t samplerCount = 0;
	bool released = false;
};

// Root parameters of a pipeline's root signature: per role, the view table
// and the sampler table (-1: none); the draw constants (b0, space15) and
// SPIRV-Cross's base vertex and instance (b1, space15) as root constants.
struct RootLayout
{
	std::array<std::int32_t, kMaxBindGroups> views{ -1, -1, -1, -1 };
	std::array<std::int32_t, kMaxBindGroups> samplers{ -1, -1, -1, -1 };
	std::int32_t drawConstants = -1;
	std::int32_t vertexInfo = -1;
};

struct PipelineRecord
{
	PipelineKind kind = PipelineKind::kGraphics;
	Com<ID3D12RootSignature> root;
	Com<ID3D12PipelineState> state;
	RootLayout params;
	D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	std::array<std::uint32_t, kMaxVertexSlots> strides{};
	std::uint8_t stencilReference = 0;
	std::vector<Format> colorFormats;
	Format depthFormat = Format::kUnknown;
	std::uint32_t sampleCount = 1;
	std::uint32_t vertexBuffers = 0;
	std::uint32_t drawConstantBytes = 0;
	std::array<BindGroupLayoutId, kMaxBindGroups> layouts{};
	std::array<bool, kMaxBindGroups> layoutHasBindings{};
	bool released = false;
};

// First-fit ranges of a descriptor heap; freed ranges merge.
class DescriptorRanges
{
public:
	void Reset( std::uint32_t capacity )
	{
		m_Free.clear();
		if ( capacity )
			m_Free.push_back( { 0, capacity } );
	}
	std::optional<std::uint32_t> Allocate( std::uint32_t count );
	void Free( std::uint32_t base, std::uint32_t count );

private:
	struct Range
	{
		std::uint32_t base;
		std::uint32_t count;
	};
	std::vector<Range> m_Free;
};

using recording::Command;
using recording::Op;
using recording::RecordingEncoder;

// Device -------------------------------------------------------------------------

class D3d12Device final : public IRenderDevice2,
                          private recording::IUploadStager,
                          private recording::IRecordedResources
{
public:
	explicit D3d12Device( const D3d12AdapterOptions &options );
	// Hosted (host_device.h): the device and queue are the host's.
	D3d12Device( const D3d12AdapterOptions &options, IDXGIFactory4 *factory, ID3D12Device *device,
	    ID3D12CommandQueue *queue );
	~D3d12Device() override;

	DeviceResult<void> Initialize();

	const DeviceFacts &Facts() const override { return m_Facts; }
	DeviceState State() const override { return m_State; }
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

	std::uint64_t ValidationMessages() const { return m_Messages.load(); }
	DeviceResult<TextureId> Import( ID3D12Resource *texture, const TextureDesc &desc,
	    ResourceUsage home );
	std::uint64_t DeferredUploads() const;
	void SimulateLoss();
	void Hold( bool held );
	ID3D12DescriptorHeap *ViewHeap() const { return m_ViewHeap.Get(); }
	ID3D12DescriptorHeap *SamplerHeap() const { return m_SamplerHeap.Get(); }
	D3D12_GPU_DESCRIPTOR_HANDLE ViewGpu( std::uint32_t index ) const;
	D3D12_GPU_DESCRIPTOR_HANDLE SamplerGpu( std::uint32_t index ) const;


private:
	friend class Replay;

	// recording::IUploadStager (any recording thread; no D3D12 call).
	void StageUpload(
	    RecordingEncoder &encoder, Command &command, std::span<const std::byte> bytes ) override;
	void AbandonUploads( const std::vector<std::uint64_t> &allocations ) override;
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
	bool CanCopyWithBuffer( const recording::TextureView &texture ) const override;

	struct PendingRelease
	{
		ResourceId resource;
		CompletionToken token;
	};
	// Objects one submission used (its allocator, list, scratch buffers),
	// kept until its fence value completes.
	struct InFlight
	{
		std::uint64_t value = 0;
		Com<ID3D12CommandAllocator> allocator;
		Com<ID3D12GraphicsCommandList> list;
		std::vector<Com<ID3D12Resource>> scratch;
	};
	struct HeldSubmission
	{
		std::vector<std::unique_ptr<IEncoderBackend>> backends;
		std::vector<RecordingEncoder *> encoders;
		CompletionToken token;
	};

	DeviceResult<void> CreateDeviceObjects();
	DeviceResult<void> OpenDeviceAndQueue();
	void DestroyDeviceObjects();
	void QueryFacts();
	void CheckRemoved();
	void CountDebugMessages();
	void UpdateCompletion() const;
	std::size_t Collect();
	void Erase( ResourceId resource );
	DeviceResult<Com<ID3D12Resource>> CommittedBuffer(
	    std::uint64_t size, D3D12_HEAP_TYPE heap, D3D12_RESOURCE_STATES state,
	    bool unordered = false );

	BufferRecord *LiveBuffer( std::uint64_t id );
	TextureRecord *LiveTexture( std::uint64_t id );
	BufferRecord *ExistingBuffer( std::uint64_t id );
	TextureRecord *ExistingTexture( std::uint64_t id );
	std::optional<LayoutView> FindLayout( BindGroupLayoutId id ) const;
	bool *ReleasedFlag( ResourceId resource );

	// pipelines.cpp
	DeviceResult<void> LoadCompiler();
	DeviceResult<std::vector<std::byte>> CompileStage( const ShaderArtifactView &stage,
	    std::span<const SpecializationConstant> constants, std::int32_t &nativeCode );
	DeviceResult<void> WriteGroupDescriptors( BindGroupRecord &group );
	D3D12_CPU_DESCRIPTOR_HANDLE ViewCpu( std::uint32_t index ) const;
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerCpu( std::uint32_t index ) const;

	ID3D12CommandSignature *IndirectSignature( std::uint32_t stride );

	// execute.cpp
	DeviceResult<void> Issue( std::vector<RecordingEncoder *> &encoders, CompletionToken token );
	void IssueHeld();

	D3d12AdapterOptions m_Options;
	bool m_Hosted = false;
	Com<IDXGIFactory4> m_Factory;
	Com<IDXGIAdapter1> m_Adapter;
	Com<ID3D12Device> m_Device;
	Com<ID3D12CommandQueue> m_Queue;
	Com<ID3D12Fence> m_Fence;
	Com<ID3D12InfoQueue> m_InfoQueue;
	Com<ID3D12DescriptorHeap> m_RtvHeap; // replay scratch: one slot per color attachment
	Com<ID3D12DescriptorHeap> m_DsvHeap;
	Com<ID3D12QueryHeap> m_Timestamps;
	Com<ID3D12DescriptorHeap> m_ViewHeap;    // shader-visible CBV/SRV/UAV
	Com<ID3D12DescriptorHeap> m_SamplerHeap; // shader-visible samplers
	std::uint32_t m_ViewStride = 0;
	std::uint32_t m_SamplerStride = 0;
	DescriptorRanges m_ViewRanges;
	DescriptorRanges m_SamplerRanges;
	// ExecuteIndirect signatures of indexed draws, by record stride (D30).
	std::unordered_map<std::uint32_t, Com<ID3D12CommandSignature>> m_IndirectSignatures;
	// The pinned DXC (dxcompiler.dll, an import); compiled stages cached by
	// their text, stage and constants.
	Com<IUnknown> m_Compiler;
	std::unordered_map<std::string, std::vector<std::byte>> m_Programs;
	std::uint32_t m_RtvStride = 0;
	std::uint32_t m_TimestampSlots = 0;
	std::uint32_t m_NextTimestamp = 0;
	void *m_Event = nullptr;
	std::string m_AdapterName;
	DeviceFacts m_Facts;
	DeviceState m_State = DeviceState::kAvailable;
	std::uint32_t m_Epoch = 1;
	std::uint64_t m_NextId = 0;
	std::unordered_map<std::uint64_t, BufferRecord> m_Buffers;
	std::unordered_map<std::uint64_t, TextureRecord> m_Textures;
	std::unordered_map<std::uint64_t, SamplerRecord> m_Samplers;
	std::unordered_map<std::uint64_t, LayoutRecord> m_Layouts;
	std::unordered_map<std::uint64_t, BindGroupRecord> m_BindGroups;
	std::unordered_map<std::uint64_t, PipelineRecord> m_Pipelines;
	std::vector<PendingRelease> m_Releases;
	std::deque<InFlight> m_InFlight;
	std::vector<Com<ID3D12CommandAllocator>> m_FreeAllocators;
	std::uint64_t m_Submitted = 0;
	mutable std::atomic<std::uint64_t> m_Completed{ 0 };
	Com<ID3D12Resource> m_Ring;
	std::byte *m_RingData = nullptr;
	recording::UploadRing m_RingState;
	mutable std::mutex m_RingLock;
	std::uint64_t m_NextAllocation = 0;
	std::uint64_t m_DeferredUploads = 0;
	std::atomic<std::uint64_t> m_Messages{ 0 };
	std::uint64_t m_MessagesSeen = 0;
	mutable std::recursive_mutex m_Lock;
	bool m_Holding = false;
	std::deque<HeldSubmission> m_Held;
};

} // namespace render::device::d3d12

#endif // RENDER_DEVICE_D3D12_D3D12_DEVICE_H
