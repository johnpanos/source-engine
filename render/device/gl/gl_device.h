//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl private: the OpenGL 4.5 adapter of
//			render.device.v2 (RFC 0016 K10). See public/render/device/gl/provider.h.
//
//			Model:
//			- Encoders record CPU command lists (any thread; uploads are copied
//			  into the adapter's persistently mapped ring as they are recorded),
//			  the port's shared recording (render/device/recording.h).
//			- Submit validates every encoder against the usage state the
//			  previous accepted submission left (recording::Validate), then replays the lists in
//			  order on the calling thread, with the context current, and ends
//			  the submission with a fence sync object: its token. A held
//			  device (tests only, HoldSubmissions) keeps accepted submissions
//			  unissued until it is released.
//			- Tokens complete in order (one context, one command stream). Poll
//			  frees released resources and ring ranges behind completed fences.
//			- Bind groups map to fixed ranges of GL's flat slots
//			  (kSlotsPerGroup per group per kind, as the GLSL 4.50 artifacts are
//			  built: tools/render/shader_artifacts.py cross_compile); the draw
//			  constants (D16) are a uniform block at kDrawConstantsSlot.
//			- The conventions (RFC 0016 decision 6) come from glClipControl
//			  (GL_UPPER_LEFT, GL_ZERO_TO_ONE): clip +Y lands on row 0, and clip
//			  depth runs 0 to 1.
//
//			Resource state is tracked per resource, not per subresource.
//
//=============================================================================//

#ifndef RENDER_DEVICE_GL_GL_DEVICE_H
#define RENDER_DEVICE_GL_GL_DEVICE_H

#include "gl_context.h"
#include "render/device/gl/provider.h"
#include "render/device/recording.h"
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
#include <vector>

namespace render::device::gl
{

GLenum CompareFunction( CompareOp op );

// The artifacts' slot layout (shader_artifacts.py GL_SLOTS_PER_GROUP and
// GL_DRAW_CONSTANTS_SLOT; the names below are its GL_TEXTURE_PREFIX,
// GL_SAMPLER_PREFIX, GL_DRAW_CONSTANTS_BLOCK and GL_SPECIALIZATION_LINE).
inline constexpr std::uint32_t kSlotsPerGroup = 16;
inline constexpr std::uint32_t kDrawConstantsSlot = kMaxBindGroups * kSlotsPerGroup;
inline constexpr const char *kCombinedPrefix = "SPIRV_Cross_Combined";
inline constexpr const char *kTexturePrefix = "rg_t";
inline constexpr const char *kSamplerPrefix = "rg_s";
inline constexpr const char *kDummySampler = "SPIRV_Cross_DummySampler";
inline constexpr const char *kBaseInstanceUniform = "SPIRV_Cross_BaseInstance";
inline constexpr const char *kSpecializationLine = "// render.device.gl specialization";
inline constexpr std::uint32_t kMaxVertexSlots = 16;

inline constexpr std::uint32_t FlatSlot( std::uint32_t group, std::uint32_t binding )
{
	return group * kSlotsPerGroup + binding;
}

// A port format in GL: its internal format, and the format and type its
// texels take in transfers (for a BGRA format the transfer swizzles, so the
// bytes in buffers are the port's).
struct GlFormat
{
	GLenum internal = 0;
	GLenum format = 0;
	GLenum type = 0;
	bool compressed = false;
};
GlFormat FormatOf( Format format );
GLenum TextureTarget( const TextureDesc &desc );

foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, std::int32_t nativeCode = 0 );

// Records -----------------------------------------------------------------------

struct BufferRecord
{
	BufferDesc desc;
	GLuint name = 0;
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;
};

struct TextureRecord
{
	TextureDesc desc;
	GLuint name = 0;
	GLenum target = 0;
	std::uint32_t layers = 1;
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;

	std::uint32_t Width( std::uint32_t mip ) const { return std::max( 1u, desc.width >> mip ); }
	std::uint32_t Height( std::uint32_t mip ) const { return std::max( 1u, desc.height >> mip ); }
};

struct SamplerRecord
{
	GLuint name = 0;
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
	bool released = false;
};

