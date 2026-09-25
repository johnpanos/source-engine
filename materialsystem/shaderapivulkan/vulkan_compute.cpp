//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The native Vulkan compute foundation; see vulkan_compute.h.
//
//===========================================================================//

#include "vulkan_compute.h"

#include "material_spv.h"
#include "render/render_backend.h"

#include <algorithm>
#include <cstring>

namespace render_vulkan
{
namespace
{

bool HasExtension( VkPhysicalDevice physical, const char *name )
{
	uint32_t count = 0;
	vkEnumerateDeviceExtensionProperties( physical, nullptr, &count, nullptr );
	std::vector<VkExtensionProperties> extensions( count );
	if ( count )
		vkEnumerateDeviceExtensionProperties( physical, nullptr, &count, extensions.data() );
	for ( const VkExtensionProperties &extension : extensions )
		if ( std::strcmp( extension.extensionName, name ) == 0 )
			return true;
	return false;
}

bool StorageFormat( VkPhysicalDevice physical, VkFormat format )
{
	VkFormatProperties properties = {};
	vkGetPhysicalDeviceFormatProperties( physical, format, &properties );
	return ( properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT ) != 0;
}

const char *const kRayQueryExtensions[] = { VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_RAY_QUERY_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME };

} // namespace

ComputeCaps QueryComputeCaps( VkPhysicalDevice physical, uint32_t queueFamily )
{
	ComputeCaps caps;
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( physical, &properties );
	caps.deviceApiVersion = properties.apiVersion;
	caps.maxWorkgroupInvocations = properties.limits.maxComputeWorkGroupInvocations;
	caps.maxBoundDescriptorSets = properties.limits.maxBoundDescriptorSets;
	uint32_t families = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( physical, &families, nullptr );
	std::vector<VkQueueFamilyProperties> family( families );
	vkGetPhysicalDeviceQueueFamilyProperties( physical, &families, family.data() );
	caps.compute =
	    queueFamily < families && ( family[queueFamily].queueFlags & VK_QUEUE_COMPUTE_BIT );
	caps.storageImages = caps.compute && StorageFormat( physical, VK_FORMAT_R16G16B16A16_SFLOAT ) &&
	                     StorageFormat( physical, VK_FORMAT_R32_SFLOAT );
	if ( VK_API_VERSION_MINOR( caps.deviceApiVersion ) >= 2 ||
	     VK_API_VERSION_MAJOR( caps.deviceApiVersion ) > 1 )
	{
		VkPhysicalDeviceVulkan12Features vulkan12 = {};
		vulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		VkPhysicalDeviceAccelerationStructureFeaturesKHR structures = {};
		structures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		VkPhysicalDeviceRayQueryFeaturesKHR rayQuery = {};
		rayQuery.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
		bool extensions = true;
		for ( const char *name : kRayQueryExtensions )
			extensions = extensions && HasExtension( physical, name );
		VkPhysicalDeviceFeatures2 features = {};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = &vulkan12;
		if ( extensions )
		{
			vulkan12.pNext = &structures;
			structures.pNext = &rayQuery;
		}
		vkGetPhysicalDeviceFeatures2( physical, &features );
		caps.timelineSemaphore = vulkan12.timelineSemaphore == VK_TRUE;
		caps.rayQuery = extensions && caps.compute && vulkan12.bufferDeviceAddress &&
		                structures.accelerationStructure && rayQuery.rayQuery;
	}
	return caps;
}

uint32_t ComputeFeatureBits( const ComputeCaps &caps )
{
	render::RenderFeatureSet set;
	if ( caps.compute )
		set.Add( render::RenderFeature::kComputeShaders );
	if ( caps.storageImages )
		set.Add( render::RenderFeature::kStorageImages );
	if ( caps.rayQuery )
		set.Add( render::RenderFeature::kRayQuery );
	return set.bits;
}

void DeviceFeatureChain::Build( VkPhysicalDevice physical, const ComputeCaps &caps,
    const VkPhysicalDeviceFeatures &baseFeatures, std::vector<const char *> *extensions )
{
	(void)physical;
	m_features2 = {};
	m_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	m_features2.features = baseFeatures;
	m_enabled = ComputeCaps();
	m_enabled.deviceApiVersion = caps.deviceApiVersion;
	m_enabled.maxWorkgroupInvocations = caps.maxWorkgroupInvocations;
	m_enabled.maxBoundDescriptorSets = caps.maxBoundDescriptorSets;
	// Compute and storage images are core: enabled by the queue and formats.
	m_enabled.compute = caps.compute;
	m_enabled.storageImages = caps.storageImages;
	const bool vulkan12 = VK_API_VERSION_MINOR( caps.deviceApiVersion ) >= 2 ||
	                      VK_API_VERSION_MAJOR( caps.deviceApiVersion ) > 1;
	if ( !vulkan12 )
		return;
	m_vulkan12 = {};
	m_vulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	m_features2.pNext = &m_vulkan12;
	m_vulkan12.timelineSemaphore = caps.timelineSemaphore ? VK_TRUE : VK_FALSE;
	m_enabled.timelineSemaphore = caps.timelineSemaphore;
	if ( caps.rayQuery )
	{
		m_vulkan12.bufferDeviceAddress = VK_TRUE;
		m_accelerationStructure = {};
		m_accelerationStructure.sType =
		    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		m_accelerationStructure.accelerationStructure = VK_TRUE;
		m_rayQuery = {};
		m_rayQuery.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
		m_rayQuery.rayQuery = VK_TRUE;
		m_vulkan12.pNext = &m_accelerationStructure;
		m_accelerationStructure.pNext = &m_rayQuery;
		for ( const char *name : kRayQueryExtensions )
			if ( std::find_if( extensions->begin(), extensions->end(),
			         [&]( const char *have )
			         {
				         return std::strcmp( have, name ) == 0;
			         } ) == extensions->end() )
				extensions->push_back( name );
		m_enabled.rayQuery = true;
	}
}

bool ComputeResources::Init(
    VkPhysicalDevice physical, VkDevice device, const ComputeCaps &enabled, std::string *error )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	m_physical = physical;
	m_device = device;
	m_enabled = enabled;
	if ( !enabled.compute && error )
		*error = "the device's graphics queue family does not compute";
	if ( enabled.compute && enabled.rayQuery )
	{
		m_createStructure = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
		    vkGetDeviceProcAddr( device, "vkCreateAccelerationStructureKHR" ) );
		m_destroyStructure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
		    vkGetDeviceProcAddr( device, "vkDestroyAccelerationStructureKHR" ) );
		m_buildSizes = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
		    vkGetDeviceProcAddr( device, "vkGetAccelerationStructureBuildSizesKHR" ) );
		m_cmdBuild = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
		    vkGetDeviceProcAddr( device, "vkCmdBuildAccelerationStructuresKHR" ) );
		m_structureAddress = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
		    vkGetDeviceProcAddr( device, "vkGetAccelerationStructureDeviceAddressKHR" ) );
		m_bufferAddress = reinterpret_cast<PFN_vkGetBufferDeviceAddress>(
		    vkGetDeviceProcAddr( device, "vkGetBufferDeviceAddress" ) );
		VkPhysicalDeviceAccelerationStructurePropertiesKHR structures = {};
		structures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
		VkPhysicalDeviceProperties2 properties = {};
		properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		properties.pNext = &structures;
		vkGetPhysicalDeviceProperties2( physical, &properties );
		m_scratchAlignment =
		    std::max<VkDeviceSize>( 1, structures.minAccelerationStructureScratchOffsetAlignment );
		if ( !m_createStructure || !m_destroyStructure || !m_buildSizes || !m_cmdBuild ||
		     !m_structureAddress || !m_bufferAddress )
		{
			// Enabled but not loadable: no structure is ever created.
			m_enabled.rayQuery = false;
		}
	}
	return enabled.compute;
}

