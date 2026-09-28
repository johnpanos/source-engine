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

#include <optional>

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

namespace render::device::vulkan
{

namespace
{

// The DRM fourcc of a format's one plane, as bytes lie in memory (a DRM
// fourcc names the channels of a little-endian word, most significant
// first); 0 for a format this adapter does not export.
std::uint32_t DrmFourcc( Format format )
{
	auto code = []( char a, char b, char c, char d )
	{
		return std::uint32_t( std::uint8_t( a ) ) | ( std::uint32_t( std::uint8_t( b ) ) << 8 ) |
		       ( std::uint32_t( std::uint8_t( c ) ) << 16 ) |
		       ( std::uint32_t( std::uint8_t( d ) ) << 24 );
	};
	switch ( format )
	{
	case Format::kRGBA8Unorm:
	case Format::kRGBA8Srgb:
		return code( 'A', 'B', '2', '4' ); // DRM_FORMAT_ABGR8888: R, G, B, A bytes
	case Format::kBGRA8Unorm:
	case Format::kBGRA8Srgb:
		return code( 'A', 'R', '2', '4' ); // DRM_FORMAT_ARGB8888: B, G, R, A bytes
	default:
		return 0;
	}
}

constexpr std::uint64_t kDrmModifierLinear = 0; // DRM_FORMAT_MOD_LINEAR

} // namespace

void VulkanDevice::DestroyTexture( TextureRecord &record )
{
	vkDestroyImageView( m_Device, record.attachmentView, nullptr );
	vkDestroyImageView( m_Device, record.view, nullptr );
	if ( record.exported != VK_NULL_HANDLE )
	{
		vkDestroyImage( m_Device, record.image, nullptr );
		vkFreeMemory( m_Device, record.exported, nullptr );
	}
	else
	{
		m_Memory.DestroyImage( record.image, record.memory );
	}
	record.attachmentView = VK_NULL_HANDLE;
	record.view = VK_NULL_HANDLE;
	record.image = VK_NULL_HANDLE;
	record.exported = VK_NULL_HANDLE;
	record.memory = nullptr;
}

IExternalImages *VulkanDevice::ExternalImages()
{
	return m_Adapter.externalImages && m_Vk.getMemoryFd && !m_Options.sensitivity.nullExternalImages
	           ? this
	           : nullptr;
}

// A LINEAR image (DRM format modifier LINEAR) in dedicated memory exported as
// a dmabuf, so any importer reads rows at the reported offset and stride.
foundation::Expected<ExternalImage, DeviceError> VulkanDevice::CreateExported(
    const TextureDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kExportTexture;
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( !ExternalImages() )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( desc.dimension != TextureDimension::k2D || desc.mipLevels != 1 ||
	     desc.depthOrLayers != 1 || desc.sampleCount != 1 ||
	     !desc.usages.Has( ResourceUsage::kExternal ) )
		return Fail( DeviceStatus::kInvalidDescription, op );
	TextureDesc plain = desc;
	plain.usages = UsageSet();
	for ( std::uint32_t bit = 0; bit < static_cast<std::uint32_t>( ResourceUsage::kCount ); ++bit )
	{
		const ResourceUsage usage = static_cast<ResourceUsage>( bit );
		if ( usage != ResourceUsage::kExternal && desc.usages.Has( usage ) )
			plain.usages.Add( usage );
	}
	if ( auto valid = ValidateTexture( plain, m_Facts.limits ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	const std::uint32_t fourcc = DrmFourcc( desc.format );
	const VkImageUsageFlags usage = ImageUsageFlags( desc.usages );
	if ( fourcc == 0 || usage == 0 )
		return Fail( DeviceStatus::kUnsupported, op );

	TextureRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	record.format = ToVkFormat( desc.format );

	// Can this format, usage and the LINEAR modifier be exported as a dmabuf?
	VkPhysicalDeviceImageDrmFormatModifierInfoEXT modifierInfo{};
	modifierInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT;
	modifierInfo.drmFormatModifier = kDrmModifierLinear;
	modifierInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkPhysicalDeviceExternalImageFormatInfo externalInfo{};
	externalInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
	externalInfo.pNext = &modifierInfo;
	externalInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
	VkPhysicalDeviceImageFormatInfo2 formatInfo{};
	formatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
	formatInfo.pNext = &externalInfo;
	formatInfo.format = record.format;
	formatInfo.type = VK_IMAGE_TYPE_2D;
	formatInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
	formatInfo.usage = usage;
	VkExternalImageFormatProperties externalProperties{};
	externalProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;
	VkImageFormatProperties2 properties{};
	properties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
	properties.pNext = &externalProperties;
	const VkResult support =
	    vkGetPhysicalDeviceImageFormatProperties2( m_Adapter.physical, &formatInfo, &properties );
	if ( support != VK_SUCCESS ||
	     !( externalProperties.externalMemoryProperties.externalMemoryFeatures &
	         VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT ) ||
	     desc.width > properties.imageFormatProperties.maxExtent.width ||
	     desc.height > properties.imageFormatProperties.maxExtent.height )
		return Fail( DeviceStatus::kUnsupported, op, support );

	VkDeviceMemory stale = VK_NULL_HANDLE; // sensitivity fixture only
	Undo undo(
	    [&]
	    {
		    DestroyTexture( record );
		    vkFreeMemory( m_Device, stale, nullptr );
	    } );
	const std::uint64_t modifiers[] = { kDrmModifierLinear };
	VkImageDrmFormatModifierListCreateInfoEXT modifierList{};
	modifierList.sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT;
	modifierList.drmFormatModifierCount = 1;
	modifierList.pDrmFormatModifiers = modifiers;
	VkExternalMemoryImageCreateInfo external{};
	external.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
	external.pNext = &modifierList;
	external.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
	VkImageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = &external;
	info.imageType = VK_IMAGE_TYPE_2D;
	info.format = record.format;
	info.extent = { desc.width, desc.height, 1 };
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
	info.usage = usage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkResult result = vkCreateImage( m_Device, &info, nullptr, &record.image );
	if ( result != VK_SUCCESS )
	{
		record.image = VK_NULL_HANDLE;
		return Fail( StatusOf( result ), op, result );
	}

	// Dedicated, exportable memory: device-local and host-visible first, so an
	// importer that maps the dmabuf (a toolkit's fallback, clause D18) can;
	// then device-local; then any type the image allows.
	VkMemoryRequirements requirements{};
	vkGetImageMemoryRequirements( m_Device, record.image, &requirements );
	std::optional<std::uint32_t> type;
	const VkMemoryPropertyFlags preferences[] = {
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0 };
	for ( VkMemoryPropertyFlags wanted : preferences )
	{
		for ( std::uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount && !type; ++i )
		{
			const bool allowed = ( requirements.memoryTypeBits & ( 1u << i ) ) != 0;
			if ( allowed && ( m_MemoryProperties.memoryTypes[i].propertyFlags & wanted ) == wanted )
				type = i;
		}
	}
	if ( !type )
		return Fail( DeviceStatus::kUnsupported, op );
	auto allocate = [&]( VkImage dedicated, VkDeviceMemory *memory )
	{
		VkMemoryDedicatedAllocateInfo dedication{};
		dedication.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
		dedication.image = dedicated;
		VkExportMemoryAllocateInfo exportInfo{};
		exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
		exportInfo.pNext = dedicated != VK_NULL_HANDLE ? &dedication : nullptr;
		exportInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
		VkMemoryAllocateInfo allocation{};
		allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocation.pNext = &exportInfo;
		allocation.allocationSize = requirements.size;
		allocation.memoryTypeIndex = *type;
		return vkAllocateMemory( m_Device, &allocation, nullptr, memory );
	};
	result = allocate( record.image, &record.exported );
	if ( result != VK_SUCCESS )
	{
		record.exported = VK_NULL_HANDLE;
		return Fail( StatusOf( result ), op, result );
	}
	result = vkBindImageMemory( m_Device, record.image, record.exported, 0 );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), op, result );

	VkImageViewCreateInfo view{};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.image = record.image;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = record.format;
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	if ( usage & ( VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT ) )
	{
		result = vkCreateImageView( m_Device, &view, nullptr, &record.view );
		if ( result != VK_SUCCESS )
		{
			record.view = VK_NULL_HANDLE;
			return Fail( StatusOf( result ), op, result );
		}
	}
	if ( usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT )
	{
		result = vkCreateImageView( m_Device, &view, nullptr, &record.attachmentView );
		if ( result != VK_SUCCESS )
		{
			record.attachmentView = VK_NULL_HANDLE;
			return Fail( StatusOf( result ), op, result );
		}
	}

	VkImageSubresource plane{};
	plane.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT;
	VkSubresourceLayout layout{};
	vkGetImageSubresourceLayout( m_Device, record.image, &plane, &layout );

	VkDeviceMemory exported = record.exported;
	if ( m_Options.sensitivity.staleExport )
	{
		// D18 sensitivity: export memory the image does not use.
		result = allocate( VK_NULL_HANDLE, &stale );
		if ( result != VK_SUCCESS )
		{
			stale = VK_NULL_HANDLE;
			return Fail( StatusOf( result ), op, result );
		}
		exported = stale;
	}
	VkMemoryGetFdInfoKHR fdInfo{};
	fdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
	fdInfo.memory = exported;
	fdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
	int fd = -1;
	result = m_Vk.getMemoryFd( m_Device, &fdInfo, &fd );
	if ( result != VK_SUCCESS || fd < 0 )
		return Fail( StatusOf( result ), op, result );
	// The descriptor keeps the stale memory alive for the importer; the
	// adapter's handle can go.
	vkFreeMemory( m_Device, stale, nullptr );
	stale = VK_NULL_HANDLE;

	Name( VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>( record.image ), desc.debugName );
	undo.Dismiss();
	const TextureId id{ ++m_NextId };
	m_Textures.emplace( id.value, std::move( record ) );
	ExternalImage image;
	image.texture = id;
	image.handle = fd;
	image.fourcc = fourcc;
	image.modifier = kDrmModifierLinear;
	image.offset = static_cast<std::uint32_t>( layout.offset );
	image.stride = static_cast<std::uint32_t>( layout.rowPitch );
	return image;
}

} // namespace render::device::vulkan
