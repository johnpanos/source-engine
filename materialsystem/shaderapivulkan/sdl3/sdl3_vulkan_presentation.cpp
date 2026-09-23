//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDL3 + Vulkan presentation bridge. See sdl3_vulkan_presentation.h.
//
//===========================================================================//

#include "sdl3_vulkan_presentation.h"

#include "../vulkan_render_backend_native.h"
#include "../../../platform/sdl3/render_surface/sdl3_render_surfaces.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace render_vulkan
{

using namespace render;

namespace
{

const uint32_t kFrameSlots = 2;
const uint64_t kAcquireTimeoutNs = 5ull * 1000 * 1000 * 1000;

void WaitToken( IRenderDevice &device, IRenderCompletionToken *token )
{
	// Completion is observed in submission order; a provider's PollCompletion
	// waits for the oldest outstanding submission.
	while ( token && !token->IsComplete() )
	{
		if ( device.PollCompletion() == 0 )
			break;
	}
}

IRenderCompletionToken *OrderingToken( IRenderDevice &device )
{
	// An empty submission is ordered after every earlier submission, so its
	// token completing proves the GPU finished all earlier use.
	IRenderCommandContext *context = device.CreateCommandContext();
	return context ? device.Submit( *context ) : nullptr;
}

VkFormat BackBufferFormat( RenderColorFormat format )
{
	switch ( format )
	{
	case RenderColorFormat::kRGBA8Srgb:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case RenderColorFormat::kBGRA8Unorm:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case RenderColorFormat::kRGBA8Unorm:
	default:
		return VK_FORMAT_R8G8B8A8_UNORM;
	}
}

void ImageBarrier( VkCommandBuffer cmd, VkImage image, VkImageLayout from, VkImageLayout to,
    VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkPipelineStageFlags dstStage,
    VkAccessFlags dstAccess )
{
	VkImageMemoryBarrier b = {};
	b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	b.srcAccessMask = srcAccess;
	b.dstAccessMask = dstAccess;
	b.oldLayout = from;
	b.newLayout = to;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.image = image;
	b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	b.subresourceRange.levelCount = 1;
	b.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier( cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &b );
}

bool FindMemoryType( VkPhysicalDevice phys, uint32_t bits, VkMemoryPropertyFlags props,
    uint32_t *outType )
{
	VkPhysicalDeviceMemoryProperties mem = {};
	vkGetPhysicalDeviceMemoryProperties( phys, &mem );
	for ( uint32_t i = 0; i < mem.memoryTypeCount; ++i )
	{
		if ( ( bits & ( 1u << i ) ) && ( mem.memoryTypes[i].propertyFlags & props ) == props )
		{
			*outType = i;
			return true;
		}
	}
	return false;
}

} // namespace

// Native objects retired from a presentation, released once 'tokens' complete.
// While it holds a VkSurfaceKHR it stays attached to the window's surface, so
// window destruction releases it (waiting for the GPU) before the window goes.
struct Sdl3VulkanRetiredNative final : public IRenderSurfaceListener
{
	IRenderDevice *device = nullptr;
	VulkanDeviceEndpoint *endpoint = nullptr;
	IRenderSurface *surface = nullptr; // set while attached as the surface listener
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	std::vector<VkSwapchainKHR> swapchains;
	std::vector<VkSemaphore> semaphores;
	std::vector<IRenderCompletionToken *> tokens;
	bool released = false;

	bool Complete() const
	{
		for ( IRenderCompletionToken *t : tokens )
			if ( t && !t->IsComplete() )
				return false;
		return true;
	}

	void Release( bool wait )
	{
		if ( released )
			return;
		if ( wait )
			for ( IRenderCompletionToken *t : tokens )
				WaitToken( *device, t );
		if ( !Complete() )
			return;
		VkDevice vkDevice = endpoint->Device();
		for ( VkSwapchainKHR s : swapchains )
			vkDestroySwapchainKHR( vkDevice, s, nullptr );
		for ( VkSemaphore s : semaphores )
			vkDestroySemaphore( vkDevice, s, nullptr );
		if ( vkSurface != VK_NULL_HANDLE )
			SDL_Vulkan_DestroySurface( endpoint->Instance(), vkSurface, nullptr );
		vkSurface = VK_NULL_HANDLE;
		swapchains.clear();
		semaphores.clear();
		if ( surface )
			surface->DetachListener( *this );
		surface = nullptr;
		released = true;
	}

	void OnNativeSurfaceReleasing() override { Release( true ); }
};

class Sdl3VulkanPresentation : public IRenderPresentation, public IRenderSurfaceListener
{
public:
	Sdl3VulkanPresentation( platform_sdl3::Sdl3RenderSurfaces &surfaces, IRenderDevice &device,
	    VulkanDeviceEndpoint &endpoint, IRenderSurface &surface,
	    const RenderPresentationConfig &config, RenderExtent extent )
	    : m_Surfaces( surfaces ), m_Device( device ), m_Ep( endpoint ),
	      m_Surface( surface ), m_Config( config ), m_Extent( extent )
	{
		VkSemaphoreCreateInfo sci = {};
		sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for ( uint32_t i = 0; i < kFrameSlots; ++i )
			vkCreateSemaphore( m_Ep.Device(), &sci, nullptr, &m_ImageAvailable[i] );
	}

	~Sdl3VulkanPresentation() override
	{
		for ( uint32_t i = 0; i < kFrameSlots; ++i )
			if ( m_ImageAvailable[i] != VK_NULL_HANDLE )
				vkDestroySemaphore( m_Ep.Device(), m_ImageAvailable[i], nullptr );
		if ( m_CaptureBuffer != VK_NULL_HANDLE )
			vkDestroyBuffer( m_Ep.Device(), m_CaptureBuffer, nullptr );
		if ( m_CaptureMemory != VK_NULL_HANDLE )
			vkFreeMemory( m_Ep.Device(), m_CaptureMemory, nullptr );
	}

	IRenderDevice &Device() { return m_Device; }
	IRenderSurface &Surface() { return m_Surface; }
	bool HasNativeSurface() const { return m_VkSurface != VK_NULL_HANDLE; }

	// Creates the VkSurfaceKHR and checks the device's queue can present to it.
	bool CreateNativeSurface( RenderCreateError *error )
	{
		SDL_Window *window = m_Surfaces.NativeWindow( m_Surface );
		if ( !window )
			return true; // unavailable now; built when the platform provides it
		const uint64_t generation = m_Surface.GetGeneration();
		if ( !SDL_Vulkan_CreateSurface( window, m_Ep.Instance(), nullptr, &m_VkSurface ) )
		{
			Fail( error, RenderCreateStatus::kSurfaceIncompatible, SDL_GetError() );
			m_VkSurface = VK_NULL_HANDLE;
			return false;
		}
		VkBool32 supported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
		    m_Ep.PhysicalDevice(), m_Ep.QueueFamily(), m_VkSurface, &supported );
		if ( !supported || !m_Ep.SwapchainEnabled() )
		{
			Fail( error, RenderCreateStatus::kSurfaceIncompatible,
			    "the device's queue cannot present to this window" );
			SDL_Vulkan_DestroySurface( m_Ep.Instance(), m_VkSurface, nullptr );
			m_VkSurface = VK_NULL_HANDLE;
			return false;
		}
		m_SurfaceGeneration = generation;
		return true;
	}

	// -- IRenderPresentation --
	RenderExtent GetExtent() const override { return m_Extent; }

	bool ResizeTo( RenderExtent extent ) override
	{
		if ( m_FrameOpen )
			return false;
		CollectRetired();
		m_Extent = extent;
		if ( m_BackBuffer != kInvalidResource && m_BackBufferExtent != extent )
			RetireBackBuffer();
		return true;
	}

	RenderPresentStatus BeginFrame( RenderResourceHandle *outBackBuffer ) override
	{
		if ( outBackBuffer )
			*outBackBuffer = kInvalidResource;
		if ( m_FrameOpen )
			return RenderPresentStatus::kInvalidSequence;
		CollectRetired();

		const RenderDeviceState state = m_Device.GetState();
		if ( state == RenderDeviceState::kDeviceLost || state == RenderDeviceState::kFatal )
			return RenderPresentStatus::kLost;
		const RenderSurfaceStatus status = m_Surface.GetStatus();
		if ( status == RenderSurfaceStatus::kDestroyed )
			return RenderPresentStatus::kLost;
		if ( status == RenderSurfaceStatus::kUnavailable )
			return RenderPresentStatus::kSuspended;
		if ( !m_Extent.IsPresentable() )
			return RenderPresentStatus::kSuspended;
		const RenderExtent drawable = m_Surface.GetDrawableExtent();
		if ( !drawable.IsPresentable() )
			return RenderPresentStatus::kSuspended;

		// A replaced or lost native surface is rebuilt before anything presents.
		if ( m_VkSurface != VK_NULL_HANDLE &&
		     ( m_SurfaceLost || m_SurfaceGeneration != m_Surface.GetGeneration() ) )
			ReleaseNative();
		if ( m_VkSurface == VK_NULL_HANDLE && !CreateNativeSurface( nullptr ) )
			return RenderPresentStatus::kRecoverable;
		if ( m_VkSurface == VK_NULL_HANDLE )
			return RenderPresentStatus::kSuspended;
		m_SurfaceLost = false;

		if ( m_Swapchain == VK_NULL_HANDLE || m_SwapDirty || drawable != m_SwapDrawable )
		{
			if ( !RebuildSwapchain( drawable ) )
				return RenderPresentStatus::kRecoverable;
			if ( m_Swapchain == VK_NULL_HANDLE )
				return RenderPresentStatus::kSuspended;
		}
		if ( m_BackBuffer == kInvalidResource && !CreateBackBuffer() )
			return RenderPresentStatus::kRecoverable;

		// Reuse a frame slot only after its previous present completed on the GPU.
		WaitToken( m_Device, m_SlotToken[m_Slot] );
		m_SlotToken[m_Slot] = nullptr;

		uint32_t index = 0;
		const VkResult r = vkAcquireNextImageKHR( m_Ep.Device(), m_Swapchain, kAcquireTimeoutNs,
		    m_ImageAvailable[m_Slot], VK_NULL_HANDLE, &index );
		if ( r == VK_ERROR_OUT_OF_DATE_KHR )
		{
			m_SwapDirty = true;
			return RenderPresentStatus::kRecoverable;
		}
		if ( r == VK_ERROR_SURFACE_LOST_KHR )
		{
			m_SurfaceLost = true;
			return RenderPresentStatus::kRecoverable;
		}
		if ( r == VK_SUBOPTIMAL_KHR )
		{
			if ( SurfaceChangedSinceSwapchain() )
				m_SwapDirty = true;
		}
		else if ( r != VK_SUCCESS )
		{
			return RenderPresentStatus::kRecoverable;
		}
		m_ImageIndex = index;
		m_FrameOpen = true;
		if ( outBackBuffer )
			*outBackBuffer = m_BackBuffer;
		return RenderPresentStatus::kOk;
	}

	RenderPresentStatus Present() override
	{
		if ( !m_FrameOpen )
			return RenderPresentStatus::kInvalidSequence;
		m_FrameOpen = false;
		if ( m_Swapchain == VK_NULL_HANDLE )
		{
			// The window released its native surface during the frame; the
			// acquired image went with it.
			return m_Surface.GetStatus() == RenderSurfaceStatus::kDestroyed
			           ? RenderPresentStatus::kLost
			           : RenderPresentStatus::kRecoverable;
		}

		IRenderCommandContext *context = m_Device.CreateCommandContext();
		if ( !context )
			return RenderPresentStatus::kRecoverable;
		context->RecordUse( m_BackBuffer );
		VkCommandBuffer cmd = m_Ep.CommandBuffer( *context );
		const VkImage back = m_Ep.Image( m_BackBuffer );
		const VkImage swap = m_SwapImages[m_ImageIndex];

		// The caller's writes to the back buffer (earlier submissions) happen
		// before the blit reads it.
		ImageBarrier( cmd, back, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
		    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_MEMORY_WRITE_BIT,
		    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT );
		ImageBarrier( cmd, swap, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    VK_ACCESS_TRANSFER_WRITE_BIT );
		VkImageBlit blit = {};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.layerCount = 1;
		blit.srcOffsets[1] = { static_cast<int32_t>( m_BackBufferExtent.width ),
			static_cast<int32_t>( m_BackBufferExtent.height ), 1 };
		blit.dstSubresource = blit.srcSubresource;
		blit.dstOffsets[1] = { static_cast<int32_t>( m_SwapExtent.width ),
			static_cast<int32_t>( m_SwapExtent.height ), 1 };
		vkCmdBlitImage( cmd, back, VK_IMAGE_LAYOUT_GENERAL, swap,
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR );

		const bool capture = m_CaptureRequested && m_SwapCapturable && PrepareCaptureBuffer();
		if ( capture )
		{
			ImageBarrier( cmd, swap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
			    VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			    VK_ACCESS_TRANSFER_READ_BIT );
			VkBufferImageCopy copy = {};
			copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy.imageSubresource.layerCount = 1;
			copy.imageExtent = { m_SwapExtent.width, m_SwapExtent.height, 1 };
			vkCmdCopyImageToBuffer(
			    cmd, swap, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_CaptureBuffer, 1, &copy );
			VkBufferMemoryBarrier host = {};
			host.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			host.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			host.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
			host.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			host.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			host.buffer = m_CaptureBuffer;
			host.size = VK_WHOLE_SIZE;
			vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
			    0, nullptr, 1, &host, 0, nullptr );
			ImageBarrier( cmd, swap, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0 );
		}
		else
		{
			ImageBarrier( cmd, swap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT,
			    VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0 );
		}
		// Later writes to the back buffer wait for this read (write-after-read).
		ImageBarrier( cmd, back, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
		    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0 );

		IRenderCompletionToken *token = m_Ep.SubmitWithSemaphores( *context,
		    m_ImageAvailable[m_Slot], VK_PIPELINE_STAGE_TRANSFER_BIT,
		    m_RenderFinished[m_ImageIndex] );
		m_SlotToken[m_Slot] = token;
		if ( capture )
		{
			m_CaptureRequested = false;
			m_CaptureToken = token;
			m_CaptureExtent = m_SwapExtent;
		}
		m_Slot = ( m_Slot + 1 ) % kFrameSlots;

		VkPresentInfoKHR present = {};
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = &m_RenderFinished[m_ImageIndex];
		present.swapchainCount = 1;
		present.pSwapchains = &m_Swapchain;
		present.pImageIndices = &m_ImageIndex;
		const VkResult r = vkQueuePresentKHR( m_Ep.Queue(), &present );
		if ( r == VK_ERROR_OUT_OF_DATE_KHR )
		{
			m_SwapDirty = true;
			return RenderPresentStatus::kRecoverable;
		}
		if ( r == VK_ERROR_SURFACE_LOST_KHR )
		{
			m_SurfaceLost = true;
			return RenderPresentStatus::kRecoverable;
		}
		if ( r == VK_SUBOPTIMAL_KHR && SurfaceChangedSinceSwapchain() )
			m_SwapDirty = true;
		if ( r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR )
			return RenderPresentStatus::kRecoverable;
		return RenderPresentStatus::kOk;
	}

	size_t GetPendingRetirementCount() const override
	{
		return m_RetiringBackBuffers.size() + m_RetiredSwapchains.size();
	}

	void CollectRetired() override
	{
		std::vector<RenderResourceHandle> still;
		for ( RenderResourceHandle h : m_RetiringBackBuffers )
			if ( m_Device.IsResourceLive( h ) )
				still.push_back( h );
		m_RetiringBackBuffers.swap( still );

		std::vector<RetiredSwapchain> pending;
		for ( const RetiredSwapchain &r : m_RetiredSwapchains )
		{
			if ( r.token && !r.token->IsComplete() )
			{
				pending.push_back( r );
				continue;
			}
			vkDestroySwapchainKHR( m_Ep.Device(), r.swapchain, nullptr );
			for ( VkSemaphore s : r.semaphores )
				vkDestroySemaphore( m_Ep.Device(), s, nullptr );
		}
		m_RetiredSwapchains.swap( pending );
	}

	// -- IRenderSurfaceListener: the window is releasing its native surface --
	void OnNativeSurfaceReleasing() override { ReleaseNative(); }

	// Moves every native object and retired back buffer out of the
	// presentation for destruction, without waiting for the GPU.
	Sdl3VulkanRetiredNative *Retire()
	{
		ConsumeOpenFrame();
		if ( m_BackBuffer != kInvalidResource )
			RetireBackBuffer();
		Sdl3VulkanRetiredNative *retired = new Sdl3VulkanRetiredNative;
		retired->device = &m_Device;
		retired->endpoint = &m_Ep;
		retired->vkSurface = m_VkSurface;
		m_VkSurface = VK_NULL_HANDLE;
		IRenderCompletionToken *ordering = nullptr;
		if ( m_Swapchain != VK_NULL_HANDLE )
		{
			retired->swapchains.push_back( m_Swapchain );
			retired->semaphores = m_RenderFinished;
			m_Swapchain = VK_NULL_HANDLE;
			m_RenderFinished.clear();
			m_SwapImages.clear();
			ordering = OrderingToken( m_Device );
		}
		for ( const RetiredSwapchain &r : m_RetiredSwapchains )
		{
			retired->swapchains.push_back( r.swapchain );
			retired->semaphores.insert(
			    retired->semaphores.end(), r.semaphores.begin(), r.semaphores.end() );
			retired->tokens.push_back( r.token );
		}
		m_RetiredSwapchains.clear();
		retired->tokens.push_back( ordering );
		for ( uint32_t i = 0; i < kFrameSlots; ++i )
			retired->tokens.push_back( m_SlotToken[i] );
		retired->tokens.push_back( m_CaptureToken );
		return retired;
	}

	// Capture endpoint.
	bool RequestCapture()
	{
		m_CaptureRequested = true;
		return true;
	}

	bool ReadCapture( std::vector<uint8_t> *outRgba, uint32_t *outWidth, uint32_t *outHeight )
	{
		if ( !m_CaptureToken )
			return false;
		WaitToken( m_Device, m_CaptureToken );
		if ( !m_CaptureToken->IsComplete() )
			return false;
		void *mapped = nullptr;
		const VkDeviceSize bytes = VkDeviceSize( m_CaptureExtent.width ) * m_CaptureExtent.height * 4;
		if ( vkMapMemory( m_Ep.Device(), m_CaptureMemory, 0, bytes, 0, &mapped ) != VK_SUCCESS )
			return false;
		outRgba->resize( static_cast<size_t>( bytes ) );
		std::memcpy( outRgba->data(), mapped, static_cast<size_t>( bytes ) );
		vkUnmapMemory( m_Ep.Device(), m_CaptureMemory );
		if ( m_CaptureFormat == VK_FORMAT_B8G8R8A8_UNORM || m_CaptureFormat == VK_FORMAT_B8G8R8A8_SRGB )
			for ( size_t i = 0; i + 3 < outRgba->size(); i += 4 )
				std::swap( ( *outRgba )[i], ( *outRgba )[i + 2] );
		*outWidth = m_CaptureExtent.width;
		*outHeight = m_CaptureExtent.height;
		m_CaptureToken = nullptr;
		return true;
	}

