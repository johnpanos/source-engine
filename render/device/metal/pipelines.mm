//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.metal: pipelines. Each stage's MSL artifact is
//			compiled into a library, its function specialized with the
//			pipeline's function constants (D20), and each group the stage
//			declares gets the function's own argument encoder.
//
//=============================================================================//

#include "metal_device.h"

#include <cstdio>
#include <cstring>
#include <sstream>

namespace render::device::metal
{

namespace
{

// The lines the artifact tool writes before the MSL (METAL_SPECIALIZATION_LINE,
// METAL_THREADGROUP_LINE).
struct ArtifactHeader
{
	std::map<std::uint32_t, MTLDataType> constants;
	std::optional<MTLSize> threadgroup;
};

ArtifactHeader ReadHeader( std::string_view text )
{
	ArtifactHeader header;
	while ( !text.empty() )
	{
		const std::size_t end = text.find( '\n' );
		const std::string_view line = text.substr( 0, end );
		text = end == std::string_view::npos ? std::string_view() : text.substr( end + 1 );
		const std::string_view specialization = kSpecializationLine;
		const std::string_view threadgroup = kThreadgroupLine;
		if ( line.substr( 0, specialization.size() ) == specialization )
		{
			std::istringstream entries( std::string( line.substr( specialization.size() ) ) );
			std::string entry;
			while ( entries >> entry )
			{
				const std::size_t colon = entry.find( ':' );
				if ( colon == std::string::npos )
					continue;
				const auto id = static_cast<std::uint32_t>( std::stoul( entry.substr( 0, colon ) ) );
				const std::string type = entry.substr( colon + 1 );
				header.constants[id] = type == "int"     ? MTLDataTypeInt
				                       : type == "uint"  ? MTLDataTypeUInt
				                       : type == "float" ? MTLDataTypeFloat
				                                         : MTLDataTypeBool;
			}
		}
		else if ( line.substr( 0, threadgroup.size() ) == threadgroup )
		{
			std::istringstream size( std::string( line.substr( threadgroup.size() ) ) );
			NSUInteger x = 1, y = 1, z = 1;
			size >> x >> y >> z;
			header.threadgroup = MTLSizeMake( x, y, z );
		}
		else if ( !line.empty() && line.substr( 0, 2 ) != "//" )
		{
			break; // the MSL itself
		}
	}
	return header;
}

MTLVertexFormat VertexFormatOf( VertexFormat format )
{
	switch ( format )
	{
	case VertexFormat::kFloat2:
		return MTLVertexFormatFloat2;
	case VertexFormat::kFloat3:
		return MTLVertexFormatFloat3;
	case VertexFormat::kFloat4:
		return MTLVertexFormatFloat4;
	case VertexFormat::kUnorm8x4:
		return MTLVertexFormatUChar4Normalized;
	}
	return MTLVertexFormatInvalid;
}

MTLCompareFunction CompareFunction( CompareOp op )
{
	switch ( op )
	{
	case CompareOp::kNever:
		return MTLCompareFunctionNever;
	case CompareOp::kLess:
		return MTLCompareFunctionLess;
	case CompareOp::kLessEqual:
		return MTLCompareFunctionLessEqual;
	case CompareOp::kEqual:
		return MTLCompareFunctionEqual;
	case CompareOp::kGreaterEqual:
		return MTLCompareFunctionGreaterEqual;
	case CompareOp::kGreater:
		return MTLCompareFunctionGreater;
	case CompareOp::kAlways:
		return MTLCompareFunctionAlways;
	case CompareOp::kNotEqual:
		return MTLCompareFunctionNotEqual;
	}
	return MTLCompareFunctionAlways;
}

MTLStencilOperation StencilOperation( StencilOp op )
{
	switch ( op )
	{
	case StencilOp::kKeep:
		return MTLStencilOperationKeep;
	case StencilOp::kZero:
		return MTLStencilOperationZero;
	case StencilOp::kReplace:
		return MTLStencilOperationReplace;
	case StencilOp::kIncrementClamp:
		return MTLStencilOperationIncrementClamp;
	case StencilOp::kDecrementClamp:
		return MTLStencilOperationDecrementClamp;
	case StencilOp::kInvert:
		return MTLStencilOperationInvert;
	case StencilOp::kIncrementWrap:
		return MTLStencilOperationIncrementWrap;
	case StencilOp::kDecrementWrap:
		return MTLStencilOperationDecrementWrap;
	}
	return MTLStencilOperationKeep;
}

void ApplyBlend( MTLRenderPipelineColorAttachmentDescriptor *color, BlendMode mode )
{
	if ( mode == BlendMode::kOpaque )
	{
		color.blendingEnabled = NO;
		return;
	}
	color.blendingEnabled = YES;
	color.rgbBlendOperation = MTLBlendOperationAdd;
	color.alphaBlendOperation = MTLBlendOperationAdd;
	auto set = [&]( MTLBlendFactor srcRgb, MTLBlendFactor dstRgb, MTLBlendFactor srcAlpha,
	               MTLBlendFactor dstAlpha )
	{
		color.sourceRGBBlendFactor = srcRgb;
		color.destinationRGBBlendFactor = dstRgb;
		color.sourceAlphaBlendFactor = srcAlpha;
		color.destinationAlphaBlendFactor = dstAlpha;
	};
	switch ( mode )
	{
	case BlendMode::kAlpha:
		set( MTLBlendFactorSourceAlpha, MTLBlendFactorOneMinusSourceAlpha, MTLBlendFactorOne,
		    MTLBlendFactorOneMinusSourceAlpha );
		break;
	case BlendMode::kPremultiplied:
		set( MTLBlendFactorOne, MTLBlendFactorOneMinusSourceAlpha, MTLBlendFactorOne,
		    MTLBlendFactorOneMinusSourceAlpha );
		break;
	case BlendMode::kModulate2x:
		set( MTLBlendFactorDestinationColor, MTLBlendFactorSourceColor, MTLBlendFactorZero,
		    MTLBlendFactorOne );
		break;
	case BlendMode::kTransmittance:
		set( MTLBlendFactorOne, MTLBlendFactorSourceAlpha, MTLBlendFactorZero, MTLBlendFactorOne );
		break;
	case BlendMode::kAlphaAdditive:
		set( MTLBlendFactorSourceAlpha, MTLBlendFactorOne, MTLBlendFactorSourceAlpha,
		    MTLBlendFactorOne );
		break;
	case BlendMode::kAdditive:
	case BlendMode::kOpaque:
		set( MTLBlendFactorOne, MTLBlendFactorOne, MTLBlendFactorOne, MTLBlendFactorOne );
		break;
	}
}

std::uint32_t StageOf( ShaderStage stage )
{
	switch ( stage )
	{
	case ShaderStage::kVertex:
		return kStageVertex;
	case ShaderStage::kFragment:
		return kStageFragment;
	case ShaderStage::kCompute:
		return kStageCompute;
	}
	return kStageVertex;
}

} // namespace

DeviceResult<void> MetalDevice::BuildStage( const ShaderArtifactView &artifact,
    const PipelineDesc &desc, PipelineRecord &record, id<MTLFunction> __strong &function,
    std::int32_t &nativeCode )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	const std::string_view text( reinterpret_cast<const char *>( artifact.code.data() ),
	    artifact.code.size() );
	const ArtifactHeader header = ReadHeader( text );
	NSString *source = [[NSString alloc] initWithBytes:text.data()
	                                            length:text.size()
	                                          encoding:NSUTF8StringEncoding];
	MTLCompileOptions *options = [MTLCompileOptions new];
	options.languageVersion = MTLLanguageVersion3_0;
	NSError *error = nil;
	id<MTLLibrary> library = [m_Device newLibraryWithSource:source options:options error:&error];
	if ( !library )
	{
		std::fprintf( stderr, "render.device.metal: %.*s: MSL compile failed: %s\n",
		    static_cast<int>( desc.debugName.size() ), desc.debugName.data(),
		    error ? error.localizedDescription.UTF8String : "?" );
		nativeCode = error ? static_cast<std::int32_t>( error.code ) : 0;
		return Fail( DeviceStatus::kInternal, op, nativeCode );
	}
	// D20: each constant this stage declares takes its value with its type;
	// ids it does not declare are ignored.
	MTLFunctionConstantValues *values = [MTLFunctionConstantValues new];
	for ( const SpecializationConstant &constant : desc.constants )
	{
		if ( constant.stage != artifact.stage )
			continue;
		const auto found = header.constants.find( constant.id );
		if ( found == header.constants.end() )
			continue;
		if ( found->second == MTLDataTypeBool )
		{
			const bool value = constant.value != 0;
			[values setConstantValue:&value type:MTLDataTypeBool atIndex:constant.id];
		}
		else
		{
			[values setConstantValue:&constant.value type:found->second atIndex:constant.id];
		}
	}
	function = [library newFunctionWithName:@( kEntryPoint ) constantValues:values error:&error];
	if ( !function )
	{
		nativeCode = error ? static_cast<std::int32_t>( error.code ) : 0;
		return Fail( DeviceStatus::kInternal, op, nativeCode );
	}
	const std::uint32_t stage = StageOf( artifact.stage );
	record.stages[stage] = true;
	if ( header.threadgroup )
		record.threadgroup = *header.threadgroup;
	for ( std::uint32_t group = 0; group < kMaxBindGroups; ++group )
	{
		StageGroup &view = record.groups[stage][group];
		for ( const ReflectedBinding &binding : artifact.bindings )
		{
			if ( binding.group == group )
				view.ids.push_back( binding.binding );
		}
		if ( view.ids.empty() )
			continue;
		std::sort( view.ids.begin(), view.ids.end() );
		view.encoder = [function newArgumentEncoderWithBufferIndex:group];
		if ( !view.encoder )
			return Fail( DeviceStatus::kLayoutMismatch, op );
	}
	return {};
}

