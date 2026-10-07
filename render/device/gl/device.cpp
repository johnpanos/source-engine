//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl: the device, its facts, resources, completion and
//			the provider's factory (public/render/device/gl/provider.h).
//
//=============================================================================//

#include "gl_device.h"

#include <cstdio>
#include <cstring>
#include <string_view>

namespace render::device::gl
{

GLenum CompareFunction( CompareOp op )
{
	switch ( op )
	{
	case CompareOp::kNotEqual:
		return GL_NOTEQUAL;
	case CompareOp::kNever:
		return GL_NEVER;
	case CompareOp::kLess:
		return GL_LESS;
	case CompareOp::kLessEqual:
		return GL_LEQUAL;
	case CompareOp::kEqual:
		return GL_EQUAL;
	case CompareOp::kGreaterEqual:
		return GL_GEQUAL;
	case CompareOp::kGreater:
		return GL_GREATER;
	case CompareOp::kAlways:
		return GL_ALWAYS;
	}
	return GL_ALWAYS;
}

namespace
{

constexpr std::uint64_t kMinimumRingBytes = 4096;

bool HasExtension( const GlApi &gl, std::string_view name )
{
	GLint count = 0;
	gl.GetIntegerv( GL_NUM_EXTENSIONS, &count );
	for ( GLint i = 0; i < count; ++i )
	{
		const auto *extension = reinterpret_cast<const char *>(
		    gl.GetStringi( GL_EXTENSIONS, static_cast<GLuint>( i ) ) );
		if ( extension && name == extension )
			return true;
	}
	return false;
}

GLint Integer( const GlApi &gl, GLenum name )
{
	GLint value = 0;
	gl.GetIntegerv( name, &value );
	return value;
}

// Consumes pending GL errors (bounded: a lost context may keep reporting).
void ClearErrors( const GlApi &gl )
{
	for ( int i = 0; i < 16 && gl.GetError() != GL_NO_ERROR; ++i )
	{
	}
}

// A query a driver may refuse (some ES 3.1 drivers tie the multisample
// texture limits to GL_OES_texture_storage_multisample_2d_array): fallback
// when it does, with the error consumed.
GLint IntegerOr( const GlApi &gl, GLenum name, GLint fallback )
{
	ClearErrors( gl );
	GLint value = 0;
	gl.GetIntegerv( name, &value );
	return gl.GetError() == GL_NO_ERROR ? value : fallback;
}

void APIENTRY DebugMessage(
    GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar *message, const void *user )
{
	// Errors and undefined, deprecated or unportable use at high or medium
	// severity; performance notes and notifications are not defects.
	const bool counted =
	    type == GL_DEBUG_TYPE_ERROR ||
	    ( ( type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR || type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ||
	          type == GL_DEBUG_TYPE_PORTABILITY ) &&
	        ( severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM ) );
	if ( !counted )
		return;
	std::fprintf( stderr, "render.device.gl: GL debug output: %s\n", message );
	const_cast<GlDevice *>( static_cast<const GlDevice *>( user ) )->CountMessage();
}

} // namespace

GlDevice::GlDevice( const GlAdapterOptions &options ) : m_Options( options )
{
}

GlDevice::~GlDevice()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( !m_Context )
		return;
	{
		ContextScope scope( *m_Context );
		if ( scope.Ok() )
		{
			if ( m_State == DeviceState::kAvailable )
				Gl().Finish();
			DestroyContextObjects();
		}
	}
	m_Context.reset();
}

void GlDevice::CountMessage()
{
	m_Messages.fetch_add( 1 );
	if ( m_Options.validationCounter )
		m_Options.validationCounter->fetch_add( 1 );
}

DeviceResult<void> GlDevice::Initialize()
{
	if ( m_Options.uploadRingBytes < kMinimumRingBytes )
		return Fail( DeviceStatus::kInvalidDescription, DeviceOperation::kCreateDevice );
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	std::string error;
	m_Context = EglContext::Create( m_Options.api, m_Options.validation, error );
	if ( !m_Context )
	{
		std::fprintf( stderr, "render.device.gl: %s\n", error.c_str() );
		return Fail( DeviceStatus::kUnavailable, DeviceOperation::kCreateDevice );
	}
	return CreateContextObjects();
}

DeviceResult<void> GlDevice::CreateContextObjects()
{
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return Fail( DeviceStatus::kUnavailable, DeviceOperation::kCreateDevice );
	const GlApi &gl = Gl();
	const GLint major = Integer( gl, GL_MAJOR_VERSION );
	const GLint minor = Integer( gl, GL_MINOR_VERSION );
	if ( IsEs() ? ( major < 3 || ( major == 3 && minor < 1 ) )
	            : ( major < 4 || ( major == 4 && minor < 5 ) ) )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
	if ( IsEs() )
	{
		if ( auto required = CheckEsRequirements( major, minor ); !required )
			return required;
	}
	// Flat slots the artifacts use: four groups of kSlotsPerGroup per kind and
	// the draw-constant block, and a texture unit per texture slot.
	if ( Integer( gl, GL_MAX_UNIFORM_BUFFER_BINDINGS ) < GLint( kDrawConstantsSlot + 1 ) ||
	     Integer( gl, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS ) < GLint( kDrawConstantsSlot ) ||
	     Integer( gl, GL_MAX_VERTEX_ATTRIB_BINDINGS ) < GLint( kMaxVertexSlots ) )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );

	if ( m_Options.validation && ( !IsEs() || Es().core.DebugMessageCallback ) )
	{
		gl.Enable( GL_DEBUG_OUTPUT );
		gl.Enable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
		gl.DebugMessageCallback( &DebugMessage, this );
		gl.DebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
	}
	// The port's conventions (RFC 0016 decision 6): clip +Y on row 0 and clip
	// depth 0 to 1. The sensitivity knobs keep GL's own instead.
	const GlAdapterOptions::Sensitivity &broken = m_Options.sensitivity;
	gl.ClipControl( broken.lowerLeftOrigin ? GL_LOWER_LEFT : GL_UPPER_LEFT,
	    broken.negativeOneToOneDepth ? GL_NEGATIVE_ONE_TO_ONE : GL_ZERO_TO_ONE );
	// Writes to sRGB attachments encode, as Vulkan's do; cube maps filter
	// across faces, as Vulkan's always do; shaders set the point size.
	// ES always does all three.
	if ( !IsEs() )
	{
		gl.Enable( GL_FRAMEBUFFER_SRGB );
		gl.Enable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
		gl.Enable( GL_PROGRAM_POINT_SIZE );
	}
	gl.PixelStorei( GL_PACK_ALIGNMENT, 1 );
	gl.PixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	if ( IsEs() && !Es().core.BufferStorage )
	{
		// RFC 0022: without EXT_buffer_storage the ring is CPU memory, copied
		// into each upload's destination when the submission replays.
		m_RingCpu.assign( m_Options.uploadRingBytes, std::byte{} );
		m_RingData = m_RingCpu.data();
	}
	else
	{
		gl.CreateBuffers( 1, &m_RingBuffer );
		const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		gl.NamedBufferStorage(
		    m_RingBuffer, static_cast<GLsizeiptr>( m_Options.uploadRingBytes ), nullptr, flags );
		m_RingData = static_cast<std::byte *>( gl.MapNamedBufferRange(
		    m_RingBuffer, 0, static_cast<GLsizeiptr>( m_Options.uploadRingBytes ), flags ) );
		if ( !m_RingData )
			return Fail(
			    DeviceStatus::kOutOfMemory, DeviceOperation::kCreateDevice, gl.GetError() );
	}
	{
		std::lock_guard<std::mutex> ring( m_RingLock );
		m_Ring.Reset( m_Options.uploadRingBytes );
	}
	QueryFacts();
	// Later calls judge their own GL errors: none may be left from setup.
	ClearErrors( gl );
	return {};
}

