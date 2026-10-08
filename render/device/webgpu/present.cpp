//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.webgpu: the browser product's presentation (RFC
//			0029), the page canvas as a WebGPU surface. See
//			PresentToCanvas in public/render/device/webgpu/provider.h.
//
//=============================================================================//

#include "webgpu_device.h"

#if defined( __EMSCRIPTEN__ )
#include <emscripten/html5.h>
#endif

#include <cstdio>
#include <mutex>

namespace render::device::webgpu
{

bool WebGpuDevice::PresentToCanvas(
    std::uint64_t texture, std::uint32_t width, std::uint32_t height, const char *selector )
{
#if defined( __EMSCRIPTEN__ )
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable || width == 0 || height == 0 || !selector )
		return false;
	TextureRecord *color = LiveTexture( texture );
	if ( !color || !color->texture || color->desc.format != Format::kRGBA8Unorm ||
	     color->desc.width < width || color->desc.height < height )
		return false;
	if ( !m_Surface )
	{
		WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvas =
		    WGPU_EMSCRIPTEN_SURFACE_SOURCE_CANVAS_HTML_SELECTOR_INIT;
		canvas.selector = { selector, WGPU_STRLEN };
		WGPUSurfaceDescriptor descriptor = WGPU_SURFACE_DESCRIPTOR_INIT;
		descriptor.nextInChain = &canvas.chain;
		m_Surface = wgpuInstanceCreateSurface( m_Instance, &descriptor );
		if ( !m_Surface )
		{
			std::fprintf( stderr, "webgpu: no surface for the canvas %s\n", selector );
			return false;
		}
	}
	const std::uint64_t extent = ( std::uint64_t( width ) << 32 ) | height;
	if ( extent != m_SurfaceExtent )
	{
		// RGBA8 (a canvas format every browser offers), so the frame copies
		// onto it as it is.
#if defined( __EMSCRIPTEN__ )
		// The canvas's backing store is the frame's size: the browser sizes
		// the surface's textures from it, and a page's canvas (or SDL's window
		// on it) may start at 0x0.
		(void)emscripten_set_canvas_element_size( selector, int( width ), int( height ) );
#endif
		WGPUSurfaceConfiguration configuration = WGPU_SURFACE_CONFIGURATION_INIT;
		configuration.device = m_Device;
		configuration.format = WGPUTextureFormat_RGBA8Unorm;
		configuration.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_RenderAttachment;
		configuration.width = width;
		configuration.height = height;
		configuration.alphaMode = WGPUCompositeAlphaMode_Opaque;
		configuration.presentMode = WGPUPresentMode_Fifo;
		wgpuSurfaceConfigure( m_Surface, &configuration );
		m_SurfaceExtent = extent;
	}
	WGPUSurfaceTexture current = WGPU_SURFACE_TEXTURE_INIT;
	wgpuSurfaceGetCurrentTexture( m_Surface, &current );
	if ( !current.texture || ( current.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
	                             current.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal ) )
	{
		if ( current.texture )
			wgpuTextureRelease( current.texture );
		m_SurfaceExtent = 0; // configured again next frame
		return false;
	}
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder( m_Device, nullptr );
	WGPUTexelCopyTextureInfo source = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
	source.texture = color->texture.Get();
	WGPUTexelCopyTextureInfo destination = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
	destination.texture = current.texture;
	const WGPUExtent3D size = { width, height, 1 };
	wgpuCommandEncoderCopyTextureToTexture( encoder, &source, &destination, &size );
	WGPUCommandBuffer commands = wgpuCommandEncoderFinish( encoder, nullptr );
	wgpuQueueSubmit( m_Queue, 1, &commands );
	wgpuCommandBufferRelease( commands );
	wgpuCommandEncoderRelease( encoder );
	wgpuTextureRelease( current.texture );
	// The browser presents the canvas when this thread yields.
	return true;
#else
	(void)texture;
	(void)width;
	(void)height;
	(void)selector;
	return false;
#endif
}

} // namespace render::device::webgpu