DeviceResult<PipelineId> MetalDevice::CreatePipeline( const PipelineDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	const LayoutLookup lookup = [this]( BindGroupLayoutId id ) { return FindLayout( id ); };
	if ( auto valid = ValidatePipeline( desc, m_Facts, lookup ); !valid )
		return foundation::MakeUnexpected( valid.Error() );

	PipelineRecord record;
	record.kind = desc.kind;
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

	std::int32_t nativeCode = 0;
	id<MTLFunction> vertex = nil;
	id<MTLFunction> fragment = nil;
	id<MTLFunction> compute = nil;
	for ( const ShaderArtifactView &artifact : desc.stages )
	{
		id<MTLFunction> __strong &function = artifact.stage == ShaderStage::kVertex ? vertex
		                                     : artifact.stage == ShaderStage::kFragment
		                                         ? fragment
		                                         : compute;
		if ( auto built = BuildStage( artifact, desc, record, function, nativeCode ); !built )
			return foundation::MakeUnexpected( built.Error() );
	}
	NSError *error = nil;
	if ( desc.kind == PipelineKind::kCompute )
	{
		if ( !compute )
			return Fail( DeviceStatus::kInvalidDescription, op );
		record.compute = [m_Device newComputePipelineStateWithFunction:compute error:&error];
		if ( !record.compute )
			return Fail( DeviceStatus::kInternal, op, error ? std::int32_t( error.code ) : 0 );
	}
	else
	{
		if ( !vertex )
			return Fail( DeviceStatus::kInvalidDescription, op );
		MTLRenderPipelineDescriptor *descriptor = [MTLRenderPipelineDescriptor new];
		descriptor.vertexFunction = vertex;
		descriptor.fragmentFunction = fragment;
		MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
		for ( const VertexAttribute &attribute : desc.vertex.attributes )
		{
			if ( attribute.bufferSlot >= kMaxVertexSlots )
				return Fail( DeviceStatus::kUnsupported, op );
			MTLVertexAttributeDescriptor *a = vertexDescriptor.attributes[attribute.location];
			a.format = VertexFormatOf( attribute.format );
			a.offset = attribute.offset;
			a.bufferIndex = kVertexBufferBase + attribute.bufferSlot;
		}
		for ( std::size_t slot = 0; slot < desc.vertex.buffers.size(); ++slot )
		{
			if ( slot >= kMaxVertexSlots )
				return Fail( DeviceStatus::kUnsupported, op );
			const VertexBufferLayout &buffer = desc.vertex.buffers[slot];
			MTLVertexBufferLayoutDescriptor *layout =
			    vertexDescriptor.layouts[kVertexBufferBase + slot];
			if ( buffer.stride == 0 )
			{
				layout.stride = 16; // ignored with a constant step
				layout.stepFunction = MTLVertexStepFunctionConstant;
				layout.stepRate = 0;
			}
			else
			{
				layout.stride = buffer.stride;
				layout.stepFunction = buffer.perInstance ? MTLVertexStepFunctionPerInstance
				                                         : MTLVertexStepFunctionPerVertex;
				layout.stepRate = 1;
			}
		}
		record.vertexBuffers = static_cast<std::uint32_t>( desc.vertex.buffers.size() );
		descriptor.vertexDescriptor = vertexDescriptor;
		for ( std::size_t i = 0; i < desc.colorFormats.size(); ++i )
		{
			const MTLPixelFormat format = PixelFormatOf( desc.colorFormats[i] );
			if ( format == MTLPixelFormatInvalid )
				return Fail( DeviceStatus::kUnsupported, op );
			MTLRenderPipelineColorAttachmentDescriptor *color = descriptor.colorAttachments[i];
			color.pixelFormat = format;
			ApplyBlend( color, desc.blends.empty() ? BlendMode::kOpaque : desc.blends[i] );
			const std::uint8_t mask =
			    desc.colorWriteMasks.empty() ? kColorWriteAll : desc.colorWriteMasks[i];
			MTLColorWriteMask write = MTLColorWriteMaskNone;
			if ( mask & kColorWriteRed )
				write |= MTLColorWriteMaskRed;
			if ( mask & kColorWriteGreen )
				write |= MTLColorWriteMaskGreen;
			if ( mask & kColorWriteBlue )
				write |= MTLColorWriteMaskBlue;
			if ( mask & kColorWriteAlpha )
				write |= MTLColorWriteMaskAlpha;
			color.writeMask = write;
		}
		if ( desc.depthFormat != Format::kUnknown )
		{
			const MTLPixelFormat depth = PixelFormatOf( desc.depthFormat );
			if ( depth == MTLPixelFormatInvalid )
				return Fail( DeviceStatus::kUnsupported, op );
			descriptor.depthAttachmentPixelFormat = depth;
			if ( HasStencil( desc.depthFormat ) )
				descriptor.stencilAttachmentPixelFormat = depth;
		}
		descriptor.rasterSampleCount = desc.sampleCount;
		descriptor.alphaToCoverageEnabled = desc.raster.alphaToCoverage;
		switch ( desc.topology )
		{
		case PrimitiveTopology::kTriangleList:
			record.primitive = MTLPrimitiveTypeTriangle;
			descriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;
			break;
		case PrimitiveTopology::kTriangleStrip:
			record.primitive = MTLPrimitiveTypeTriangleStrip;
			descriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;
			break;
		case PrimitiveTopology::kLineList:
			record.primitive = MTLPrimitiveTypeLine;
			descriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassLine;
			break;
		case PrimitiveTopology::kPointList:
			record.primitive = MTLPrimitiveTypePoint;
			descriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassPoint;
			break;
		}
		if ( !desc.debugName.empty() )
			descriptor.label = [[NSString alloc] initWithBytes:desc.debugName.data()
			                                            length:desc.debugName.size()
			                                          encoding:NSUTF8StringEncoding];
		record.render = [m_Device newRenderPipelineStateWithDescriptor:descriptor error:&error];
		if ( !record.render )
		{
			std::fprintf( stderr, "render.device.metal: pipeline state failed: %s\n",
			    error ? error.localizedDescription.UTF8String : "?" );
			return Fail( DeviceStatus::kInternal, op, error ? std::int32_t( error.code ) : 0 );
		}
		// Fixed-function state Metal sets on the encoder. The conventions
		// are Metal's own (clip +Y up to row 0): facing is judged as in clip
		// space, so the winding passes through.
		record.cull = desc.raster.cull == CullMode::kNone   ? MTLCullModeNone
		              : desc.raster.cull == CullMode::kBack ? MTLCullModeBack
		                                                    : MTLCullModeFront;
		record.winding = desc.raster.frontCounterClockwise ? MTLWindingCounterClockwise
		                                                   : MTLWindingClockwise;
		record.depthBiasConstant = desc.raster.depthBiasConstant;
		record.depthBiasSlope = desc.raster.depthBiasSlope;
		MTLDepthStencilDescriptor *depthStencil = [MTLDepthStencilDescriptor new];
		// As in the port: no depth writes without the depth test.
		depthStencil.depthCompareFunction = desc.depthStencil.depthTest
		                                        ? CompareFunction( desc.depthStencil.compare )
		                                        : MTLCompareFunctionAlways;
		depthStencil.depthWriteEnabled =
		    desc.depthStencil.depthTest && desc.depthStencil.depthWrite;
		const StencilState &stencil = desc.depthStencil.stencil;
		if ( stencil.enabled )
		{
			MTLStencilDescriptor *face = [MTLStencilDescriptor new];
			face.stencilCompareFunction = CompareFunction( stencil.compare );
			face.stencilFailureOperation = StencilOperation( stencil.fail );
			face.depthFailureOperation = StencilOperation( stencil.depthFail );
			face.depthStencilPassOperation = StencilOperation( stencil.pass );
			face.readMask = stencil.readMask;
			face.writeMask = stencil.writeMask;
			depthStencil.frontFaceStencil = face;
			depthStencil.backFaceStencil = face;
			record.stencilReference = stencil.reference;
		}
		record.depthStencil = [m_Device newDepthStencilStateWithDescriptor:depthStencil];
	}
	const PipelineId id{ ++m_NextId };
	m_Pipelines.emplace( id.value, std::move( record ) );
	return id;
}

} // namespace render::device::metal