void ComputeResources::Shutdown()
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	for ( Resource &resource : m_resources )
		Destroy( resource );
	m_resources.clear();
	m_retired.clear();
	m_device = VK_NULL_HANDLE;
}

ComputeResources::Resource *ComputeResources::Find( uint32_t handle )
{
	for ( Resource &resource : m_resources )
		if ( resource.handle == handle )
			return &resource;
	return nullptr;
}

const ComputeResources::Resource *ComputeResources::Find( uint32_t handle ) const
{
	for ( const Resource &resource : m_resources )
		if ( resource.handle == handle )
			return &resource;
	return nullptr;
}

bool ComputeResources::Memory( const VkMemoryRequirements &requirements,
    VkMemoryPropertyFlags flags, VkDeviceMemory *memory, std::string *error, bool deviceAddress )
{
	VkMemoryAllocateFlagsInfo addressed = {};
	addressed.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	addressed.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	VkPhysicalDeviceMemoryProperties properties = {};
	vkGetPhysicalDeviceMemoryProperties( m_physical, &properties );
	for ( uint32_t i = 0; i < properties.memoryTypeCount; ++i )
	{
		if ( !( requirements.memoryTypeBits & ( 1u << i ) ) ||
		     ( properties.memoryTypes[i].propertyFlags & flags ) != flags )
			continue;
		VkMemoryAllocateInfo allocate = {};
		allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate.allocationSize = requirements.size;
		allocate.memoryTypeIndex = i;
		allocate.pNext = deviceAddress ? &addressed : nullptr;
		if ( vkAllocateMemory( m_device, &allocate, nullptr, memory ) == VK_SUCCESS )
			return true;
	}
	if ( error )
		*error = "no memory type for a compute resource";
	return false;
}

