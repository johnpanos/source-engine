//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.compute (RFC 0011 G5): the native Vulkan compute
//          foundation (materialsystem/shaderapivulkan/vulkan_compute.h) on a
//          real device, headless (no window or surface), so the same program
//          runs on the Linux GPU runner and on an Android device from
//          /data/local/tmp.
//
//  - Capabilities come from queries: the device is created through the
//    VkPhysicalDeviceFeatures2 chain, and every capability it reports
//    enabled is supported by the device's own feature query.
//  - A compute program writes a storage buffer and an R32F storage image on
//    the graphics queue; both read back exactly.
//  - Retirement: a resource retired behind a submission stays alive until
//    that submission completes, then is destroyed; the seeded early-free
//    defect is detected by the same check.
//  - With the validation layer present, no message is reported.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/material_spv.h"
#include "../../materialsystem/shaderapivulkan/vulkan_compute.h"
#include "headless_vulkan.h"
#include "render/render_profile.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace
{
using namespace render_vulkan;
using namespace headless_vulkan;

unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool condition, const std::string &description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description.c_str() );
	}
}

void Capabilities( Device &d )
{
	const ComputeCaps &enabled = d.chain.Enabled();
	std::printf( "device %s api %u.%u: compute %d storage images %d ray query %d timeline %d\n",
	    d.name.c_str(), VK_API_VERSION_MAJOR( d.queried.deviceApiVersion ),
	    VK_API_VERSION_MINOR( d.queried.deviceApiVersion ), enabled.compute,
	    enabled.storageImages, enabled.rayQuery, enabled.timelineSemaphore );
	Check( enabled.compute, "the graphics queue family computes (every Vulkan graphics device)" );
	Check( enabled.compute == d.queried.compute && enabled.storageImages == d.queried.storageImages &&
	           enabled.rayQuery == d.queried.rayQuery &&
	           enabled.timelineSemaphore == d.queried.timelineSemaphore,
	    "the chain enables exactly what the queries report" );
	// Truthfulness: what the chain enabled, the device's own queries support.
	if ( enabled.rayQuery )
	{
		VkPhysicalDeviceRayQueryFeaturesKHR rayQuery = {};
		rayQuery.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
		VkPhysicalDeviceFeatures2 features = {};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = &rayQuery;
		vkGetPhysicalDeviceFeatures2( d.physical, &features );
		Check( rayQuery.rayQuery == VK_TRUE, "an enabled ray query is supported by the device" );
	}
	// The provider's claim for this device passes render.profile's check; a
	// provider claiming the queried ray query while not enabling it fails it.
	render::RenderAdapterInfo adapter;
	adapter.supportedFeatures.bits = ComputeFeatureBits( enabled );
	render::RenderFeatureSet enabledSet;
	enabledSet.bits = ComputeFeatureBits( enabled );
	render::RenderProfileError error;
	Check( render::ValidateDeviceClaims( adapter, enabledSet, &error ),
	    "the claim from the enabled chain is valid" );
	if ( d.queried.rayQuery )
	{
		ComputeCaps withoutRayQuery = enabled;
		withoutRayQuery.rayQuery = false;
		render::RenderAdapterInfo bad;
		bad.supportedFeatures.bits = ComputeFeatureBits( d.queried );
		render::RenderFeatureSet badEnabled;
		badEnabled.bits = ComputeFeatureBits( withoutRayQuery );
		Check( !render::ValidateDeviceClaims( bad, badEnabled, &error ) &&
		           error.feature == render::RenderFeature::kRayQuery,
		    "a provider claiming ray query it did not enable is rejected" );
	}
	if ( enabled.storageImages )
	{
		VkFormatProperties properties = {};
		vkGetPhysicalDeviceFormatProperties( d.physical, VK_FORMAT_R32_SFLOAT, &properties );
		Check( properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
		    "enabled storage images are supported for R32F" );
	}
}

