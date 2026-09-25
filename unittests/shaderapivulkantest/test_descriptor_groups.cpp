//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.grouped-descriptors: the PBR and GI stages' descriptor sets
//          grouped by update frequency
//          (materialsystem/shaderapivulkan/vulkan_descriptor_groups.h) on a
//          real device, headless.
//
//  - Equal requests in one frame slot share a set; a different view, sampler
//    or group gets its own; an invalid image gets none.
//  - Slots are independent, and a slot's sets return to its pools only at its
//    BeginFrame (after its submission completed, the owner's rule).
//  - A frame that needs more sets than one pool holds grows the slot's pools.
//  - Invalidate forgets the reuse table (a sampler replaced in place).
//  - The seeded defect (reuse keyed by views alone) is caught by the sampler
//    check.
//  - With the validation layer present, no message is reported.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/vulkan_descriptor_groups.h"
#include "headless_vulkan.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <set>
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

struct Images
{
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	std::vector<VkImageView> views;
	std::vector<VkSampler> samplers;
};

bool CreateImages( Device &d, Images *out, uint32_t views, uint32_t samplers )
{
	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = VK_FORMAT_R8G8B8A8_UNORM;
	image.extent = { 4, 4, 1 };
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	if ( vkCreateImage( d.device, &image, nullptr, &out->image ) != VK_SUCCESS )
		return false;
	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements( d.device, out->image, &requirements );
	VkPhysicalDeviceMemoryProperties memory = {};
	vkGetPhysicalDeviceMemoryProperties( d.physical, &memory );
	VkMemoryAllocateInfo allocate = {};
	allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate.allocationSize = requirements.size;
	allocate.memoryTypeIndex = UINT32_MAX;
	for ( uint32_t i = 0; i < memory.memoryTypeCount && allocate.memoryTypeIndex == UINT32_MAX;
	    ++i )
		if ( requirements.memoryTypeBits & ( 1u << i ) )
			allocate.memoryTypeIndex = i;
	if ( allocate.memoryTypeIndex == UINT32_MAX ||
	     vkAllocateMemory( d.device, &allocate, nullptr, &out->memory ) != VK_SUCCESS ||
	     vkBindImageMemory( d.device, out->image, out->memory, 0 ) != VK_SUCCESS )
		return false;
	for ( uint32_t i = 0; i < views; ++i )
	{
		VkImageViewCreateInfo view = {};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.image = out->image;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = VK_FORMAT_R8G8B8A8_UNORM;
		view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		VkImageView handle = VK_NULL_HANDLE;
		if ( vkCreateImageView( d.device, &view, nullptr, &handle ) != VK_SUCCESS )
			return false;
		out->views.push_back( handle );
	}
	for ( uint32_t i = 0; i < samplers; ++i )
	{
		VkSamplerCreateInfo sampler = {};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter = i % 2 ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		sampler.minFilter = sampler.magFilter;
		sampler.maxLod = static_cast<float>( i );
		VkSampler handle = VK_NULL_HANDLE;
		if ( vkCreateSampler( d.device, &sampler, nullptr, &handle ) != VK_SUCCESS )
			return false;
		out->samplers.push_back( handle );
	}
	return true;
}

void DestroyImages( Device &d, Images &images )
{
	for ( VkSampler sampler : images.samplers )
		vkDestroySampler( d.device, sampler, nullptr );
	for ( VkImageView view : images.views )
		vkDestroyImageView( d.device, view, nullptr );
	if ( images.image != VK_NULL_HANDLE )
		vkDestroyImage( d.device, images.image, nullptr );
	if ( images.memory != VK_NULL_HANDLE )
		vkFreeMemory( d.device, images.memory, nullptr );
}

// A request of every binding on view `view` and sampler `sampler`.
std::vector<CGroupedDescriptors::Image> Request( const Images &images, size_t view, size_t sampler )
{
	CGroupedDescriptors::Image image;
	image.view = images.views[view];
	image.sampler = images.samplers[sampler];
	return std::vector<CGroupedDescriptors::Image>( CGroupedDescriptors::kMaxBindings, image );
}

// Whether requests that differ only in their sampler get different sets.
bool SamplersDistinguished( CGroupedDescriptors &groups, const Images &images )
{
	groups.BeginFrame( 0 );
	const VkDescriptorSet nearest =
	    groups.Acquire( CGroupedDescriptors::kMaterialGroup, Request( images, 0, 0 ).data() );
	const VkDescriptorSet linear =
	    groups.Acquire( CGroupedDescriptors::kMaterialGroup, Request( images, 0, 1 ).data() );
	return nearest != VK_NULL_HANDLE && linear != VK_NULL_HANDLE && nearest != linear;
}