uint32_t ComputeResources::CreateBuffer( size_t bytes, std::string *error, bool readback )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	if ( !Ready() || bytes == 0 )
		return 0;
	Resource resource;
	resource.kind = ComputeBinding::StorageBuffer;
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = bytes;
	info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
	             VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if ( vkCreateBuffer( m_device, &info, nullptr, &resource.buffer ) != VK_SUCCESS )
		return 0;
	VkMemoryRequirements requirements = {};
	vkGetBufferMemoryRequirements( m_device, resource.buffer, &requirements );
	const VkMemoryPropertyFlags visible =
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	const bool placed =
	    ( readback && Memory( requirements, visible | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
	                      &resource.memory, nullptr ) ) ||
	    Memory( requirements, visible, &resource.memory, error );
	if ( !placed ||
	     vkBindBufferMemory( m_device, resource.buffer, resource.memory, 0 ) != VK_SUCCESS ||
	     vkMapMemory( m_device, resource.memory, 0, bytes, 0, &resource.mapped ) != VK_SUCCESS )
	{
		Destroy( resource );
		return 0;
	}
	resource.handle = ++m_next;
	m_resources.push_back( resource );
	return resource.handle;
}

bool ComputeResources::MakeBuffer( VkDeviceSize bytes, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags flags, VkBuffer *buffer, VkDeviceMemory *memory, void **mapped,
    std::string *error )
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = std::max<VkDeviceSize>( bytes, 4 );
	info.usage = usage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	*buffer = VK_NULL_HANDLE;
	*memory = VK_NULL_HANDLE;
	if ( vkCreateBuffer( m_device, &info, nullptr, buffer ) != VK_SUCCESS )
		return false;
	VkMemoryRequirements requirements = {};
	vkGetBufferMemoryRequirements( m_device, *buffer, &requirements );
	const bool addressed = ( usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) != 0;
	if ( !Memory( requirements, flags, memory, error, addressed ) ||
	     vkBindBufferMemory( m_device, *buffer, *memory, 0 ) != VK_SUCCESS ||
	     ( mapped && vkMapMemory( m_device, *memory, 0, info.size, 0, mapped ) != VK_SUCCESS ) )
	{
		vkDestroyBuffer( m_device, *buffer, nullptr );
		if ( *memory != VK_NULL_HANDLE )
			vkFreeMemory( m_device, *memory, nullptr );
		*buffer = VK_NULL_HANDLE;
		*memory = VK_NULL_HANDLE;
		return false;
	}
	return true;
}

VkDeviceAddress ComputeResources::Address( VkBuffer buffer ) const
{
	VkBufferDeviceAddressInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	info.buffer = buffer;
	return m_bufferAddress( m_device, &info );
}

uint32_t ComputeResources::CreateGeometry( const float *positions, uint32_t vertexCount,
    const uint32_t *indices, uint32_t indexCount, std::string *error )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	if ( !Ready() || !m_enabled.rayQuery || !vertexCount || !indexCount || indexCount % 3 )
	{
		if ( error )
			*error = "ray query is not enabled, or the geometry is empty";
		return 0;
	}
	Resource resource;
	resource.kind = ComputeBinding::AccelerationStructure;
	resource.structureType = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	const VkBufferUsageFlags input =
	    VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
	    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	const VkMemoryPropertyFlags visible =
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	const VkDeviceSize vertexBytes = VkDeviceSize( vertexCount ) * 12;
	const VkDeviceSize indexBytes = VkDeviceSize( indexCount ) * 4;
	std::pair<VkBuffer, VkDeviceMemory> vertices, triangles;
	void *vertexData = nullptr, *indexData = nullptr;
	if ( !MakeBuffer(
	         vertexBytes, input, visible, &vertices.first, &vertices.second, &vertexData, error ) )
		return 0;
	resource.extra.push_back( vertices );
	if ( !MakeBuffer(
	         indexBytes, input, visible, &triangles.first, &triangles.second, &indexData, error ) )
	{
		Destroy( resource );
		return 0;
	}
	resource.extra.push_back( triangles );
	std::memcpy( vertexData, positions, size_t( vertexBytes ) );
	std::memcpy( indexData, indices, size_t( indexBytes ) );
	VkAccelerationStructureGeometryKHR &geometry = resource.geometry;
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	VkAccelerationStructureGeometryTrianglesDataKHR &data = geometry.geometry.triangles;
	data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	data.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	data.vertexData.deviceAddress = Address( vertices.first );
	data.vertexStride = 12;
	data.maxVertex = vertexCount - 1;
	data.indexType = VK_INDEX_TYPE_UINT32;
	data.indexData.deviceAddress = Address( triangles.first );
	resource.primitives = indexCount / 3;
	return FinishStructure(
	    resource, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, error );
}

