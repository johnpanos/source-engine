/*========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan-features probe (RFC 0016 K1). Prints, as one
//			JSON document on stdout, every physical device the Vulkan loader
//			lists with the three features the Vulkan adapter requires
//			(timeline semaphores, synchronization2, dynamic rendering): whether
//			each is core for the device's API version or offered as an
//			extension, and whether the feature bit is supported. It also
//			reports what render/device/vulkan/instance.cpp's SelectAdapter
//			would decide (Evaluate's rules and TypeRank's order), as fields,
//			and what host mode (SelectHostAdapter) would: Vulkan 1.1 with the
//			timeline and synchronization2 extensions, dynamic rendering
//			optional (user decision, 2026-09-29).
//
//			Links only the Vulkan loader. Creates no window, surface or
//			logical device. Built and run by tools/render/vulkan_features.py.
//
//=============================================================================*/

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROBE_VERSION "render-device-vulkan-features-probe/v2"

static void PrintString( const char *value )
{
	putchar( '"' );
	for ( const char *p = value; *p; ++p )
	{
		const unsigned char c = (unsigned char)*p;
		if ( c == '"' || c == '\\' )
			printf( "\\%c", c );
		else if ( c < 0x20 )
			printf( "\\u%04x", c );
		else
			putchar( c );
	}
	putchar( '"' );
}

static void PrintVersion( uint32_t version )
{
	printf( "\"%u.%u.%u\"", VK_API_VERSION_MAJOR( version ), VK_API_VERSION_MINOR( version ),
	    VK_API_VERSION_PATCH( version ) );
}

static int HasExtension( const VkExtensionProperties *extensions, uint32_t count, const char *name )
{
	for ( uint32_t i = 0; i < count; ++i )
	{
		if ( strcmp( extensions[i].extensionName, name ) == 0 )
			return 1;
	}
	return 0;
}

/* SelectAdapter's TypeRank: lower is preferred. */
static int TypeRank( VkPhysicalDeviceType type )
{
	switch ( type )
	{
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return 0;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return 1;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return 2;
	default:
		return 3;
	}
}

static const char *TypeName( VkPhysicalDeviceType type )
{
	switch ( type )
	{
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return "discrete";
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return "integrated";
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return "virtual";
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		return "cpu";
	default:
		return "other";
	}
}

static void PrintFeature( const char *name, int core, int extension, int supported )
{
	printf( "\"%s\": {\"core\": %s, \"extension\": %s, \"supported\": %s}", name,
	    core ? "true" : "false", extension ? "true" : "false", supported ? "true" : "false" );
}

