//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Descriptor sets grouped by update frequency (see the header).
//
//===========================================================================//

#include "vulkan_descriptor_groups.h"

#include <functional>

namespace render_vulkan
{
namespace
{

// Sets per pool; a slot adds pools as a frame needs them.
constexpr uint32_t kSetsPerPool = 256;

void SetError( std::string *outError, const char *message )
{
	if ( outError )
		*outError = message;
}

} // namespace

size_t CGroupedDescriptors::KeyHash::operator()( const Key &key ) const
{
	size_t hash = std::hash<uint32_t>()( key.group );
	for ( const Image &image : key.images )
	{
		hash = hash * 1099511628211ull ^
		       std::hash<uint64_t>()( reinterpret_cast<uint64_t>( image.view ) );
		hash = hash * 1099511628211ull ^
		       std::hash<uint64_t>()( reinterpret_cast<uint64_t>( image.sampler ) );
	}
	return hash;
}

bool CGroupedDescriptors::Init( VkDevice device, uint32_t slots, std::string *outError )
{
	Shutdown();
	if ( device == VK_NULL_HANDLE || slots == 0 )
	{
		SetError( outError, "grouped descriptors need a device and a frame slot" );
		return false;
	}
	m_device = device;
	const uint32_t counts[kGroupCount] = { kFrameBindings, kMaterialBindings };
	for ( int group = 0; group < kGroupCount; ++group )
	{
		VkDescriptorSetLayoutBinding bindings[kMaxBindings] = {};
		for ( uint32_t i = 0; i < counts[group]; ++i )
		{
			bindings[i].binding = i;
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[i].descriptorCount = 1;
			bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = counts[group];
		info.pBindings = bindings;
		if ( vkCreateDescriptorSetLayout( m_device, &info, nullptr, &m_layouts[group] ) !=
		     VK_SUCCESS )
		{
			SetError( outError, "vkCreateDescriptorSetLayout (grouped descriptors) failed" );
			Shutdown();
			return false;
		}
	}
	m_slots.resize( slots );
	for ( Slot &slot : m_slots )
	{
		const VkDescriptorPool pool = NewPool();
		if ( pool == VK_NULL_HANDLE )
		{
			SetError( outError, "vkCreateDescriptorPool (grouped descriptors) failed" );
			Shutdown();
			return false;
		}
		slot.pools.push_back( pool );
	}
	m_slot = 0;
	return true;
}

void CGroupedDescriptors::Shutdown()
{
	if ( m_device == VK_NULL_HANDLE )
		return;
	for ( Slot &slot : m_slots )
		for ( VkDescriptorPool pool : slot.pools )
			vkDestroyDescriptorPool( m_device, pool, nullptr );
	m_slots.clear();
	for ( VkDescriptorSetLayout &layout : m_layouts )
	{
		if ( layout != VK_NULL_HANDLE )
			vkDestroyDescriptorSetLayout( m_device, layout, nullptr );
		layout = VK_NULL_HANDLE;
	}
	m_device = VK_NULL_HANDLE;
}

VkDescriptorPool CGroupedDescriptors::NewPool()
{
	VkDescriptorPoolSize size = {};
	size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	size.descriptorCount = kSetsPerPool * kMaxBindings;
	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.maxSets = kSetsPerPool;
	info.poolSizeCount = 1;
	info.pPoolSizes = &size;
	VkDescriptorPool pool = VK_NULL_HANDLE;
	if ( vkCreateDescriptorPool( m_device, &info, nullptr, &pool ) != VK_SUCCESS )
		return VK_NULL_HANDLE;
	return pool;
}

void CGroupedDescriptors::BeginFrame( uint32_t slot )
{
	if ( m_device == VK_NULL_HANDLE || m_slots.empty() )
		return;
	m_slot = slot % static_cast<uint32_t>( m_slots.size() );
	Slot &current = m_slots[m_slot];
	for ( VkDescriptorPool pool : current.pools )
		vkResetDescriptorPool( m_device, pool, 0 );
	current.current = 0;
	current.reuse.clear();
}

void CGroupedDescriptors::Invalidate()
{
	if ( !m_slots.empty() )
		m_slots[m_slot].reuse.clear();
}

VkDescriptorSet CGroupedDescriptors::Allocate( Slot &slot, Group group )
{
	for ( ;; )
	{
		if ( slot.current == slot.pools.size() )
		{
			const VkDescriptorPool pool = NewPool();
			if ( pool == VK_NULL_HANDLE )
				return VK_NULL_HANDLE;
			slot.pools.push_back( pool );
		}
		VkDescriptorSetAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.descriptorPool = slot.pools[slot.current];
		info.descriptorSetCount = 1;
		info.pSetLayouts = &m_layouts[group];
		VkDescriptorSet set = VK_NULL_HANDLE;
		const VkResult result = vkAllocateDescriptorSets( m_device, &info, &set );
		if ( result == VK_SUCCESS )
			return set;
		if ( result != VK_ERROR_OUT_OF_POOL_MEMORY && result != VK_ERROR_FRAGMENTED_POOL )
			return VK_NULL_HANDLE;
		++slot.current; // this pool is full for the rest of the frame
	}
}

VkDescriptorSet CGroupedDescriptors::Acquire( Group group, const Image *images )
{
	if ( m_device == VK_NULL_HANDLE || m_slots.empty() || group < 0 || group >= kGroupCount ||
	     !images )
		return VK_NULL_HANDLE;
	const uint32_t count = group == kFrameGroup ? kFrameBindings : kMaterialBindings;
	Key key;
	key.group = static_cast<uint32_t>( group );
	for ( uint32_t i = 0; i < count; ++i )
	{
		if ( images[i].view == VK_NULL_HANDLE || images[i].sampler == VK_NULL_HANDLE )
			return VK_NULL_HANDLE;
		key.images[i] = images[i];
		if ( m_samplerKeyDefect )
			key.images[i].sampler = VK_NULL_HANDLE;
	}
	Slot &slot = m_slots[m_slot];
	const auto existing = slot.reuse.find( key );
	if ( existing != slot.reuse.end() )
	{
		++m_reused;
		return existing->second;
	}
	const VkDescriptorSet set = Allocate( slot, group );
	if ( set == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkDescriptorImageInfo infos[kMaxBindings] = {};
	VkWriteDescriptorSet writes[kMaxBindings] = {};
	for ( uint32_t i = 0; i < count; ++i )
	{
		infos[i].sampler = images[i].sampler;
		infos[i].imageView = images[i].view;
		infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = set;
		writes[i].dstBinding = i;
		writes[i].descriptorCount = 1;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[i].pImageInfo = &infos[i];
	}
	vkUpdateDescriptorSets( m_device, count, writes, 0, nullptr );
	++m_written;
	slot.reuse.emplace( key, set );
	return set;
}

} // namespace render_vulkan
