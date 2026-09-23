//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDL3 + Vulkan presentation bridge, window-host half (RFC 0001
//          "Window and render interop", roadmap R16). This directory is the only
//          code in the Vulkan backend that includes SDL; the core
//          (vulkan_device.cpp) reaches the window through IVulkanSurfaceHost.
//
//          This header is free of SDL types so legacy callers can include it.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_SDL3_VULKAN_SURFACE_HOST_H
#define SHADERAPIVULKAN_SDL3_VULKAN_SURFACE_HOST_H

#ifdef _WIN32
#pragma once
#endif

#include "../vulkan_surface_host.h"

#include <memory>
#include <string>

namespace render_vulkan
{

// Legacy migration membrane: IShaderAPI::SetMode receives the engine window as
// an untyped reference, which on SDL3 builds is the launcher's SDL_Window. This
// is the single place that interprets it. Returns nullptr with *outError for a
// null reference.
std::unique_ptr<IVulkanSurfaceHost> MakeSdl3LegacySurfaceHost(
    void *legacyWindowRef, std::string *outError );

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_SDL3_VULKAN_SURFACE_HOST_H
