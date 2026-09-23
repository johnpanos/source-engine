//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDL3 + Vulkan presentation bridge (RFC 0001 "Window and render
//          interop", roadmap R16): render::IRenderPresentationBridgeFactory for
//          the ("sdl3", "vulkan") pair. It joins SDL3 render surfaces to devices
//          of the native Vulkan provider through both providers' private
//          endpoints; it is the only new-contract code that turns a window into
//          a VkSurfaceKHR and swapchain.
//
//          Back buffers are device images handed to the caller; Present blits
//          the back buffer into the acquired swapchain image (scaling to the
//          drawable) in a device submission ordered after the caller's work.
//          Replaced back buffers and swapchains are released only after the
//          device's completion token for that ordering submission completes.
//
//          This header is free of SDL types.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_SDL3_VULKAN_PRESENTATION_H
#define SHADERAPIVULKAN_SDL3_VULKAN_PRESENTATION_H

#ifdef _WIN32
#pragma once
#endif

#include "render/render_presentation.h"

#include <cstdint>
#include <string>
#include <vector>

namespace platform_sdl3
{
class Sdl3RenderSurfaces;
}

namespace render_vulkan
{

class VulkanRenderBackend;
class Sdl3VulkanPresentation;
struct Sdl3VulkanRetiredNative;

// Instance extensions SDL3 needs for Vulkan surfaces. Call after SDL video is
// initialized; pass them in VulkanProviderOptions::instanceExtensions.
bool Sdl3VulkanInstanceExtensions( std::vector<std::string> *outExtensions, std::string *outError );

class Sdl3VulkanPresentationBridge : public render::IRenderPresentationBridgeFactory
{
public:
	// Both providers must outlive the bridge.
	Sdl3VulkanPresentationBridge( VulkanRenderBackend &provider,
	    platform_sdl3::Sdl3RenderSurfaces &surfaces, uint32_t maxPresentations = 4 );
	~Sdl3VulkanPresentationBridge() override;

	Sdl3VulkanPresentationBridge( const Sdl3VulkanPresentationBridge & ) = delete;
	Sdl3VulkanPresentationBridge &operator=( const Sdl3VulkanPresentationBridge & ) = delete;

	render::RenderPresentationPairId GetPairId() const override;
	uint32_t GetMaxPresentations() const override { return m_Max; }
	render::IRenderPresentation *CreatePresentation( render::IRenderDevice &device,
	    render::IRenderSurface &surface, const render::RenderPresentationConfig &config,
	    render::RenderCreateError *error ) override;
	void DestroyPresentation( render::IRenderPresentation *presentation ) override;
	size_t GetLivePresentationCount() const override { return m_Live.size(); }
	bool ReleaseDevice( render::IRenderDevice &device ) override;

	// -- Native test endpoint --
	// VkSurfaceKHR objects alive for 'surface' (live presentations and retired
	// ones still waiting for the GPU).
	size_t NativeSurfaceCount( const render::IRenderSurface &surface ) const;
	// Copies the next presented swapchain image to host memory. ReadCapture waits
	// for that frame's GPU completion and returns tightly packed RGBA8.
	bool RequestCapture( render::IRenderPresentation &presentation );
	bool ReadCapture( render::IRenderPresentation &presentation, std::vector<uint8_t> *outRgba,
	    uint32_t *outWidth, uint32_t *outHeight );

	// For presentations: park native objects that must outlive in-flight work.
	void Park( Sdl3VulkanRetiredNative *retired );

private:
	Sdl3VulkanPresentation *Find( render::IRenderPresentation &presentation ) const;
	void CollectParked( bool wait, render::IRenderDevice *onlyDevice,
	    const render::IRenderSurface *onlySurface );

	VulkanRenderBackend &m_Provider;
	platform_sdl3::Sdl3RenderSurfaces &m_Surfaces;
	uint32_t m_Max;
	std::vector<Sdl3VulkanPresentation *> m_Live;
	std::vector<Sdl3VulkanRetiredNative *> m_Parked;
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_SDL3_VULKAN_PRESENTATION_H
