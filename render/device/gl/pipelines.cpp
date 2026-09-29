//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl: pipelines. GL has no pipeline objects, so a
//			pipeline is a cached, linked program (shared by pipelines whose
//			stages and specialization constants are the same), a vertex array
//			for its vertex input and the fixed-function state the replay applies.
//
//			The GLSL 4.50 artifacts follow tools/render/shader_artifacts.py
//			cross_compile: flat slots per group, combined samplers named after
//			their texture and sampler bindings, the draw constants a uniform
//			block at kDrawConstantsSlot, and specialization constant n
//			SPIRV-Cross's macro SPIRV_CROSS_CONSTANT_ID_n, with its type listed on
//			the line after #version (kSpecializationLine); the adapter defines
//			the macro as a literal of that type (clause D20).
//
//=============================================================================//

#include "gl_device.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string_view>

namespace render::device::gl
{

namespace
{

bool PowerOfTwo( std::uint32_t value )
{
	return value != 0 && ( value & ( value - 1 ) ) == 0;
}

GLenum StageType( ShaderStage stage )
{
	switch ( stage )
	{
	case ShaderStage::kVertex:
		return GL_VERTEX_SHADER;
	case ShaderStage::kFragment:
		return GL_FRAGMENT_SHADER;
	case ShaderStage::kCompute:
		return GL_COMPUTE_SHADER;
	}
	return GL_VERTEX_SHADER;
}

GLenum Topology( PrimitiveTopology topology )
{
	switch ( topology )
	{
	case PrimitiveTopology::kTriangleList:
		return GL_TRIANGLES;
	case PrimitiveTopology::kTriangleStrip:
		return GL_TRIANGLE_STRIP;
	case PrimitiveTopology::kLineList:
		return GL_LINES;
	case PrimitiveTopology::kPointList:
		return GL_POINTS;
	}
	return GL_TRIANGLES;
}

// "<prefix><group>_<binding>" at text; advances text past it.
bool ParseSlot(
    std::string_view &text, std::string_view prefix, std::uint32_t &group, std::uint32_t &binding )
{
	if ( text.substr( 0, prefix.size() ) != prefix )
		return false;
	text.remove_prefix( prefix.size() );
	auto number = [&]( std::uint32_t &value )
	{
		std::size_t digits = 0;
		value = 0;
		while ( digits < text.size() && text[digits] >= '0' && text[digits] <= '9' )
			value = value * 10 + static_cast<std::uint32_t>( text[digits++] - '0' );
		text.remove_prefix( digits );
		return digits > 0;
	};
	if ( !number( group ) || text.empty() || text[0] != '_' )
		return false;
	text.remove_prefix( 1 );
	return number( binding );
}

// The type the artifact's specialization line gives constant id, if any
// ("<id>:<type>" entries after kSpecializationLine).
std::string_view SpecializationType( std::string_view line, std::uint32_t id )
{
	const std::string key = " " + std::to_string( id ) + ":";
	const std::size_t at = line.find( key );
	if ( at == std::string_view::npos )
		return {};
	std::string_view type = line.substr( at + key.size() );
	return type.substr( 0, type.find( ' ' ) );
}

// A GLSL literal of type holding the constant's 32 bits; nothing for a
// type the artifacts do not carry or a float GLSL cannot write as a literal.
std::optional<std::string> Literal( std::string_view type, std::uint32_t bits )
{
	char text[64];
	if ( type == "int" )
		std::snprintf( text, sizeof( text ), "%d", static_cast<int>( bits ) );
	else if ( type == "uint" )
		std::snprintf( text, sizeof( text ), "%uu", bits );
	else if ( type == "bool" )
		std::snprintf( text, sizeof( text ), "%s", bits ? "true" : "false" );
	else if ( type == "float" )
	{
		float value = 0.0f;
		std::memcpy( &value, &bits, sizeof( value ) );
		if ( !std::isfinite( value ) )
			return std::nullopt;
		// Nine significant digits round-trip every finite float.
		std::snprintf( text, sizeof( text ), "%.9e", static_cast<double>( value ) );
	}
	else
		return std::nullopt;
	return std::string( text );
}

// The source a stage compiles: the artifact, with the stage's specialization
// constants defined after the #version line as literals of the types the
// artifact lists (clause D20); a constant the stage does not declare is
// ignored.
std::optional<std::string> StageSource(
    const ShaderArtifactView &stage, std::span<const SpecializationConstant> constants )
{
	std::string_view text( reinterpret_cast<const char *>( stage.code.data() ), stage.code.size() );
	while ( !text.empty() && text.back() == '\0' )
		text.remove_suffix( 1 );
	if ( text.substr( 0, 8 ) != "#version" || text.find( '\0' ) != std::string_view::npos )
		return std::nullopt;
	const std::size_t line = text.find( '\n' );
	if ( line == std::string_view::npos )
		return std::nullopt;
	std::string_view listed = text.substr( line + 1 );
	listed = listed.substr( 0, listed.find( '\n' ) );
	if ( listed.substr( 0, std::strlen( kSpecializationLine ) ) != kSpecializationLine )
		listed = {};
	std::string source( text.substr( 0, line + 1 ) );
	for ( const SpecializationConstant &constant : constants )
	{
		if ( constant.stage != stage.stage )
			continue;
		const std::string_view type = SpecializationType( listed, constant.id );
		if ( type.empty() )
			continue;
		const std::optional<std::string> literal = Literal( type, constant.value );
		if ( !literal )
			return std::nullopt;
		source += "#define SPIRV_CROSS_CONSTANT_ID_" + std::to_string( constant.id ) + " " +
		          *literal + "\n";
	}
	source.append( text.substr( line + 1 ) );
	return source;
}

} // namespace

DeviceResult<const Program *> GlDevice::AcquireProgram(
    const PipelineDesc &desc, std::string &key, std::uint32_t &nativeCode )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	const GlApi &gl = Gl();
	std::vector<std::pair<GLenum, std::string>> sources;
	const std::span<const SpecializationConstant> constants =
	    m_Options.sensitivity.dropSpecialization ? std::span<const SpecializationConstant>()
	                                             : desc.constants;
	key.clear();
	for ( const ShaderArtifactView &stage : desc.stages )
	{
		std::optional<std::string> source = StageSource( stage, constants );
		if ( !source )
			return Fail( DeviceStatus::kInvalidDescription, op );
		key += std::to_string( static_cast<int>( stage.stage ) ) + ":" + *source + "\n";
		sources.emplace_back( StageType( stage.stage ), std::move( *source ) );
	}
	if ( const auto found = m_Programs.find( key ); found != m_Programs.end() )
	{
		++found->second.users;
		return &found->second;
	}

