//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.webgpu: pipelines. Each stage's WGSL artifact
//			becomes a shader module; its header lines give the WebGPU binding
//			types the stage uses, from which the pipeline's group layouts are
//			made (or found in the cache), and the overrides it declares, which
//			take the pipeline's specialization constants (D20).
//
//=============================================================================//

#include "webgpu_device.h"

#include <bit>
#include <charconv>
#include <string_view>
#include <optional>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace render::device::webgpu
{

namespace
{

// The next whitespace-separated word of text, consumed.
std::string_view Word( std::string_view &text )
{
	const std::size_t begin = text.find_first_not_of( ' ' );
	if ( begin == std::string_view::npos )
	{
		text = {};
		return {};
	}
	text.remove_prefix( begin );
	const std::size_t end = std::min( text.find( ' ' ), text.size() );
	const std::string_view word = text.substr( 0, end );
	text.remove_prefix( end );
	return word;
}

bool Number( std::string_view word, std::uint32_t &value )
{
	const auto [end, error] = std::from_chars( word.data(), word.data() + word.size(), value );
	return error == std::errc() && end == word.data() + word.size();
}

std::optional<WGPUTextureViewDimension> DimensionOf( std::string_view word )
{
	if ( word == "1d" )
		return WGPUTextureViewDimension_1D;
	if ( word == "2d" )
		return WGPUTextureViewDimension_2D;
	if ( word == "2d-array" )
		return WGPUTextureViewDimension_2DArray;
	if ( word == "3d" )
		return WGPUTextureViewDimension_3D;
	if ( word == "cube" )
		return WGPUTextureViewDimension_Cube;
	if ( word == "cube-array" )
		return WGPUTextureViewDimension_CubeArray;
	return std::nullopt;
}

WGPUTextureFormat StorageFormatOf( std::string_view word )
{
	struct Name
	{
		std::string_view wgsl;
		WGPUTextureFormat format;
	};
	static constexpr Name kNames[] = { { "rgba8unorm", WGPUTextureFormat_RGBA8Unorm },
	    { "rgba8snorm", WGPUTextureFormat_RGBA8Snorm },
	    { "rgba8uint", WGPUTextureFormat_RGBA8Uint }, { "rgba8sint", WGPUTextureFormat_RGBA8Sint },
	    { "bgra8unorm", WGPUTextureFormat_BGRA8Unorm },
	    { "rgba16float", WGPUTextureFormat_RGBA16Float },
	    { "rgba16uint", WGPUTextureFormat_RGBA16Uint },
	    { "rgba16sint", WGPUTextureFormat_RGBA16Sint }, { "r32float", WGPUTextureFormat_R32Float },
	    { "r32uint", WGPUTextureFormat_R32Uint }, { "r32sint", WGPUTextureFormat_R32Sint },
	    { "rg32float", WGPUTextureFormat_RG32Float }, { "rg32uint", WGPUTextureFormat_RG32Uint },
	    { "rg32sint", WGPUTextureFormat_RG32Sint },
	    { "rgba32float", WGPUTextureFormat_RGBA32Float },
	    { "rgba32uint", WGPUTextureFormat_RGBA32Uint },
	    { "rgba32sint", WGPUTextureFormat_RGBA32Sint } };
	for ( const Name &name : kNames )
	{
		if ( name.wgsl == word )
			return name.format;
	}
	return WGPUTextureFormat_Undefined;
}

WGPUShaderStage VisibilityOf( ShaderStage stage )
{
	switch ( stage )
	{
	case ShaderStage::kVertex:
		return WGPUShaderStage_Vertex;
	case ShaderStage::kFragment:
		return WGPUShaderStage_Fragment;
	case ShaderStage::kCompute:
		return WGPUShaderStage_Compute;
	}
	return WGPUShaderStage_None;
}

bool ReadBinding( std::string_view rest, BindingLine &line )
{
	if ( !Number( Word( rest ), line.group ) || !Number( Word( rest ), line.binding ) ||
	     line.group >= kMaxBindGroups )
		return false;
	const std::string_view kind = Word( rest );
	if ( kind == "uniform" )
		line.kind = BindingLine::Kind::kUniform;
	else if ( kind == "storage" )
		line.kind = BindingLine::Kind::kStorage;
	else if ( kind == "read-only-storage" )
		line.kind = BindingLine::Kind::kReadOnlyStorage;
	else if ( kind == "sampler" )
	{
		line.kind = BindingLine::Kind::kSampler;
		const std::string_view type = Word( rest );
		if ( type == "filtering" )
			line.sampler = WGPUSamplerBindingType_Filtering;
		else if ( type == "non-filtering" )
			line.sampler = WGPUSamplerBindingType_NonFiltering;
		else if ( type == "comparison" )
			line.sampler = WGPUSamplerBindingType_Comparison;
		else
			return false;
	}
	else if ( kind == "texture" )
	{
		line.kind = BindingLine::Kind::kTexture;
		const std::string_view sample = Word( rest );
		if ( sample == "float" )
			line.sampleType = WGPUTextureSampleType_Float;
		else if ( sample == "unfilterable-float" )
			line.sampleType = WGPUTextureSampleType_UnfilterableFloat;
		else if ( sample == "depth" )
			line.sampleType = WGPUTextureSampleType_Depth;
		else if ( sample == "sint" )
			line.sampleType = WGPUTextureSampleType_Sint;
		else if ( sample == "uint" )
			line.sampleType = WGPUTextureSampleType_Uint;
		else
			return false;
		const std::optional<WGPUTextureViewDimension> dimension = DimensionOf( Word( rest ) );
		std::uint32_t multisampled = 0;
		if ( !dimension || !Number( Word( rest ), multisampled ) )
			return false;
		line.dimension = *dimension;
		line.multisampled = multisampled != 0;
	}
	else if ( kind == "storage-texture" )
	{
		line.kind = BindingLine::Kind::kStorageTexture;
		const std::string_view access = Word( rest );
		if ( access == "write-only" )
			line.access = WGPUStorageTextureAccess_WriteOnly;
		else if ( access == "read-only" )
			line.access = WGPUStorageTextureAccess_ReadOnly;
		else if ( access == "read-write" )
			line.access = WGPUStorageTextureAccess_ReadWrite;
		else
			return false;
		line.storageFormat = StorageFormatOf( Word( rest ) );
		const std::optional<WGPUTextureViewDimension> dimension = DimensionOf( Word( rest ) );
		if ( line.storageFormat == WGPUTextureFormat_Undefined || !dimension )
			return false;
		line.dimension = *dimension;
	}
	else
	{
		return false;
	}
	// "alias <binding>": the WGSL moved a second view of the port binding
	// <binding> here (WGSL binds one variable a binding); the group's entry
	// for <binding> fills it.
	// Optional: "min <bytes>", the WGSL's minimum binding size (a buffer
	// smaller than it is bound up to it); "alias <binding>".
	line.source = line.binding;
	for ( std::string_view key = Word( rest ); !key.empty(); key = Word( rest ) )
	{
		std::uint32_t value = 0;
		if ( !Number( Word( rest ), value ) )
			return false;
		if ( key == "min" )
			line.minSize = value;
		else if ( key == "alias" )
			line.source = value;
		else
			return false;
	}
	return true;
}

// Two stages' uses of one binding as one layout entry: filtering wins over
// unfilterable textures (one stage samples it), non-filtering over filtering
// samplers (one stage reads a depth texture through it).
bool Merge( BindingLine &into, const BindingLine &from )
{
	if ( into.kind != from.kind || into.source != from.source )
		return false;
	into.visibility |= from.visibility;
	into.minSize = std::max( into.minSize, from.minSize );
	if ( into.kind == BindingLine::Kind::kTexture )
	{
		if ( into.dimension != from.dimension || into.multisampled != from.multisampled )
			return false;
		if ( into.sampleType == WGPUTextureSampleType_UnfilterableFloat &&
		     from.sampleType == WGPUTextureSampleType_Float )
			into.sampleType = from.sampleType;
		else if ( !( into.sampleType == WGPUTextureSampleType_Float &&
		              from.sampleType == WGPUTextureSampleType_UnfilterableFloat ) &&
		          into.sampleType != from.sampleType )
			return false;
	}
	else if ( into.kind == BindingLine::Kind::kSampler )
	{
		if ( into.sampler == WGPUSamplerBindingType_Comparison ||
		     from.sampler == WGPUSamplerBindingType_Comparison )
			return into.sampler == from.sampler;
		if ( from.sampler == WGPUSamplerBindingType_NonFiltering )
			into.sampler = from.sampler;
	}
	else if ( into.kind == BindingLine::Kind::kStorageTexture )
	{
		return into.access == from.access && into.storageFormat == from.storageFormat &&
		       into.dimension == from.dimension;
	}
	return true;
}

WGPUVertexFormat VertexFormatOf( VertexFormat format )
{
	switch ( format )
	{
	case VertexFormat::kFloat2:
		return WGPUVertexFormat_Float32x2;
	case VertexFormat::kFloat3:
		return WGPUVertexFormat_Float32x3;
	case VertexFormat::kFloat4:
		return WGPUVertexFormat_Float32x4;
	case VertexFormat::kUnorm8x4:
		return WGPUVertexFormat_Unorm8x4;
	}
	return WGPUVertexFormat_Float32x4;
}

WGPUCompareFunction CompareFunction( CompareOp op )
{
	switch ( op )
	{
	case CompareOp::kNever:
		return WGPUCompareFunction_Never;
	case CompareOp::kLess:
		return WGPUCompareFunction_Less;
	case CompareOp::kLessEqual:
		return WGPUCompareFunction_LessEqual;
	case CompareOp::kEqual:
		return WGPUCompareFunction_Equal;
	case CompareOp::kGreaterEqual:
		return WGPUCompareFunction_GreaterEqual;
	case CompareOp::kGreater:
		return WGPUCompareFunction_Greater;
	case CompareOp::kAlways:
		return WGPUCompareFunction_Always;
	case CompareOp::kNotEqual:
		return WGPUCompareFunction_NotEqual;
	}
	return WGPUCompareFunction_Always;
}

WGPUStencilOperation StencilOperation( StencilOp op )
{
	switch ( op )
	{
	case StencilOp::kKeep:
		return WGPUStencilOperation_Keep;
	case StencilOp::kZero:
		return WGPUStencilOperation_Zero;
	case StencilOp::kReplace:
		return WGPUStencilOperation_Replace;
	case StencilOp::kIncrementClamp:
		return WGPUStencilOperation_IncrementClamp;
	case StencilOp::kDecrementClamp:
		return WGPUStencilOperation_DecrementClamp;
	case StencilOp::kInvert:
		return WGPUStencilOperation_Invert;
	case StencilOp::kIncrementWrap:
		return WGPUStencilOperation_IncrementWrap;
	case StencilOp::kDecrementWrap:
		return WGPUStencilOperation_DecrementWrap;
	}
	return WGPUStencilOperation_Keep;
}

// The port's blend modes (pipeline.h); nullopt for opaque.
std::optional<WGPUBlendState> BlendOf( BlendMode mode )
{
	auto state = []( WGPUBlendFactor srcRgb, WGPUBlendFactor dstRgb, WGPUBlendFactor srcAlpha,
	                 WGPUBlendFactor dstAlpha )
	{
		WGPUBlendState blend;
		blend.color = { WGPUBlendOperation_Add, srcRgb, dstRgb };
		blend.alpha = { WGPUBlendOperation_Add, srcAlpha, dstAlpha };
		return blend;
	};
	switch ( mode )
	{
	case BlendMode::kOpaque:
		return std::nullopt;
	case BlendMode::kAlpha:
		return state( WGPUBlendFactor_SrcAlpha, WGPUBlendFactor_OneMinusSrcAlpha,
		    WGPUBlendFactor_One, WGPUBlendFactor_OneMinusSrcAlpha );
	case BlendMode::kPremultiplied:
		return state( WGPUBlendFactor_One, WGPUBlendFactor_OneMinusSrcAlpha, WGPUBlendFactor_One,
		    WGPUBlendFactor_OneMinusSrcAlpha );
	case BlendMode::kAdditive:
		return state(
		    WGPUBlendFactor_One, WGPUBlendFactor_One, WGPUBlendFactor_One, WGPUBlendFactor_One );
	case BlendMode::kTransmittance:
		return state( WGPUBlendFactor_One, WGPUBlendFactor_SrcAlpha, WGPUBlendFactor_Zero,
		    WGPUBlendFactor_One );
	case BlendMode::kModulate2x:
		return state(
		    WGPUBlendFactor_Dst, WGPUBlendFactor_Src, WGPUBlendFactor_Zero, WGPUBlendFactor_One );
	case BlendMode::kAlphaAdditive:
		return state( WGPUBlendFactor_SrcAlpha, WGPUBlendFactor_One, WGPUBlendFactor_SrcAlpha,
		    WGPUBlendFactor_One );
	}
	return std::nullopt;
}

// An error scope's verdict, waited for: false when the calls since the push
// raised a validation error (printed with what).
bool ScopeClean( WGPUInstance instance, WGPUDevice device, std::string_view what )
{
	struct Result
	{
		bool clean = true;
		std::string_view what;
	} result{ true, what };
	WGPUPopErrorScopeCallbackInfo scope = WGPU_POP_ERROR_SCOPE_CALLBACK_INFO_INIT;
	scope.mode = WGPUCallbackMode_WaitAnyOnly;
	scope.callback = []( WGPUPopErrorScopeStatus status, WGPUErrorType type, WGPUStringView message,
	                     void *userdata1, void * )
	{
		auto *r = static_cast<Result *>( userdata1 );
		if ( status != WGPUPopErrorScopeStatus_Success || type != WGPUErrorType_NoError )
		{
			r->clean = false;
			std::fprintf( stderr, "render.device.webgpu: %.*s refused: %.*s\n",
			    static_cast<int>( r->what.size() ), r->what.data(),
			    static_cast<int>( Text( message ).size() ), Text( message ).data() );
		}
	};
	scope.userdata1 = &result;
	WGPUFutureWaitInfo wait{ wgpuDevicePopErrorScope( device, scope ), false };
	if ( wgpuInstanceWaitAny( instance, 1, &wait, std::numeric_limits<std::uint64_t>::max() ) !=
	     WGPUWaitStatus_Success )
		return false;
	return result.clean;
}

} // namespace

bool ReadArtifactHeader( std::string_view text, ShaderStage stage, ArtifactHeader &header )
{
	while ( !text.empty() )
	{
		const std::size_t end = text.find( '\n' );
		std::string_view line = text.substr( 0, end );
		text = end == std::string_view::npos ? std::string_view() : text.substr( end + 1 );
		if ( line.substr( 0, kArtifactLine.size() ) != kArtifactLine )
			break; // the WGSL itself
		line.remove_prefix( kArtifactLine.size() );
		const std::string_view what = Word( line );
		if ( what == "binding" )
		{
			BindingLine binding;
			binding.visibility = VisibilityOf( stage );
			if ( !ReadBinding( line, binding ) )
				return false;
			header.bindings.push_back( binding );
		}
		else if ( what == "draw-constants" )
		{
			if ( !Number( Word( line ), header.drawConstantBytes ) )
				return false;
		}
		else if ( what == "override" )
		{
			OverrideLine entry;
			const std::string_view type = ( Number( Word( line ), entry.id ), Word( line ) );
			if ( type == "bool" )
				entry.type = OverrideLine::Type::kBool;
			else if ( type == "i32" )
				entry.type = OverrideLine::Type::kInt;
			else if ( type == "u32" )
				entry.type = OverrideLine::Type::kUint;
			else if ( type == "f32" )
				entry.type = OverrideLine::Type::kFloat;
			else
				return false;
			header.overrides.push_back( entry );
		}
		else
		{
			return false;
		}
	}
	return true;
}

const GroupLayout *WebGpuDevice::CachedLayout(
    std::vector<BindingLine> entries, bool drawConstants )
{
	std::sort( entries.begin(), entries.end(),
	    []( const BindingLine &a, const BindingLine &b )
	    {
		    return a.binding < b.binding;
	    } );
	std::string key;
	for ( const BindingLine &e : entries )
	{
		key += std::to_string( e.binding ) + "<" + std::to_string( e.source ) + "^" +
		       std::to_string( e.minSize ) + ":" +
		       std::to_string( int( e.kind ) ) + ":" +
		       std::to_string( int( e.sampleType ) ) + ":" + std::to_string( int( e.dimension ) ) +
		       ":" + std::to_string( int( e.multisampled ) ) + ":" +
		       std::to_string( int( e.sampler ) ) + ":" + std::to_string( int( e.access ) ) + ":" +
		       std::to_string( int( e.storageFormat ) ) + ":" +
		       std::to_string( std::uint64_t( e.visibility ) ) + ";";
	}
	if ( drawConstants )
		key += "constants";
	if ( const auto found = m_GroupLayouts.find( key ); found != m_GroupLayouts.end() )
		return &found->second;
	std::vector<WGPUBindGroupLayoutEntry> native;
	for ( const BindingLine &e : entries )
	{
		WGPUBindGroupLayoutEntry entry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
		entry.binding = e.binding;
		entry.visibility = e.visibility;
		switch ( e.kind )
		{
		case BindingLine::Kind::kUniform:
			entry.buffer.type = WGPUBufferBindingType_Uniform;
			break;
		case BindingLine::Kind::kStorage:
			entry.buffer.type = WGPUBufferBindingType_Storage;
			break;
		case BindingLine::Kind::kReadOnlyStorage:
			entry.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
			break;
		case BindingLine::Kind::kTexture:
			entry.texture.sampleType = e.sampleType;
			entry.texture.viewDimension = e.dimension;
			entry.texture.multisampled = e.multisampled;
			break;
		case BindingLine::Kind::kSampler:
			entry.sampler.type = e.sampler;
			break;
		case BindingLine::Kind::kStorageTexture:
			entry.storageTexture.access = e.access;
			entry.storageTexture.format = e.storageFormat;
			entry.storageTexture.viewDimension = e.dimension;
			break;
		}
		native.push_back( entry );
	}
	if ( drawConstants )
	{
		WGPUBindGroupLayoutEntry entry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
		entry.binding = kDrawConstantsBinding;
		entry.visibility =
		    WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute;
		entry.buffer.type = WGPUBufferBindingType_Uniform;
		entry.buffer.hasDynamicOffset = true;
		entry.buffer.minBindingSize = kMaxDrawConstantBytes;
		native.push_back( entry );
	}
	WGPUBindGroupLayoutDescriptor descriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
	descriptor.entryCount = native.size();
	descriptor.entries = native.data();
	GroupLayout layout;
	layout.key = key;
	layout.layout.Reset( wgpuDeviceCreateBindGroupLayout( m_Device, &descriptor ) );
	layout.entries = std::move( entries );
	layout.drawConstants = drawConstants;
	return &m_GroupLayouts.emplace( key, std::move( layout ) ).first->second;
}

// The WGSL with every module-scope override as a const: `@id(N) override x :
// T = d;` takes values[N] (or d), and a derived `override y : T = e;` keeps e.
// Lines are rewritten in place; nothing else in the text changes.
static std::string SpecializeOverrides(
    std::string_view text, const std::map<std::uint32_t, std::string> &values )
{
	std::string out;
	out.reserve( text.size() );
	size_t at = 0;
	while ( at < text.size() )
	{
		const size_t eol = std::min( text.find( '\n', at ), text.size() );
		std::string_view line = text.substr( at, eol - at );
		at = eol + 1;
		std::optional<std::uint32_t> id;
		std::string_view rest = line;
		if ( rest.starts_with( "@id(" ) )
		{
			const size_t close = rest.find( ')' );
			std::uint32_t n = 0;
			if ( close == std::string_view::npos ||
			     std::from_chars( rest.data() + 4, rest.data() + close, n ).ec != std::errc() )
			{
				out.append( line ).push_back( '\n' );
				continue;
			}
			id = n;
			rest = rest.substr( close + 1 );
			while ( !rest.empty() && rest.front() == ' ' )
				rest.remove_prefix( 1 );
		}
		if ( !rest.starts_with( "override " ) )
		{
			out.append( line ).push_back( '\n' );
			continue;
		}
		std::string_view declaration = rest.substr( 9 ); // "x : T = d;"
		const auto value = id ? values.find( *id ) : values.end();
		if ( value != values.end() )
		{
			const size_t assign = declaration.find( '=' );
			const size_t semicolon = declaration.rfind( ';' );
			const std::string_view head =
			    declaration.substr( 0, std::min( assign, semicolon ) ); // "x : T "
			out.append( "const " ).append( head );
			if ( !head.empty() && head.back() != ' ' )
				out.push_back( ' ' );
			out.append( "= " ).append( value->second ).append( ";\n" );
		}
		else if ( declaration.find( ": bool" ) != std::string_view::npos )
		{
			// Boolean | and & as || and && (the same values without side
			// effects): WGSL compilers before naga 30 cannot evaluate the
			// former in a const expression, and tint writes them for SPIR-V's
			// logical operations.
			std::string rewritten( declaration );
			for ( size_t p = 0; ( p = rewritten.find( " | ", p ) ) != std::string::npos; p += 4 )
				rewritten.replace( p, 3, " || " );
			for ( size_t p = 0; ( p = rewritten.find( " & ", p ) ) != std::string::npos; p += 4 )
				rewritten.replace( p, 3, " && " );
			out.append( "const " ).append( rewritten ).push_back( '\n' );
		}
		else
		{
			out.append( "const " ).append( declaration ).push_back( '\n' );
		}
	}
	if ( !text.empty() && text.back() != '\n' && !out.empty() )
		out.pop_back();
	return out;
}

DeviceResult<void> WebGpuDevice::BuildShader( const ShaderArtifactView &artifact,
    const PipelineDesc &desc, ShaderModule &module, ArtifactHeader &header,
    std::deque<std::string> &keys, std::vector<WGPUConstantEntry> &constants )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	const std::string_view text(
	    reinterpret_cast<const char *>( artifact.code.data() ), artifact.code.size() );
	if ( !ReadArtifactHeader( text, artifact.stage, header ) )
		return Fail( DeviceStatus::kInvalidDescription, op );
	// D20: each constant this stage declares takes its value with its type;
	// ids it does not declare are ignored. The values are written into the
	// WGSL (each override becomes a const) rather than passed as pipeline
	// constants: browsers' WGSL compilers differ in how well they specialize
	// overrides (Firefox's refuses the surface programs' derived overrides),
	// while const expressions are core WGSL every implementation evaluates.
	std::map<std::uint32_t, std::string> values;
	for ( const SpecializationConstant &constant : desc.constants )
	{
		if ( constant.stage != artifact.stage )
			continue;
		const auto declared = std::find_if( header.overrides.begin(), header.overrides.end(),
		    [&]( const OverrideLine &o )
		    {
			    return o.id == constant.id;
		    } );
		if ( declared == header.overrides.end() )
			continue;
		std::string literal;
		switch ( declared->type )
		{
		case OverrideLine::Type::kBool:
			literal = constant.value != 0 ? "true" : "false";
			break;
		case OverrideLine::Type::kInt:
			literal = std::to_string( static_cast<std::int32_t>( constant.value ) ) + "i";
			break;
		case OverrideLine::Type::kUint:
			literal = std::to_string( constant.value ) + "u";
			break;
		case OverrideLine::Type::kFloat:
		{
			char digits[32];
			const auto written = std::to_chars(
			    digits, digits + sizeof( digits ), std::bit_cast<float>( constant.value ) );
			literal = std::string( digits, written.ptr ) + "f";
			break;
		}
		}
		if ( std::getenv( "SOURCE_WEBGPU_LOG_CONSTANTS" ) )
			std::fprintf( stderr, "render.device.webgpu: %.*s constant %u = %s\n",
			    int( desc.debugName.size() ), desc.debugName.data(), constant.id, literal.c_str() );
		values[constant.id] = std::move( literal );
	}
	const std::string specialized = SpecializeOverrides( text, values );
	// SOURCE_WEBGPU_DUMP_WGSL=<dir>: each module's specialized WGSL, to check it
	// with another WGSL compiler (naga, tint).
	if ( const char *dump = std::getenv( "SOURCE_WEBGPU_DUMP_WGSL" ) )
	{
		static int s_dumped = 0;
		const std::string path = std::string( dump ) + "/" + std::to_string( ++s_dumped ) + "-" +
		                         std::string( desc.debugName ) +
		                         ( artifact.stage == ShaderStage::kFragment ? ".frag" : ".other" ) +
		                         ".wgsl";
		if ( FILE *f = std::fopen( path.c_str(), "wb" ) )
		{
			std::fwrite( specialized.data(), 1, specialized.size(), f );
			std::fclose( f );
		}
	}
	WGPUShaderSourceWGSL source = WGPU_SHADER_SOURCE_WGSL_INIT;
	source.code = View( specialized );
	WGPUShaderModuleDescriptor descriptor = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
	descriptor.nextInChain = &source.chain;
	descriptor.label = View( desc.debugName );
	wgpuDevicePushErrorScope( m_Device, WGPUErrorFilter_Validation );
	module.Reset( wgpuDeviceCreateShaderModule( m_Device, &descriptor ) );
	if ( !ScopeClean( m_Instance, m_Device, "WGSL module" ) || !module )
		return Fail( DeviceStatus::kInternal, op );
	(void)keys;
	(void)constants;
	return {};
}