void GlDevice::DestroyContextObjects()
{
	const GlApi &gl = Gl();
	for ( auto &[id, buffer] : m_Buffers )
		gl.DeleteBuffers( 1, &buffer.name );
	for ( auto &[id, texture] : m_Textures )
		gl.DeleteTextures( 1, &texture.name );
	for ( auto &[id, sampler] : m_Samplers )
		gl.DeleteSamplers( 1, &sampler.name );
	for ( auto &[id, pipeline] : m_Pipelines )
		gl.DeleteVertexArrays( 1, &pipeline.vertexArray );
	for ( auto &[key, program] : m_Programs )
		gl.DeleteProgram( program.name );
	for ( auto &[key, framebuffer] : m_Framebuffers )
		gl.DeleteFramebuffers( 1, &framebuffer );
	for ( GLuint &framebuffer : m_CopyFramebuffers )
	{
		if ( framebuffer )
			gl.DeleteFramebuffers( 1, &framebuffer );
		framebuffer = 0;
	}
	for ( const Transient &transient : m_Transients )
	{
		if ( transient.query )
			gl.DeleteQueries( 1, &transient.buffer );
		else
			gl.DeleteBuffers( 1, &transient.buffer );
	}
	for ( const Fence &fence : m_Fences )
		gl.DeleteSync( fence.sync );
	if ( m_RingBuffer )
	{
		gl.UnmapNamedBuffer( m_RingBuffer );
		gl.DeleteBuffers( 1, &m_RingBuffer );
	}
	m_RingBuffer = 0;
	if ( m_IndexScratch )
		gl.DeleteBuffers( 1, &m_IndexScratch );
	m_IndexScratch = 0;
	m_IndexScratchBytes = 0;
	m_RingData = nullptr;
	m_RingCpu.clear();
	if ( EsState *es = m_Context->Es() )
	{
		for ( GLuint *framebuffer : { &es->readFramebuffer, &es->clearFramebuffer } )
		{
			if ( *framebuffer )
				gl.DeleteFramebuffers( 1, framebuffer );
			*framebuffer = 0;
		}
		for ( GLuint *program : { &es->depthCopy2D, &es->depthCopyArray } )
		{
			if ( *program )
				gl.DeleteProgram( *program );
			*program = 0;
		}
	}
	m_Buffers.clear();
	m_Textures.clear();
	m_Samplers.clear();
	m_Layouts.clear();
	m_BindGroups.clear();
	m_Pipelines.clear();
	m_Programs.clear();
	m_Framebuffers.clear();
	m_Transients.clear();
	m_Fences.clear();
	m_Releases.clear();
	// Held work never runs. Its encoders were submitted, so they return no
	// ring range, and the ring goes with the context.
	m_Held.clear();
}