	Program program;
	program.name = gl.CreateProgram();
	std::vector<GLuint> shaders;
	auto cleanup = [&]
	{
		for ( GLuint shader : shaders )
		{
			gl.DetachShader( program.name, shader );
			gl.DeleteShader( shader );
		}
	};
	for ( const auto &[type, source] : sources )
	{
		const GLuint shader = gl.CreateShader( type );
		shaders.push_back( shader );
		const GLchar *text = source.c_str();
		gl.ShaderSource( shader, 1, &text, nullptr );
		gl.CompileShader( shader );
		GLint compiled = GL_FALSE;
		gl.GetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
		if ( !compiled )
		{
			char log[1024] = {};
			gl.GetShaderInfoLog( shader, sizeof( log ), nullptr, log );
			std::fprintf( stderr, "render.device.gl: %s does not compile: %s\n",
			    desc.debugName.empty() ? "a pipeline stage" : std::string( desc.debugName ).c_str(),
			    log );
			cleanup();
			gl.DeleteProgram( program.name );
			return Fail( DeviceStatus::kInvalidDescription, op );
		}
		gl.AttachShader( program.name, shader );
	}
	gl.LinkProgram( program.name );
	cleanup();
	GLint linked = GL_FALSE;
	gl.GetProgramiv( program.name, GL_LINK_STATUS, &linked );
	if ( !linked )
	{
		char log[1024] = {};
		gl.GetProgramInfoLog( program.name, sizeof( log ), nullptr, log );
		std::fprintf( stderr, "render.device.gl: a pipeline does not link: %s\n", log );
		gl.DeleteProgram( program.name );
		return Fail( DeviceStatus::kInvalidDescription, op );
	}