int main( void )
{
	uint32_t loaderVersion = VK_API_VERSION_1_0;
	PFN_vkEnumerateInstanceVersion enumerateVersion =
	    (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr( NULL, "vkEnumerateInstanceVersion" );
	if ( enumerateVersion )
		enumerateVersion( &loaderVersion );
	if ( loaderVersion < VK_API_VERSION_1_1 )
	{
		printf( "{\"probe\": \"%s\", \"error\": \"loader below Vulkan 1.1\"}\n", PROBE_VERSION );
		return 2;
	}

	/* Portability implementations are listed only to an instance that asks. */
	uint32_t instanceExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties( NULL, &instanceExtensionCount, NULL );
	VkExtensionProperties *instanceExtensions = calloc(
	    instanceExtensionCount ? instanceExtensionCount : 1, sizeof( VkExtensionProperties ) );
	vkEnumerateInstanceExtensionProperties( NULL, &instanceExtensionCount, instanceExtensions );
	const int portabilityEnumeration = HasExtension(
	    instanceExtensions, instanceExtensionCount, "VK_KHR_portability_enumeration" );
	free( instanceExtensions );

	VkApplicationInfo application = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	application.pApplicationName = "render.device.vulkan-features";
	application.pEngineName = "Source";
	application.apiVersion = loaderVersion;
	const char *enabled[] = { "VK_KHR_portability_enumeration" };
	VkInstanceCreateInfo info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	info.pApplicationInfo = &application;
	if ( portabilityEnumeration )
	{
		info.flags |= 0x00000001; /* VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR */
		info.enabledExtensionCount = 1;
		info.ppEnabledExtensionNames = enabled;
	}
	VkInstance instance = VK_NULL_HANDLE;
	const VkResult created = vkCreateInstance( &info, NULL, &instance );
	if ( created != VK_SUCCESS )
	{
		printf( "{\"probe\": \"%s\", \"error\": \"vkCreateInstance failed (%d)\"}\n", PROBE_VERSION,
		    (int)created );
		return 2;
	}

	uint32_t count = 0;
	vkEnumeratePhysicalDevices( instance, &count, NULL );
	VkPhysicalDevice *devices = calloc( count ? count : 1, sizeof( VkPhysicalDevice ) );
	vkEnumeratePhysicalDevices( instance, &count, devices );

	/* SelectAdapter's choice: the first eligible device of the lowest rank. */
	int selected = -1;
	int selectedRank = 0;
	int hostSelected = -1;
	int hostSelectedRank = 0;
	int *eligible = calloc( count ? count : 1, sizeof( int ) );

	printf( "{\"probe\": \"%s\", \"loader_api\": ", PROBE_VERSION );
	PrintVersion( loaderVersion );
	printf( ", \"devices\": [" );
	for ( uint32_t d = 0; d < count; ++d )
	{
		VkPhysicalDevice physical = devices[d];
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties( physical, &properties );

		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties( physical, NULL, &extensionCount, NULL );
		VkExtensionProperties *extensions =
		    calloc( extensionCount ? extensionCount : 1, sizeof( VkExtensionProperties ) );
		vkEnumerateDeviceExtensionProperties( physical, NULL, &extensionCount, extensions );

		const int api12 = properties.apiVersion >= VK_API_VERSION_1_2;
		const int api13 = properties.apiVersion >= VK_API_VERSION_1_3;
		const int extTimeline =
		    HasExtension( extensions, extensionCount, "VK_KHR_timeline_semaphore" );
		const int extSync2 = HasExtension( extensions, extensionCount, "VK_KHR_synchronization2" );
		const int extRendering =
		    HasExtension( extensions, extensionCount, "VK_KHR_dynamic_rendering" );
		const int portabilitySubset =
		    HasExtension( extensions, extensionCount, "VK_KHR_portability_subset" );
		const int driverProperties =
		    api12 || HasExtension( extensions, extensionCount, "VK_KHR_driver_properties" );

		/* The feature bits: core structures where the device's version has
		   them, the extension structures where it offers the extension. */
		VkPhysicalDeviceVulkan12Features features12 = {
		    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		VkPhysicalDeviceVulkan13Features features13 = {
		    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timeline = {
		    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR };
		VkPhysicalDeviceSynchronization2FeaturesKHR sync2 = {
		    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR };
		VkPhysicalDeviceDynamicRenderingFeaturesKHR rendering = {
		    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
		VkPhysicalDeviceFeatures2 features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		void **tail = &features.pNext;
		if ( api12 )
		{
			*tail = &features12;
			tail = &features12.pNext;
		}
		if ( api13 )
		{
			*tail = &features13;
			tail = &features13.pNext;
		}
		if ( !api12 && extTimeline )
		{
			*tail = &timeline;
			tail = &timeline.pNext;
		}
		if ( !api13 && extSync2 )
		{
			*tail = &sync2;
			tail = &sync2.pNext;
		}
		if ( !api13 && extRendering )
		{
			*tail = &rendering;
			tail = &rendering.pNext;
		}
		vkGetPhysicalDeviceFeatures2( physical, &features );
		const int timelineSupported =
		    api12 ? features12.timelineSemaphore : timeline.timelineSemaphore;
		const int sync2Supported = api13 ? features13.synchronization2 : sync2.synchronization2;
		const int renderingSupported =
		    api13 ? features13.dynamicRendering : rendering.dynamicRendering;

		VkPhysicalDeviceDriverProperties driver = {
		    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES };
		if ( driverProperties )
		{
			VkPhysicalDeviceProperties2 properties2 = {
			    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
			properties2.pNext = &driver;
			vkGetPhysicalDeviceProperties2( physical, &properties2 );
		}

		/* Evaluate(): API 1.2, four bound sets, a graphics+compute family,
		   the extensions below 1.3, and the three feature bits. */
		const char *reason = NULL;
		int graphicsCompute = 0;
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( physical, &familyCount, NULL );
		VkQueueFamilyProperties *families =
		    calloc( familyCount ? familyCount : 1, sizeof( VkQueueFamilyProperties ) );
		vkGetPhysicalDeviceQueueFamilyProperties( physical, &familyCount, families );
		for ( uint32_t i = 0; i < familyCount; ++i )
		{
			const VkQueueFlags wanted = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
			if ( ( families[i].queueFlags & wanted ) == wanted && families[i].queueCount > 0 )
				graphicsCompute = 1;
		}
		free( families );
		if ( !api12 )
			reason = "API below 1.2";
		else if ( properties.limits.maxBoundDescriptorSets < 4 )
			reason = "fewer than four bound descriptor sets";
		else if ( !graphicsCompute )
			reason = "no graphics and compute queue family";
		else if ( !api13 && ( !extSync2 || !extRendering ) )
			reason = "below 1.3 without VK_KHR_synchronization2 and VK_KHR_dynamic_rendering";
		else if ( !timelineSupported || !sync2Supported || !renderingSupported )
			reason = "a required feature bit is not supported";
		eligible[d] = reason == NULL;
		const int rank = TypeRank( properties.deviceType );
		if ( eligible[d] && ( selected < 0 || rank < selectedRank ) )
		{
			selected = (int)d;
			selectedRank = rank;
		}
		/* Evaluate( physical, host = true ): API 1.1, the timeline extension
		   below 1.2, synchronization2 below 1.3; dynamic rendering optional. */
		const char *hostReason = NULL;
		if ( properties.apiVersion < VK_API_VERSION_1_1 )
			hostReason = "API below 1.1";
		else if ( properties.limits.maxBoundDescriptorSets < 4 )
			hostReason = "fewer than four bound descriptor sets";
		else if ( !graphicsCompute )
			hostReason = "no graphics and compute queue family";
		else if ( !api12 && !extTimeline )
			hostReason = "below 1.2 without VK_KHR_timeline_semaphore";
		else if ( !api13 && !extSync2 )
			hostReason = "below 1.3 without VK_KHR_synchronization2";
		else if ( !timelineSupported || !sync2Supported )
			hostReason = "a required feature bit is not supported";
		if ( !hostReason && ( hostSelected < 0 || rank < hostSelectedRank ) )
		{
			hostSelected = (int)d;
			hostSelectedRank = rank;
		}

		printf( "%s{\"index\": %u, \"name\": ", d ? ", " : "", d );
		PrintString( properties.deviceName );
		printf( ", \"type\": \"%s\", \"vendor_id\": \"0x%04x\", \"device_id\": \"0x%04x\"",
		    TypeName( properties.deviceType ), properties.vendorID, properties.deviceID );
		printf( ", \"api_version\": " );
		PrintVersion( properties.apiVersion );
		printf( ", \"driver_version\": %u, \"driver_name\": ", properties.driverVersion );
		PrintString( driverProperties ? driver.driverName : "" );
		printf( ", \"driver_info\": " );
		PrintString( driverProperties ? driver.driverInfo : "" );
		printf(
		    ", \"portability_subset\": %s, \"features\": {", portabilitySubset ? "true" : "false" );
		PrintFeature( "timeline_semaphore", api12, extTimeline, timelineSupported );
		printf( ", " );
		PrintFeature( "synchronization2", api13, extSync2, sync2Supported );
		printf( ", " );
		PrintFeature( "dynamic_rendering", api13, extRendering, renderingSupported );
		printf( "}, \"adapter\": {\"eligible\": %s, \"path\": \"%s\", \"reason\": ",
		    eligible[d] ? "true" : "false", api13 ? "core13" : "vulkan12+extensions" );
		PrintString( reason ? reason : "" );
		printf( "}, \"host\": {\"eligible\": %s, \"dynamic_rendering\": %s, \"reason\": ",
		    hostReason ? "false" : "true", renderingSupported ? "true" : "false" );
		PrintString( hostReason ? hostReason : "" );
		printf( "}}" );
		free( extensions );
	}
	printf( "], \"adapter_selects\": %d, \"host_selects\": %d}\n", selected, hostSelected );

	free( eligible );
	free( devices );
	vkDestroyInstance( instance, NULL );
	return 0;
}