void GlDevice::QueryFacts()
{
	const GlApi &gl = Gl();
	const char *renderer = reinterpret_cast<const char *>( gl.GetString( GL_RENDERER ) );
	m_AdapterName = renderer ? renderer : "OpenGL";
	m_Facts = {};
	m_Facts.diagnosticBackend = IsEs() ? "gles" : "gl";
	m_Facts.adapterName = m_AdapterName;
	m_Facts.artifactFormat = IsEs() ? ArtifactFormat::kGlslEs310 : ArtifactFormat::kGlsl450;

	CapabilitySet have;
	// Compute and storage bindings take the flat slots of all four groups; ES
	// also has to allow storage blocks outside compute (its minimum is none).
	if ( Integer( gl, GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS ) >= GLint( kDrawConstantsSlot ) &&
	     Integer( gl, GL_MAX_IMAGE_UNITS ) >= GLint( kDrawConstantsSlot ) &&
	     Integer( gl, GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS ) > 0 &&
	     ( !IsEs() || ( Integer( gl, GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS ) > 0 &&
	                      Integer( gl, GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS ) > 0 ) ) )
	{
		have.Add( Capability::kCompute );
		have.Add( Capability::kStorageBuffers );
	}
	// RFC 0022: ES claims neither block compression (it cannot read a
	// compressed texture back, D19) nor timestamps (no query-buffer write, D23).
	if ( !IsEs() && HasExtension( gl, "GL_EXT_texture_compression_s3tc" ) &&
	     ( HasExtension( gl, "GL_EXT_texture_sRGB" ) ||
	         HasExtension( gl, "GL_EXT_texture_compression_s3tc_srgb" ) ) )
		have.Add( Capability::kTextureCompressionBC );
	// D23: GL timestamps are nanoseconds.
	GLint timestampBits = 0;
	if ( !IsEs() )
		gl.GetQueryiv( GL_TIMESTAMP, GL_QUERY_COUNTER_BITS, &timestampBits );
	if ( timestampBits > 0 )
		have.Add( Capability::kTimestamps );
	// D30: GL 4.3's glMultiDrawElementsIndirect; ES 3.1 draws one GPU-read
	// record at a time. D31 where the context has the count entry point (GL
	// 4.6 or ARB_indirect_parameters; ES has none). Neither honours a
	// record's firstInstance in the shaders' instance index, so
	// kIndirectFirstInstance is not claimed.
	have.Add( Capability::kMultiDrawIndirect );
	// D36: ARB_texture_cube_map_array is core since GL 4.0; ES has them from 3.2
	// or the extensions.
	if ( !IsEs() || Integer( gl, GL_MINOR_VERSION ) >= 2 || Integer( gl, GL_MAJOR_VERSION ) > 3 ||
	     HasExtension( gl, "GL_EXT_texture_cube_map_array" ) ||
	     HasExtension( gl, "GL_OES_texture_cube_map_array" ) )
		have.Add( Capability::kCubeArrays );
	if ( !IsEs() && gl.MultiDrawElementsIndirectCount )
		have.Add( Capability::kDrawIndirectCount );
	// D38: glPolygonMode is desktop GL's; ES has no line fill.
	if ( !IsEs() && gl.PolygonMode )
		have.Add( Capability::kFillModeLines );
	CapabilitySet claimed;
	for ( std::uint32_t bit = 0; bit < static_cast<std::uint32_t>( Capability::kCount ); ++bit )
	{
		const Capability capability = static_cast<Capability>( bit );
		if ( ( have.Has( capability ) && m_Options.allowed.Has( capability ) ) ||
		     m_Options.sensitivity.falseClaims.Has( capability ) )
			claimed.Add( capability );
	}
	m_Facts.capabilities = claimed;
	if ( claimed.Has( Capability::kTimestamps ) )
		m_Facts.timestampPeriodNs = 1.0;
	m_Anisotropy = HasExtension( gl, "GL_ARB_texture_filter_anisotropic" ) ||
	               HasExtension( gl, "GL_EXT_texture_filter_anisotropic" );
	if ( IsEs() )
	{
		const bool es32 =
		    Integer( gl, GL_MINOR_VERSION ) >= 2 || Integer( gl, GL_MAJOR_VERSION ) > 3;
		m_EsFloatTargets = HasExtension( gl, "GL_EXT_color_buffer_float" );
		m_EsHalfFloatTargets =
		    m_EsFloatTargets || HasExtension( gl, "GL_EXT_color_buffer_half_float" );
		m_EsNorm16 = HasExtension( gl, "GL_EXT_texture_norm16" );
		m_EsCubeArrays = es32 || HasExtension( gl, "GL_EXT_texture_cube_map_array" ) ||
		                 HasExtension( gl, "GL_OES_texture_cube_map_array" );
		m_EsMultisampleArrays = Es().core.TexStorage3DMultisample != nullptr;
	}

	Limits &limits = m_Facts.limits;
	limits.maxBindGroups = kMaxBindGroups;
	limits.maxTextureDimension2D = static_cast<std::uint32_t>( Integer( gl, GL_MAX_TEXTURE_SIZE ) );
	limits.maxColorAttachments = static_cast<std::uint32_t>(
	    std::min( Integer( gl, GL_MAX_COLOR_ATTACHMENTS ), Integer( gl, GL_MAX_DRAW_BUFFERS ) ) );
	limits.maxVertexBuffers = kMaxVertexSlots;
	limits.uniformBufferAlignment =
	    static_cast<std::uint32_t>( Integer( gl, GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT ) );
	// A refused multisample-texture limit claims single sampling only.
	const GLint samples =
	    std::min( { Integer( gl, GL_MAX_SAMPLES ), IntegerOr( gl, GL_MAX_COLOR_TEXTURE_SAMPLES, 1 ),
	        IntegerOr( gl, GL_MAX_DEPTH_TEXTURE_SAMPLES, 1 ) } );
	limits.sampleCounts = 0;
	for ( GLint count = 1; count <= samples && count <= 64; count *= 2 )
		limits.sampleCounts |= static_cast<std::uint32_t>( count );
}

DeviceResult<void> GlDevice::CheckEsRequirements( GLint major, GLint minor )
{
	// RFC 0022 decision 3. eglGetProcAddress may return an entry point the
	// driver does not support, so the extensions decide, not the loads.
	const GlApi &gl = Gl();
	EsState &es = Es();
	const bool es32 = major > 3 || minor >= 2;
	const char *missing = nullptr;
	if ( !HasExtension( gl, "GL_EXT_clip_control" ) )
		missing = "GL_EXT_clip_control";
	else if ( !es32 && !HasExtension( gl, "GL_OES_draw_buffers_indexed" ) &&
	          !HasExtension( gl, "GL_EXT_draw_buffers_indexed" ) )
		missing = "per-attachment blend state (ES 3.2 or GL_OES_draw_buffers_indexed)";
	else if ( !es32 && !HasExtension( gl, "GL_OES_draw_elements_base_vertex" ) &&
	          !HasExtension( gl, "GL_EXT_draw_elements_base_vertex" ) )
		missing = "base-vertex draws (ES 3.2 or GL_OES_draw_elements_base_vertex)";
	if ( missing )
	{
		std::fprintf( stderr, "render.device.gl: the OpenGL ES context lacks %s\n", missing );
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
	}
	if ( !HasExtension( gl, "GL_EXT_buffer_storage" ) )
		es.core.BufferStorage = nullptr;
	if ( !es32 && !HasExtension( gl, "GL_OES_texture_storage_multisample_2d_array" ) )
		es.core.TexStorage3DMultisample = nullptr;
	if ( !m_Context->Robust() )
		es.core.GetGraphicsResetStatus = nullptr;
	if ( !es32 && !HasExtension( gl, "GL_KHR_debug" ) )
	{
		es.core.DebugMessageCallback = nullptr;
		es.core.DebugMessageControl = nullptr;
		es.core.ObjectLabel = nullptr;
		es.core.PushDebugGroup = nullptr;
		es.core.PopDebugGroup = nullptr;
	}
	// Edits bind the last texture unit, beyond every artifact slot.
	es.editUnit = static_cast<GLuint>( Integer( gl, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS ) - 1 );
	return {};
}