	// What the program reads beyond its slots: combined samplers (each on its
	// texture's slot, paired with a sampler binding), the draw constants and
	// the base instance.
	bool mismatch = false;
	GLint uniforms = 0;
	gl.GetProgramInterfaceiv( program.name, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniforms );
	for ( GLint index = 0; index < uniforms; ++index )
	{
		char name[256] = {};
		gl.GetProgramResourceName(
		    program.name, GL_UNIFORM, static_cast<GLuint>( index ), sizeof( name ), nullptr, name );
		std::string_view text( name );
		if ( text.substr( 0, std::strlen( kCombinedPrefix ) ) != kCombinedPrefix )
			continue;
		text.remove_prefix( std::strlen( kCombinedPrefix ) );
		Program::Combined combined;
		std::uint32_t group = 0, binding = 0;
		if ( !ParseSlot( text, kTexturePrefix, group, binding ) )
		{
			mismatch = true;
			continue;
		}
		const GLenum property = GL_LOCATION;
		GLint location = -1;
		gl.GetProgramResourceiv( program.name, GL_UNIFORM, static_cast<GLuint>( index ), 1,
		    &property, 1, nullptr, &location );
		gl.GetUniformiv( program.name, location, &combined.unit );
		if ( combined.unit != GLint( FlatSlot( group, binding ) ) )
			mismatch = true;
		if ( ParseSlot( text, kSamplerPrefix, combined.samplerGroup, combined.samplerBinding ) )
			combined.sampler = true;
		else if ( text.substr( 0, std::strlen( kDummySampler ) ) != kDummySampler )
			mismatch = true;
		program.combined.push_back( combined );
	}
	program.baseInstance = gl.GetUniformLocation( program.name, kBaseInstanceUniform );
	const GLuint block = gl.GetUniformBlockIndex( program.name, "RenderDrawConstants" );
	if ( block != GL_INVALID_INDEX )
	{
		GLint slot = -1;
		gl.GetActiveUniformBlockiv( program.name, block, GL_UNIFORM_BLOCK_BINDING, &slot );
		if ( slot != GLint( kDrawConstantsSlot ) )
			mismatch = true;
	}
	if ( mismatch )
	{
		// Not an artifact of shader_artifacts.py's form.
		gl.DeleteProgram( program.name );
		nativeCode = 0;
		return Fail( DeviceStatus::kLayoutMismatch, op );
	}
	program.users = 1;
	auto inserted = m_Programs.emplace( key, std::move( program ) );
	return &inserted.first->second;
}

void GlDevice::ReleaseProgram( const std::string &key )
{
	const auto found = m_Programs.find( key );
	if ( found == m_Programs.end() || --found->second.users > 0 )
		return;
	Gl().DeleteProgram( found->second.name );
	m_Programs.erase( found );
}

