//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VK_EXT_debug_utils object names and command-buffer labels, read by
//          capture tools (RenderDoc) and validation messages. The one owner of
//          the extension's entry points and of whether names are emitted.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_DEBUG_UTILS_H
#define SHADERAPIVULKAN_VULKAN_DEBUG_UTILS_H

#include <vulkan/vulkan.h>

#include <cstdint>
#include <cstdio>

namespace render_vulkan
{

// When names and labels are emitted: Auto while the validation layer is on or
// a tool reports VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT (RenderDoc does), so a
// plain run pays nothing; On and Off are -vkdebuglabels and -novkdebuglabels.
enum class DebugLabelPolicy
{
	Auto,
	On,
	Off,
};

[[nodiscard]] bool DebugLabelsWanted(
    DebugLabelPolicy policy, bool validationEnabled, bool toolWantsMarkers );

// A Vulkan handle as the 64-bit value debug utils takes: non-dispatchable
// handles are pointers on 64-bit targets and integers on 32-bit ones.
template <typename Handle> uint64_t DebugHandle( Handle *handle )
{
	return static_cast<uint64_t>( reinterpret_cast<uintptr_t>( handle ) );
}
inline uint64_t DebugHandle( uint64_t handle )
{
	return handle;
}

// Inert (every call returns at once) until Load succeeds. Not thread-safe: it
// is used on the thread that records the owner's frame command buffer.
//
// Frame labels nest within one frame command buffer, between
// BeginFrameCommands and EndFrameCommands; regions still open at the end are
// closed there (and counted), so an unbalanced caller cannot leave a command
// buffer invalid. Pushes and pops outside a frame's recording are ignored.
class VulkanDebugUtils
{
public:
	struct EntryPoints
	{
		PFN_vkSetDebugUtilsObjectNameEXT setObjectName = nullptr;
		PFN_vkCmdBeginDebugUtilsLabelEXT beginLabel = nullptr;
		PFN_vkCmdEndDebugUtilsLabelEXT endLabel = nullptr;
		PFN_vkCmdInsertDebugUtilsLabelEXT insertLabel = nullptr;
	};

	// The entry points of an instance created with VK_EXT_debug_utils; stays
	// inert when any is missing. The device outlives this object's use.
	bool Load( VkInstance instance, VkDevice device );
	bool Load( VkDevice device, const EntryPoints &entryPoints );
	void Reset();
	bool Active() const { return m_device != VK_NULL_HANDLE; }

	void Name( VkObjectType type, uint64_t handle, const char *name ) const;
	template <typename Handle> void Name( VkObjectType type, Handle handle, const char *name ) const
	{
		if ( Active() )
			Name( type, DebugHandle( handle ), name );
	}
	// printf-style (truncated to 255 characters); formats only when active.
	template <typename Handle, typename... Args>
	void NameF( VkObjectType type, Handle handle, const char *format, Args... args ) const
	{
		if ( !Active() )
			return;
		char name[256];
		std::snprintf( name, sizeof( name ), format, args... );
		Name( type, DebugHandle( handle ), name );
	}

	// The frame command buffer's recording bounds.
	void BeginFrameCommands( VkCommandBuffer cmd );
	void EndFrameCommands();
	VkCommandBuffer FrameCommands() const { return m_frame; }

	void PushLabel( const char *name, const float color[4] = nullptr );
	// A pop with no open region in this frame is ignored and counted.
	void PopLabel();
	void InsertLabel( const char *name ) const;
	uint32_t OpenLabels() const { return m_open; }
	uint32_t UnbalancedPops() const { return m_unbalancedPops; }
	uint32_t ClosedAtFrameEnd() const { return m_closedAtFrameEnd; }

	// A label around one explicitly recorded command buffer (an upload or
	// capture submission), outside the frame's stack.
	void BeginLabel( VkCommandBuffer cmd, const char *name ) const;
	void EndLabel( VkCommandBuffer cmd ) const;

private:
	void Emit( VkCommandBuffer cmd, const char *name, const float color[4] ) const;

	VkDevice m_device = VK_NULL_HANDLE;
	EntryPoints m_entry;
	VkCommandBuffer m_frame = VK_NULL_HANDLE;
	uint32_t m_open = 0;
	uint32_t m_unbalancedPops = 0;
	uint32_t m_closedAtFrameEnd = 0;
};

// A frame label for the lifetime of a scope.
class ScopedDebugLabel
{
public:
	ScopedDebugLabel( VulkanDebugUtils &debug, const char *name ) : m_debug( debug )
	{
		m_debug.PushLabel( name );
	}
	~ScopedDebugLabel() { m_debug.PopLabel(); }
	ScopedDebugLabel( const ScopedDebugLabel & ) = delete;
	ScopedDebugLabel &operator=( const ScopedDebugLabel & ) = delete;

private:
	VulkanDebugUtils &m_debug;
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_DEBUG_UTILS_H
