//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan (RFC 0016 K1): bind-group layouts, bind
//			groups and pipelines.
//
//			- A bind-group layout is a VkDescriptorSetLayout. Every pipeline
//			  layout has exactly four sets in role order (frame, view, material,
//			  draw); a role the pipeline does not use gets the device's empty
//			  set layout.
//			- A bind group is a descriptor set from the device's pools, which
//			  grow by adding a pool when one is exhausted. Sampled textures are
//			  read in SHADER_READ_ONLY_OPTIMAL, storage textures in GENERAL.
//			- Graphics pipelines use dynamic rendering (no render pass objects)
//			  and dynamic viewport and scissor.
//
//=============================================================================//

#include "vulkan_device.h"

#include <cstring>
#include <iterator>

namespace render::device::vulkan
{

namespace
{

constexpr std::uint32_t kPoolSets = 256;
constexpr std::uint32_t kPoolDescriptors = 1024;

VkDescriptorType DescriptorType( BindingKind kind )
{
	switch ( kind )
	{
	case BindingKind::kUniformBuffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case BindingKind::kStorageBuffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case BindingKind::kSampledTexture:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case BindingKind::kStorageTexture:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case BindingKind::kSampler:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	}
	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

VkShaderStageFlags StageFlags( ShaderStageSet stages )
{
	VkShaderStageFlags flags = 0;
	if ( stages.Has( ShaderStage::kVertex ) )
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	if ( stages.Has( ShaderStage::kFragment ) )
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if ( stages.Has( ShaderStage::kCompute ) )
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	return flags;
}

VkShaderStageFlagBits StageBit( ShaderStage stage )
{
	switch ( stage )
	{
	case ShaderStage::kVertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderStage::kFragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderStage::kCompute:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	}
	return VK_SHADER_STAGE_VERTEX_BIT;
}

VkFormat ToVkVertexFormat( render::device::VertexFormat format )
{
	switch ( format )
	{
	case render::device::VertexFormat::kFloat2:
		return VK_FORMAT_R32G32_SFLOAT;
	case render::device::VertexFormat::kFloat3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case render::device::VertexFormat::kFloat4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case render::device::VertexFormat::kUnorm8x4:
		return VK_FORMAT_R8G8B8A8_UNORM;
	}
	return VK_FORMAT_UNDEFINED;
}

VkPrimitiveTopology Topology( PrimitiveTopology topology )
{
	switch ( topology )
	{
	case PrimitiveTopology::kTriangleList:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case PrimitiveTopology::kTriangleStrip:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case PrimitiveTopology::kLineList:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case PrimitiveTopology::kPointList:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	}
	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkCompareOp Compare( CompareOp op )
{
	switch ( op )
	{
	case CompareOp::kNever:
		return VK_COMPARE_OP_NEVER;
	case CompareOp::kLess:
		return VK_COMPARE_OP_LESS;
	case CompareOp::kLessEqual:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case CompareOp::kEqual:
		return VK_COMPARE_OP_EQUAL;
	case CompareOp::kGreaterEqual:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case CompareOp::kGreater:
		return VK_COMPARE_OP_GREATER;
	case CompareOp::kAlways:
		return VK_COMPARE_OP_ALWAYS;
	}
	return VK_COMPARE_OP_ALWAYS;
}

VkPipelineColorBlendAttachmentState Blend( BlendMode mode )
{
	VkPipelineColorBlendAttachmentState state{};
	state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	state.colorBlendOp = VK_BLEND_OP_ADD;
	state.alphaBlendOp = VK_BLEND_OP_ADD;
	switch ( mode )
	{
	case BlendMode::kOpaque:
		state.blendEnable = VK_FALSE;
		break;
	case BlendMode::kAlpha:
		state.blendEnable = VK_TRUE;
		state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		break;
	case BlendMode::kPremultiplied:
		state.blendEnable = VK_TRUE;
		state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		break;
	case BlendMode::kAdditive:
		state.blendEnable = VK_TRUE;
		state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		break;
	case BlendMode::kTransmittance:
		state.blendEnable = VK_TRUE;
		state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		state.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		break;
	}
	return state;
}

bool PowerOfTwo( std::uint32_t value )
{
	return value != 0 && ( value & ( value - 1 ) ) == 0;
}

} // namespace

// Bind-group layouts ---------------------------------------------------------

DeviceResult<BindGroupLayoutId> VulkanDevice::CreateBindGroupLayout(
    const BindGroupLayoutDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroupLayout;
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBindGroupLayout( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.reserve( desc.bindings.size() );
	for ( const BindingDesc &binding : desc.bindings )
	{
		VkDescriptorSetLayoutBinding out{};
		out.binding = binding.binding;
		out.descriptorType = DescriptorType( binding.kind );
		out.descriptorCount = binding.count;
		out.stageFlags = StageFlags( binding.stages );
		bindings.push_back( out );
	}
	VkDescriptorSetLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = static_cast<std::uint32_t>( bindings.size() );
	info.pBindings = bindings.data();
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	const VkResult result = vkCreateDescriptorSetLayout( m_Device, &info, nullptr, &layout );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), op, result );
	LayoutRecord record;
	record.role = desc.role;
	record.bindings.assign( desc.bindings.begin(), desc.bindings.end() );
	record.handle = std::make_shared<SetLayout>( m_Device, layout );
	const BindGroupLayoutId id{ ++m_NextId };
	m_Layouts.emplace( id.value, std::move( record ) );
	return id;
}

// Bind groups ------------------------------------------------------------------

DeviceResult<VkDescriptorPool> VulkanDevice::CreateDescriptorPool()
{
	const VkDescriptorPoolSize sizes[] = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kPoolDescriptors },
	    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, kPoolDescriptors },
	    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, kPoolDescriptors },
	    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, kPoolDescriptors },
	    { VK_DESCRIPTOR_TYPE_SAMPLER, kPoolDescriptors } };
	VkDescriptorPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	info.maxSets = kPoolSets;
	info.poolSizeCount = static_cast<std::uint32_t>( std::size( sizes ) );
	info.pPoolSizes = sizes;
	VkDescriptorPool pool = VK_NULL_HANDLE;
	const VkResult result = vkCreateDescriptorPool( m_Device, &info, nullptr, &pool );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), DeviceOperation::kCreateBindGroup, result );
	m_DescriptorPools.push_back( pool );
	return pool;
}