uint32_t ComputeResources::CreateScene(
    const Instance *instances, uint32_t count, std::string *error )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	if ( !Ready() || !m_enabled.rayQuery || !count )
	{
		if ( error )
			*error = "ray query is not enabled, or the scene is empty";
		return 0;
	}
	Resource resource;
	resource.kind = ComputeBinding::AccelerationStructure;
	resource.structureType = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	std::pair<VkBuffer, VkDeviceMemory> table;
	void *mapped = nullptr;
	if ( !MakeBuffer( VkDeviceSize( count ) * sizeof( VkAccelerationStructureInstanceKHR ),
	         VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
	             VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &table.first, &table.second, &mapped, error ) )
		return 0;
	resource.extra.push_back( table );
	VkAccelerationStructureInstanceKHR *out =
	    static_cast<VkAccelerationStructureInstanceKHR *>( mapped );
	for ( uint32_t i = 0; i < count; ++i )
	{
		const Resource *geometry = Find( instances[i].geometry );
		if ( !geometry || geometry->structure == VK_NULL_HANDLE ||
		     geometry->structureType != VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR )
		{
			if ( error )
				*error = "a scene instance names no geometry";
			Destroy( resource );
			return 0;
		}
		VkAccelerationStructureDeviceAddressInfoKHR address = {};
		address.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		address.accelerationStructure = geometry->structure;
		VkAccelerationStructureInstanceKHR instance = {};
		std::memcpy( instance.transform.matrix, instances[i].transform, sizeof( float ) * 12 );
		instance.instanceCustomIndex = instances[i].customIndex & 0xFFFFFFu;
		instance.mask = instances[i].mask & 0xFFu;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = m_structureAddress( m_device, &address );
		out[i] = instance;
	}
	VkAccelerationStructureGeometryKHR &geometry = resource.geometry;
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	geometry.geometry.instances.sType =
	    VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geometry.geometry.instances.data.deviceAddress = Address( table.first );
	resource.primitives = count;
	return FinishStructure(
	    resource, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, error );
}

// Sizes, storage, the structure itself and its scratch; registers it.
uint32_t ComputeResources::FinishStructure(
    Resource &resource, VkBuildAccelerationStructureFlagsKHR flags, std::string *error )
{
	VkAccelerationStructureBuildGeometryInfoKHR build = {};
	build.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build.type = resource.structureType;
	build.flags = flags;
	build.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	build.geometryCount = 1;
	build.pGeometries = &resource.geometry;
	VkAccelerationStructureBuildSizesInfoKHR sizes = {};
	sizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	m_buildSizes( m_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build,
	    &resource.primitives, &sizes );
	std::pair<VkBuffer, VkDeviceMemory> scratch;
	bool ok =
	    MakeBuffer( sizes.accelerationStructureSize,
	        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
	            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &resource.buffer, &resource.memory, nullptr,
	        error ) &&
	    MakeBuffer( sizes.buildScratchSize + m_scratchAlignment,
	        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &scratch.first, &scratch.second, nullptr, error );
	if ( scratch.first != VK_NULL_HANDLE )
	{
		resource.extra.push_back( scratch );
		const VkDeviceAddress base = Address( scratch.first );
		resource.scratch =
		    ( base + m_scratchAlignment - 1 ) / m_scratchAlignment * m_scratchAlignment;
	}
	if ( ok )
	{
		VkAccelerationStructureCreateInfoKHR create = {};
		create.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		create.buffer = resource.buffer;
		create.size = sizes.accelerationStructureSize;
		create.type = resource.structureType;
		ok = m_createStructure( m_device, &create, nullptr, &resource.structure ) == VK_SUCCESS;
	}
	if ( !ok )
	{
		if ( error && error->empty() )
			*error = "acceleration structure creation failed";
		Destroy( resource );
		return 0;
	}
	resource.handle = ++m_next;
	m_resources.push_back( resource );
	return resource.handle;
}