bool GlDevice::EsFormatSupported( const TextureDesc &desc, bool attachment ) const
{
	switch ( desc.format )
	{
	// No BGRA storage in ES 3.1 (RFC 0022 decision 6).
	case Format::kBGRA8Unorm:
	case Format::kBGRA8Srgb:
		return false;
	case Format::kRGBA16Unorm:
		return m_EsNorm16;
	case Format::kRG16Float:
	case Format::kRGBA16Float:
		return !attachment || m_EsHalfFloatTargets;
	case Format::kR32Float:
	case Format::kRGBA32Float:
	case Format::kRG11B10Float: // colour-renderable under EXT_color_buffer_float
		return !attachment || m_EsFloatTargets;
	default:
		break;
	}
	if ( desc.dimension == TextureDimension::kCube && desc.depthOrLayers > 6 && !m_EsCubeArrays )
		return false;
	if ( desc.sampleCount > 1 && desc.depthOrLayers > 1 && !m_EsMultisampleArrays )
		return false;
	return true;
}

void GlDevice::SimulateLoss()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	m_State = DeviceState::kLost;
}

void GlDevice::CheckReset()
{
	if ( m_State != DeviceState::kAvailable || !m_Context->Robust() )
		return;
	if ( Gl().GetGraphicsResetStatus() != GL_NO_ERROR )
		m_State = DeviceState::kLost;
}

// Resources ---------------------------------------------------------------------

BufferRecord *GlDevice::LiveBuffer( std::uint64_t id )
{
	const auto found = m_Buffers.find( id );
	return found != m_Buffers.end() && !found->second.released ? &found->second : nullptr;
}

TextureRecord *GlDevice::LiveTexture( std::uint64_t id )
{
	const auto found = m_Textures.find( id );
	return found != m_Textures.end() && !found->second.released ? &found->second : nullptr;
}

BufferRecord *GlDevice::ExistingBuffer( std::uint64_t id )
{
	const auto found = m_Buffers.find( id );
	return found != m_Buffers.end() ? &found->second : nullptr;
}

TextureRecord *GlDevice::ExistingTexture( std::uint64_t id )
{
	const auto found = m_Textures.find( id );
	return found != m_Textures.end() ? &found->second : nullptr;
}

std::optional<LayoutView> GlDevice::FindLayout( BindGroupLayoutId id ) const
{
	const auto found = m_Layouts.find( id.value );
	if ( found == m_Layouts.end() || found->second.released )
		return std::nullopt;
	return LayoutView{ found->second.role, found->second.bindings };
}

bool *GlDevice::ReleasedFlag( ResourceId resource )
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
	case ResourceKind::kPipeline:
		return flag( m_Pipelines );
	case ResourceKind::kBindGroupLayout:
		return flag( m_Layouts );
	case ResourceKind::kBindGroup:
		return flag( m_BindGroups );
	case ResourceKind::kNone:
		break;
	}
	return nullptr;
}

// Recorded resources (recording::IRecordedResources) --------------------------------

std::optional<recording::TextureView> GlDevice::Texture( std::uint64_t id ) const
{
	const auto found = m_Textures.find( id );
	if ( found == m_Textures.end() || found->second.released )
		return std::nullopt;
	return recording::TextureView{ found->second.desc, found->second.layers, found->second.usage };
}

std::optional<recording::BufferView> GlDevice::Buffer( std::uint64_t id ) const
{
	const auto found = m_Buffers.find( id );
	if ( found == m_Buffers.end() || found->second.released )
		return std::nullopt;
	return recording::BufferView{ found->second.desc, found->second.usage };
}

std::optional<recording::PipelineView> GlDevice::Pipeline( std::uint64_t id ) const
{
	const auto found = m_Pipelines.find( id );
	if ( found == m_Pipelines.end() || found->second.released )
		return std::nullopt;
	const PipelineRecord &p = found->second;
	return recording::PipelineView{ p.kind, p.colorFormats, p.depthFormat, p.sampleCount,
	    p.vertexBuffers, p.drawConstantBytes, p.layouts, p.layoutHasBindings };
}

std::optional<BindGroupLayoutId> GlDevice::BindGroup( std::uint64_t id ) const
{
	const auto group = m_BindGroups.find( id );
	if ( group == m_BindGroups.end() || group->second.released )
		return std::nullopt;
	auto live = [&]( const auto &map, std::uint64_t key )
	{
		const auto found = map.find( key );
		return found != map.end() && !found->second.released;
	};
	for ( const BindGroupEntry &entry : group->second.entries )
	{
		if ( ( entry.buffer.IsValid() && !live( m_Buffers, entry.buffer.value ) ) ||
		     ( entry.texture.IsValid() && !live( m_Textures, entry.texture.value ) ) ||
		     ( entry.sampler.IsValid() && !live( m_Samplers, entry.sampler.value ) ) )
			return std::nullopt;
	}
	return group->second.layout;
}

bool GlDevice::CanCopyWithBuffer( const recording::TextureView &texture ) const
{
	// D24's depth has no 32-bit transfer equal to the port's (formats.cpp).
	return texture.desc.format != Format::kD24UnormS8;
}

bool GlDevice::CanCopyTexture( const recording::TextureView &texture ) const
{
	// Through a framebuffer blit, which copies no cube face and no
	// block-compressed texture: GL refuses those by name.
	return texture.desc.dimension != TextureDimension::kCube &&
	       !IsBlockCompressed( texture.desc.format );
}

bool GlDevice::QueueSupported( QueueKind queue ) const
{
	// One context, one command stream: no separate compute or transfer queue.
	return queue == QueueKind::kGraphics;
}