DeviceResult<std::pair<VkDescriptorPool, VkDescriptorSet>> VulkanDevice::AllocateSet(
    VkDescriptorSetLayout layout )
{
	VkDescriptorSetAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &layout;
	// Newest pool first; a full pool fails with OUT_OF_POOL_MEMORY or
	// FRAGMENTED_POOL, and a new pool is added.
	for ( auto it = m_DescriptorPools.rbegin(); it != m_DescriptorPools.rend(); ++it )
	{
		info.descriptorPool = *it;
		VkDescriptorSet set = VK_NULL_HANDLE;
		const VkResult result = vkAllocateDescriptorSets( m_Device, &info, &set );
		if ( result == VK_SUCCESS )
			return std::make_pair( *it, set );
		if ( result != VK_ERROR_OUT_OF_POOL_MEMORY && result != VK_ERROR_FRAGMENTED_POOL )
			return Fail( StatusOf( result ), DeviceOperation::kCreateBindGroup, result );
	}
	auto pool = CreateDescriptorPool();
	if ( !pool )
		return foundation::MakeUnexpected( pool.Error() );
	info.descriptorPool = pool.Value();
	VkDescriptorSet set = VK_NULL_HANDLE;
	const VkResult result = vkAllocateDescriptorSets( m_Device, &info, &set );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), DeviceOperation::kCreateBindGroup, result );
	return std::make_pair( pool.Value(), set );
}

