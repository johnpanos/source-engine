//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The port's shared description rules (RFC 0016); see
//			render/device/validation.h.
//
//=============================================================================//

#include "render/device/validation.h"

#include <algorithm>

namespace render::device
{

namespace
{

foundation::Unexpected<DeviceError> Fail( DeviceStatus status, DeviceOperation operation )
{
	return foundation::MakeUnexpected( DeviceError{ status, operation, 0 } );
}

bool PowerOfTwo( std::uint32_t value )
{
	return value != 0 && ( value & ( value - 1 ) ) == 0;
}

} // namespace

DeviceResult<void> ValidateBuffer( const BufferDesc &desc )
{
	if ( desc.size == 0 || desc.usages.Empty() )
		return Fail( DeviceStatus::kInvalidDescription, DeviceOperation::kCreateBuffer );
	return {};
}

DeviceResult<void> ValidateTexture( const TextureDesc &desc, const Limits &limits )
{
	const DeviceOperation op = DeviceOperation::kCreateTexture;
	if ( desc.format == Format::kUnknown || desc.format >= Format::kCount || desc.width == 0 ||
	     desc.height == 0 || desc.depthOrLayers == 0 || desc.mipLevels == 0 ||
	     desc.usages.Empty() || !PowerOfTwo( desc.sampleCount ) )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( desc.dimension == TextureDimension::kCube && desc.depthOrLayers % 6 != 0 )
		return Fail( DeviceStatus::kInvalidDescription, op );
	// Only IExternalImages::CreateExported makes a texture another API reads.
	if ( desc.usages.Has( ResourceUsage::kExternal ) )
		return Fail( DeviceStatus::kInvalidDescription, op );
	std::uint32_t extent = std::max( desc.width, desc.height );
	std::uint32_t mips = 1;
	while ( extent > 1 )
	{
		extent >>= 1;
		++mips;
	}
	if ( desc.mipLevels > mips )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( limits.maxTextureDimension2D && ( desc.width > limits.maxTextureDimension2D ||
	                                         desc.height > limits.maxTextureDimension2D ) )
		return Fail( DeviceStatus::kUnsupported, op );
	std::uint32_t sampleBit = 0;
	while ( ( 1u << sampleBit ) < desc.sampleCount )
		++sampleBit;
	if ( !( limits.sampleCounts & ( 1u << sampleBit ) ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( desc.sampleCount > 1 && desc.mipLevels != 1 )
		return Fail( DeviceStatus::kInvalidDescription, op );
	return {};
}

DeviceResult<void> ValidateBindGroupLayout( const BindGroupLayoutDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroupLayout;
	if ( static_cast<std::uint32_t>( desc.role ) >= kMaxBindGroups )
		return Fail( DeviceStatus::kInvalidDescription, op );
	for ( std::size_t i = 0; i < desc.bindings.size(); ++i )
	{
		if ( desc.bindings[i].count == 0 || desc.bindings[i].stages.Bits() == 0 )
			return Fail( DeviceStatus::kInvalidDescription, op );
		for ( std::size_t j = 0; j < i; ++j )
		{
			if ( desc.bindings[j].binding == desc.bindings[i].binding )
				return Fail( DeviceStatus::kInvalidDescription, op );
		}
	}
	return {};
}

DeviceResult<void> ValidatePipeline(
    const PipelineDesc &desc, const DeviceFacts &facts, const LayoutLookup &lookup )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	if ( desc.layouts.size() > kMaxBindGroups || desc.layouts.size() > facts.limits.maxBindGroups )
		return Fail( DeviceStatus::kTooManyBindGroups, op );
	if ( desc.stages.empty() )
		return Fail( DeviceStatus::kInvalidDescription, op );

	// D16: the draw-constant block, and every stage's reflected use of it.
	if ( desc.drawConstantBytes > kMaxDrawConstantBytes || desc.drawConstantBytes % 4 != 0 )
		return Fail( DeviceStatus::kInvalidDescription, op );
	for ( const ShaderArtifactView &stage : desc.stages )
	{
		if ( stage.drawConstantBytes > desc.drawConstantBytes )
			return Fail( DeviceStatus::kLayoutMismatch, op );
	}

	bool vertex = false;
	bool fragment = false;
	bool compute = false;
	for ( const ShaderArtifactView &stage : desc.stages )
	{
		if ( stage.format != facts.artifactFormat )
			return Fail( DeviceStatus::kUnsupported, op );
		if ( stage.code.empty() )
			return Fail( DeviceStatus::kInvalidDescription, op );
		vertex |= stage.stage == ShaderStage::kVertex;
		fragment |= stage.stage == ShaderStage::kFragment;
		compute |= stage.stage == ShaderStage::kCompute;
	}
	if ( desc.kind == PipelineKind::kCompute )
	{
		if ( !compute || vertex || fragment || desc.stages.size() != 1 )
			return Fail( DeviceStatus::kInvalidDescription, op );
		if ( !facts.capabilities.Has( Capability::kCompute ) )
			return Fail( DeviceStatus::kUnsupported, op );
	}
	else
	{
		if ( !vertex || compute )
			return Fail( DeviceStatus::kInvalidDescription, op );
		if ( desc.colorFormats.empty() && desc.depthFormat == Format::kUnknown )
			return Fail( DeviceStatus::kInvalidDescription, op );
		if ( !desc.blends.empty() && desc.blends.size() != desc.colorFormats.size() )
			return Fail( DeviceStatus::kInvalidDescription, op );
		// D17: a write mask per color format, each within the four channels.
		if ( !desc.colorWriteMasks.empty() &&
		     desc.colorWriteMasks.size() != desc.colorFormats.size() )
			return Fail( DeviceStatus::kInvalidDescription, op );
		for ( std::uint8_t mask : desc.colorWriteMasks )
		{
			if ( mask > kColorWriteAll )
				return Fail( DeviceStatus::kInvalidDescription, op );
		}
		if ( desc.colorFormats.size() > facts.limits.maxColorAttachments )
			return Fail( DeviceStatus::kUnsupported, op );
	}

	// Each layout sits at the index of its role.
	std::optional<LayoutView> layouts[kMaxBindGroups];
	for ( std::size_t i = 0; i < desc.layouts.size(); ++i )
	{
		if ( !desc.layouts[i].IsValid() )
			continue;
		layouts[i] = lookup( desc.layouts[i] );
		if ( !layouts[i] )
			return Fail( DeviceStatus::kInvalidHandle, op );
		if ( static_cast<std::size_t>( layouts[i]->role ) != i )
			return Fail( DeviceStatus::kInvalidDescription, op );
	}
	for ( const ShaderArtifactView &stage : desc.stages )
	{
		for ( const ReflectedBinding &used : stage.bindings )
		{
			if ( used.group >= kMaxBindGroups )
				return Fail( DeviceStatus::kTooManyBindGroups, op );
			const std::optional<LayoutView> &layout = layouts[used.group];
			bool declared = false;
			if ( layout )
			{
				for ( const BindingDesc &binding : layout->bindings )
				{
					declared |= binding.binding == used.binding && binding.kind == used.kind &&
					            binding.stages.Has( stage.stage );
				}
			}
			if ( !declared )
				return Fail( DeviceStatus::kLayoutMismatch, op );
		}
	}
	return {};
}

DeviceResult<void> ValidateBindGroup( const BindGroupDesc &desc, const LayoutView &layout )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroup;
	for ( const BindGroupEntry &entry : desc.entries )
	{
		const BindingDesc *binding = nullptr;
		for ( const BindingDesc &candidate : layout.bindings )
		{
			if ( candidate.binding == entry.binding )
				binding = &candidate;
		}
		if ( !binding )
			return Fail( DeviceStatus::kLayoutMismatch, op );
		bool matches = false;
		switch ( binding->kind )
		{
		case BindingKind::kUniformBuffer:
		case BindingKind::kStorageBuffer:
			matches = entry.buffer.IsValid() && !entry.texture.IsValid();
			break;
		case BindingKind::kSampledTexture:
		case BindingKind::kStorageTexture:
			matches = entry.texture.IsValid() && !entry.buffer.IsValid();
			break;
		case BindingKind::kSampler:
			matches = entry.sampler.IsValid();
			break;
		}
		if ( !matches )
			return Fail( DeviceStatus::kInvalidDescription, op );
	}
	return {};
}

bool DrawConstantCoverage::Write( std::uint32_t offset, std::size_t size )
{
	if ( size == 0 || offset % 4 != 0 || size % 4 != 0 || offset > m_Bytes ||
	     size > m_Bytes - offset )
		return false;
	for ( std::uint32_t word = offset / 4; word < ( offset + size ) / 4; ++word )
		m_Written |= 1u << word;
	return true;
}

bool DrawConstantCoverage::Ready() const
{
	const std::uint32_t words = m_Bytes / 4;
	const std::uint32_t all = words >= 32 ? ~0u : ( 1u << words ) - 1u;
	return ( m_Written & all ) == all;
}

} // namespace render::device