DeviceResult<BufferId> GlDevice::CreateBuffer( const BufferDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBuffer( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return Fail( DeviceStatus::kUnavailable, op );
	const GlApi &gl = Gl();
	BufferRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	gl.CreateBuffers( 1, &record.name );
	// Every buffer takes deferred uploads (glNamedBufferSubData); readback
	// buffers are read on the CPU.
	GLbitfield flags = GL_DYNAMIC_STORAGE_BIT;
	if ( desc.memory == MemoryKind::kReadback )
		flags |= GL_MAP_READ_BIT | GL_CLIENT_STORAGE_BIT;
	gl.NamedBufferStorage( record.name, static_cast<GLsizeiptr>( desc.size ), nullptr, flags );
	if ( const GLenum error = gl.GetError(); error != GL_NO_ERROR )
	{
		gl.DeleteBuffers( 1, &record.name );
		return Fail(
		    error == GL_OUT_OF_MEMORY ? DeviceStatus::kOutOfMemory : DeviceStatus::kInternal, op,
		    error );
	}
	if ( !desc.debugName.empty() )
		gl.ObjectLabel( GL_BUFFER, record.name, static_cast<GLsizei>( desc.debugName.size() ),
		    desc.debugName.data() );
	const BufferId id{ ++m_NextId };
	m_Buffers.emplace( id.value, record );
	return id;
}

DeviceResult<BufferId> GlDevice::CreateUploadBuffer( std::span<const std::byte> bytes )
{
	const DeviceOperation op = DeviceOperation::kCreateBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	BufferDesc desc;
	desc.size = bytes.size();
	desc.memory = MemoryKind::kUpload;
	desc.usages = { ResourceUsage::kCopySource };
	// Keep creation and initialization on the same current context. A failed
	// upload never publishes a partially initialized handle.
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return Fail( DeviceStatus::kUnavailable, op );
	auto buffer = CreateBuffer( desc );
	if ( !buffer )
		return buffer;
	BufferRecord &record = m_Buffers.at( buffer.Value().value );
	Gl().NamedBufferSubData(
	    record.name, 0, static_cast<GLsizeiptr>( bytes.size() ), bytes.data() );
	if ( const GLenum error = Gl().GetError(); error != GL_NO_ERROR )
	{
		Gl().DeleteBuffers( 1, &record.name );
		m_Buffers.erase( buffer.Value().value );
		return Fail(
		    error == GL_OUT_OF_MEMORY ? DeviceStatus::kOutOfMemory : DeviceStatus::kInternal, op,
		    error );
	}
	record.usage = ResourceUsage::kCopySource;
	return buffer;
}

DeviceResult<TextureId> GlDevice::CreateTexture( const TextureDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateTexture;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateTexture( desc, m_Facts.limits ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	// Presentation belongs to a render.presentation.v1 bridge.
	if ( desc.usages.Has( ResourceUsage::kPresent ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( IsBlockCompressed( desc.format ) &&
	     !m_Facts.capabilities.Has( Capability::kTextureCompressionBC ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( desc.dimension == TextureDimension::kCube && desc.depthOrLayers > 6 &&
	     !m_Facts.capabilities.Has( Capability::kCubeArrays ) )
		return Fail( DeviceStatus::kUnsupported, op );
	const bool attachment = desc.usages.Has( ResourceUsage::kColorAttachment ) ||
	                        desc.usages.Has( ResourceUsage::kDepthWrite ) ||
	                        desc.usages.Has( ResourceUsage::kDepthRead ) ||
	                        desc.usages.Has( ResourceUsage::kResolveDestination );
	const bool storage = desc.usages.Has( ResourceUsage::kStorageRead ) ||
	                     desc.usages.Has( ResourceUsage::kStorageWrite );
	const GlFormat format = FormatOf( desc.format );
	// GL image units take no sRGB or depth format.
	if ( ( attachment && desc.dimension == TextureDimension::k3D ) ||
	     ( storage && ( IsDepthFormat( desc.format ) || desc.format == Format::kRGBA8Srgb ||
	                      desc.format == Format::kBGRA8Srgb ||
	                      !m_Facts.capabilities.Has( Capability::kStorageBuffers ) ) ) ||
	     format.internal == 0 || ( IsEs() && !EsFormatSupported( desc, attachment ) ) )
		return Fail( DeviceStatus::kUnsupported, op );
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return Fail( DeviceStatus::kUnavailable, op );
	const GlApi &gl = Gl();
	TextureRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	record.layers = desc.dimension == TextureDimension::k3D ? 1u : desc.depthOrLayers;
	record.target = TextureTarget( desc );
	gl.CreateTextures( record.target, 1, &record.name );
	const auto levels = static_cast<GLsizei>( desc.mipLevels );
	const auto width = static_cast<GLsizei>( desc.width );
	const auto height = static_cast<GLsizei>( desc.height );
	const auto depth = static_cast<GLsizei>( desc.depthOrLayers );
	switch ( record.target )
	{
	case GL_TEXTURE_2D:
	case GL_TEXTURE_CUBE_MAP:
		gl.TextureStorage2D( record.name, levels, format.internal, width, height );
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
	case GL_TEXTURE_3D:
		gl.TextureStorage3D( record.name, levels, format.internal, width, height, depth );
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
		gl.TextureStorage2DMultisample( record.name, static_cast<GLsizei>( desc.sampleCount ),
		    format.internal, width, height, GL_TRUE );
		break;
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		gl.TextureStorage3DMultisample( record.name, static_cast<GLsizei>( desc.sampleCount ),
		    format.internal, width, height, depth, GL_TRUE );
		break;
	default:
		break;
	}
	if ( const GLenum error = gl.GetError(); error != GL_NO_ERROR )
	{
		gl.DeleteTextures( 1, &record.name );
		return Fail(
		    error == GL_OUT_OF_MEMORY ? DeviceStatus::kOutOfMemory : DeviceStatus::kUnsupported, op,
		    error );
	}
	if ( !desc.debugName.empty() )
		gl.ObjectLabel( GL_TEXTURE, record.name, static_cast<GLsizei>( desc.debugName.size() ),
		    desc.debugName.data() );
	const TextureId id{ ++m_NextId };
	m_Textures.emplace( id.value, record );
	return id;
}

DeviceResult<SamplerId> GlDevice::CreateSampler( const SamplerDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateSampler;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateSampler( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	if ( desc.maxAnisotropy > 1 && !m_Anisotropy )
		return Fail( DeviceStatus::kUnsupported, op );
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return Fail( DeviceStatus::kUnavailable, op );
	const GlApi &gl = Gl();
	SamplerRecord record;
	gl.CreateSamplers( 1, &record.name );
	const bool linear = desc.minFilter == Filter::kLinear;
	const bool linearMip = desc.mipFilter == Filter::kLinear;
	const GLint minFilter =
	    linear ? ( linearMip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST )
	           : ( linearMip ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST );
	gl.SamplerParameteri( record.name, GL_TEXTURE_MIN_FILTER, minFilter );
	gl.SamplerParameteri( record.name, GL_TEXTURE_MAG_FILTER,
	    desc.magFilter == Filter::kLinear ? GL_LINEAR : GL_NEAREST );
	const GLint wrap = desc.address == AddressMode::kClampToEdge      ? GL_CLAMP_TO_EDGE
	                   : desc.address == AddressMode::kMirroredRepeat ? GL_MIRRORED_REPEAT
	                                                                  : GL_REPEAT;
	for ( const GLenum axis : { GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R } )
		gl.SamplerParameteri( record.name, axis, wrap );
	if ( desc.maxAnisotropy > 1 )
		gl.SamplerParameterf(
		    record.name, GL_TEXTURE_MAX_ANISOTROPY, static_cast<GLfloat>( desc.maxAnisotropy ) );
	gl.SamplerParameteri( record.name, GL_TEXTURE_COMPARE_MODE,
	    desc.comparison ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE );
	gl.SamplerParameteri( record.name, GL_TEXTURE_COMPARE_FUNC,
	    CompareFunction( m_Options.sensitivity.reverseSamplerComparison &&
	                             desc.comparison == CompareOp::kLessEqual
	                         ? CompareOp::kGreater
	                         : desc.comparison.value_or( CompareOp::kAlways ) ) );
	const SamplerId id{ ++m_NextId };
	m_Samplers.emplace( id.value, record );
	return id;
}

DeviceResult<BindGroupLayoutId> GlDevice::CreateBindGroupLayout( const BindGroupLayoutDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroupLayout;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBindGroupLayout( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	for ( const BindingDesc &binding : desc.bindings )
	{
		// Each group owns kSlotsPerGroup flat slots of each kind.
		if ( binding.binding + binding.count > kSlotsPerGroup )
			return Fail( DeviceStatus::kUnsupported, op );
		if ( ( binding.kind == BindingKind::kStorageBuffer ||
		         binding.kind == BindingKind::kStorageTexture ) &&
		     !m_Facts.capabilities.Has( Capability::kStorageBuffers ) )
			return Fail( DeviceStatus::kUnsupported, op );
	}
	LayoutRecord record;
	record.role = desc.role;
	record.bindings.assign( desc.bindings.begin(), desc.bindings.end() );
	const BindGroupLayoutId id{ ++m_NextId };
	m_Layouts.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BindGroupId> GlDevice::CreateBindGroup( const BindGroupDesc &desc )
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
		const auto sampler = m_Samplers.find( entry.sampler.value );
		if ( ( entry.buffer.IsValid() && !LiveBuffer( entry.buffer.value ) ) ||
		     ( entry.texture.IsValid() && !LiveTexture( entry.texture.value ) ) ||
		     ( entry.sampler.IsValid() &&
		         ( sampler == m_Samplers.end() || sampler->second.released ) ) )
			return Fail( DeviceStatus::kInvalidHandle, op );
	}
	BindGroupRecord record;
	record.layout = desc.layout;
	record.bindings.assign( layout->bindings.begin(), layout->bindings.end() );
	record.entries.assign( desc.entries.begin(), desc.entries.end() );
	const BindGroupId id{ ++m_NextId };
	m_BindGroups.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<void> GlDevice::Release( ResourceId resource, CompletionToken releaseAfter )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	bool *released = ReleasedFlag( resource );
	if ( !released || *released )
		return Fail( DeviceStatus::kInvalidHandle, DeviceOperation::kRelease );
	*released = true;
	m_Releases.push_back( { resource, releaseAfter } );
	return {};
}

void GlDevice::Erase( ResourceId resource )
{
	const GlApi &gl = Gl();
	switch ( resource.kind )
	{
	case ResourceKind::kBuffer:
		if ( const auto found = m_Buffers.find( resource.value ); found != m_Buffers.end() )
		{
			gl.DeleteBuffers( 1, &found->second.name );
			m_Buffers.erase( found );
		}
		break;
	case ResourceKind::kTexture:
		if ( const auto found = m_Textures.find( resource.value ); found != m_Textures.end() )
		{
			ForgetFramebuffers( resource.value );
			gl.DeleteTextures( 1, &found->second.name );
			m_Textures.erase( found );
		}
		break;
	case ResourceKind::kSampler:
		if ( const auto found = m_Samplers.find( resource.value ); found != m_Samplers.end() )
		{
			gl.DeleteSamplers( 1, &found->second.name );
			m_Samplers.erase( found );
		}
		break;
	case ResourceKind::kPipeline:
		if ( const auto found = m_Pipelines.find( resource.value ); found != m_Pipelines.end() )
		{
			gl.DeleteVertexArrays( 1, &found->second.vertexArray );
			const std::string key = found->second.programKey;
			m_Pipelines.erase( found );
			ReleaseProgram( key );
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

std::size_t GlDevice::LiveResourceCount() const
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	return m_Buffers.size() + m_Textures.size() + m_Samplers.size() + m_Layouts.size() +
	       m_BindGroups.size() + m_Pipelines.size();
}

MemoryBudgetSnapshot GlDevice::ReadMemoryBudget() const
{
	MemoryBudgetSnapshot result;
	if ( m_State != DeviceState::kAvailable )
		return result;
	result.supported = true;
	result.epoch = m_Epoch;
	HeapMemoryBudget heap;
	// Core GL exposes no portable heap budget query. Report the adapter's
	// logical buffer and texture storage estimate, and leave budgetKnown false.
	heap.usageKnown = true;
	heap.usageEstimated = true;
	for ( const auto &entry : m_Buffers )
		heap.usageBytes += entry.second.desc.size;
	for ( const auto &entry : m_Textures )
	{
		const TextureDesc &desc = entry.second.desc;
		const std::uint64_t layers =
		    desc.dimension == TextureDimension::kCube
		        ? std::uint64_t( desc.depthOrLayers )
		        : ( desc.dimension == TextureDimension::k3D ? 1u : desc.depthOrLayers );
		for ( std::uint32_t mip = 0; mip < desc.mipLevels; ++mip )
		{
			const std::uint32_t width = std::max( 1u, desc.width >> mip );
			const std::uint32_t height = std::max( 1u, desc.height >> mip );
			const std::uint64_t depth = desc.dimension == TextureDimension::k3D
			                                ? std::max( 1u, desc.depthOrLayers >> mip )
			                                : 1u;
			heap.usageBytes += RegionBytes( desc.format, width, height ) * layers * depth;
		}
	}
	result.heaps.push_back( heap );
	return result;
}

// Encoders, submission and completion -------------------------------------------

DeviceResult<CommandEncoder> GlDevice::BeginEncoder( QueueKind queue )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kBeginEncoder );
	if ( !QueueSupported( queue ) )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kBeginEncoder );
	return CommandEncoder( queue, std::make_unique<RecordingEncoder>(
	    static_cast<recording::IUploadStager &>( *this ), this, queue ) );
}

DeviceResult<CompletionToken> GlDevice::Submit(
    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits )
{
	const DeviceOperation op = DeviceOperation::kSubmit;
	// Encoders are consumed whatever the outcome.
	std::vector<std::unique_ptr<IEncoderBackend>> backends;
	backends.reserve( encoders.size() );
	for ( CommandEncoder &encoder : encoders )
		backends.push_back( encoder.TakeBackend() );

	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( !QueueSupported( queue ) )
		return Fail( DeviceStatus::kUnsupported, op );
	for ( const CompletionToken &wait : waits.tokens )
	{
		if ( wait.NamesSubmission() && wait.epoch < m_Epoch )
			return Fail( DeviceStatus::kStaleEpoch, op );
		// One command stream: a wait on an earlier graphics submission holds.
		if ( wait.queue != QueueKind::kGraphics || wait.epoch > m_Epoch ||
		     wait.value > m_Submitted )
			return Fail( DeviceStatus::kInvalidDescription, op );
	}
	std::unordered_map<std::uint64_t, ResourceUsage> states;
	std::vector<RecordingEncoder *> recorded;
	for ( std::unique_ptr<IEncoderBackend> &backend : backends )
	{
		RecordingEncoder *encoder = dynamic_cast<RecordingEncoder *>( backend.get() );
		if ( !encoder || encoder->Owner() != this || encoder->Queue() != queue )
			return Fail( DeviceStatus::kInvalidHandle, op );
		if ( !encoder->Complete() || !recording::Validate( encoder->Commands(), states, *this ) )
			return Fail( DeviceStatus::kInvalidState, op );
		recorded.push_back( encoder );
	}
	// D23, D30, D31: the capabilities the lists use, and each timestamp
	// buffer ends the submission in kCopyDestination.
	if ( const std::optional<DeviceStatus> refused =
	         recording::CheckSubmission( recorded, m_Facts.capabilities, states ) )
		return Fail( *refused, op );
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return Fail( DeviceStatus::kUnavailable, op );
	CheckReset();
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	// Usage state is the state after every accepted submission, whether or not
	// it has run yet: the next submission continues from it.
	for ( const auto &[id, usage] : states )
	{
		if ( TextureRecord *t = LiveTexture( id ) )
			t->usage = usage;
		else if ( BufferRecord *b = LiveBuffer( id ) )
			b->usage = usage;
	}
	const CompletionToken token{ queue, m_Epoch, ++m_Submitted };
	{
		std::lock_guard<std::mutex> ring( m_RingLock );
		for ( RecordingEncoder *encoder : recorded )
		{
			for ( std::uint64_t allocation : encoder->RingAllocations() )
			{
				if ( m_Options.sensitivity.unsafeUploadReuse )
					m_Ring.Abandon( allocation );
				else
					m_Ring.Submit( allocation, token );
			}
			encoder->MarkSubmitted();
		}
	}
	if ( m_Holding )
	{
		m_Held.push_back( { std::move( backends ), std::move( recorded ), token } );
		return token;
	}
	Issue( recorded, token );
	Gl().Flush();
	return token;
}

void GlDevice::Issue( std::vector<RecordingEncoder *> &encoders, CompletionToken token )
{
	Execute( encoders, token );
	m_Fences.push_back( { token.value, Gl().FenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ) } );
}

void GlDevice::IssueHeld()
{
	if ( m_Held.empty() )
		return;
	for ( HeldSubmission &held : m_Held )
		Issue( held.encoders, held.token );
	m_Held.clear();
	Gl().Flush();
}

void GlDevice::Hold( bool held )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	m_Holding = held;
	if ( held || m_State != DeviceState::kAvailable )
		return;
	ContextScope scope( *m_Context );
	if ( scope.Ok() )
		IssueHeld();
}

void GlDevice::UpdateCompletion() const
{
	const GlApi &gl = Gl();
	while ( !m_Fences.empty() )
	{
		const GLenum status = gl.ClientWaitSync( m_Fences.front().sync, 0, 0 );
		if ( status != GL_ALREADY_SIGNALED && status != GL_CONDITION_SATISFIED )
			break;
		m_Completed.store( m_Fences.front().value, std::memory_order_release );
		gl.DeleteSync( m_Fences.front().sync );
		m_Fences.pop_front();
	}
}

bool GlDevice::IsComplete( CompletionToken token ) const
{
	if ( !token.NamesSubmission() )
		return true;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( token.epoch < m_Epoch )
		return true;
	if ( token.epoch > m_Epoch || token.queue != QueueKind::kGraphics )
		return false;
	if ( token.value <= m_Completed.load( std::memory_order_acquire ) )
		return true;
	if ( m_State != DeviceState::kAvailable )
		return false;
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return false;
	UpdateCompletion();
	return token.value <= m_Completed.load( std::memory_order_acquire );
}

std::size_t GlDevice::Collect()
{
	const GlApi &gl = Gl();
	const std::uint64_t completed = m_Completed.load( std::memory_order_acquire );
	std::size_t freed = 0;
	for ( auto it = m_Releases.begin(); it != m_Releases.end(); )
	{
		const CompletionToken &token = it->token;
		const bool done = !token.NamesSubmission() || token.epoch < m_Epoch ||
		                  ( token.epoch == m_Epoch && token.value <= completed );
		if ( !done )
		{
			++it;
			continue;
		}
		Erase( it->resource );
		it = m_Releases.erase( it );
		++freed;
	}
	for ( auto it = m_Transients.begin(); it != m_Transients.end(); )
	{
		if ( it->value > completed )
		{
			++it;
			continue;
		}
		if ( it->query )
			gl.DeleteQueries( 1, &it->buffer );
		else
			gl.DeleteBuffers( 1, &it->buffer );
		it = m_Transients.erase( it );
	}
	std::lock_guard<std::mutex> ring( m_RingLock );
	m_Ring.Retire( m_Epoch, completed );
	return freed;
}

std::size_t GlDevice::Poll()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( !m_Context )
		return 0;
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return 0;
	if ( m_State == DeviceState::kAvailable )
		UpdateCompletion();
	return Collect();
}

DeviceResult<void> GlDevice::ReadBuffer(
    BufferId id, std::uint64_t offset, std::span<std::byte> out )
{
	const DeviceOperation op = DeviceOperation::kReadBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	BufferRecord *buffer = LiveBuffer( id.value );
	if ( !buffer )
		return Fail( DeviceStatus::kInvalidHandle, op );
	if ( buffer->desc.memory != MemoryKind::kReadback || offset > buffer->desc.size ||
	     out.size() > buffer->desc.size - offset )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return Fail( DeviceStatus::kUnavailable, op );
	Gl().GetNamedBufferSubData( buffer->name, static_cast<GLintptr>( offset ),
	    static_cast<GLsizeiptr>( out.size() ), out.data() );
	return {};
}

DeviceResult<void> GlDevice::WaitIdle()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return {};
	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return Fail( DeviceStatus::kUnavailable, DeviceOperation::kSubmit );
	// Idle means every accepted submission has run, held ones included.
	IssueHeld();
	Gl().Finish();
	UpdateCompletion();
	return {};
}

DeviceResult<void> GlDevice::Recover()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State == DeviceState::kAvailable )
		return {};
	m_State = DeviceState::kRecovering;
	// The lost context and everything in it go; a new context starts the new
	// epoch with nothing live. Tokens of the old epoch are complete.
	{
		ContextScope scope( *m_Context );
		if ( scope.Ok() )
			DestroyContextObjects();
	}
	m_Buffers.clear();
	m_Textures.clear();
	m_Samplers.clear();
	m_Layouts.clear();
	m_BindGroups.clear();
	m_Pipelines.clear();
	m_Programs.clear();
	m_Framebuffers.clear();
	m_Transients.clear();
	m_Fences.clear();
	m_Releases.clear();
	m_Held.clear();
	m_Context.reset();
	++m_Epoch;
	m_Submitted = 0;
	m_Completed.store( 0 );
	std::string error;
	m_Context = EglContext::Create( m_Options.api, m_Options.validation, error );
	if ( !m_Context || !CreateContextObjects() )
	{
		m_State = DeviceState::kFatal;
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreateDevice );
	}
	m_State = DeviceState::kAvailable;
	return {};
}