void Contract( Device &d, const Images &images )
{
	CGroupedDescriptors groups;
	std::string error;
	Check( groups.Init( d.device, 2, &error ), "grouped descriptors initialize: " + error );
	if ( !groups.Ready() )
		return;
	Check( groups.Layout( CGroupedDescriptors::kFrameGroup ) != VK_NULL_HANDLE &&
	           groups.Layout( CGroupedDescriptors::kMaterialGroup ) != VK_NULL_HANDLE,
	    "the frame and material set layouts exist" );

	groups.BeginFrame( 0 );
	const auto frame = [&]( size_t view, size_t sampler )
	{
		return groups.Acquire(
		    CGroupedDescriptors::kFrameGroup, Request( images, view, sampler ).data() );
	};
	const VkDescriptorSet a = frame( 0, 0 );
	const VkDescriptorSet again = frame( 0, 0 );
	Check(
	    a != VK_NULL_HANDLE && a == again && groups.SetsWritten() == 1 && groups.SetsReused() == 1,
	    "an equal request in the same slot reuses the written set" );
	const VkDescriptorSet otherView = frame( 1, 0 );
	Check( otherView != VK_NULL_HANDLE && otherView != a, "another view gets its own set" );
	const VkDescriptorSet material =
	    groups.Acquire( CGroupedDescriptors::kMaterialGroup, Request( images, 0, 0 ).data() );
	Check( material != VK_NULL_HANDLE && material != a, "another group gets its own set" );
	std::vector<CGroupedDescriptors::Image> invalid = Request( images, 0, 0 );
	invalid[2].view = VK_NULL_HANDLE;
	Check( groups.Acquire( CGroupedDescriptors::kFrameGroup, invalid.data() ) == VK_NULL_HANDLE,
	    "a request with a missing view gets no set" );

	// Slot 1 is independent; slot 0's sets stay until its next BeginFrame.
	const uint64_t writtenBefore = groups.SetsWritten();
	groups.BeginFrame( 1 );
	const VkDescriptorSet slotOne = frame( 0, 0 );
	Check( slotOne != VK_NULL_HANDLE && slotOne != a && groups.SetsWritten() == writtenBefore + 1,
	    "another slot writes its own set" );
	groups.BeginFrame( 0 );
	const VkDescriptorSet recycled = frame( 0, 0 );
	Check( recycled != VK_NULL_HANDLE && groups.SetsWritten() == writtenBefore + 2,
	    "a slot's sets are rewritten after its BeginFrame" );

	// A frame larger than one pool: every set distinct and valid.
	groups.BeginFrame( 1 );
	std::set<VkDescriptorSet> distinct;
	bool allValid = true;
	for ( size_t view = 0; view < images.views.size(); ++view )
		for ( size_t sampler = 0; sampler < images.samplers.size(); ++sampler )
		{
			const VkDescriptorSet set = frame( view, sampler );
			allValid = allValid && set != VK_NULL_HANDLE;
			distinct.insert( set );
		}
	const size_t requested = images.views.size() * images.samplers.size();
	std::printf( "%zu distinct requests in one slot: %zu sets\n", requested, distinct.size() );
	Check( allValid && distinct.size() == requested,
	    "a frame beyond one pool's sets grows the slot's pools" );

	groups.BeginFrame( 0 );
	const VkDescriptorSet before = frame( 0, 0 );
	const uint64_t written = groups.SetsWritten();
	groups.Invalidate();
	const VkDescriptorSet after = frame( 0, 0 );
	Check( before != VK_NULL_HANDLE && after != VK_NULL_HANDLE && after != before &&
	           groups.SetsWritten() == written + 1,
	    "Invalidate forgets the slot's reuse table" );

	Check( SamplersDistinguished( groups, images ), "another sampler gets its own set" );
	groups.SetSamplerKeyDefect( true );
	Check( !SamplersDistinguished( groups, images ),
	    "the seeded view-only reuse key is caught by the sampler check" );
	groups.SetSamplerKeyDefect( false );
	groups.Shutdown();
	Check( !groups.Ready(), "Shutdown releases the pools and layouts" );
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
	std::printf( "device %s\n", d.name.c_str() );
	Images images;
	// 40 views x 16 samplers = 640 distinct requests, over two pools' worth.
	const bool created = CreateImages( d, &images, 40, 16 );
	Check( created, "test images, views and samplers are created" );
	if ( created )
		Contract( d, images );
	vkDeviceWaitIdle( d.device );
	DestroyImages( d, images );
	DestroyDevice( d );
	std::printf( "validation layer %s, %lu message(s)\n", d.validation ? "on" : "absent",
	    g_validationMessages );
	if ( d.validation )
		Check( g_validationMessages == 0, "no validation messages" );
	return testing::ReportConformance( g_checks, g_failures );
}
