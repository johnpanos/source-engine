//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan (RFC 0016 K1): buffers, textures, samplers
//			and their memory. Each creation applies the port's shared rules
//			first, and a failure destroys whatever it had created, so no live
//			resource or Vulkan object leaks.
//
//			Memory is suballocated by the pinned Vulkan Memory Allocator
//			(memory.cpp, RFC 0016 decision 4). Device-local buffers and
//			textures take DEVICE_LOCAL memory, upload buffers HOST_VISIBLE |
//			HOST_COHERENT, and readback buffers HOST_VISIBLE, preferring
//			HOST_CACHED (their reads invalidate when the type is not coherent).
//
//=============================================================================//

#include "vulkan_device.h"

namespace render::device::vulkan
{

namespace
{

// Destroys what a creation made unless it is dismissed on success.
template <typename F> class Undo
{
public:
	explicit Undo( F action ) : m_Action( action ) {}
	~Undo()
	{
		if ( m_Armed )
			m_Action();
	}
	void Dismiss() { m_Armed = false; }

private:
	F m_Action;
	bool m_Armed = true;
};

} // namespace

HostMemory VulkanDevice::MemoryFor( MemoryKind kind )
{
	constexpr VkMemoryPropertyFlags kVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	constexpr VkMemoryPropertyFlags kCoherent = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	constexpr VkMemoryPropertyFlags kCached = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	HostMemory memory;
	switch ( kind )
	{
	case MemoryKind::kDeviceLocal:
		memory.required = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	case MemoryKind::kUpload:
		memory.required = kVisible | kCoherent;
		memory.mapped = true;
		break;
	case MemoryKind::kReadback:
		memory.required = kVisible;
		memory.preferred = kCached | kCoherent;
		memory.mapped = true;
		break;
	}
	return memory;
}

void VulkanDevice::Name( VkObjectType type, std::uint64_t handle, std::string_view name )
{
	if ( name.empty() || !m_Instance->setObjectName || m_Device == VK_NULL_HANDLE )
		return;
	const std::string owned( name );
	VkDebugUtilsObjectNameInfoEXT info{};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectType = type;
	info.objectHandle = handle;
	info.pObjectName = owned.c_str();
	(void)m_Instance->setObjectName( m_Device, &info );
}

DeviceResult<HostBuffer> VulkanDevice::CreateHostBuffer(
    std::uint64_t size, VkBufferUsageFlags usage, DeviceOperation op )
{
	HostBuffer out;
	VkBufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = size;
	info.usage = usage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	void *mapped = nullptr;
	const VkResult result = m_Memory.CreateBuffer(
	    info, MemoryFor( MemoryKind::kUpload ), &out.buffer, &out.memory, &mapped );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), op, result );
	out.mapped = static_cast<std::byte *>( mapped );
	return out;
}

void VulkanDevice::DestroyHostBuffer( HostBuffer &buffer )
{
	if ( m_Device == VK_NULL_HANDLE )
		return;
	m_Memory.DestroyBuffer( buffer.buffer, buffer.memory );
	buffer = {};
}