private:
	struct RetiredSwapchain
	{
		VkSwapchainKHR swapchain;
		std::vector<VkSemaphore> semaphores;
		IRenderCompletionToken *token;
	};

	static void Fail( RenderCreateError *error, RenderCreateStatus status, const char *message )
	{
		if ( !error )
			return;
		error->status = status;
		std::snprintf( error->message, sizeof( error->message ), "%s", message );
	}

	void RetireBackBuffer()
	{
		IRenderCompletionToken *token = OrderingToken( m_Device );
		if ( token )
			m_Device.DestroyResourceWhenComplete( m_BackBuffer, *token );
		m_RetiringBackBuffers.push_back( m_BackBuffer );
		m_BackBuffer = kInvalidResource;
	}

	bool CreateBackBuffer()
	{
		const VkFormat format = BackBufferFormat( m_Config.format );
		m_BackBuffer = m_Ep.CreateImage( m_Extent.width, m_Extent.height, format,
		    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT );
		if ( m_BackBuffer == kInvalidResource )
			return false;
		m_BackBufferExtent = m_Extent;
		// The back buffer rests in GENERAL, cleared to black, before first use.
		IRenderCommandContext *context = m_Device.CreateCommandContext();
		if ( !context )
			return false;
		context->RecordUse( m_BackBuffer );
		VkCommandBuffer cmd = m_Ep.CommandBuffer( *context );
		const VkImage image = m_Ep.Image( m_BackBuffer );
		ImageBarrier( cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
		    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    VK_ACCESS_TRANSFER_WRITE_BIT );
		VkClearColorValue black = {};
		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.levelCount = 1;
		range.layerCount = 1;
		vkCmdClearColorImage( cmd, image, VK_IMAGE_LAYOUT_GENERAL, &black, 1, &range );
		ImageBarrier( cmd, image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
		    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
		    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT );
		return m_Device.Submit( *context ) != nullptr;
	}

	bool SurfaceChangedSinceSwapchain()
	{
		VkSurfaceCapabilitiesKHR caps = {};
		if ( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_Ep.PhysicalDevice(), m_VkSurface, &caps ) !=
		     VK_SUCCESS )
			return true;
		return caps.currentTransform != m_SwapSurfaceTransform ||
		       caps.currentExtent.width != m_SwapSurfaceExtent.width ||
		       caps.currentExtent.height != m_SwapSurfaceExtent.height;
	}

	void RetireSwapchain()
	{
		if ( m_Swapchain == VK_NULL_HANDLE )
			return;
		RetiredSwapchain r;
		r.swapchain = m_Swapchain;
		r.semaphores = m_RenderFinished;
		r.token = OrderingToken( m_Device );
		m_RetiredSwapchains.push_back( r );
		m_Swapchain = VK_NULL_HANDLE;
		m_RenderFinished.clear();
		m_SwapImages.clear();
	}

	bool RebuildSwapchain( RenderExtent drawable )
	{
		VkPhysicalDevice phys = m_Ep.PhysicalDevice();
		VkSurfaceCapabilitiesKHR caps = {};
		if ( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( phys, m_VkSurface, &caps ) != VK_SUCCESS )
		{
			m_SurfaceLost = true;
			return false;
		}
		VkExtent2D extent = caps.currentExtent;
		if ( extent.width == UINT32_MAX )
		{
			extent.width = std::max( caps.minImageExtent.width,
			    std::min( caps.maxImageExtent.width, drawable.width ) );
			extent.height = std::max( caps.minImageExtent.height,
			    std::min( caps.maxImageExtent.height, drawable.height ) );
		}
		if ( extent.width == 0 || extent.height == 0 )
		{
			RetireSwapchain(); // minimized: nothing to present to
			m_SwapDrawable = drawable;
			m_SwapDirty = false;
			return true;
		}
		if ( !( caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT ) )
			return false;

		uint32_t count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR( phys, m_VkSurface, &count, nullptr );
		std::vector<VkSurfaceFormatKHR> formats( count );
		if ( count )
			vkGetPhysicalDeviceSurfaceFormatsKHR( phys, m_VkSurface, &count, formats.data() );
		VkSurfaceFormatKHR chosen = {};
		chosen.format = VK_FORMAT_UNDEFINED;
		const VkFormat preferred[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM };
		for ( VkFormat want : preferred )
		{
			for ( const VkSurfaceFormatKHR &f : formats )
				if ( f.format == want && chosen.format == VK_FORMAT_UNDEFINED )
					chosen = f;
		}
		if ( chosen.format == VK_FORMAT_UNDEFINED && !formats.empty() )
			chosen = formats[0];
		VkFormatProperties fp = {};
		vkGetPhysicalDeviceFormatProperties( phys, chosen.format, &fp );
		if ( !( fp.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT ) )
			return false;

		VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
		if ( !m_Config.vsync )
		{
			uint32_t n = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR( phys, m_VkSurface, &n, nullptr );
			std::vector<VkPresentModeKHR> modes( n );
			if ( n )
				vkGetPhysicalDeviceSurfacePresentModesKHR( phys, m_VkSurface, &n, modes.data() );
			for ( VkPresentModeKHR m : modes )
				if ( m == VK_PRESENT_MODE_MAILBOX_KHR )
					mode = m;
		}

		uint32_t imageCount = caps.minImageCount + 1;
		if ( caps.maxImageCount > 0 && imageCount > caps.maxImageCount )
			imageCount = caps.maxImageCount;

		VkSwapchainCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = m_VkSurface;
		info.minImageCount = imageCount;
		info.imageFormat = chosen.format;
		info.imageColorSpace = chosen.colorSpace;
		info.imageExtent = extent;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		m_SwapCapturable = ( caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT ) != 0;
		if ( m_SwapCapturable )
			info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		// Drawn in window orientation; the compositor rotates on rotated displays.
		info.preTransform = ( caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
		                        ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
		                        : caps.currentTransform;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		if ( !( caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR ) )
			info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
		info.presentMode = mode;
		info.clipped = VK_TRUE;
		info.oldSwapchain = m_Swapchain;

		VkSwapchainKHR created = VK_NULL_HANDLE;
		if ( vkCreateSwapchainKHR( m_Ep.Device(), &info, nullptr, &created ) != VK_SUCCESS )
			return false;
		RetireSwapchain(); // the old one, behind the GPU work that may still use it
		m_Swapchain = created;
		m_SwapFormat = chosen.format;
		m_SwapExtent = extent;
		m_SwapDrawable = drawable;
		m_SwapSurfaceExtent = caps.currentExtent;
		m_SwapSurfaceTransform = caps.currentTransform;
		m_SwapDirty = false;

		uint32_t actual = 0;
		vkGetSwapchainImagesKHR( m_Ep.Device(), m_Swapchain, &actual, nullptr );
		m_SwapImages.resize( actual );
		vkGetSwapchainImagesKHR( m_Ep.Device(), m_Swapchain, &actual, m_SwapImages.data() );
		VkSemaphoreCreateInfo sci = {};
		sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		m_RenderFinished.assign( actual, VK_NULL_HANDLE );
		for ( VkSemaphore &s : m_RenderFinished )
			vkCreateSemaphore( m_Ep.Device(), &sci, nullptr, &s );
		return true;
	}

	bool PrepareCaptureBuffer()
	{
		const VkDeviceSize bytes = VkDeviceSize( m_SwapExtent.width ) * m_SwapExtent.height * 4;
		if ( m_CaptureBuffer != VK_NULL_HANDLE && m_CaptureBytes >= bytes )
		{
			m_CaptureFormat = m_SwapFormat;
			return true;
		}
		// A previous capture's frame must finish before its buffer is replaced.
		WaitToken( m_Device, m_CaptureToken );
		if ( m_CaptureBuffer != VK_NULL_HANDLE )
			vkDestroyBuffer( m_Ep.Device(), m_CaptureBuffer, nullptr );
		if ( m_CaptureMemory != VK_NULL_HANDLE )
			vkFreeMemory( m_Ep.Device(), m_CaptureMemory, nullptr );
		m_CaptureBuffer = VK_NULL_HANDLE;
		m_CaptureMemory = VK_NULL_HANDLE;
		VkBufferCreateInfo bi = {};
		bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bi.size = bytes;
		bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if ( vkCreateBuffer( m_Ep.Device(), &bi, nullptr, &m_CaptureBuffer ) != VK_SUCCESS )
			return false;
		VkMemoryRequirements req = {};
		vkGetBufferMemoryRequirements( m_Ep.Device(), m_CaptureBuffer, &req );
		uint32_t type = 0;
		if ( !FindMemoryType( m_Ep.PhysicalDevice(), req.memoryTypeBits,
		         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		         &type ) )
			return false;
		VkMemoryAllocateInfo ai = {};
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = req.size;
		ai.memoryTypeIndex = type;
		if ( vkAllocateMemory( m_Ep.Device(), &ai, nullptr, &m_CaptureMemory ) != VK_SUCCESS )
			return false;
		vkBindBufferMemory( m_Ep.Device(), m_CaptureBuffer, m_CaptureMemory, 0 );
		m_CaptureBytes = bytes;
		m_CaptureFormat = m_SwapFormat;
		return true;
	}

	// An acquired-but-unpresented image leaves its semaphore with a pending
	// signal; consume it so the semaphore can be reused or destroyed.
	void ConsumeOpenFrame()
	{
		if ( !m_FrameOpen )
			return;
		m_FrameOpen = false;
		const uint32_t slot = m_Slot;
		IRenderCommandContext *context = m_Device.CreateCommandContext();
		if ( context )
			m_SlotToken[slot] = m_Ep.SubmitWithSemaphores( *context, m_ImageAvailable[slot],
			    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_NULL_HANDLE );
		m_Slot = ( m_Slot + 1 ) % kFrameSlots;
	}

	// Waits for every submission that used the native surface, then destroys the
	// swapchains and the VkSurfaceKHR. The window still exists during the call.
	void ReleaseNative()
	{
		ConsumeOpenFrame();
		for ( uint32_t i = 0; i < kFrameSlots; ++i )
			WaitToken( m_Device, m_SlotToken[i] );
		WaitToken( m_Device, m_CaptureToken );
		RetireSwapchain();
		for ( const RetiredSwapchain &r : m_RetiredSwapchains )
			WaitToken( m_Device, r.token );
		CollectRetired();
		if ( m_VkSurface != VK_NULL_HANDLE )
			SDL_Vulkan_DestroySurface( m_Ep.Instance(), m_VkSurface, nullptr );
		m_VkSurface = VK_NULL_HANDLE;
		m_SurfaceLost = false;
	}

	platform_sdl3::Sdl3RenderSurfaces &m_Surfaces;
	IRenderDevice &m_Device;
	VulkanDeviceEndpoint &m_Ep;
	IRenderSurface &m_Surface;
	RenderPresentationConfig m_Config;
	RenderExtent m_Extent;

	VkSurfaceKHR m_VkSurface = VK_NULL_HANDLE;
	uint64_t m_SurfaceGeneration = 0;
	bool m_SurfaceLost = false;

	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
	VkFormat m_SwapFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D m_SwapExtent = { 0, 0 };
	RenderExtent m_SwapDrawable;
	VkExtent2D m_SwapSurfaceExtent = { 0, 0 };
	VkSurfaceTransformFlagBitsKHR m_SwapSurfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	bool m_SwapDirty = false;
	bool m_SwapCapturable = false;
	std::vector<VkImage> m_SwapImages;
	std::vector<VkSemaphore> m_RenderFinished; // per swapchain image
	std::vector<RetiredSwapchain> m_RetiredSwapchains;

	VkSemaphore m_ImageAvailable[kFrameSlots] = {};
	IRenderCompletionToken *m_SlotToken[kFrameSlots] = {};
	uint32_t m_Slot = 0;
	uint32_t m_ImageIndex = 0;
	bool m_FrameOpen = false;

	RenderResourceHandle m_BackBuffer = kInvalidResource;
	RenderExtent m_BackBufferExtent;
	std::vector<RenderResourceHandle> m_RetiringBackBuffers;

	bool m_CaptureRequested = false;
	VkBuffer m_CaptureBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_CaptureMemory = VK_NULL_HANDLE;
	VkDeviceSize m_CaptureBytes = 0;
	VkFormat m_CaptureFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D m_CaptureExtent = { 0, 0 };
	IRenderCompletionToken *m_CaptureToken = nullptr;
};