// A linked program, shared by the pipelines whose stages and constants are
// the same (GL has no pipeline objects: the adapter caches programs).
struct Program
{
	// A combined sampler SPIRV-Cross made: its texture unit (the texture's
	// flat slot) and the sampler binding it pairs with, if any.
	struct Combined
	{
		GLint unit = 0;
		std::uint32_t samplerGroup = 0;
		std::uint32_t samplerBinding = 0;
		bool sampler = false;
	};
	GLuint name = 0;
	std::vector<Combined> combined;
	GLint baseInstance = -1; // SPIRV_Cross_BaseInstance, when the stages read the instance index
	std::uint32_t users = 0;
};

struct PipelineRecord
{
	PipelineKind kind = PipelineKind::kGraphics;
	std::string programKey;
	const Program *program = nullptr;
	GLuint vertexArray = 0;
	std::array<std::uint32_t, kMaxVertexSlots> strides{};
	std::array<bool, kMaxVertexSlots> perInstance{}; // ES: offset by the first instance
	std::uint32_t vertexBuffers = 0;
	GLenum topology = 0;
	RasterState raster;
	DepthStencilState depthStencil;
	std::vector<Format> colorFormats;
	std::vector<BlendMode> blends;
	std::vector<std::uint8_t> writeMasks;
	Format depthFormat = Format::kUnknown;
	std::uint32_t sampleCount = 1;
	std::uint32_t drawConstantBytes = 0;
	std::array<BindGroupLayoutId, kMaxBindGroups> layouts{};
	std::array<bool, kMaxBindGroups> layoutHasBindings{};
	bool released = false;
};

using recording::Command;
using recording::Op;
using recording::RecordingEncoder;
using recording::UploadRing;

// Device -------------------------------------------------------------------------