namespace
{

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateWithApi(
    const DeviceRequest &request, GlApiKind api )
{
	GlAdapterOptions options;
	options.api = api;
	options.validation = request.validation;
	auto created = Create( options );
	if ( !created )
		return created;
	if ( FirstMissing( created.Value()->Facts().capabilities, request.required ) )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
	return created;
}

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateFromRequest( const DeviceRequest &request )
{
	return CreateWithApi( request, GlApiKind::kDesktop45 );
}

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateEsFromRequest( const DeviceRequest &request )
{
	return CreateWithApi( request, GlApiKind::kEs31 );
}

GlDevice *Of( const IRenderDevice2 &device )
{
	return dynamic_cast<GlDevice *>( const_cast<IRenderDevice2 *>( &device ) );
}

} // namespace

const DeviceProviderDescriptor &Describe()
{
	static const DeviceProviderDescriptor descriptor{ "gl", &CreateFromRequest };
	return descriptor;
}

const DeviceProviderDescriptor &DescribeEs()
{
	static const DeviceProviderDescriptor descriptor{ "gles", &CreateEsFromRequest };
	return descriptor;
}

DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const GlAdapterOptions &options )
{
	auto device = std::make_unique<GlDevice>( options );
	if ( auto initialized = device->Initialize(); !initialized )
		return foundation::MakeUnexpected( initialized.Error() );
	return std::unique_ptr<IRenderDevice2>( std::move( device ) );
}

std::uint64_t ValidationMessages( const IRenderDevice2 &device )
{
	const GlDevice *gl = Of( device );
	return gl ? gl->ValidationMessages() : 0;
}

std::uint64_t DeferredUploads( const IRenderDevice2 &device )
{
	const GlDevice *gl = Of( device );
	return gl ? gl->DeferredUploads() : 0;
}

bool HoldSubmissions( IRenderDevice2 &device, bool held )
{
	GlDevice *gl = Of( device );
	if ( gl )
		gl->Hold( held );
	return gl != nullptr;
}

bool SimulateContextLoss( IRenderDevice2 &device )
{
	GlDevice *gl = Of( device );
	if ( gl )
		gl->SimulateLoss();
	return gl != nullptr;
}

} // namespace render::device::gl