// ---------------------------------------------------------------------------
// Bridge
// ---------------------------------------------------------------------------

bool Sdl3VulkanInstanceExtensions( std::vector<std::string> *outExtensions, std::string *outError )
{
	Uint32 count = 0;
	char const *const *names = SDL_Vulkan_GetInstanceExtensions( &count );
	if ( !names )
	{
		if ( outError )
			*outError = std::string( "SDL_Vulkan_GetInstanceExtensions failed: " ) + SDL_GetError();
		return false;
	}
	outExtensions->assign( names, names + count );
	return true;
}

Sdl3VulkanPresentationBridge::Sdl3VulkanPresentationBridge( VulkanRenderBackend &provider,
    platform_sdl3::Sdl3RenderSurfaces &surfaces, uint32_t maxPresentations )
    : m_Provider( provider ), m_Surfaces( surfaces ), m_Max( maxPresentations )
{
}

Sdl3VulkanPresentationBridge::~Sdl3VulkanPresentationBridge()
{
	while ( !m_Live.empty() )
		DestroyPresentation( m_Live.back() );
	CollectParked( true, nullptr, nullptr );
}

RenderPresentationPairId Sdl3VulkanPresentationBridge::GetPairId() const
{
	RenderPresentationPairId pair;
	pair.windowSystem = "sdl3";
	pair.renderBackend = m_Provider.GetBackendId().id;
	return pair;
}