DeviceResult<BindGroupId> VulkanDevice::CreateBindGroup( const BindGroupDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroup;
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	const std::optional<LayoutView> layout = FindLayout( desc.layout );
	if ( !layout )
		return Fail( DeviceStatus::kInvalidHandle, op );
	if ( auto valid = ValidateBindGroup( desc, *layout ); !valid )
		return foundation::MakeUnexpected( valid.Error() );

	// Resolve every entry before allocating, so a bad one allocates nothing.
	struct Resolved
	{
		const BindingDesc *binding = nullptr;
		std::uint32_t element = 0;
		VkDescriptorBufferInfo buffer{};
		VkDescriptorImageInfo image{};
	};
	std::vector<Resolved> resolved;
	resolved.reserve( desc.entries.size() );
	BindGroupRecord record;
	record.layout = desc.layout;
	for ( const BindGroupEntry &entry : desc.entries )
	{
		Resolved out;
		for ( const BindingDesc &candidate : layout->bindings )
		{
			if ( candidate.binding == entry.binding )
				out.binding = &candidate;
		}
		for ( const Resolved &earlier : resolved )
			out.element += earlier.binding->binding == entry.binding ? 1u : 0u;
		if ( !out.binding || out.element >= out.binding->count )
			return Fail( DeviceStatus::kInvalidDescription, op );
		switch ( out.binding->kind )
		{
		case BindingKind::kUniformBuffer:
		case BindingKind::kStorageBuffer:
		{
			const BufferRecord *buffer = LiveBuffer( entry.buffer.value );
			if ( !buffer )
				return Fail( DeviceStatus::kInvalidHandle, op );
			const bool uniform = out.binding->kind == BindingKind::kUniformBuffer;
			const bool declared = uniform
			                          ? buffer->desc.usages.Has( ResourceUsage::kUniform )
			                          : buffer->desc.usages.Has( ResourceUsage::kStorageRead ) ||
			                                buffer->desc.usages.Has( ResourceUsage::kStorageWrite );
			const VkDeviceSize alignment =
			    uniform ? m_Properties.limits.minUniformBufferOffsetAlignment
			            : m_Properties.limits.minStorageBufferOffsetAlignment;
			if ( !declared || entry.offset >= buffer->desc.size ||
			     entry.size > buffer->desc.size - entry.offset ||
			     ( alignment && entry.offset % alignment != 0 ) )
				return Fail( DeviceStatus::kInvalidDescription, op );
			out.buffer.buffer = buffer->buffer;
			out.buffer.offset = entry.offset;
			out.buffer.range = entry.size == 0 ? VK_WHOLE_SIZE : entry.size;
			record.resources.push_back( entry.buffer );
			break;
		}
		case BindingKind::kSampledTexture:
		case BindingKind::kStorageTexture:
		{
			const TextureRecord *texture = LiveTexture( entry.texture.value );
			if ( !texture )
				return Fail( DeviceStatus::kInvalidHandle, op );
			const bool sampled = out.binding->kind == BindingKind::kSampledTexture;
			const bool declared =
			    sampled ? texture->desc.usages.Has( ResourceUsage::kSampled )
			            : texture->desc.usages.Has( ResourceUsage::kStorageRead ) ||
			                  texture->desc.usages.Has( ResourceUsage::kStorageWrite );
			if ( !declared || texture->view == VK_NULL_HANDLE )
				return Fail( DeviceStatus::kInvalidDescription, op );
			out.image.imageView = texture->view;
			out.image.imageLayout =
			    sampled ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
			record.resources.push_back( entry.texture );
			break;
		}
		case BindingKind::kSampler:
		{
			const auto sampler = m_Samplers.find( entry.sampler.value );
			if ( sampler == m_Samplers.end() || sampler->second.released )
				return Fail( DeviceStatus::kInvalidHandle, op );
			out.image.sampler = sampler->second.sampler;
			record.resources.push_back( entry.sampler );
			break;
		}
		}
		resolved.push_back( out );
	}

	const auto layoutRecord = m_Layouts.find( desc.layout.value );
	record.setLayout = layoutRecord->second.handle;
	auto set = AllocateSet( record.setLayout->layout );
	if ( !set )
		return foundation::MakeUnexpected( set.Error() );
	record.pool = set.Value().first;
	record.set = set.Value().second;
	std::vector<VkWriteDescriptorSet> writes;
	writes.reserve( resolved.size() );
	for ( const Resolved &entry : resolved )
	{
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = record.set;
		write.dstBinding = entry.binding->binding;
		write.dstArrayElement = entry.element;
		write.descriptorCount = 1;
		write.descriptorType = DescriptorType( entry.binding->kind );
		if ( entry.binding->kind == BindingKind::kUniformBuffer ||
		     entry.binding->kind == BindingKind::kStorageBuffer )
			write.pBufferInfo = &entry.buffer;
		else
			write.pImageInfo = &entry.image;
		writes.push_back( write );
	}
	if ( !writes.empty() )
		vkUpdateDescriptorSets(
		    m_Device, static_cast<std::uint32_t>( writes.size() ), writes.data(), 0, nullptr );
	const BindGroupId id{ ++m_NextId };
	m_BindGroups.emplace( id.value, std::move( record ) );
	return id;
}