DeviceResult<PipelineId> GlDevice::CreatePipeline( const PipelineDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	auto valid = ValidatePipeline( desc, m_Facts,
	    [this]( BindGroupLayoutId id )
	    {
		    return FindLayout( id );
	    } );
	if ( !valid )
		return foundation::MakeUnexpected( valid.Error() );

	// What GL needs beyond the shared rules: one stage of each kind, the
	// stages of the pipeline's kind, formats in their places, a supported
	// sample count and vertex attributes that name a declared buffer.
	bool seen[3] = {};
	for ( const ShaderArtifactView &stage : desc.stages )
	{
		const std::size_t index = static_cast<std::size_t>( stage.stage );
		if ( index >= 3 || seen[index] )
			return Fail( DeviceStatus::kInvalidDescription, op );
		seen[index] = true;
	}
	const bool compute = desc.kind == PipelineKind::kCompute;
	if ( compute ? ( !seen[2] || seen[0] || seen[1] ) : ( !seen[0] || seen[2] ) )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( compute && !m_Facts.capabilities.Has( Capability::kCompute ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( !compute )
	{
		if ( !PowerOfTwo( desc.sampleCount ) )
			return Fail( DeviceStatus::kInvalidDescription, op );
		if ( !( m_Facts.limits.sampleCounts & desc.sampleCount ) )
			return Fail( DeviceStatus::kUnsupported, op );
		if ( desc.depthFormat != Format::kUnknown && !IsDepthFormat( desc.depthFormat ) )
			return Fail( DeviceStatus::kInvalidDescription, op );
		for ( Format format : desc.colorFormats )
		{
			if ( format == Format::kUnknown || format >= Format::kCount || IsDepthFormat( format ) )
				return Fail( DeviceStatus::kInvalidDescription, op );
		}
		if ( desc.colorFormats.size() > m_Facts.limits.maxColorAttachments ||
		     desc.vertex.buffers.size() > kMaxVertexSlots ||
		     desc.vertex.attributes.size() > kMaxVertexSlots )
			return Fail( DeviceStatus::kUnsupported, op );
		for ( const VertexAttribute &attribute : desc.vertex.attributes )
		{
			if ( attribute.bufferSlot >= desc.vertex.buffers.size() ||
			     attribute.location >= kMaxVertexSlots )
				return Fail( DeviceStatus::kInvalidDescription, op );
		}
	}

	ContextScope scope( *m_Context );
	if ( !scope.Ok() )
		return Fail( DeviceStatus::kUnavailable, op );
	const GlApi &gl = Gl();
	PipelineRecord record;
	std::uint32_t nativeCode = 0;
	auto program = AcquireProgram( desc, record.programKey, nativeCode );
	if ( !program )
		return foundation::MakeUnexpected( program.Error() );
	record.program = program.Value();
	record.kind = desc.kind;
	record.drawConstantBytes = desc.drawConstantBytes;
	record.topology = Topology( desc.topology );
	record.raster = desc.raster;
	record.depthStencil = desc.depthStencil;
	record.colorFormats.assign( desc.colorFormats.begin(), desc.colorFormats.end() );
	record.blends.assign( desc.blends.begin(), desc.blends.end() );
	record.writeMasks.assign( desc.colorWriteMasks.begin(), desc.colorWriteMasks.end() );
	record.depthFormat = desc.depthFormat;
	record.sampleCount = desc.sampleCount;
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		if ( role >= desc.layouts.size() || !desc.layouts[role].IsValid() )
			continue;
		record.layouts[role] = desc.layouts[role];
		record.layoutHasBindings[role] =
		    !m_Layouts.find( desc.layouts[role].value )->second.bindings.empty();
	}
	if ( !compute )
	{
		gl.CreateVertexArrays( 1, &record.vertexArray );
		record.vertexBuffers = static_cast<std::uint32_t>( desc.vertex.buffers.size() );
		for ( std::uint32_t slot = 0; slot < record.vertexBuffers; ++slot )
		{
			record.strides[slot] = desc.vertex.buffers[slot].stride;
			gl.VertexArrayBindingDivisor(
			    record.vertexArray, slot, desc.vertex.buffers[slot].perInstance ? 1 : 0 );
		}
		for ( const VertexAttribute &attribute : desc.vertex.attributes )
		{
			GLint size = 4;
			GLenum type = GL_FLOAT;
			GLboolean normalized = GL_FALSE;
			switch ( attribute.format )
			{
			case VertexFormat::kFloat2:
				size = 2;
				break;
			case VertexFormat::kFloat3:
				size = 3;
				break;
			case VertexFormat::kFloat4:
				break;
			case VertexFormat::kUnorm8x4:
				type = GL_UNSIGNED_BYTE;
				normalized = GL_TRUE;
				break;
			}
			gl.EnableVertexArrayAttrib( record.vertexArray, attribute.location );
			gl.VertexArrayAttribFormat(
			    record.vertexArray, attribute.location, size, type, normalized, attribute.offset );
			gl.VertexArrayAttribBinding(
			    record.vertexArray, attribute.location, attribute.bufferSlot );
		}
	}
	if ( !desc.debugName.empty() )
		gl.ObjectLabel( GL_PROGRAM, record.program->name,
		    static_cast<GLsizei>( desc.debugName.size() ), desc.debugName.data() );
	const PipelineId id{ ++m_NextId };
	m_Pipelines.emplace( id.value, std::move( record ) );
	return id;
}

} // namespace render::device::gl