IRenderPresentation *Sdl3VulkanPresentationBridge::CreatePresentation( IRenderDevice &device,
    IRenderSurface &surface, const RenderPresentationConfig &config, RenderCreateError *error )
{
	RenderCreateError local;
	RenderCreateError &err = error ? *error : local;
	err = RenderCreateError();
	CollectParked( false, nullptr, nullptr );

	auto fail = [&err]( RenderCreateStatus status, const char *message ) -> IRenderPresentation * {
		err.status = status;
		std::snprintf( err.message, sizeof( err.message ), "%s", message );
		return nullptr;
	};
	VulkanDeviceEndpoint *endpoint = m_Provider.FindDevice( device );
	if ( !endpoint || !m_Surfaces.Owns( surface ) )
		return fail( RenderCreateStatus::kForeignObject, "device or surface is foreign to this bridge" );
	if ( surface.GetStatus() == RenderSurfaceStatus::kDestroyed )
		return fail( RenderCreateStatus::kSurfaceLost, "the window is destroyed" );
	const RenderDeviceState state = device.GetState();
	if ( state == RenderDeviceState::kDeviceLost || state == RenderDeviceState::kFatal )
		return fail( RenderCreateStatus::kDeviceUnavailable, "the device is lost" );
	if ( m_Live.size() >= m_Max )
		return fail( RenderCreateStatus::kTooManyPresentations, "presentation limit reached" );
	for ( Sdl3VulkanPresentation *p : m_Live )
		if ( &p->Surface() == &surface )
			return fail( RenderCreateStatus::kSurfaceBusy, "the surface is already presented" );
	// A destroyed presentation's native surface for this window goes first.
	CollectParked( true, nullptr, &surface );

	const RenderExtent extent = ( config.extent.width == 0 && config.extent.height == 0 )
	                                ? surface.GetDrawableExtent()
	                                : config.extent;
	Sdl3VulkanPresentation *p = new Sdl3VulkanPresentation(
	    m_Surfaces, device, *endpoint, surface, config, extent );
	if ( !surface.AttachListener( *p ) )
	{
		delete p;
		return fail( RenderCreateStatus::kSurfaceBusy, "the surface is already presented" );
	}
	if ( !p->CreateNativeSurface( &err ) )
	{
		surface.DetachListener( *p );
		delete p;
		return nullptr;
	}
	m_Live.push_back( p );
	return p;
}

