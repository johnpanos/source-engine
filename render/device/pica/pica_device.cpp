//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica's device: the citro3d context, resources,
//			submission and completion (pica_device.h). The replay of a
//			submission is replay.cpp.
//
//=============================================================================//

#if defined( __3DS__ )

#include "pica_device.h"

#include "device.h"
#include "render/device/pica_codes.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

// libctru's linear heap, which holds every resource of the device.
extern "C" u32 __ctru_linear_heap, __ctru_linear_heap_size;

namespace render::device::pica
{
namespace
{

foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, std::int32_t nativeCode = 0 )
{
	return foundation::MakeUnexpected( DeviceError{ status, operation, nativeCode } );
}

// citro3d is one context per process. The first device initializes it, and
// it then stays for the process: citro3d does not
// survive C3D_Fini followed by C3D_Init (the next device's first draw faulted
// in Azahar, 2026-10-07), and devices come and go (each suite clause makes
// one).
std::mutex &ContextLock()
{
	static std::mutex lock;
	return lock;
}

void FreePipeline( PipelineRecord &p )
{
	shaderProgramFree( &p.program );
	if ( p.dvlb )
		DVLB_Free( p.dvlb );
	p.dvlb = nullptr;
}

bool AcquireContext( std::uint32_t commandBytes );

constexpr std::size_t kLinearAlignment = 0x80;

std::byte *AllocateLinear( std::uint64_t bytes )
{
	if ( bytes == 0 || bytes > 0x7FFFFFFF )
		return nullptr;
	return static_cast<std::byte *>(
	    linearMemAlign( std::size_t( ( bytes + 15 ) & ~std::uint64_t( 15 ) ), kLinearAlignment ) );
}

bool UsesAny( const UsageSet &usages, std::initializer_list<ResourceUsage> any )
{
	for ( ResourceUsage usage : any )
		if ( usages.Has( usage ) )
			return true;
	return false;
}

bool AcquireContext( std::uint32_t commandBytes )
{
	static bool initialized = false;
	std::lock_guard<std::mutex> guard( ContextLock() );
	if ( initialized )
		return true;
	if ( !C3D_Init( commandBytes ) )
		return false;
	SharedContext &shared = Shared();
	shared.vertexIndices = static_cast<float *>(
	    linearMemAlign( kVertexIndexCount * sizeof( float ), kLinearAlignment ) );
	constexpr std::size_t kPlaceholderBytes = kTile * kTile * kStoredTexelBytes;
	shared.placeholderTexels = AllocateLinear( kPlaceholderBytes );
	if ( !shared.vertexIndices || !shared.placeholderTexels )
		return false;
	for ( std::uint32_t i = 0; i < kVertexIndexCount; ++i )
		shared.vertexIndices[i] = float( i );
	GSPGPU_FlushDataCache( shared.vertexIndices, kVertexIndexCount * sizeof( float ) );
	std::memset( shared.placeholderTexels, 0, kPlaceholderBytes );
	GSPGPU_FlushDataCache( shared.placeholderTexels, kPlaceholderBytes );
	C3D_Tex &placeholder = shared.placeholder;
	placeholder.data = shared.placeholderTexels;
	placeholder.fmt = GPU_RGBA8;
	placeholder.size = kPlaceholderBytes;
	placeholder.width = kTile;
	placeholder.height = kTile;
	placeholder.param = GPU_TEXTURE_MODE( GPU_TEX_2D );
	initialized = true;
	return true;
}

} // namespace

SharedContext &Shared()
{
	static SharedContext shared;
	return shared;
}

void BindProgram( shaderProgram_s &program )
{
	SharedContext &shared = Shared();
	C3D_BindProgram( &program );
	shared.bound = &program;
	if ( shared.retired && &shared.retired->program != shared.bound )
	{
		FreePipeline( *shared.retired );
		shared.retired.reset();
	}
}

PicaDevice::PicaDevice( const PicaAdapterOptions &options ) : m_Options( options )
{
	m_Facts = AdapterFacts();
}

PicaDevice::~PicaDevice()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( !m_Context )
		return;
	Drain();
	if ( m_Screen )
		C3D_RenderTargetDelete( m_Screen );
	if ( m_PresentQuad )
		linearFree( m_PresentQuad );
	EraseAll();
	m_Context = false;
}