void Dispatch( Device &d, ComputeResources &compute )
{
	std::string error;
	const uint32_t width = 64;
	const uint32_t values = compute.CreateBuffer( width * width * 4, &error );
	const uint32_t image = compute.CreateStorageImage( width, width, VK_FORMAT_R32_SFLOAT, &error );
	const uint32_t readback = compute.CreateBuffer( width * width * 4, &error );
	const uint32_t program = compute.CreateProgram( g_computeFillSpv, sizeof( g_computeFillSpv ),
	    { ComputeBinding::StorageBuffer, ComputeBinding::StorageImage }, 8, &error );
	Check( values && image && readback && program, "buffers, a storage image and a program: " + error );
	if ( !values || !image || !readback || !program )
		return;
	const uint32_t push[2] = { width, 7 };
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	bool recorded = false;
	VkFence fence = Submit( d, &cmd,
	    [&]( VkCommandBuffer c )
	    {
		    compute.RecordImageLayout( c, image );
		    recorded = compute.RecordDispatch(
		        c, 1, program, { values, image }, push, 8, width / 8, width / 8, 1 );
		    VkBufferImageCopy copy = {};
		    copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		    copy.imageExtent = { width, width, 1 };
		    // The dispatch's barrier covers the transfer read; the image stays GENERAL.
		    vkCmdCopyImageToBuffer( c, compute.Image( image ), VK_IMAGE_LAYOUT_GENERAL,
		        compute.BufferHandle( readback ), 1, &copy );
		    VkMemoryBarrier host = {};
		    host.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		    host.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		    host.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
		    vkCmdPipelineBarrier( c, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
		        1, &host, 0, nullptr, 0, nullptr );
	    } );
	Finish( d, fence, cmd );
	compute.Collect( 1 );
	Check( recorded, "the dispatch records on the graphics queue" );
	const uint32_t *written = static_cast<const uint32_t *>( compute.Map( values ) );
	const float *pattern = static_cast<const float *>( compute.Map( readback ) );
	bool buffer = true, texels = true;
	for ( uint32_t y = 0; y < width; ++y )
		for ( uint32_t x = 0; x < width; ++x )
		{
			const uint32_t index = y * width + x;
			buffer = buffer && written[index] == index * 2 + 7;
			texels = texels && std::fabs( pattern[index] - ( float( x ) + float( y ) * 0.5f + 7.0f ) ) < 1e-5f;
		}
	Check( buffer, "the storage buffer holds every invocation's value" );
	Check( texels, "the storage image holds every invocation's pattern" );
	for ( uint32_t resource : { values, image, readback, program } )
		compute.Retire( resource, 1 );
	compute.Collect( 1 );
}

// A resource retired behind a pending submission outlives it; returns
// whether the rule held. With `gpuUse` the pending submission really reads
// the resource; the defect run passes false, so the seeded early free is
// caught by the same check without the GPU touching freed memory.
bool RetirementHolds( Device &d, ComputeResources &compute, uint64_t serial, bool gpuUse )
{
	std::string error;
	const uint32_t buffer = compute.CreateBuffer( 1 << 20, &error );
	const uint32_t program = compute.CreateProgram( g_computeFillSpv, sizeof( g_computeFillSpv ),
	    { ComputeBinding::StorageBuffer, ComputeBinding::StorageImage }, 8, &error );
	const uint32_t image = compute.CreateStorageImage( 256, 256, VK_FORMAT_R32_SFLOAT, &error );
	const uint32_t push[2] = { 256, 1 };
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkFence fence = Submit( d, &cmd,
	    [&]( VkCommandBuffer c )
	    {
		    compute.RecordImageLayout( c, image );
		    if ( gpuUse )
			    compute.RecordDispatch( c, serial, program, { buffer, image }, push, 8, 32, 32, 1 );
	    } );
	// Retired while the submission may still run; nothing completed yet.
	compute.Retire( buffer, serial );
	compute.Collect( serial - 1 );
	const bool heldWhilePending = compute.Alive( buffer );
	Finish( d, fence, cmd );
	compute.Collect( serial );
	const bool freedAfter = !compute.Alive( buffer );
	compute.Retire( image, serial );
	compute.Retire( program, serial );
	compute.Collect( serial );
	return heldWhilePending && freedAfter;
}

} // namespace

int main()
{
	Device d;
	if ( !CreateDevice( &d ) )
	{
		std::fprintf( stderr, "SKIP: no Vulkan device\n" );
		return 77;
	}
	Capabilities( d );
	ComputeResources compute;
	std::string error;
	Check( compute.Init( d.physical, d.device, d.chain.Enabled(), &error ), "compute init: " + error );
	Dispatch( d, compute );
	Check( RetirementHolds( d, compute, 2, true ),
	    "a retired resource lives until its submission completes, then is freed" );
	compute.SetEarlyFreeDefect( true );
	const bool defectHeld = RetirementHolds( d, compute, 3, false );
	Check( !defectHeld, "the seeded early-free defect is detected by the retirement check" );
	compute.SetEarlyFreeDefect( false );
	vkDeviceWaitIdle( d.device );
	compute.Shutdown();
	DestroyDevice( d );
	std::printf( "validation layer %s, %lu message(s)\n", d.validation ? "on" : "absent",
	    g_validationMessages );
	if ( d.validation )
		Check( g_validationMessages == 0, "no validation messages" );
	return testing::ReportConformance( g_checks, g_failures );
}