void Sdl3VulkanPresentationBridge::DestroyPresentation( IRenderPresentation *presentation )
{
	for ( size_t i = 0; i < m_Live.size(); ++i )
	{
		if ( m_Live[i] != presentation )
			continue;
		Sdl3VulkanPresentation *p = m_Live[i];
		m_Live.erase( m_Live.begin() + static_cast<std::ptrdiff_t>( i ) );
		IRenderSurface &surface = p->Surface();
		Sdl3VulkanRetiredNative *retired = p->Retire();
		surface.DetachListener( *p );
		delete p;
		Park( retired );
		// Releases now when the GPU is already done; otherwise the parked objects
		// wait for their tokens, the window's release, or ReleaseDevice.
		if ( retired->Complete() )
			retired->Release( false );
		else if ( retired->vkSurface != VK_NULL_HANDLE && surface.AttachListener( *retired ) )
			retired->surface = &surface;
		CollectParked( false, nullptr, nullptr );
		return;
	}
}

void Sdl3VulkanPresentationBridge::Park( Sdl3VulkanRetiredNative *retired )
{
	m_Parked.push_back( retired );
}

void Sdl3VulkanPresentationBridge::CollectParked(
    bool wait, IRenderDevice *onlyDevice, const IRenderSurface *onlySurface )
{
	std::vector<Sdl3VulkanRetiredNative *> keep;
	for ( Sdl3VulkanRetiredNative *r : m_Parked )
	{
		const bool selected = ( !onlyDevice || r->device == onlyDevice ) &&
		                      ( !onlySurface || r->surface == onlySurface );
		if ( selected || !wait )
			r->Release( wait && selected );
		if ( r->released )
			delete r;
		else
			keep.push_back( r );
	}
	m_Parked.swap( keep );
}