DeviceResult<void> PicaDevice::Initialize()
{
	if ( !AcquireContext( m_Options.commandListBytes ) )
		return Fail( DeviceStatus::kUnavailable, DeviceOperation::kCreateDevice );
	m_Context = true;
	return {};
}

void PicaDevice::SimulateLoss()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	m_State = DeviceState::kLost;
}

// GPU frames ---------------------------------------------------------------------

void PicaDevice::OpenFrame()
{
	if ( m_FrameOpen )
		return;
	// Without SYNCDRAW: waits only for the previous frame's GPU work, never
	// for a display refresh.
	C3D_FrameBegin( 0 );
	m_FrameOpen = true;
}

void PicaDevice::Drain()
{
	if ( m_FrameOpen )
	{
		// Flushes the linear heap's data cache and runs the command list.
		C3D_FrameEnd( 0 );
		m_FrameOpen = false;
		m_GpuPending = true;
		m_Targets.clear();
	}
	if ( m_GpuPending )
	{
		// A frame begins only once the queue holding the last one is empty.
		C3D_FrameBegin( 0 );
		C3D_FrameEnd( 0 );
		m_GpuPending = false;
		m_GpuUses.clear();
		// The GPU wrote render targets behind the CPU's cache.
		GSPGPU_InvalidateDataCache(
		    reinterpret_cast<const void *>( __ctru_linear_heap ), __ctru_linear_heap_size );
	}
}

void PicaDevice::BeforeCpuAccess( std::initializer_list<std::uint64_t> resources )
{
	if ( m_Options.sensitivity.cpuRacesGpu )
		return;
	for ( std::uint64_t id : resources )
	{
		if ( m_GpuUses.count( id ) )
		{
			Drain();
			return;
		}
	}
}

// Resources -----------------------------------------------------------------------

std::optional<recording::TextureView> PicaDevice::Texture( std::uint64_t id ) const
{
	const auto found = m_Textures.find( id );
	if ( found == m_Textures.end() || found->second.released )
		return std::nullopt;
	return recording::TextureView{ found->second.desc, 1, found->second.usage };
}

std::optional<recording::BufferView> PicaDevice::Buffer( std::uint64_t id ) const
{
	const auto found = m_Buffers.find( id );
	if ( found == m_Buffers.end() || found->second.released )
		return std::nullopt;
	return recording::BufferView{ found->second.desc, found->second.usage };
}

std::optional<recording::PipelineView> PicaDevice::Pipeline( std::uint64_t id ) const
{
	const auto found = m_Pipelines.find( id );
	if ( found == m_Pipelines.end() || found->second->released )
		return std::nullopt;
	const PipelineRecord &p = *found->second;
	recording::PipelineView view;
	view.kind = PipelineKind::kGraphics;
	view.colorFormats = p.colorFormats;
	view.depthFormat = p.depthFormat;
	view.sampleCount = 1;
	view.vertexBuffers = p.vertexBuffers;
	view.drawConstantBytes = p.drawConstantBytes;
	view.layouts = p.layouts;
	view.layoutHasBindings = p.layoutHasBindings;
	return view;
}

std::optional<BindGroupLayoutId> PicaDevice::BindGroup( std::uint64_t id ) const
{
	const auto found = m_BindGroups.find( id );
	if ( found == m_BindGroups.end() || found->second.released )
		return std::nullopt;
	for ( const BindGroupEntry &entry : found->second.entries )
	{
		if ( ( entry.buffer.IsValid() && !Buffer( entry.buffer.value ) ) ||
		     ( entry.texture.IsValid() && !Texture( entry.texture.value ) ) ||
		     ( entry.sampler.IsValid() && !Live( entry.sampler ) ) )
			return std::nullopt;
	}
	return found->second.layout;
}

std::optional<LayoutView> PicaDevice::FindLayout( BindGroupLayoutId id ) const
{
	const auto found = m_Layouts.find( id.value );
	if ( found == m_Layouts.end() || found->second.released )
		return std::nullopt;
	return LayoutView{ found->second.role, found->second.bindings };
}

bool PicaDevice::Live( ResourceId resource ) const
{
	auto live = [&]( const auto &map )
	{
		const auto found = map.find( resource.value );
		return found != map.end() && !found->second.released;
	};
	switch ( resource.kind )
	{
	case ResourceKind::kBuffer:
		return live( m_Buffers );
	case ResourceKind::kTexture:
		return live( m_Textures );
	case ResourceKind::kSampler:
		return live( m_Samplers );
	case ResourceKind::kBindGroupLayout:
		return live( m_Layouts );
	case ResourceKind::kBindGroup:
		return live( m_BindGroups );
	case ResourceKind::kPipeline:
	{
		const auto found = m_Pipelines.find( resource.value );
		return found != m_Pipelines.end() && !found->second->released;
	}
	case ResourceKind::kNone:
		break;
	}
	return false;
}

