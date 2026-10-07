//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica's device over citro3d (RFC 0026 P1-P2). Built
//			for the 3DS alone (__3DS__); every other build has only the
//			portable parts and refuses creation.
//
//			Execution (RFC 0026 decision 6): encoders record CPU command lists
//			(recording.h); Submit validates a submission and replays it at
//			once, in order. Copies, writes and clears run on the CPU against
//			linear memory; draws are batched into citro3d frames. Before any
//			CPU access the open frame is submitted and the GPU drained, and the
//			data cache invalidated; citro3d flushes it before each frame runs.
//			Every token is therefore complete when Submit returns.
//
//=============================================================================//

#ifndef RENDER_DEVICE_PICA_PICA_DEVICE_H
#define RENDER_DEVICE_PICA_PICA_DEVICE_H

#if defined( __3DS__ )

#include "render/device/pica_format.h"
#include "render/device/pica_codes.h"
#include "texel_layout.h"

#include "render/device/pica/provider.h"
#include "render/device/recording.h"
#include "render/device/validation.h"

#include <3ds.h>
#include <citro3d.h>

#include <array>
#include <deque>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace render::device::pica
{

// The kPica forms and codes (render.device).
using namespace ::render::device::pica_format;

class Replayer;

struct BufferRecord
{
	BufferDesc desc;
	std::byte *data = nullptr; // linear memory
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;
};

struct TextureRecord
{
	TextureDesc desc;
	TextureLayout layout;
	std::byte *data = nullptr; // linear memory, every level
	C3D_Tex sampled{};         // the GPU's description when sampleable
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;
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

struct BindGroupRecord
{
	BindGroupLayoutId layout;
	std::vector<BindGroupEntry> entries;
	bool released = false;
};

struct AttributeLoad
{
	std::uint8_t location = 0;
	GPU_FORMATS format = GPU_FLOAT;
	std::uint8_t components = 0;
	std::uint32_t offset = 0;
	std::uint32_t bytes = 0;
};

struct PipelineRecord
{
	std::vector<u32> dvlbWords; // the DVLB DVLB_ParseFile keeps pointers into
	DVLB_s *dvlb = nullptr;
	shaderProgram_s program{};
	VertexProgram vertex;              // its dvlb view points at dvlbWords
	FragmentProgram fragment;          // empty stages: primary colour
	std::vector<CombinerStage> stages; // after specialization
	std::array<std::vector<AttributeLoad>, kInputRegisters> slotAttributes; // by vertex slot
	std::array<std::uint32_t, kInputRegisters> strides{};
	std::uint32_t vertexBuffers = 0; // one past the highest slot used
	GPU_Primitive_t topology = GPU_TRIANGLES;
	RasterState raster;
	DepthStencilState depthStencil;
	std::vector<Format> colorFormats;
	BlendFactors blend;
	std::uint8_t writeMask = kColorWriteAll;
	Format depthFormat = Format::kUnknown;
	std::uint32_t drawConstantBytes = 0;
	std::array<BindGroupLayoutId, kMaxBindGroups> layouts{};
	std::array<bool, kMaxBindGroups> layoutHasBindings{};
	bool released = false;
};

class PicaDevice final : public IRenderDevice2,
                         public recording::IUploadStager,
                         public recording::IRecordedResources
{
public:
	explicit PicaDevice( const PicaAdapterOptions &options );
	~PicaDevice() override;

	DeviceResult<void> Initialize();
	void SimulateLoss();
	// provider.h's MapUploadBuffer and FlushUploadBuffer.
	std::span<std::byte> MapUploadBuffer( BufferId buffer );
	void FlushUploadBuffer( BufferId buffer, std::uint64_t offset, std::uint64_t size );
	// presenter.cpp (provider.h's PresentTopScreen).
	DeviceResult<void> PresentTopScreen(
	    TextureId source, std::uint32_t width, std::uint32_t height );

	// IRenderDevice2
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

	// recording::IUploadStager: uploads stay in the command (the replay is a
	// CPU copy; no ring range outlives Submit).
	void StageUpload( recording::RecordingEncoder &encoder, recording::Command &command,
	    std::span<const std::byte> bytes ) override;
	void AbandonUploads( const std::vector<std::uint64_t> & ) override {}

	// recording::IRecordedResources
	std::optional<recording::TextureView> Texture( std::uint64_t id ) const override;
	std::optional<recording::BufferView> Buffer( std::uint64_t id ) const override;
	std::optional<recording::PipelineView> Pipeline( std::uint64_t id ) const override;
	std::optional<BindGroupLayoutId> BindGroup( std::uint64_t id ) const override;
	std::uint32_t MaxColorAttachments() const override { return 1; }
	std::uint32_t MaxVertexSlots() const override { return kInputRegisters - 1; }

private:
	friend class Replayer;

	struct PendingRelease
	{
		ResourceId resource;
		CompletionToken token;
	};

	bool Live( ResourceId resource ) const;
	bool *ReleasedFlag( ResourceId resource );
	void Erase( ResourceId resource );
	void EraseAll();
	std::size_t Collect();
	std::optional<LayoutView> FindLayout( BindGroupLayoutId id ) const;
	// The PICA's own limits on a validated submission (RFC 0026 decision 4):
	// nullopt when it may run.
	std::optional<DeviceStatus> CheckPicaLimits(
	    std::span<const recording::RecordingEncoder *const> encoders ) const;

	// GPU frames (pica_device.cpp): the replay's draws go into one open
	// citro3d frame; Drain submits it and waits.
	void OpenFrame();
	void Drain();
	// The CPU is about to read or write these resources: drains first when
	// work the GPU may not have finished uses one of them.
	void BeforeCpuAccess( std::initializer_list<std::uint64_t> resources );

	PicaAdapterOptions m_Options;
	DeviceFacts m_Facts;
	DeviceState m_State = DeviceState::kAvailable;
	std::uint32_t m_Epoch = 1;
	std::uint64_t m_NextId = 0;
	std::uint64_t m_Submitted = 0;
	std::uint64_t m_Completed = 0;
	bool m_Context = false;                 // this device holds a reference on citro3d
	bool m_FrameOpen = false;               // a citro3d frame is recording
	bool m_GpuPending = false;              // a submitted frame may still run
	std::deque<C3D_RenderTarget> m_Targets; // the open frame's passes
	// Every buffer and texture a draw since the last drain reads or writes on
	// the GPU (vertex, index and sampled resources, attachments). Uniform
	// buffers are not among them: the replay reads those on the CPU when it
	// records a draw.
	std::unordered_set<std::uint64_t> m_GpuUses;
	// The top screen's target and the presenter's quad (presenter.cpp),
	// made at the first present.
	C3D_RenderTarget *m_Screen = nullptr;
	float *m_PresentQuad = nullptr;
	std::unordered_map<std::uint64_t, BufferRecord> m_Buffers;
	std::unordered_map<std::uint64_t, TextureRecord> m_Textures;
	std::unordered_map<std::uint64_t, SamplerRecord> m_Samplers;
	std::unordered_map<std::uint64_t, LayoutRecord> m_Layouts;
	std::unordered_map<std::uint64_t, BindGroupRecord> m_BindGroups;
	std::unordered_map<std::uint64_t, std::unique_ptr<PipelineRecord>> m_Pipelines;
	std::vector<PendingRelease> m_Releases;
	mutable std::recursive_mutex m_Lock;
};

// What citro3d's one context keeps pointers to, which therefore outlives
// every device (pica_device.cpp): citro3d reads the bound program and each
// texture unit's description again at the next bind or draw, after a device
// may be gone. A pipeline erased while its program is bound is retired here
// and freed once another program is bound (the command buffer holds a copy
// of what the GPU runs).
struct SharedContext
{
	float *vertexIndices = nullptr; // 0, 1, 2, ... for PVS1's vertex index input
	std::byte *placeholderTexels = nullptr;
	C3D_Tex placeholder{}; // unused units (C3D_TexBind takes no null)
	C3D_Tex units[kMaxTextureUnits]{};
	const shaderProgram_s *bound = nullptr;
	std::unique_ptr<PipelineRecord> retired;
	// The presenter's program (present.v.pica), made at the first present.
	DVLB_s *presentDvlb = nullptr;
	shaderProgram_s presentProgram{};
};
SharedContext &Shared();
// Binds a program and frees a retired pipeline no longer bound.
void BindProgram( shaderProgram_s &program );
inline void BindProgram( PipelineRecord &pipeline )
{
	BindProgram( pipeline.program );
}

// Runs one validated encoder's commands in order (replay.cpp).
void ReplayCommands( PicaDevice &device, const std::vector<recording::Command> &commands );

// The highest vertex index PVS1's vertex index input reaches.
inline constexpr std::uint32_t kVertexIndexCount = 65536;

} // namespace render::device::pica

#endif // __3DS__

#endif // RENDER_DEVICE_PICA_PICA_DEVICE_H