bool Sdl3VulkanPresentationBridge::ReleaseDevice( IRenderDevice &device )
{
	for ( Sdl3VulkanPresentation *p : m_Live )
		if ( &p->Device() == &device )
			return false;
	CollectParked( true, &device, nullptr );
	return true;
}

Sdl3VulkanPresentation *Sdl3VulkanPresentationBridge::Find( IRenderPresentation &presentation ) const
{
	for ( Sdl3VulkanPresentation *p : m_Live )
		if ( p == &presentation )
			return p;
	return nullptr;
}

size_t Sdl3VulkanPresentationBridge::NativeSurfaceCount( const IRenderSurface &surface ) const
{
	size_t n = 0;
	for ( Sdl3VulkanPresentation *p : m_Live )
		if ( &p->Surface() == &surface && p->HasNativeSurface() )
			++n;
	for ( Sdl3VulkanRetiredNative *r : m_Parked )
		if ( r->surface == &surface && r->vkSurface != VK_NULL_HANDLE )
			++n;
	return n;
}

bool Sdl3VulkanPresentationBridge::RequestCapture( IRenderPresentation &presentation )
{
	Sdl3VulkanPresentation *p = Find( presentation );
	return p && p->RequestCapture();
}

bool Sdl3VulkanPresentationBridge::ReadCapture( IRenderPresentation &presentation,
    std::vector<uint8_t> *outRgba, uint32_t *outWidth, uint32_t *outHeight )
{
	Sdl3VulkanPresentation *p = Find( presentation );
	return p && p->ReadCapture( outRgba, outWidth, outHeight );
}

} // namespace render_vulkan