bool *PicaDevice::ReleasedFlag( ResourceId resource )
{
	auto flag = [&]( auto &map ) -> bool *
	{
		const auto found = map.find( resource.value );
		return found == map.end() ? nullptr : &found->second.released;
	};
	switch ( resource.kind )
	{
	case ResourceKind::kBuffer:
		return flag( m_Buffers );
	case ResourceKind::kTexture:
		return flag( m_Textures );
	case ResourceKind::kSampler:
		return flag( m_Samplers );
	case ResourceKind::kBindGroupLayout:
		return flag( m_Layouts );
	case ResourceKind::kBindGroup:
		return flag( m_BindGroups );
	case ResourceKind::kPipeline:
	{
		const auto found = m_Pipelines.find( resource.value );
		return found == m_Pipelines.end() ? nullptr : &found->second->released;
	}
	case ResourceKind::kNone:
		break;
	}
	return nullptr;
}

void PicaDevice::Erase( ResourceId resource )
{
	switch ( resource.kind )
	{
	case ResourceKind::kBuffer:
		if ( const auto found = m_Buffers.find( resource.value ); found != m_Buffers.end() )
		{
			linearFree( found->second.data );
			m_Buffers.erase( found );
		}
		break;
	case ResourceKind::kTexture:
		if ( const auto found = m_Textures.find( resource.value ); found != m_Textures.end() )
		{
			linearFree( found->second.data );
			m_Textures.erase( found );
		}
		break;
	case ResourceKind::kSampler:
		m_Samplers.erase( resource.value );
		break;
	case ResourceKind::kPipeline:
		if ( const auto found = m_Pipelines.find( resource.value ); found != m_Pipelines.end() )
		{
			SharedContext &shared = Shared();
			if ( shared.bound == &found->second->program )
			{
				// citro3d reads the bound program again at the next bind.
				if ( shared.retired )
					FreePipeline( *shared.retired );
				shared.retired = std::move( found->second );
			}
			else
				FreePipeline( *found->second );
			m_Pipelines.erase( found );
		}
		break;
	case ResourceKind::kBindGroupLayout:
		m_Layouts.erase( resource.value );
		break;
	case ResourceKind::kBindGroup:
		m_BindGroups.erase( resource.value );
		break;
	case ResourceKind::kNone:
		break;
	}
}

void PicaDevice::EraseAll()
{
	std::vector<ResourceId> all;
	for ( const auto &[id, record] : m_Buffers )
		all.push_back( BufferId{ id } );
	for ( const auto &[id, record] : m_Textures )
		all.push_back( TextureId{ id } );
	for ( const auto &[id, record] : m_Pipelines )
		all.push_back( PipelineId{ id } );
	for ( ResourceId resource : all )
		Erase( resource );
	m_Samplers.clear();
	m_Layouts.clear();
	m_BindGroups.clear();
	m_Releases.clear();
}

std::size_t PicaDevice::LiveResourceCount() const
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	return m_Buffers.size() + m_Textures.size() + m_Samplers.size() + m_Layouts.size() +
	       m_BindGroups.size() + m_Pipelines.size();
}

DeviceResult<BufferId> PicaDevice::CreateBuffer( const BufferDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBuffer( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	// A buffer is memory whatever its usages; what the PICA200 cannot do with
	// one (storage bindings, indirect draws) is refused where it is used.
	BufferRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	record.data = AllocateLinear( desc.size );
	if ( !record.data )
		return Fail( DeviceStatus::kOutOfMemory, op );
	std::memset( record.data, 0, std::size_t( desc.size ) );
	const BufferId id{ ++m_NextId };
	m_Buffers.emplace( id.value, record );
	return id;
}

std::span<std::byte> PicaDevice::MapUploadBuffer( BufferId buffer )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	const auto found = m_Buffers.find( buffer.value );
	if ( found == m_Buffers.end() || found->second.released ||
	     found->second.desc.memory != MemoryKind::kUpload )
		return {};
	return { found->second.data, std::size_t( found->second.desc.size ) };
}

