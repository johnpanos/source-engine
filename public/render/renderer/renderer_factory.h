//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.renderer, the full renderer (RFC 0016), an adapter of
//			render.frame.v1. It owns the frame's features, builds each frame's
//			graph from them in order, and executes it with the serial graph
//			executor.
//
//			CreateRenderer fails, naming the feature and the capability, when a
//			feature requires something the device lacks. Declared fallbacks are
//			the composition root's choice, not the renderer's.
//
//=============================================================================//

#ifndef RENDER_RENDERER_RENDERER_FACTORY_H
#define RENDER_RENDERER_RENDERER_FACTORY_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/frame/feature.h"
#include "render/frame/renderer.h"

#include <memory>
#include <vector>

namespace render::renderer
{

struct RendererDeps
{
	device::IRenderDevice2 *device = nullptr; // outlives the renderer
	std::vector<std::unique_ptr<frame::IRenderFeature>> features;
};

enum class RendererStatus : std::uint8_t
{
	kNoDevice = 1,
	kMissingCapability
};

struct RendererError
{
	RendererStatus status = RendererStatus::kNoDevice;
	const char *feature = nullptr; // the feature's Name()
	device::Capability capability = device::Capability::kCompute;
};

foundation::Expected<std::unique_ptr<frame::IRenderer>, RendererError> CreateRenderer(
    RendererDeps &&deps );

} // namespace render::renderer

#endif // RENDER_RENDERER_RENDERER_FACTORY_H