DeviceResult<BufferId> VulkanDevice::CreateBuffer( const BufferDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBuffer;
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBuffer( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );

	BufferRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	Undo undo(
	    [&]
	    {
		    m_Memory.DestroyBuffer( record.buffer, record.memory );
	    } );
	VkBufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = desc.size;
	info.usage = BufferUsageFlags( desc.usages );
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	void *mapped = nullptr;
	const HostMemory memory = MemoryFor( desc.memory );
	const VkResult result =
	    m_Memory.CreateBuffer( info, memory, &record.buffer, &record.memory, &mapped );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), op, result );
	if ( memory.mapped )
	{
		record.mapped = static_cast<std::byte *>( mapped );
		record.coherent =
		    ( m_Memory.Properties( record.memory ) & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) != 0;
	}
	Name( VK_OBJECT_TYPE_BUFFER, reinterpret_cast<std::uint64_t>( record.buffer ), desc.debugName );
	undo.Dismiss();
	const BufferId id{ ++m_NextId };
	m_Buffers.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<TextureId> VulkanDevice::CreateTexture( const TextureDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateTexture;
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateTexture( desc, m_Facts.limits ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	// Presentation belongs to the render.presentation.v1 bridge; this headless
	// device has no swapchain, so a presentable texture is unsupported here.
	if ( desc.usages.Has( ResourceUsage::kPresent ) )
		return Fail( DeviceStatus::kUnsupported, op );
	const VkImageUsageFlags usage = ImageUsageFlags( desc.usages );
	if ( usage == 0 )
		return Fail( DeviceStatus::kInvalidDescription, op );
	const bool attachment = ( usage & ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
	                                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) ) != 0;
	if ( attachment && desc.dimension == TextureDimension::k3D )
		return Fail( DeviceStatus::kUnsupported, op );

	TextureRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	record.layers = desc.dimension == TextureDimension::k3D ? 1u : desc.depthOrLayers;
	record.format = ToVkFormat( desc.format );

	const VkImageType imageType =
	    desc.dimension == TextureDimension::k3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
	const VkImageCreateFlags flags =
	    desc.dimension == TextureDimension::kCube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	VkImageFormatProperties supported{};
	const VkResult support = vkGetPhysicalDeviceImageFormatProperties( m_Adapter.physical,
	    record.format, imageType, VK_IMAGE_TILING_OPTIMAL, usage, flags, &supported );
	if ( support != VK_SUCCESS || !( supported.sampleCounts & desc.sampleCount ) ||
	     desc.mipLevels > supported.maxMipLevels || record.layers > supported.maxArrayLayers ||
	     desc.width > supported.maxExtent.width || desc.height > supported.maxExtent.height )
		return Fail( DeviceStatus::kUnsupported, op, support );

	Undo undo(
	    [&]
	    {
		    vkDestroyImageView( m_Device, record.attachmentView, nullptr );
		    vkDestroyImageView( m_Device, record.view, nullptr );
		    m_Memory.DestroyImage( record.image, record.memory );
	    } );
	VkImageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.flags = flags;
	info.imageType = imageType;
	info.format = record.format;
	info.extent = { desc.width, desc.height,
	    desc.dimension == TextureDimension::k3D ? desc.depthOrLayers : 1u };
	info.mipLevels = desc.mipLevels;
	info.arrayLayers = record.layers;
	info.samples = static_cast<VkSampleCountFlagBits>( desc.sampleCount );
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = usage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkResult result = m_Memory.CreateImage(
	    info, MemoryFor( MemoryKind::kDeviceLocal ), &record.image, &record.memory );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), op, result );

	VkImageViewCreateInfo view{};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.image = record.image;
	view.format = record.format;
	// Shaders read depth; a combined format's view names only its depth aspect.
	view.subresourceRange = { CopyAspect( desc.format ), 0, desc.mipLevels, 0, record.layers };
	if ( usage & ( VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT ) )
	{
		switch ( desc.dimension )
		{
		case TextureDimension::k2D:
			view.viewType = record.layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
			break;
		case TextureDimension::kCube:
			view.viewType =
			    record.layers > 6 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
			break;
		case TextureDimension::k3D:
			view.viewType = VK_IMAGE_VIEW_TYPE_3D;
			break;
		}
		result = vkCreateImageView( m_Device, &view, nullptr, &record.view );
		if ( result != VK_SUCCESS )
		{
			record.view = VK_NULL_HANDLE;
			return Fail( StatusOf( result ), op, result );
		}
	}
	if ( attachment )
	{
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.subresourceRange = { BarrierAspects( desc.format ), 0, 1, 0, 1 };
		result = vkCreateImageView( m_Device, &view, nullptr, &record.attachmentView );
		if ( result != VK_SUCCESS )
		{
			record.attachmentView = VK_NULL_HANDLE;
			return Fail( StatusOf( result ), op, result );
		}
	}
	Name( VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>( record.image ), desc.debugName );
	undo.Dismiss();
	const TextureId id{ ++m_NextId };
	m_Textures.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<SamplerId> VulkanDevice::CreateSampler( const SamplerDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateSampler;
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( desc.maxAnisotropy == 0 || desc.maxAnisotropy > 16 )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( desc.maxAnisotropy > 1 &&
	     ( !m_Adapter.anisotropy ||
	         static_cast<float>( desc.maxAnisotropy ) > m_Properties.limits.maxSamplerAnisotropy ) )
		return Fail( DeviceStatus::kUnsupported, op );
	auto filter = []( Filter value )
	{
		return value == Filter::kNearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
	};
	VkSamplerAddressMode address = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	switch ( desc.address )
	{
	case AddressMode::kRepeat:
		address = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	case AddressMode::kClampToEdge:
		address = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	case AddressMode::kMirroredRepeat:
		address = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		break;
	}
	VkSamplerCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.magFilter = filter( desc.magFilter );
	info.minFilter = filter( desc.minFilter );
	info.mipmapMode = desc.mipFilter == Filter::kNearest ? VK_SAMPLER_MIPMAP_MODE_NEAREST
	                                                     : VK_SAMPLER_MIPMAP_MODE_LINEAR;
	info.addressModeU = address;
	info.addressModeV = address;
	info.addressModeW = address;
	info.anisotropyEnable = desc.maxAnisotropy > 1 ? VK_TRUE : VK_FALSE;
	info.maxAnisotropy = static_cast<float>( desc.maxAnisotropy );
	info.maxLod = VK_LOD_CLAMP_NONE;
	SamplerRecord record;
	const VkResult result = vkCreateSampler( m_Device, &info, nullptr, &record.sampler );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), op, result );
	const SamplerId id{ ++m_NextId };
	m_Samplers.emplace( id.value, record );
	return id;
}

} // namespace render::device::vulkan