void PicaDevice::FlushUploadBuffer( BufferId buffer, std::uint64_t offset, std::uint64_t size )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	const auto found = m_Buffers.find( buffer.value );
	if ( found == m_Buffers.end() || found->second.released || offset >= found->second.desc.size )
		return;
	size = std::min<std::uint64_t>( size, found->second.desc.size - offset );
	GSPGPU_FlushDataCache( found->second.data + offset, u32( size ) );
}

DeviceResult<BufferId> PicaDevice::CreateUploadBuffer( std::span<const std::byte> bytes )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( bytes.empty() )
		return Fail( DeviceStatus::kInvalidDescription, DeviceOperation::kCreateBuffer );
	BufferDesc desc;
	desc.size = bytes.size();
	desc.memory = MemoryKind::kUpload;
	desc.usages = { ResourceUsage::kCopySource };
	auto buffer = CreateBuffer( desc );
	if ( !buffer )
		return buffer;
	BufferRecord &record = m_Buffers.at( buffer.Value().value );
	std::memcpy( record.data, bytes.data(), bytes.size() );
	record.usage = ResourceUsage::kCopySource;
	return buffer;
}

DeviceResult<TextureId> PicaDevice::CreateTexture( const TextureDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateTexture;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateTexture( desc, m_Facts.limits ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	// RFC 0026 decision 4: one 2D layer, one sample, the storable formats;
	// presentation belongs to a bridge; no storage images, no MSAA resolves.
	const bool depth = IsDepthFormat( desc.format );
	if ( desc.dimension != TextureDimension::k2D || desc.depthOrLayers != 1 ||
	     desc.sampleCount != 1 || !Storable( desc.format ) ||
	     UsesAny(
	         desc.usages, { ResourceUsage::kPresent, ResourceUsage::kStorageRead,
	                          ResourceUsage::kStorageWrite, ResourceUsage::kResolveDestination } ) )
		return Fail( DeviceStatus::kUnsupported, op );
	// The GPU renders RGBA8 colour and D24S8 depth only, and samples no depth.
	if ( ( desc.usages.Has( ResourceUsage::kColorAttachment ) &&
	         !ColorBufferFormat( desc.format ) ) ||
	     ( UsesAny( desc.usages, { ResourceUsage::kDepthWrite, ResourceUsage::kDepthRead } ) &&
	         !DepthBufferFormat( desc.format ) ) ||
	     ( depth && desc.usages.Has( ResourceUsage::kSampled ) ) )
		return Fail( DeviceStatus::kUnsupported, op );
	TextureRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	if ( !LayoutOf( desc.format, desc.width, desc.height, desc.mipLevels, record.layout ) )
		return Fail( DeviceStatus::kUnsupported, op );
	record.data = AllocateLinear( record.layout.bytes );
	if ( !record.data )
		return Fail( DeviceStatus::kOutOfMemory, op );
	std::memset( record.data, 0, std::size_t( record.layout.bytes ) );
	if ( record.layout.sampledLevels > 0 && desc.usages.Has( ResourceUsage::kSampled ) )
	{
		C3D_Tex &tex = record.sampled;
		tex.data = record.data;
		tex.fmt = GPU_TEXCOLOR( *SampledFormat( desc.format ) );
		tex.size = std::size_t( record.layout.levels[0].bytes );
		tex.width = std::uint16_t( desc.width );
		tex.height = std::uint16_t( desc.height );
		tex.param = GPU_TEXTURE_MODE( GPU_TEX_2D );
		tex.border = 0;
		tex.lodBias = 0;
		tex.maxLevel = std::uint8_t( record.layout.sampledLevels - 1 );
		tex.minLevel = 0;
	}
	const TextureId id{ ++m_NextId };
	m_Textures.emplace( id.value, record );
	return id;
}

DeviceResult<SamplerId> PicaDevice::CreateSampler( const SamplerDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateSampler;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateSampler( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	if ( desc.maxAnisotropy > 1 )
		return Fail( DeviceStatus::kUnsupported, op );
	// A comparison sampler is a valid description (D24); binding one is
	// refused, since the PICA200 samples no depth texture.
	const SamplerId id{ ++m_NextId };
	m_Samplers.emplace( id.value, SamplerRecord{ desc, false } );
	return id;
}

DeviceResult<BindGroupLayoutId> PicaDevice::CreateBindGroupLayout( const BindGroupLayoutDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroupLayout;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBindGroupLayout( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	for ( const BindingDesc &binding : desc.bindings )
	{
		if ( binding.kind == BindingKind::kStorageBuffer ||
		     binding.kind == BindingKind::kStorageTexture ||
		     binding.stages.Has( ShaderStage::kCompute ) )
			return Fail( DeviceStatus::kUnsupported, op );
	}
	LayoutRecord record;
	record.role = desc.role;
	record.bindings.assign( desc.bindings.begin(), desc.bindings.end() );
	const BindGroupLayoutId id{ ++m_NextId };
	m_Layouts.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BindGroupId> PicaDevice::CreateBindGroup( const BindGroupDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroup;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	const std::optional<LayoutView> layout = FindLayout( desc.layout );
	if ( !layout )
		return Fail( DeviceStatus::kInvalidHandle, op );
	if ( auto valid = ValidateBindGroup( desc, *layout ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	for ( const BindGroupEntry &entry : desc.entries )
	{
		if ( ( entry.buffer.IsValid() && !Buffer( entry.buffer.value ) ) ||
		     ( entry.texture.IsValid() && !Texture( entry.texture.value ) ) ||
		     ( entry.sampler.IsValid() && !Live( entry.sampler ) ) )
			return Fail( DeviceStatus::kInvalidHandle, op );
		// RFC 0026 decision 4: the GPU samples power-of-two textures of at
		// least 8x8, and compares no depth.
		if ( entry.texture.IsValid() &&
		     m_Textures.at( entry.texture.value ).layout.sampledLevels == 0 )
			return Fail( DeviceStatus::kUnsupported, op );
		if ( entry.sampler.IsValid() && m_Samplers.at( entry.sampler.value ).desc.comparison )
			return Fail( DeviceStatus::kUnsupported, op );
	}
	BindGroupRecord record;
	record.layout = desc.layout;
	record.entries.assign( desc.entries.begin(), desc.entries.end() );
	const BindGroupId id{ ++m_NextId };
	m_BindGroups.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<void> PicaDevice::Release( ResourceId resource, CompletionToken releaseAfter )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	bool *released = ReleasedFlag( resource );
	if ( !released || *released )
		return Fail( DeviceStatus::kInvalidHandle, DeviceOperation::kRelease );
	*released = true;
	m_Releases.push_back( { resource, releaseAfter } );
	return {};
}

std::size_t PicaDevice::Collect()
{
	std::size_t freed = 0;
	for ( auto it = m_Releases.begin(); it != m_Releases.end(); )
	{
		const CompletionToken &token = it->token;
		const bool done = !token.NamesSubmission() || token.epoch < m_Epoch ||
		                  ( token.epoch == m_Epoch && token.value <= m_Completed );
		if ( !done )
		{
			++it;
			continue;
		}
		Erase( it->resource );
		it = m_Releases.erase( it );
		++freed;
	}
	return freed;
}

// Encoders, submission and completion -------------------------------------------

DeviceResult<CommandEncoder> PicaDevice::BeginEncoder( QueueKind queue )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kBeginEncoder );
	if ( queue != QueueKind::kGraphics )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kBeginEncoder );
	return CommandEncoder(
	    queue, std::make_unique<recording::RecordingEncoder>( *this, this, queue ) );
}

void PicaDevice::StageUpload(
    recording::RecordingEncoder &, recording::Command &command, std::span<const std::byte> bytes )
{
	command.bytes.assign( bytes.begin(), bytes.end() );
}

std::optional<DeviceStatus> PicaDevice::CheckPicaLimits(
    std::span<const recording::RecordingEncoder *const> encoders ) const
{
	using recording::Op;
	for ( const recording::RecordingEncoder *encoder : encoders )
	{
		for ( const recording::Command &command : encoder->Commands() )
		{
			switch ( command.op )
			{
			case Op::kDraw:
			case Op::kDrawIndexed:
				// No instancing on the PICA200 (RFC 0026 decision 4).
				if ( command.instances != 1 || command.firstInstance != 0 )
					return DeviceStatus::kUnsupported;
				if ( command.op == Op::kDraw && command.first + command.count > kVertexIndexCount )
					return DeviceStatus::kUnsupported;
				break;
			case Op::kBeginRendering:
			{
				// Colour and depth share the framebuffer's one tiled extent.
				if ( command.colors.empty() || !command.depth )
					break;
				const LevelLayout &color =
				    m_Textures.at( command.colors[0].texture.value ).layout.levels[0];
				const LevelLayout &depth =
				    m_Textures.at( command.depth->texture.value ).layout.levels[0];
				if ( color.storedWidth != depth.storedWidth ||
				     color.storedHeight != depth.storedHeight )
					return DeviceStatus::kUnsupported;
				break;
			}
			case Op::kSetIndexBuffer:
				// 8- and 16-bit indices only.
				if ( command.indexFormat != IndexFormat::kUint16 )
					return DeviceStatus::kUnsupported;
				break;
			default:
				break;
			}
		}
	}
	return std::nullopt;
}

DeviceResult<CompletionToken> PicaDevice::Submit(
    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits )
{
	const DeviceOperation op = DeviceOperation::kSubmit;
	std::vector<std::unique_ptr<IEncoderBackend>> backends;
	backends.reserve( encoders.size() );
	for ( CommandEncoder &encoder : encoders )
		backends.push_back( encoder.TakeBackend() );
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( queue != QueueKind::kGraphics )
		return Fail( DeviceStatus::kUnsupported, op );
	for ( const CompletionToken &wait : waits.tokens )
	{
		if ( wait.NamesSubmission() && wait.epoch < m_Epoch )
			return Fail( DeviceStatus::kStaleEpoch, op );
		if ( wait.queue != QueueKind::kGraphics || wait.epoch > m_Epoch ||
		     wait.value > m_Submitted )
			return Fail( DeviceStatus::kInvalidDescription, op );
	}
	std::unordered_map<std::uint64_t, ResourceUsage> states;
	std::vector<const recording::RecordingEncoder *> recorded;
	for ( std::unique_ptr<IEncoderBackend> &backend : backends )
	{
		const auto *encoder = dynamic_cast<const recording::RecordingEncoder *>( backend.get() );
		if ( !encoder || encoder->Owner() != this || encoder->Queue() != queue )
			return Fail( DeviceStatus::kInvalidHandle, op );
		if ( !encoder->Complete() || !recording::Validate( encoder->Commands(), states, *this ) )
			return Fail( DeviceStatus::kInvalidState, op );
		recorded.push_back( encoder );
	}
	if ( auto refused = recording::CheckSubmission( recorded, m_Facts.capabilities, states ) )
		return Fail( *refused, op );
	if ( auto refused = CheckPicaLimits( recorded ) )
		return Fail( *refused, op );
	for ( const auto &[id, usage] : states )
	{
		if ( auto t = m_Textures.find( id ); t != m_Textures.end() )
			t->second.usage = usage;
		else if ( auto b = m_Buffers.find( id ); b != m_Buffers.end() )
			b->second.usage = usage;
	}
	const CompletionToken token{ queue, m_Epoch, ++m_Submitted };
	for ( const recording::RecordingEncoder *encoder : recorded )
		ReplayCommands( *this, encoder->Commands() );
	Drain();
	m_Completed = token.value;
	return token;
}

bool PicaDevice::IsComplete( CompletionToken token ) const
{
	if ( !token.NamesSubmission() )
		return true;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( token.epoch < m_Epoch )
		return true;
	return token.epoch == m_Epoch && token.queue == QueueKind::kGraphics &&
	       token.value <= m_Completed;
}

std::size_t PicaDevice::Poll()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	return Collect();
}

DeviceResult<void> PicaDevice::ReadBuffer(
    BufferId id, std::uint64_t offset, std::span<std::byte> out )
{
	const DeviceOperation op = DeviceOperation::kReadBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	const auto found = m_Buffers.find( id.value );
	if ( found == m_Buffers.end() || found->second.released )
		return Fail( DeviceStatus::kInvalidHandle, op );
	const BufferRecord &buffer = found->second;
	if ( buffer.desc.memory != MemoryKind::kReadback || offset > buffer.desc.size ||
	     out.size() > buffer.desc.size - offset )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	std::memcpy( out.data(), buffer.data + offset, out.size() );
	return {};
}

DeviceResult<void> PicaDevice::WaitIdle()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	Drain();
	return {};
}

DeviceResult<void> PicaDevice::Recover()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State == DeviceState::kAvailable )
		return {};
	m_State = DeviceState::kRecovering;
	Drain();
	EraseAll();
	++m_Epoch;
	m_Submitted = 0;
	m_Completed = 0;
	m_State = DeviceState::kAvailable;
	return {};
}

} // namespace render::device::pica

#endif // __3DS__