class GlDevice final : public IRenderDevice2,
                       private recording::IUploadStager,
                       private recording::IRecordedResources
{
public:
	explicit GlDevice( const GlAdapterOptions &options );
	~GlDevice() override;

	// Creates the context and the device's GL objects; the reason on failure.
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

	// For the provider's hooks.
	std::uint64_t ValidationMessages() const { return m_Messages.load(); }
	std::uint64_t DeferredUploads() const;
	void SimulateLoss();
	void CountMessage();
	// Tests only (gl::HoldSubmissions): while held, Submit accepts work and
	// returns its token but issues nothing to GL; releasing issues the held
	// submissions in order.
	void Hold( bool held );


private:
	friend class Replay;

	// recording::IUploadStager: copies an upload into the ring, or into the
	// command when the ring is full. Any recording thread; no GL call.
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
	bool CanCopyTexture( const recording::TextureView &texture ) const override;

	struct PendingRelease
	{
		ResourceId resource;
		CompletionToken token;
	};
	struct Fence
	{
		std::uint64_t value;
		GLsync sync;
	};
	// A GL object the device made for one submission (its draw constants, or
	// a timestamp query), deleted once the submission completes.
	struct Transient
	{
		GLuint buffer;
		std::uint64_t value;
		bool query = false; // `buffer` names a query object
	};
	// A submission accepted while held: its encoders, kept until issued.
	struct HeldSubmission
	{
		std::vector<std::unique_ptr<IEncoderBackend>> backends;
		std::vector<RecordingEncoder *> encoders;
		CompletionToken token;
	};

	const GlApi &Gl() const { return m_Context->Api(); }
	// The ES dialect (RFC 0022); Es() only on an ES device, context current.
	bool IsEs() const { return m_Options.api == GlApiKind::kEs31; }
	EsState &Es() const { return *m_Context->Es(); }
	DeviceResult<void> CheckEsRequirements( GLint major, GLint minor );
	bool EsFormatSupported( const TextureDesc &desc, bool attachment ) const;
	DeviceResult<void> CreateContextObjects();
	void DestroyContextObjects();
	void QueryFacts();
	void CheckReset();
	// Queries fences, in order, and advances m_Completed. Context current.
	void UpdateCompletion() const;
	std::size_t Collect();
	void Erase( ResourceId resource );

	BufferRecord *LiveBuffer( std::uint64_t id );
	TextureRecord *LiveTexture( std::uint64_t id );
	BufferRecord *ExistingBuffer( std::uint64_t id );
	TextureRecord *ExistingTexture( std::uint64_t id );
	std::optional<LayoutView> FindLayout( BindGroupLayoutId id ) const;
	bool *ReleasedFlag( ResourceId resource );
	bool QueueSupported( QueueKind queue ) const;

	// pipelines.cpp
	DeviceResult<const Program *> AcquireProgram(
	    const PipelineDesc &desc, std::string &key, std::uint32_t &nativeCode );
	void ReleaseProgram( const std::string &key );

	// execute.cpp
	void Execute( std::vector<RecordingEncoder *> &encoders, CompletionToken token );
	// Replays one submission and ends it with its fence. Context current.
	void Issue( std::vector<RecordingEncoder *> &encoders, CompletionToken token );
	// Issues every held submission, in order, and flushes. Context current.
	void IssueHeld();
	GLuint Framebuffer( const std::vector<std::uint64_t> &colors, std::uint64_t depth );
	void ForgetFramebuffers( std::uint64_t texture );

	GlAdapterOptions m_Options;
	std::unique_ptr<EglContext> m_Context;
	std::string m_AdapterName;
	DeviceFacts m_Facts;
	bool m_Anisotropy = false;
	DeviceState m_State = DeviceState::kAvailable;
	std::uint32_t m_Epoch = 1;
	std::uint64_t m_NextId = 0;
	std::unordered_map<std::uint64_t, BufferRecord> m_Buffers;
	std::unordered_map<std::uint64_t, TextureRecord> m_Textures;
	std::unordered_map<std::uint64_t, SamplerRecord> m_Samplers;
	std::unordered_map<std::uint64_t, LayoutRecord> m_Layouts;
	std::unordered_map<std::uint64_t, BindGroupRecord> m_BindGroups;
	std::unordered_map<std::uint64_t, PipelineRecord> m_Pipelines;
	std::map<std::string, Program> m_Programs;
	std::map<std::vector<std::uint64_t>, GLuint> m_Framebuffers; // colors..., depth (0: none)
	GLuint m_CopyFramebuffers[2] = {}; // D37's read and draw scratch framebuffers
	std::vector<PendingRelease> m_Releases;
	std::vector<Transient> m_Transients;
	mutable std::deque<Fence> m_Fences;
	std::uint64_t m_Submitted = 0;
	// Completed graphics submissions of the current epoch: written with the
	// context current, read by recording threads (ring retirement).
	mutable std::atomic<std::uint64_t> m_Completed{ 0 };
	GLuint m_RingBuffer = 0; // 0: the ring is m_RingCpu (ES without EXT_buffer_storage)
	// Indirect draws read indices from the element buffer's start; a bound
	// index offset is honoured by copying the indices here first (D30).
	GLuint m_IndexScratch = 0;
	std::uint64_t m_IndexScratchBytes = 0;
	std::byte *m_RingData = nullptr;
	std::vector<std::byte> m_RingCpu;
	// ES features beyond 3.1 core (QueryFacts).
	bool m_EsFloatTargets = false;
	bool m_EsHalfFloatTargets = false;
	bool m_EsNorm16 = false;
	bool m_EsCubeArrays = false;
	bool m_EsMultisampleArrays = false;
	UploadRing m_Ring;
	mutable std::mutex m_RingLock; // guards m_Ring, m_NextAllocation, m_DeferredUploads
	std::uint64_t m_NextAllocation = 0;
	std::uint64_t m_DeferredUploads = 0;
	std::atomic<std::uint64_t> m_Messages{ 0 };
	// One sequence at a time calls the device (RFC 0016 "Threading"); the lock
	// keeps a stray concurrent call from interleaving GL state.
	mutable std::recursive_mutex m_Lock;
	bool m_Holding = false;
	std::deque<HeldSubmission> m_Held;
};

} // namespace render::device::gl

#endif // RENDER_DEVICE_GL_GL_DEVICE_H