// Pipelines --------------------------------------------------------------------

DeviceResult<PipelineId> VulkanDevice::CreatePipeline( const PipelineDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	auto valid = ValidatePipeline( desc, m_Facts,
	    [this]( BindGroupLayoutId id )
	    {
		    return FindLayout( id );
	    } );
	if ( !valid )
		return foundation::MakeUnexpected( valid.Error() );

	// What Vulkan needs beyond the shared rules: one module per stage, SPIR-V
	// words, depth and color formats in their places, a supported sample count
	// and vertex attributes that name a declared buffer.
	bool seen[3] = {};
	for ( const ShaderArtifactView &stage : desc.stages )
	{
		const std::size_t index = static_cast<std::size_t>( stage.stage );
		if ( index >= 3 || seen[index] || stage.code.size() % 4 != 0 )
			return Fail( DeviceStatus::kInvalidDescription, op );
		seen[index] = true;
	}
	if ( desc.kind == PipelineKind::kGraphics )
	{
		// Graphics pipelines are built for dynamic rendering; a host device
		// without it (Vulkan 1.1) builds compute pipelines only.
		if ( !m_Adapter.dynamicRendering )
			return Fail( DeviceStatus::kUnsupported, op );
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
		if ( desc.vertex.buffers.size() > m_Properties.limits.maxVertexInputBindings ||
		     desc.vertex.attributes.size() > m_Properties.limits.maxVertexInputAttributes )
			return Fail( DeviceStatus::kUnsupported, op );
		for ( const VertexAttribute &attribute : desc.vertex.attributes )
		{
			if ( attribute.bufferSlot >= desc.vertex.buffers.size() )
				return Fail( DeviceStatus::kInvalidDescription, op );
		}
	}

	PipelineRecord record;
	record.kind = desc.kind;
	record.drawConstantBytes = desc.drawConstantBytes;
	record.colorFormats.assign( desc.colorFormats.begin(), desc.colorFormats.end() );
	record.depthFormat = desc.depthFormat;
	record.sampleCount = desc.sampleCount;
	record.vertexBuffers = static_cast<std::uint32_t>( desc.vertex.buffers.size() );
	VkDescriptorSetLayout setLayouts[kMaxBindGroups];
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		setLayouts[role] = m_EmptySetLayout;
		if ( role >= desc.layouts.size() || !desc.layouts[role].IsValid() )
			continue;
		const LayoutRecord &layout = m_Layouts.find( desc.layouts[role].value )->second;
		record.layouts[role] = desc.layouts[role];
		record.layoutHasBindings[role] = !layout.bindings.empty();
		record.setLayouts[role] = layout.handle;
		setLayouts[role] = layout.handle->layout;
	}

	std::vector<VkShaderModule> modules;
	auto cleanup = [&]
	{
		for ( VkShaderModule module : modules )
			vkDestroyShaderModule( m_Device, module, nullptr );
	};
	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = kMaxBindGroups;
	layoutInfo.pSetLayouts = setLayouts;
	// D16: draw constants are push constants every stage may read (Vulkan
	// guarantees at least kMaxDrawConstantBytes).
	VkPushConstantRange constants{};
	constants.stageFlags = DrawConstantStages( desc.kind );
	constants.size = desc.drawConstantBytes;
	if ( desc.drawConstantBytes > 0 )
	{
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &constants;
	}
	VkResult result =
	    vkCreatePipelineLayout( m_Device, &layoutInfo, nullptr, &record.pipelineLayout );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), op, result );

	std::vector<std::string> entryPoints;
	std::vector<VkPipelineShaderStageCreateInfo> stages;
	entryPoints.reserve( desc.stages.size() );
	// D20: each stage's specialization constants, as 4-byte entries.
	std::vector<std::vector<VkSpecializationMapEntry>> specEntries( desc.stages.size() );
	std::vector<std::vector<std::uint32_t>> specValues( desc.stages.size() );
	std::vector<VkSpecializationInfo> specInfos( desc.stages.size() );
	for ( const ShaderArtifactView &stage : desc.stages )
	{
		// Copied so the words are aligned whatever the caller's bytes are.
		std::vector<std::uint32_t> words( stage.code.size() / 4 );
		std::memcpy( words.data(), stage.code.data(), stage.code.size() );
		VkShaderModuleCreateInfo moduleInfo{};
		moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleInfo.codeSize = stage.code.size();
		moduleInfo.pCode = words.data();
		VkShaderModule module = VK_NULL_HANDLE;
		result = vkCreateShaderModule( m_Device, &moduleInfo, nullptr, &module );
		if ( result != VK_SUCCESS )
		{
			cleanup();
			vkDestroyPipelineLayout( m_Device, record.pipelineLayout, nullptr );
			return Fail( StatusOf( result ) == DeviceStatus::kInternal
			                 ? DeviceStatus::kInvalidDescription
			                 : StatusOf( result ),
			    op, result );
		}
		modules.push_back( module );
		entryPoints.emplace_back( stage.entryPoint );
		VkPipelineShaderStageCreateInfo stageInfo{};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = StageBit( stage.stage );
		stageInfo.module = module;
		stageInfo.pName = entryPoints.back().c_str();
		const std::size_t index = stages.size();
		for ( const SpecializationConstant &constant : desc.constants )
		{
			if ( constant.stage != stage.stage )
				continue;
			specEntries[index].push_back(
			    { constant.id, std::uint32_t( specValues[index].size() * sizeof( std::uint32_t ) ),
			        sizeof( std::uint32_t ) } );
			specValues[index].push_back( constant.value );
		}
		if ( !specEntries[index].empty() )
		{
			VkSpecializationInfo &spec = specInfos[index];
			spec.mapEntryCount = std::uint32_t( specEntries[index].size() );
			spec.pMapEntries = specEntries[index].data();
			spec.dataSize = specValues[index].size() * sizeof( std::uint32_t );
			spec.pData = specValues[index].data();
			stageInfo.pSpecializationInfo = &spec;
		}
		stages.push_back( stageInfo );
	}

	if ( desc.kind == PipelineKind::kCompute )
	{
		VkComputePipelineCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.stage = stages[0];
		info.layout = record.pipelineLayout;
		result = vkCreateComputePipelines(
		    m_Device, VK_NULL_HANDLE, 1, &info, nullptr, &record.pipeline );
	}
	else
	{
		std::vector<VkVertexInputBindingDescription> bindings;
		for ( std::size_t i = 0; i < desc.vertex.buffers.size(); ++i )
		{
			VkVertexInputBindingDescription binding{};
			binding.binding = static_cast<std::uint32_t>( i );
			binding.stride = desc.vertex.buffers[i].stride;
			binding.inputRate = desc.vertex.buffers[i].perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE
			                                                       : VK_VERTEX_INPUT_RATE_VERTEX;
			bindings.push_back( binding );
		}
		std::vector<VkVertexInputAttributeDescription> attributes;
		for ( const VertexAttribute &attribute : desc.vertex.attributes )
		{
			VkVertexInputAttributeDescription out{};
			out.location = attribute.location;
			out.binding = attribute.bufferSlot;
			out.format = ToVkVertexFormat( attribute.format );
			out.offset = attribute.offset;
			attributes.push_back( out );
		}
		VkPipelineVertexInputStateCreateInfo vertexInput{};
		vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput.vertexBindingDescriptionCount = static_cast<std::uint32_t>( bindings.size() );
		vertexInput.pVertexBindingDescriptions = bindings.data();
		vertexInput.vertexAttributeDescriptionCount =
		    static_cast<std::uint32_t>( attributes.size() );
		vertexInput.pVertexAttributeDescriptions = attributes.data();

		VkPipelineInputAssemblyStateCreateInfo assembly{};
		assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assembly.topology = Topology( desc.topology );

		VkPipelineViewportStateCreateInfo viewport{};
		viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport.viewportCount = 1;
		viewport.scissorCount = 1;

		// The adapter flips Y in the viewport (clip Y up, row 0 at the top),
		// which also keeps the port's counter-clockwise front faces.
		VkPipelineRasterizationStateCreateInfo raster{};
		raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster.polygonMode = VK_POLYGON_MODE_FILL;
		raster.cullMode = desc.raster.cull == CullMode::kNone   ? VK_CULL_MODE_NONE
		                  : desc.raster.cull == CullMode::kBack ? VK_CULL_MODE_BACK_BIT
		                                                        : VK_CULL_MODE_FRONT_BIT;
		raster.frontFace = desc.raster.frontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE
		                                                     : VK_FRONT_FACE_CLOCKWISE;
		raster.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisample{};
		multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample.rasterizationSamples = static_cast<VkSampleCountFlagBits>( desc.sampleCount );

		VkPipelineDepthStencilStateCreateInfo depth{};
		depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth.depthTestEnable = desc.depthStencil.depthTest ? VK_TRUE : VK_FALSE;
		depth.depthWriteEnable = desc.depthStencil.depthWrite ? VK_TRUE : VK_FALSE;
		depth.depthCompareOp = Compare( desc.depthStencil.compare );
		depth.maxDepthBounds = 1.0f;

		std::vector<VkPipelineColorBlendAttachmentState> blends;
		for ( std::size_t i = 0; i < desc.colorFormats.size(); ++i )
		{
			BlendMode mode = desc.blends.empty() ? BlendMode::kOpaque : desc.blends[i];
			if ( mode == BlendMode::kTransmittance &&
			     m_Options.sensitivity.transmittanceAsPremultiplied )
				mode = BlendMode::kPremultiplied;
			blends.push_back( Blend( mode ) );
			// D17: the channels the pipeline writes.
			const std::uint8_t mask =
			    desc.colorWriteMasks.empty() || m_Options.sensitivity.ignoreColorWriteMasks
			        ? kColorWriteAll
			        : desc.colorWriteMasks[i];
			VkColorComponentFlags components = 0;
			if ( mask & kColorWriteRed )
				components |= VK_COLOR_COMPONENT_R_BIT;
			if ( mask & kColorWriteGreen )
				components |= VK_COLOR_COMPONENT_G_BIT;
			if ( mask & kColorWriteBlue )
				components |= VK_COLOR_COMPONENT_B_BIT;
			if ( mask & kColorWriteAlpha )
				components |= VK_COLOR_COMPONENT_A_BIT;
			blends.back().colorWriteMask = components;
		}
		VkPipelineColorBlendStateCreateInfo blend{};
		blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blend.attachmentCount = static_cast<std::uint32_t>( blends.size() );
		blend.pAttachments = blends.data();

		const VkDynamicState dynamics[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamic{};
		dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic.dynamicStateCount = 2;
		dynamic.pDynamicStates = dynamics;

		std::vector<VkFormat> colorFormats;
		for ( Format format : desc.colorFormats )
			colorFormats.push_back( ToVkFormat( format ) );
		VkPipelineRenderingCreateInfo rendering{};
		rendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		rendering.colorAttachmentCount = static_cast<std::uint32_t>( colorFormats.size() );
		rendering.pColorAttachmentFormats = colorFormats.data();
		rendering.depthAttachmentFormat = ToVkFormat( desc.depthFormat );
		rendering.stencilAttachmentFormat =
		    HasStencil( desc.depthFormat ) ? ToVkFormat( desc.depthFormat ) : VK_FORMAT_UNDEFINED;

		VkGraphicsPipelineCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.pNext = &rendering;
		info.stageCount = static_cast<std::uint32_t>( stages.size() );
		info.pStages = stages.data();
		info.pVertexInputState = &vertexInput;
		info.pInputAssemblyState = &assembly;
		info.pViewportState = &viewport;
		info.pRasterizationState = &raster;
		info.pMultisampleState = &multisample;
		info.pDepthStencilState = &depth;
		info.pColorBlendState = &blend;
		info.pDynamicState = &dynamic;
		info.layout = record.pipelineLayout;
		result = vkCreateGraphicsPipelines(
		    m_Device, VK_NULL_HANDLE, 1, &info, nullptr, &record.pipeline );
	}
	cleanup();
	if ( result != VK_SUCCESS )
	{
		vkDestroyPipelineLayout( m_Device, record.pipelineLayout, nullptr );
		return Fail( StatusOf( result ), op, result );
	}
	Name( VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<std::uint64_t>( record.pipeline ),
	    desc.debugName );
	const PipelineId id{ ++m_NextId };
	m_Pipelines.emplace( id.value, std::move( record ) );
	return id;
}

} // namespace render::device::vulkan