bool ComputeResources::RecordBuild( VkCommandBuffer cmd, uint32_t structure )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	Resource *resource = Find( structure );
	if ( !resource || resource->structure == VK_NULL_HANDLE )
		return false;
	VkAccelerationStructureBuildGeometryInfoKHR build = {};
	build.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build.type = resource->structureType;
	build.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	build.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	build.dstAccelerationStructure = resource->structure;
	build.geometryCount = 1;
	build.pGeometries = &resource->geometry;
	build.scratchData.deviceAddress = resource->scratch;
	VkAccelerationStructureBuildRangeInfoKHR range = {};
	range.primitiveCount = resource->primitives;
	const VkAccelerationStructureBuildRangeInfoKHR *ranges = &range;
	m_cmdBuild( cmd, 1, &build, &ranges );
	VkMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
	    VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR |
	        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	    0, 1, &barrier, 0, nullptr, 0, nullptr );
	return true;
}

uint32_t ComputeResources::CreateStorageImage(
    uint32_t width, uint32_t height, VkFormat format, std::string *error )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	if ( !Ready() || !m_enabled.storageImages || !width || !height )
	{
		if ( error )
			*error = "storage images are not enabled on this device";
		return 0;
	}
	Resource resource;
	resource.kind = ComputeBinding::StorageImage;
	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.imageType = VK_IMAGE_TYPE_2D;
	info.format = format;
	info.extent = { width, height, 1 };
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
	             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if ( vkCreateImage( m_device, &info, nullptr, &resource.image ) != VK_SUCCESS )
		return 0;
	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements( m_device, resource.image, &requirements );
	VkImageViewCreateInfo view = {};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = format;
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	if ( !Memory( requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &resource.memory, error ) ||
	     vkBindImageMemory( m_device, resource.image, resource.memory, 0 ) != VK_SUCCESS ||
	     ( view.image = resource.image,
	         vkCreateImageView( m_device, &view, nullptr, &resource.view ) != VK_SUCCESS ) )
	{
		Destroy( resource );
		return 0;
	}
	resource.handle = ++m_next;
	m_resources.push_back( resource );
	return resource.handle;
}

uint32_t ComputeResources::CreateProgram( const uint32_t *spirv, size_t bytes,
    const std::vector<ComputeBinding> &bindings, uint32_t pushBytes, std::string *error )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	if ( !Ready() )
		return 0;
	Resource resource;
	resource.program = true;
	resource.bindings = bindings;
	resource.pushBytes = pushBytes;
	std::vector<VkDescriptorSetLayoutBinding> layout( bindings.size() );
	uint32_t buffers = 0, images = 0, structures = 0;
	for ( size_t i = 0; i < bindings.size(); ++i )
	{
		if ( bindings[i] == ComputeBinding::AccelerationStructure && !m_enabled.rayQuery )
		{
			if ( error )
				*error = "an acceleration-structure binding needs ray query";
			return 0;
		}
		layout[i].binding = uint32_t( i );
		layout[i].descriptorType = bindings[i] == ComputeBinding::StorageBuffer
		                               ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		                           : bindings[i] == ComputeBinding::StorageImage
		                               ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		                               : VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		layout[i].descriptorCount = 1;
		layout[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		( bindings[i] == ComputeBinding::StorageBuffer    ? buffers
		    : bindings[i] == ComputeBinding::StorageImage ? images
		                                                  : structures ) += 1;
	}
	VkDescriptorSetLayoutCreateInfo setInfo = {};
	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setInfo.bindingCount = uint32_t( layout.size() );
	setInfo.pBindings = layout.data();
	VkPushConstantRange push = { VK_SHADER_STAGE_COMPUTE_BIT, 0, pushBytes };
	VkPipelineLayoutCreateInfo pipelineLayout = {};
	pipelineLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayout.setLayoutCount = 1;
	pipelineLayout.pushConstantRangeCount = pushBytes ? 1 : 0;
	pipelineLayout.pPushConstantRanges = &push;
	ShaderModuleCode code;
	code.code = spirv;
	code.sizeBytes = bytes;
	if ( m_shaders )
		code = m_shaders->Resolve( spirv, bytes );
	resource.name = code.name;
	VkShaderModuleCreateInfo module = {};
	module.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module.codeSize = code.sizeBytes;
	module.pCode = code.code;
	VkShaderModule shader = VK_NULL_HANDLE;
	// One descriptor set per dispatch in flight, freed when its submission
	// completes (Collect).
	std::vector<VkDescriptorPoolSize> sizes;
	if ( buffers )
		sizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffers * 256 } );
	if ( images )
		sizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, images * 256 } );
	if ( structures )
		sizes.push_back( { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, structures * 256 } );
	VkDescriptorPoolCreateInfo pool = {};
	pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool.maxSets = 256;
	pool.poolSizeCount = uint32_t( sizes.size() );
	pool.pPoolSizes = sizes.data();
	bool ok = vkCreateDescriptorSetLayout( m_device, &setInfo, nullptr, &resource.setLayout ) ==
	          VK_SUCCESS;
	pipelineLayout.pSetLayouts = &resource.setLayout;
	ok = ok && vkCreatePipelineLayout(
	               m_device, &pipelineLayout, nullptr, &resource.pipelineLayout ) == VK_SUCCESS;
	ok = ok && ( sizes.empty() || vkCreateDescriptorPool(
	                                  m_device, &pool, nullptr, &resource.pool ) == VK_SUCCESS );
	ok = ok && vkCreateShaderModule( m_device, &module, nullptr, &shader ) == VK_SUCCESS;
	if ( ok )
	{
		VkComputePipelineCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		info.stage.module = shader;
		info.stage.pName = "main";
		info.layout = resource.pipelineLayout;
		ok = vkCreateComputePipelines(
		         m_device, VK_NULL_HANDLE, 1, &info, nullptr, &resource.pipeline ) == VK_SUCCESS;
	}
	if ( shader != VK_NULL_HANDLE )
		vkDestroyShaderModule( m_device, shader, nullptr );
	if ( !ok )
	{
		if ( error )
			*error = "compute pipeline creation failed";
		Destroy( resource );
		return 0;
	}
	if ( m_debug && code.name )
		m_debug->Name( VK_OBJECT_TYPE_PIPELINE, resource.pipeline, code.name );
	resource.handle = ++m_next;
	m_resources.push_back( resource );
	return resource.handle;
}

