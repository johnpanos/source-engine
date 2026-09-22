//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A native Vulkan provider of the backend-agnostic render capability
//          contract (public/render/render_backend.h, RFC 0001 render.contracts).
//
//          This is the substitutability piece of the goal: the material system
//          depends on the render.contracts interfaces, and both the D3D9/null
//          providers and this Vulkan provider implement the SAME contract, so
//          the ONE shared conformance suite in unittests/rendertest runs against
//          each of them (LSP: interchangeable implementations of one behavioral
//          contract). Like abstracting a machine-dependent layer, the backend is
//          architecture-dependent below this line and portable above it.
//
//          It is genuinely Vulkan-backed where the contract is a fact about the
//          GPU: real instance and physical-device enumeration, a real logical
//          device and queue, real device-memory-backed buffer/image resources,
//          and real command-buffer submission with VkFence completion. Contract
//          semantics that are lifecycle state rather than GPU fact -- presentation
//          resize/suspend and device-loss transitions -- are modeled as the
//          contract defines them (the suite certifies lifecycle here, not image
//          fidelity; real pixels are proven separately by CVulkanContext).
//
//          Depends only on render_backend.h and <vulkan/vulkan.h>: no SDL, no
//          native window, no legacy IShaderAPI. Presentation in this provider is
//          therefore surfaceless/offscreen at the contract level.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_RENDER_BACKEND_H
#define SHADERAPIVULKAN_VULKAN_RENDER_BACKEND_H

#include "render/render_backend.h"

#include <memory>
#include <string>

namespace render_vulkan
{

// Builds the native Vulkan render-backend provider. Returns nullptr and fills
// *outError when no usable Vulkan instance/device is available (so a harness can
// report an explicit skip rather than a false failure on a headless host).
std::unique_ptr<render::IRenderBackendProvider> MakeVulkanRenderBackend( std::string *outError );

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_RENDER_BACKEND_H