DeviceResult<PipelineId> WebGpuDevice::CreatePipeline( const PipelineDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	const LayoutLookup lookup = [this]( BindGroupLayoutId id )
	{
		return FindLayout( id );
	};
	if ( auto valid = ValidatePipeline( desc, m_Facts, lookup ); !valid )
		return foundation::MakeUnexpected( valid.Error() );

	PipelineRecord record;
	record.kind = desc.kind;
	record.name = std::string( desc.debugName );
	record.drawConstantBytes = desc.drawConstantBytes;
	record.colorFormats.assign( desc.colorFormats.begin(), desc.colorFormats.end() );
	record.depthFormat = desc.depthFormat;
	record.sampleCount = desc.sampleCount;
	for ( std::size_t role = 0; role < desc.layouts.size() && role < kMaxBindGroups; ++role )
	{
		record.layouts[role] = desc.layouts[role];
		if ( const std::optional<LayoutView> layout = FindLayout( desc.layouts[role] ) )
			record.layoutHasBindings[role] = !layout->bindings.empty();
	}

	ShaderModule vertex, fragment, compute;
	std::vector<WGPUConstantEntry> vertexConstants, fragmentConstants, computeConstants;
	std::deque<std::string> keys;
	std::array<std::vector<BindingLine>, kMaxBindGroups> groups;
	bool drawConstants = false;
	for ( const ShaderArtifactView &artifact : desc.stages )
	{
		ShaderModule &module = artifact.stage == ShaderStage::kVertex     ? vertex
		                       : artifact.stage == ShaderStage::kFragment ? fragment
		                                                                  : compute;
		std::vector<WGPUConstantEntry> &constants =
		    artifact.stage == ShaderStage::kVertex     ? vertexConstants
		    : artifact.stage == ShaderStage::kFragment ? fragmentConstants
		                                               : computeConstants;
		ArtifactHeader header;
		if ( auto built = BuildShader( artifact, desc, module, header, keys, constants ); !built )
			return foundation::MakeUnexpected( built.Error() );
		drawConstants |= header.drawConstantBytes > 0;
		for ( const BindingLine &line : header.bindings )
		{
			std::vector<BindingLine> &group = groups[line.group];
			const auto same = std::find_if( group.begin(), group.end(),
			    [&]( const BindingLine &b )
			    {
				    return b.binding == line.binding;
			    } );
			if ( same == group.end() )
				group.push_back( line );
			else if ( !Merge( *same, line ) )
				return Fail( DeviceStatus::kLayoutMismatch, op );
		}
	}
	if ( drawConstants && desc.drawConstantBytes == 0 )
		return Fail( DeviceStatus::kLayoutMismatch, op );
	// The pipeline layout's groups: up to the last one a stage uses (group 3
	// when the draw constants are read); the ones in between are empty.
	std::uint32_t count = drawConstants ? kMaxBindGroups : 0;
	for ( std::uint32_t g = 0; g < kMaxBindGroups; ++g )
	{
		if ( !groups[g].empty() )
			count = std::max( count, g + 1 );
	}
	std::array<WGPUBindGroupLayout, kMaxBindGroups> layouts{};
	for ( std::uint32_t g = 0; g < count; ++g )
	{
		record.groups[g] = CachedLayout( groups[g], drawConstants && g == kDrawGroup );
		layouts[g] = record.groups[g]->layout.Get();
	}
	record.groupCount = count;
	WGPUPipelineLayoutDescriptor layoutDescriptor = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
	layoutDescriptor.bindGroupLayoutCount = count;
	layoutDescriptor.bindGroupLayouts = layouts.data();
	PipelineLayout pipelineLayout( wgpuDeviceCreatePipelineLayout( m_Device, &layoutDescriptor ) );

	wgpuDevicePushErrorScope( m_Device, WGPUErrorFilter_Validation );
	// Every path pops the scope it pushed: an early refusal that left it
	// pushed would capture (and hide) every later error on the device.
	struct ScopeGuard
	{
		WGPUInstance instance;
		WGPUDevice device;
		bool popped = false;
		~ScopeGuard()
		{
			if ( !popped )
				(void)ScopeClean( instance, device, "refused pipeline" );
		}
	} scopeGuard{ m_Instance, m_Device };
	if ( desc.kind == PipelineKind::kCompute )
	{
		if ( !compute )
			return Fail( DeviceStatus::kInvalidDescription, op );
		WGPUComputePipelineDescriptor descriptor = WGPU_COMPUTE_PIPELINE_DESCRIPTOR_INIT;
		descriptor.label = View( desc.debugName );
		descriptor.layout = pipelineLayout.Get();
		descriptor.compute.module = compute.Get();
		descriptor.compute.entryPoint = View( kEntryPoint );
		descriptor.compute.constantCount = computeConstants.size();
		descriptor.compute.constants = computeConstants.data();
		record.compute.Reset( wgpuDeviceCreateComputePipeline( m_Device, &descriptor ) );
	}
	else
	{
		if ( !vertex )
			return Fail( DeviceStatus::kInvalidDescription, op );
		// A pipeline without a fragment stage leaves its color targets as
		// they are; WebGPU matches a pass's attachments only to targets a
		// fragment stage declares, so it gets one that writes none of them.
		const bool silent = !fragment && !desc.colorFormats.empty();
		if ( silent )
		{
			std::string text = "struct Outputs {\n";
			for ( std::size_t i = 0; i < desc.colorFormats.size(); ++i )
				text += "  @location(" + std::to_string( i ) + ") c" + std::to_string( i ) +
				        " : vec4f,\n";
			text += "}\n@fragment fn main() -> Outputs {\n  var o : Outputs;\n  return o;\n}\n";
			WGPUShaderSourceWGSL source = WGPU_SHADER_SOURCE_WGSL_INIT;
			source.code = View( text );
			WGPUShaderModuleDescriptor module = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
			module.nextInChain = &source.chain;
			fragment.Reset( wgpuDeviceCreateShaderModule( m_Device, &module ) );
		}
		if ( desc.sampleCount != 1 && desc.sampleCount != 4 )
			return Fail( DeviceStatus::kUnsupported, op );
		std::vector<std::vector<WGPUVertexAttribute>> attributes( desc.vertex.buffers.size() );
		for ( const VertexAttribute &attribute : desc.vertex.attributes )
		{
			if ( attribute.bufferSlot >= attributes.size() )
				return Fail( DeviceStatus::kInvalidDescription, op );
			WGPUVertexAttribute a = WGPU_VERTEX_ATTRIBUTE_INIT;
			a.format = VertexFormatOf( attribute.format );
			a.offset = attribute.offset;
			a.shaderLocation = attribute.location;
			attributes[attribute.bufferSlot].push_back( a );
		}
		std::vector<WGPUVertexBufferLayout> buffers;
		for ( std::size_t slot = 0; slot < desc.vertex.buffers.size(); ++slot )
		{
			if ( slot >= kMaxVertexSlots )
				return Fail( DeviceStatus::kUnsupported, op );
			const VertexBufferLayout &buffer = desc.vertex.buffers[slot];
			WGPUVertexBufferLayout layout = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
			// A stride of 0 reads one element for every vertex.
			layout.arrayStride = buffer.stride;
			layout.stepMode =
			    buffer.perInstance ? WGPUVertexStepMode_Instance : WGPUVertexStepMode_Vertex;
			layout.attributeCount = attributes[slot].size();
			layout.attributes = attributes[slot].data();
			buffers.push_back( layout );
		}
		record.vertexBuffers = static_cast<std::uint32_t>( desc.vertex.buffers.size() );

		std::vector<WGPUBlendState> blends( desc.colorFormats.size() );
		std::vector<WGPUColorTargetState> targets;
		for ( std::size_t i = 0; i < desc.colorFormats.size(); ++i )
		{
			WGPUColorTargetState target = WGPU_COLOR_TARGET_STATE_INIT;
			target.format = TextureFormatOf( desc.colorFormats[i] );
			if ( target.format == WGPUTextureFormat_Undefined )
				return Fail( DeviceStatus::kUnsupported, op );
			if ( const std::optional<WGPUBlendState> blend =
			         BlendOf( desc.blends.empty() ? BlendMode::kOpaque : desc.blends[i] ) )
			{
				blends[i] = *blend;
				target.blend = &blends[i];
			}
			const std::uint8_t mask =
			    desc.colorWriteMasks.empty() ? kColorWriteAll : desc.colorWriteMasks[i];
			WGPUColorWriteMask write = WGPUColorWriteMask_None;
			if ( mask & kColorWriteRed )
				write |= WGPUColorWriteMask_Red;
			if ( mask & kColorWriteGreen )
				write |= WGPUColorWriteMask_Green;
			if ( mask & kColorWriteBlue )
				write |= WGPUColorWriteMask_Blue;
			if ( mask & kColorWriteAlpha )
				write |= WGPUColorWriteMask_Alpha;
			target.writeMask = silent ? WGPUColorWriteMask_None : write;
			targets.push_back( target );
		}

		WGPURenderPipelineDescriptor descriptor = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
		descriptor.label = View( desc.debugName );
		descriptor.layout = pipelineLayout.Get();
		descriptor.vertex.module = vertex.Get();
		descriptor.vertex.entryPoint = View( kEntryPoint );
		descriptor.vertex.constantCount = vertexConstants.size();
		descriptor.vertex.constants = vertexConstants.data();
		descriptor.vertex.bufferCount = buffers.size();
		descriptor.vertex.buffers = buffers.data();
		switch ( desc.topology )
		{
		case PrimitiveTopology::kTriangleList:
			descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
			break;
		case PrimitiveTopology::kTriangleStrip:
			descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
			break;
		case PrimitiveTopology::kLineList:
			descriptor.primitive.topology = WGPUPrimitiveTopology_LineList;
			break;
		case PrimitiveTopology::kPointList:
			descriptor.primitive.topology = WGPUPrimitiveTopology_PointList;
			break;
		}
		// The conventions are WebGPU's own (clip +Y up to row 0): the winding
		// passes through.
		descriptor.primitive.frontFace =
		    desc.raster.frontCounterClockwise ? WGPUFrontFace_CCW : WGPUFrontFace_CW;
		descriptor.primitive.cullMode = desc.raster.cull == CullMode::kNone   ? WGPUCullMode_None
		                                : desc.raster.cull == CullMode::kBack ? WGPUCullMode_Back
		                                                                      : WGPUCullMode_Front;
		WGPUDepthStencilState depthStencil = WGPU_DEPTH_STENCIL_STATE_INIT;
		if ( desc.depthFormat != Format::kUnknown )
		{
			depthStencil.format = TextureFormatOf( desc.depthFormat );
			if ( depthStencil.format == WGPUTextureFormat_Undefined )
				return Fail( DeviceStatus::kUnsupported, op );
			// As in the port: no depth writes without the depth test.
			depthStencil.depthWriteEnabled =
			    desc.depthStencil.depthTest && desc.depthStencil.depthWrite
			        ? WGPUOptionalBool_True
			        : WGPUOptionalBool_False;
			depthStencil.depthCompare = desc.depthStencil.depthTest
			                                ? CompareFunction( desc.depthStencil.compare )
			                                : WGPUCompareFunction_Always;
			const StencilState &stencil = desc.depthStencil.stencil;
			WGPUStencilFaceState face;
			face.compare = WGPUCompareFunction_Always;
			face.failOp = face.depthFailOp = face.passOp = WGPUStencilOperation_Keep;
			if ( stencil.enabled && HasStencil( desc.depthFormat ) )
			{
				face.compare = CompareFunction( stencil.compare );
				face.failOp = StencilOperation( stencil.fail );
				face.depthFailOp = StencilOperation( stencil.depthFail );
				face.passOp = StencilOperation( stencil.pass );
				depthStencil.stencilReadMask = stencil.readMask;
				depthStencil.stencilWriteMask = stencil.writeMask;
				record.stencilReference = stencil.reference;
			}
			else
			{
				depthStencil.stencilReadMask = 0;
				depthStencil.stencilWriteMask = 0;
			}
			depthStencil.stencilFront = face;
			depthStencil.stencilBack = face;
			depthStencil.depthBias = static_cast<std::int32_t>( desc.raster.depthBiasConstant );
			depthStencil.depthBiasSlopeScale = desc.raster.depthBiasSlope;
			descriptor.depthStencil = &depthStencil;
		}
		descriptor.multisample.count = desc.sampleCount;
		descriptor.multisample.alphaToCoverageEnabled = desc.raster.alphaToCoverage;
		WGPUFragmentState fragmentState = WGPU_FRAGMENT_STATE_INIT;
		if ( fragment )
		{
			fragmentState.module = fragment.Get();
			fragmentState.entryPoint = View( kEntryPoint );
			fragmentState.constantCount = fragmentConstants.size();
			fragmentState.constants = fragmentConstants.data();
			fragmentState.targetCount = targets.size();
			fragmentState.targets = targets.data();
			descriptor.fragment = &fragmentState;
		}
		record.render.Reset( wgpuDeviceCreateRenderPipeline( m_Device, &descriptor ) );
	}
	scopeGuard.popped = true;
	if ( !ScopeClean( m_Instance, m_Device, "pipeline" ) || ( !record.render && !record.compute ) )
		return Fail( DeviceStatus::kInternal, op );
	const PipelineId id{ ++m_NextId };
	m_Pipelines.emplace( id.value, std::move( record ) );
	return id;
}