void *ComputeResources::Map( uint32_t buffer )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	Resource *resource = Find( buffer );
	return resource ? resource->mapped : nullptr;
}

VkImage ComputeResources::Image( uint32_t image ) const
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	const Resource *resource = Find( image );
	return resource ? resource->image : VK_NULL_HANDLE;
}

VkBuffer ComputeResources::BufferHandle( uint32_t buffer ) const
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	const Resource *resource = Find( buffer );
	return resource ? resource->buffer : VK_NULL_HANDLE;
}

void ComputeResources::RecordImageLayout( VkCommandBuffer cmd, uint32_t image )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	const Resource *resource = Find( image );
	if ( !resource || resource->image == VK_NULL_HANDLE )
		return;
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = resource->image;
	barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier );
}

bool ComputeResources::RecordDispatch( VkCommandBuffer cmd, uint64_t serial, uint32_t program,
    const std::vector<uint32_t> &resources, const void *push, uint32_t pushBytes, uint32_t groupsX,
    uint32_t groupsY, uint32_t groupsZ )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	Resource *pipeline = Find( program );
	if ( !pipeline || !pipeline->program || resources.size() != pipeline->bindings.size() ||
	     pushBytes != pipeline->pushBytes )
		return false;
	VkDescriptorSet set = VK_NULL_HANDLE;
	if ( pipeline->pool != VK_NULL_HANDLE )
	{
		VkDescriptorSetAllocateInfo allocate = {};
		allocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate.descriptorPool = pipeline->pool;
		allocate.descriptorSetCount = 1;
		allocate.pSetLayouts = &pipeline->setLayout;
		// A full pool means 256 dispatches of this program are in flight.
		if ( vkAllocateDescriptorSets( m_device, &allocate, &set ) != VK_SUCCESS )
			return false;
		pipeline->sets.emplace_back( set, serial );
	}
	std::vector<VkDescriptorBufferInfo> buffers( resources.size() );
	std::vector<VkDescriptorImageInfo> images( resources.size() );
	std::vector<VkWriteDescriptorSetAccelerationStructureKHR> structures( resources.size() );
	std::vector<VkWriteDescriptorSet> writes;
	for ( size_t i = 0; i < resources.size(); ++i )
	{
		const Resource *resource = Find( resources[i] );
		if ( !resource || resource->program || resource->kind != pipeline->bindings[i] )
			return false;
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = set;
		write.dstBinding = uint32_t( i );
		write.descriptorCount = 1;
		if ( resource->kind == ComputeBinding::StorageBuffer )
		{
			buffers[i] = { resource->buffer, 0, VK_WHOLE_SIZE };
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.pBufferInfo = &buffers[i];
		}
		else if ( resource->kind == ComputeBinding::StorageImage )
		{
			images[i] = { VK_NULL_HANDLE, resource->view, VK_IMAGE_LAYOUT_GENERAL };
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			write.pImageInfo = &images[i];
		}
		else
		{
			structures[i] = {};
			structures[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
			structures[i].accelerationStructureCount = 1;
			structures[i].pAccelerationStructures = &resource->structure;
			write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
			write.pNext = &structures[i];
		}
		writes.push_back( write );
	}
	if ( !writes.empty() )
		vkUpdateDescriptorSets( m_device, uint32_t( writes.size() ), writes.data(), 0, nullptr );
	vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline );
	if ( set != VK_NULL_HANDLE )
		vkCmdBindDescriptorSets(
		    cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipelineLayout, 0, 1, &set, 0, nullptr );
	if ( pushBytes )
		vkCmdPushConstants(
		    cmd, pipeline->pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, pushBytes, push );
	if ( m_debug )
		m_debug->BeginLabel( cmd, pipeline->name ? pipeline->name : "compute dispatch" );
	vkCmdDispatch( cmd, groupsX, groupsY, groupsZ );
	if ( m_debug )
		m_debug->EndLabel( cmd );
	VkMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask =
	    VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
	        VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT,
	    0, 1, &barrier, 0, nullptr, 0, nullptr );
	return true;
}

