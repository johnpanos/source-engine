//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VK_EXT_debug_utils object names and command-buffer labels
//          (vulkan_debug_utils.h).
//
//===========================================================================//

#include "vulkan_debug_utils.h"

namespace render_vulkan
{

bool DebugLabelsWanted( DebugLabelPolicy policy, bool validationEnabled, bool toolWantsMarkers )
{
	switch ( policy )
	{
	case DebugLabelPolicy::On:
		return true;
	case DebugLabelPolicy::Off:
		return false;
	case DebugLabelPolicy::Auto:
		break;
	}
	return validationEnabled || toolWantsMarkers;
}

bool VulkanDebugUtils::Load( VkInstance instance, VkDevice device )
{
	EntryPoints entry;
	entry.setObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
	    vkGetInstanceProcAddr( instance, "vkSetDebugUtilsObjectNameEXT" ) );
	entry.beginLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
	    vkGetInstanceProcAddr( instance, "vkCmdBeginDebugUtilsLabelEXT" ) );
	entry.endLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
	    vkGetInstanceProcAddr( instance, "vkCmdEndDebugUtilsLabelEXT" ) );
	entry.insertLabel = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(
	    vkGetInstanceProcAddr( instance, "vkCmdInsertDebugUtilsLabelEXT" ) );
	return Load( device, entry );
}

bool VulkanDebugUtils::Load( VkDevice device, const EntryPoints &entryPoints )
{
	Reset();
	if ( device == VK_NULL_HANDLE || !entryPoints.setObjectName || !entryPoints.beginLabel ||
	     !entryPoints.endLabel || !entryPoints.insertLabel )
		return false;
	m_device = device;
	m_entry = entryPoints;
	return true;
}

void VulkanDebugUtils::Reset()
{
	m_device = VK_NULL_HANDLE;
	m_entry = EntryPoints();
	m_frame = VK_NULL_HANDLE;
	m_open = m_unbalancedPops = m_closedAtFrameEnd = 0;
}

void VulkanDebugUtils::Name( VkObjectType type, uint64_t handle, const char *name ) const
{
	if ( !Active() || handle == 0 || !name )
		return;
	VkDebugUtilsObjectNameInfoEXT info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectType = type;
	info.objectHandle = handle;
	info.pObjectName = name;
	m_entry.setObjectName( m_device, &info );
}

void VulkanDebugUtils::Emit( VkCommandBuffer cmd, const char *name, const float color[4] ) const
{
	VkDebugUtilsLabelEXT info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	info.pLabelName = name ? name : "";
	if ( color )
		for ( int i = 0; i < 4; ++i )
			info.color[i] = color[i];
	m_entry.beginLabel( cmd, &info );
}

void VulkanDebugUtils::BeginFrameCommands( VkCommandBuffer cmd )
{
	if ( !Active() )
		return;
	m_frame = cmd;
	m_open = 0;
}

void VulkanDebugUtils::EndFrameCommands()
{
	if ( !Active() || m_frame == VK_NULL_HANDLE )
		return;
	m_closedAtFrameEnd += m_open;
	for ( ; m_open > 0; --m_open )
		m_entry.endLabel( m_frame );
	m_frame = VK_NULL_HANDLE;
}

void VulkanDebugUtils::PushLabel( const char *name, const float color[4] )
{
	if ( !Active() || m_frame == VK_NULL_HANDLE )
		return;
	Emit( m_frame, name, color );
	++m_open;
}

void VulkanDebugUtils::PopLabel()
{
	if ( !Active() || m_frame == VK_NULL_HANDLE )
		return;
	if ( m_open == 0 )
	{
		++m_unbalancedPops;
		return;
	}
	--m_open;
	m_entry.endLabel( m_frame );
}

void VulkanDebugUtils::InsertLabel( const char *name ) const
{
	if ( !Active() || m_frame == VK_NULL_HANDLE || !name )
		return;
	VkDebugUtilsLabelEXT info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	info.pLabelName = name;
	m_entry.insertLabel( m_frame, &info );
}

void VulkanDebugUtils::BeginLabel( VkCommandBuffer cmd, const char *name ) const
{
	if ( !Active() || cmd == VK_NULL_HANDLE || !name )
		return;
	Emit( cmd, name, nullptr );
}

void VulkanDebugUtils::EndLabel( VkCommandBuffer cmd ) const
{
	if ( !Active() || cmd == VK_NULL_HANDLE )
		return;
	m_entry.endLabel( cmd );
}

} // namespace render_vulkan