const WebGpuDevice::DepthUpload *WebGpuDevice::DepthUploadFor( WGPUTextureFormat format )
{
	if ( const auto found = m_DepthUploads.find( format ); found != m_DepthUploads.end() )
		return &found->second;
	static constexpr std::string_view kSource = R"wgsl(
@group(0) @binding(0) var<storage, read> source : array<f32>;
struct Region { base : u32, width : u32, x : u32, y : u32 }
@group(0) @binding(1) var<uniform> region : Region;
@vertex fn vertexMain(@builtin(vertex_index) i : u32) -> @builtin(position) vec4f {
  let p = vec2f(f32((i << 1u) & 2u), f32(i & 2u));
  return vec4f(p * 2.0 - 1.0, 0.0, 1.0);
}
@fragment fn fragmentMain(@builtin(position) position : vec4f) -> @builtin(frag_depth) f32 {
  let texel = vec2u(position.xy) - vec2u(region.x, region.y);
  return source[region.base + texel.y * region.width + texel.x];
}
)wgsl";
	WGPUShaderSourceWGSL source = WGPU_SHADER_SOURCE_WGSL_INIT;
	source.code = View( kSource );
	WGPUShaderModuleDescriptor moduleDescriptor = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
	moduleDescriptor.nextInChain = &source.chain;
	ShaderModule module( wgpuDeviceCreateShaderModule( m_Device, &moduleDescriptor ) );
	WGPUBindGroupLayoutEntry entries[2] = {
	    WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT };
	entries[0].binding = 0;
	entries[0].visibility = WGPUShaderStage_Fragment;
	entries[0].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
	entries[1].binding = 1;
	entries[1].visibility = WGPUShaderStage_Fragment;
	entries[1].buffer.type = WGPUBufferBindingType_Uniform;
	WGPUBindGroupLayoutDescriptor layoutDescriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
	layoutDescriptor.entryCount = 2;
	layoutDescriptor.entries = entries;
	DepthUpload upload;
	upload.layout.Reset( wgpuDeviceCreateBindGroupLayout( m_Device, &layoutDescriptor ) );
	const WGPUBindGroupLayout layout = upload.layout.Get();
	WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
	pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
	pipelineLayoutDescriptor.bindGroupLayouts = &layout;
	PipelineLayout pipelineLayout(
	    wgpuDeviceCreatePipelineLayout( m_Device, &pipelineLayoutDescriptor ) );
	WGPUDepthStencilState depth = WGPU_DEPTH_STENCIL_STATE_INIT;
	depth.format = format;
	depth.depthWriteEnabled = WGPUOptionalBool_True;
	depth.depthCompare = WGPUCompareFunction_Always;
	depth.stencilReadMask = 0;
	depth.stencilWriteMask = 0;
	WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
	fragment.module = module.Get();
	fragment.entryPoint = View( "fragmentMain" );
	WGPURenderPipelineDescriptor descriptor = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
	descriptor.label = View( "render.device.webgpu depth upload" );
	descriptor.layout = pipelineLayout.Get();
	descriptor.vertex.module = module.Get();
	descriptor.vertex.entryPoint = View( "vertexMain" );
	descriptor.depthStencil = &depth;
	descriptor.fragment = &fragment;
	upload.pipeline.Reset( wgpuDeviceCreateRenderPipeline( m_Device, &descriptor ) );
	return &m_DepthUploads.emplace( format, std::move( upload ) ).first->second;
}

} // namespace render::device::webgpu