void ComputeResources::Destroy( Resource &resource )
{
	if ( m_device == VK_NULL_HANDLE )
		return;
	if ( resource.pipeline != VK_NULL_HANDLE )
		vkDestroyPipeline( m_device, resource.pipeline, nullptr );
	if ( resource.pool != VK_NULL_HANDLE )
		vkDestroyDescriptorPool( m_device, resource.pool, nullptr );
	if ( resource.pipelineLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, resource.pipelineLayout, nullptr );
	if ( resource.setLayout != VK_NULL_HANDLE )
		vkDestroyDescriptorSetLayout( m_device, resource.setLayout, nullptr );
	if ( resource.structure != VK_NULL_HANDLE )
		m_destroyStructure( m_device, resource.structure, nullptr );
	for ( const auto &extra : resource.extra )
	{
		vkDestroyBuffer( m_device, extra.first, nullptr );
		vkFreeMemory( m_device, extra.second, nullptr );
	}
	if ( resource.view != VK_NULL_HANDLE )
		vkDestroyImageView( m_device, resource.view, nullptr );
	if ( resource.image != VK_NULL_HANDLE )
		vkDestroyImage( m_device, resource.image, nullptr );
	if ( resource.buffer != VK_NULL_HANDLE )
		vkDestroyBuffer( m_device, resource.buffer, nullptr );
	if ( resource.memory != VK_NULL_HANDLE )
		vkFreeMemory( m_device, resource.memory, nullptr );
	resource = Resource();
}

void ComputeResources::Retire( uint32_t resource, uint64_t afterSerial )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	if ( !Find( resource ) )
		return;
	m_retired.push_back( { resource, m_earlyFree ? 0 : afterSerial } );
	if ( m_earlyFree )
		Collect( 0 );
}

void ComputeResources::Collect( uint64_t completedSerial )
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	for ( Resource &resource : m_resources )
	{
		size_t live = 0;
		for ( const auto &entry : resource.sets )
		{
			if ( entry.second <= completedSerial )
				vkFreeDescriptorSets( m_device, resource.pool, 1, &entry.first );
			else
				resource.sets[live++] = entry;
		}
		resource.sets.resize( live );
	}
	size_t kept = 0;
	for ( const Retired &retired : m_retired )
	{
		if ( retired.afterSerial <= completedSerial )
		{
			for ( size_t i = 0; i < m_resources.size(); ++i )
			{
				if ( m_resources[i].handle == retired.handle )
				{
					Destroy( m_resources[i] );
					m_resources.erase( m_resources.begin() + std::ptrdiff_t( i ) );
					break;
				}
			}
		}
		else
			m_retired[kept++] = retired;
	}
	m_retired.resize( kept );
}

bool ComputeResources::Alive( uint32_t resource ) const
{
	std::lock_guard<std::recursive_mutex> lock( m_mutex );
	return Find( resource ) != nullptr;
}

gpu_compute::Caps GpuComputeService::Capabilities() const
{
	gpu_compute::Caps caps;
	caps.compute = m_resources.Ready();
	caps.storageImages = caps.compute && m_resources.Enabled().storageImages;
	caps.rayQuery = caps.compute && m_resources.Enabled().rayQuery;
	return caps;
}

uint32_t GpuComputeService::CreateBuffer( size_t bytes, gpu_compute::BufferUse use )
{
	std::string error;
	return m_resources.CreateBuffer( bytes, &error, use == gpu_compute::BufferUse::Readback );
}

