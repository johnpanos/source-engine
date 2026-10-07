//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica's pipelines: the kPica artifacts read and
//			checked against their reflection, the PICA program, and the
//			fixed-function state (RFC 0026 decisions 2-5).
//
//=============================================================================//

#if defined( __3DS__ )

#include "pica_device.h"

#include "render/device/pica_codes.h"

#include <algorithm>
#include <cstring>

namespace render::device::pica
{
namespace
{

foundation::Unexpected<DeviceError> Fail( DeviceStatus status )
{
	return foundation::MakeUnexpected( DeviceError{ status, DeviceOperation::kCreatePipeline, 0 } );
}

// Every binding an artifact uses must be in its stage's reflection: a
// reflection that hides one would let a layout without it pass.
bool Reflected( std::span<const ReflectedBinding> used, std::span<const ReflectedBinding> declared )
{
	for ( const ReflectedBinding &binding : used )
	{
		const bool found = std::any_of( declared.begin(), declared.end(),
		    [&]( const ReflectedBinding &d )
		    {
			    return d.group == binding.group && d.binding == binding.binding &&
			           d.kind == binding.kind;
		    } );
		if ( !found )
			return false;
	}
	return true;
}

struct AttributeFormat
{
	GPU_FORMATS format;
	std::uint8_t components;
	std::uint32_t bytes;
};

AttributeFormat FormatOf( VertexFormat format )
{
	switch ( format )
	{
	case VertexFormat::kFloat2:
		return { GPU_FLOAT, 2, 8 };
	case VertexFormat::kFloat3:
		return { GPU_FLOAT, 3, 12 };
	case VertexFormat::kFloat4:
		return { GPU_FLOAT, 4, 16 };
	case VertexFormat::kUnorm8x4:
		return { GPU_UNSIGNED_BYTE, 4, 4 };
	}
	return { GPU_FLOAT, 4, 16 };
}

} // namespace

DeviceResult<PipelineId> PicaDevice::CreatePipeline( const PipelineDesc &desc )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost );
	if ( auto valid = ValidatePipeline( desc, m_Facts,
	         [this]( BindGroupLayoutId id )
	         {
		         return FindLayout( id );
	         } );
	    !valid )
		return foundation::MakeUnexpected( valid.Error() );

	// The PICA200's fixed function (RFC 0026 decision 4): triangles, one RGBA8
	// target, D24S8 depth, one sample, no depth bias, no alpha to coverage.
	if ( desc.kind != PipelineKind::kGraphics || desc.sampleCount != 1 ||
	     desc.raster.alphaToCoverage || desc.raster.depthBiasConstant != 0.0f ||
	     desc.raster.depthBiasSlope != 0.0f ||
	     ( desc.topology != PrimitiveTopology::kTriangleList &&
	         desc.topology != PrimitiveTopology::kTriangleStrip ) )
		return Fail( DeviceStatus::kUnsupported );
	for ( Format format : desc.colorFormats )
		if ( !ColorBufferFormat( format ) )
			return Fail( DeviceStatus::kUnsupported );
	if ( desc.depthFormat != Format::kUnknown && !DepthBufferFormat( desc.depthFormat ) )
		return Fail( DeviceStatus::kUnsupported );

	auto record = std::make_unique<PipelineRecord>();
	PipelineRecord &p = *record;
	const ShaderArtifactView *vertex = nullptr;
	const ShaderArtifactView *fragment = nullptr;
	for ( const ShaderArtifactView &stage : desc.stages )
		( stage.stage == ShaderStage::kVertex ? vertex : fragment ) = &stage;

	// The vertex program: its DVLB kept in word-aligned storage the parser
	// points into.
	if ( ReadVertexProgram( vertex->code, desc.drawConstantBytes, p.vertex ) )
		return Fail( DeviceStatus::kInvalidDescription );
	if ( !Reflected( BindingsOf( p.vertex ), vertex->bindings ) )
		return Fail( DeviceStatus::kLayoutMismatch );
	p.dvlbWords.resize( ( p.vertex.dvlb.size() + 3 ) / 4 );
	std::memcpy( p.dvlbWords.data(), p.vertex.dvlb.data(), p.vertex.dvlb.size() );
	p.vertex.dvlb = std::as_bytes( std::span( p.dvlbWords ) ).first( p.vertex.dvlb.size() );

	if ( fragment )
	{
		if ( ReadFragmentProgram( fragment->code, desc.drawConstantBytes, p.fragment ) )
			return Fail( DeviceStatus::kInvalidDescription );
		if ( !Reflected( BindingsOf( p.fragment ), fragment->bindings ) ||
		     DrawConstantBytesOf( p.fragment ) > fragment->drawConstantBytes )
			return Fail( DeviceStatus::kLayoutMismatch );
	}
	p.stages = Specialize( p.fragment, desc.constants );

	// Vertex attributes by buffer slot, in offset order; the vertex index
	// input takes its own loader.
	for ( const VertexAttribute &attribute : desc.vertex.attributes )
	{
		const AttributeFormat format = FormatOf( attribute.format );
		if ( attribute.location >= kInputRegisters ||
		     attribute.location == p.vertex.vertexIndexRegister ||
		     attribute.bufferSlot >= desc.vertex.buffers.size() ||
		     attribute.bufferSlot >= kInputRegisters - 1 || attribute.offset % 4 != 0 )
			return Fail( DeviceStatus::kInvalidDescription );
		p.slotAttributes[attribute.bufferSlot].push_back( { std::uint8_t( attribute.location ),
		    format.format, format.components, attribute.offset, format.bytes } );
		p.vertexBuffers = std::max( p.vertexBuffers, attribute.bufferSlot + 1 );
	}
	std::uint32_t loaders = p.vertex.vertexIndexRegister != kNone ? 1 : 0;
	for ( std::size_t slot = 0; slot < desc.vertex.buffers.size(); ++slot )
	{
		const VertexBufferLayout &layout = desc.vertex.buffers[slot];
		// No instancing; strides fit the GPU's 8-bit field, in whole words
		// (padding is counted in words).
		if ( layout.perInstance || layout.stride > 255 || layout.stride % 4 != 0 )
			return Fail( DeviceStatus::kUnsupported );
		auto &attributes = p.slotAttributes[slot];
		std::sort( attributes.begin(), attributes.end(),
		    []( const AttributeLoad &a, const AttributeLoad &b )
		    {
			    return a.offset < b.offset;
		    } );
		std::uint32_t end = 0;
		for ( const AttributeLoad &attribute : attributes )
		{
			if ( attribute.offset < end )
				return Fail( DeviceStatus::kUnsupported ); // overlapping attributes
			end = attribute.offset + attribute.bytes;
		}
		if ( end > layout.stride )
			return Fail( DeviceStatus::kInvalidDescription );
		p.strides[slot] = layout.stride;
		loaders += std::uint32_t( attributes.size() );
	}
	if ( loaders > 12 )
		return Fail( DeviceStatus::kUnsupported );

	// The PICA program, and its own constants clear of the port's registers.
	p.dvlb = DVLB_ParseFile( p.dvlbWords.data(), u32( p.vertex.dvlb.size() ) );
	if ( !p.dvlb || p.dvlb->numDVLE != 1 )
	{
		if ( p.dvlb )
			DVLB_Free( p.dvlb );
		p.dvlb = nullptr;
		return Fail( DeviceStatus::kInvalidDescription );
	}
	const DVLE_s &dvle = p.dvlb->DVLE[0];
	for ( u32 i = 0; i < dvle.constTableSize; ++i )
	{
		const DVLE_constEntry_s &constant = dvle.constTableData[i];
		if ( constant.type != DVLE_CONST_FLOAT24 )
			continue;
		bool clash = false;
		for ( const UniformRange &range : p.vertex.uniforms )
			clash |= constant.id >= range.firstRegister &&
			         constant.id < range.firstRegister + range.registerCount;
		if ( p.vertex.drawConstantRegister != kNone )
			clash |=
			    constant.id >= p.vertex.drawConstantRegister &&
			    constant.id < p.vertex.drawConstantRegister +
			                      ( desc.drawConstantBytes + kRegisterBytes - 1 ) / kRegisterBytes;
		if ( clash )
		{
			DVLB_Free( p.dvlb );
			p.dvlb = nullptr;
			return Fail( DeviceStatus::kInvalidDescription );
		}
	}
	shaderProgramInit( &p.program );
	shaderProgramSetVsh( &p.program, &p.dvlb->DVLE[0] );

	p.topology =
	    desc.topology == PrimitiveTopology::kTriangleStrip ? GPU_TRIANGLE_STRIP : GPU_TRIANGLES;
	p.raster = desc.raster;
	p.depthStencil = desc.depthStencil;
	p.colorFormats.assign( desc.colorFormats.begin(), desc.colorFormats.end() );
	if ( !desc.blends.empty() )
	{
		const std::optional<BlendFactors> factors = Blend( desc.blends[0] );
		if ( !factors )
		{
			shaderProgramFree( &p.program );
			DVLB_Free( p.dvlb );
			p.dvlb = nullptr;
			return Fail( DeviceStatus::kUnsupported );
		}
		p.blend = *factors;
	}
	p.writeMask = desc.colorWriteMasks.empty() ? kColorWriteAll : desc.colorWriteMasks[0];
	if ( desc.colorFormats.empty() )
		p.writeMask = 0;
	p.depthFormat = desc.depthFormat;
	p.drawConstantBytes = desc.drawConstantBytes;
	for ( std::size_t i = 0; i < desc.layouts.size(); ++i )
	{
		p.layouts[i] = desc.layouts[i];
		if ( desc.layouts[i].IsValid() )
			p.layoutHasBindings[i] = !m_Layouts.at( desc.layouts[i].value ).bindings.empty();
	}
	const PipelineId id{ ++m_NextId };
	m_Pipelines.emplace( id.value, std::move( record ) );
	return id;
}

} // namespace render::device::pica

#endif // __3DS__