void *GpuComputeService::Map( uint32_t buffer )
{
	return m_resources.Map( buffer );
}

uint32_t GpuComputeService::CreateProgram(
    const char *name, const gpu_compute::Binding *bindings, uint32_t count, uint32_t pushBytes )
{
	// The renderer's built-in programs; their bindings are storage buffers.
	struct Builtin
	{
		const char *name;
		const uint32_t *words;
		size_t bytes;
	};
	static const Builtin builtins[] = {
	    { "sdf-probe-trace", g_sdfProbeTraceSpv, sizeof( g_sdfProbeTraceSpv ) },
	    { "ray-query-probe-trace", g_rayQueryProbeTraceSpv, sizeof( g_rayQueryProbeTraceSpv ) },
	};
	std::vector<ComputeBinding> kinds( count );
	for ( uint32_t i = 0; i < count; ++i )
		kinds[i] = bindings[i] == gpu_compute::Binding::Scene
		               ? ComputeBinding::AccelerationStructure
		               : ComputeBinding::StorageBuffer;
	for ( const Builtin &builtin : builtins )
	{
		if ( std::strcmp( builtin.name, name ) != 0 )
			continue;
		std::string error;
		return m_resources.CreateProgram( builtin.words, builtin.bytes, kinds, pushBytes, &error );
	}
	return 0;
}

uint32_t GpuComputeService::CreateGeometry(
    const float *positions, uint32_t vertexCount, const uint32_t *indices, uint32_t indexCount )
{
	std::string error;
	const uint32_t geometry =
	    m_resources.CreateGeometry( positions, vertexCount, indices, indexCount, &error );
	if ( geometry )
	{
		Queued queued;
		queued.build = geometry;
		std::lock_guard<std::mutex> lock( m_queueMutex );
		m_queue.push_back( std::move( queued ) );
	}
	return geometry;
}

uint32_t GpuComputeService::CreateScene(
    const gpu_compute::SceneInstance *instances, uint32_t count )
{
	std::vector<ComputeResources::Instance> placed( count );
	for ( uint32_t i = 0; i < count; ++i )
	{
		placed[i].geometry = instances[i].geometry;
		std::memcpy( placed[i].transform, instances[i].transform, sizeof( placed[i].transform ) );
		placed[i].customIndex = instances[i].customIndex;
		placed[i].mask = instances[i].mask;
	}
	std::string error;
	const uint32_t scene = m_resources.CreateScene( placed.data(), count, &error );
	if ( scene )
	{
		Queued queued;
		queued.build = scene;
		std::lock_guard<std::mutex> lock( m_queueMutex );
		m_queue.push_back( std::move( queued ) );
	}
	return scene;
}

uint64_t GpuComputeService::QueueDispatch( uint32_t program, const uint32_t *buffers,
    uint32_t count, const void *push, uint32_t pushBytes, uint32_t groupsX, uint32_t groupsY,
    uint32_t groupsZ )
{
	if ( !m_resources.Alive( program ) )
		return 0;
	Queued queued;
	queued.program = program;
	queued.buffers.assign( buffers, buffers + count );
	const unsigned char *bytes = static_cast<const unsigned char *>( push );
	queued.push.assign( bytes, bytes + pushBytes );
	queued.groups[0] = groupsX;
	queued.groups[1] = groupsY;
	queued.groups[2] = groupsZ;
	// Under the lock Record takes: a dispatch queued after a recording goes
	// into a later submission, never one already recorded.
	std::lock_guard<std::mutex> lock( m_queueMutex );
	m_queue.push_back( std::move( queued ) );
	return std::max( m_nextSerial(), m_lastRecorded + 1 );
}

bool GpuComputeService::Pending() const
{
	std::lock_guard<std::mutex> lock( m_queueMutex );
	return !m_queue.empty();
}

void GpuComputeService::Record( VkCommandBuffer cmd, uint64_t serial )
{
	std::lock_guard<std::mutex> lock( m_queueMutex );
	if ( !m_queue.empty() )
		m_lastRecorded = std::max( m_lastRecorded, serial );
	for ( const Queued &queued : m_queue )
	{
		if ( queued.build )
			m_resources.RecordBuild( cmd, queued.build );
		else
			m_resources.RecordDispatch( cmd, serial, queued.program, queued.buffers,
			    queued.push.data(), uint32_t( queued.push.size() ), queued.groups[0],
			    queued.groups[1], queued.groups[2] );
	}
	m_queue.clear();
}

} // namespace render_vulkan
