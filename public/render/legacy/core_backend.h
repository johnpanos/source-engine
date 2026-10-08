//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.legacy-frontend (RFC 0016): the one place legacy rendering
//			meets the core. It gives the material system its shader provider
//			(through the existing MaterialSystem_BindShaderProvider), gives the
//			client RenderStageMarkers001, adds the legacy stream to each
//			frame's graph as a feature, and hands the engine the backend's
//			optional capabilities in frame order (capabilities.h).
//
//			Until K3 the provider forwards to the linked legacy backend it
//			wraps, keeping that backend's id so profile selection and quirks
//			are unchanged, and the legacy-stream pass only marks where the
//			legacy work sits in the frame. At K3 the frontend implements
//			IShaderAPI itself and records the legacy work into that pass.
//
//=============================================================================//

#ifndef RENDER_LEGACY_CORE_BACKEND_H
#define RENDER_LEGACY_CORE_BACKEND_H

#include "render/frame/feature.h"
#include "render/frame/renderer.h"
#include "render/legacy/capabilities.h"
#include "render/legacy/core_passes.h"
class IRenderStageMarkers;   // render/legacy/stage_markers.h (legacy-interop)
class IRenderMaterialBlocks; // render/legacy/material_blocks.h (legacy-interop)
#include "render/legacy_shader_provider.h"

#include <cstdint>
#include <memory>

namespace render::legacy
{

// What the frontend's core-pass recorder records until the world draws from
// the scene (RFC 0016 K5 plan, step 3's oracle).
enum class CorePassProbe : std::uint8_t
{
	kNone,       // no slot is marked: the legacy stream replays unchanged
	kEmpty,      // a slot at each view's opaque stage, recording only a label
	kSeededClear // as kEmpty, clearing the slot's color target (negative control)
};

class ILegacyFrontend
{
public:
	virtual ~ILegacyFrontend() = default;

	// The provider the material system binds; valid while the frontend lives.
	virtual const LegacyShaderProvider *Provider() const = 0;
	virtual IRenderStageMarkers *Markers() = 0;
	// RenderMaterialBlocks001: bound materials' variables as family blocks.
	virtual IRenderMaterialBlocks *MaterialBlocks() = 0;
	// The legacy stream as a frame feature; the renderer owns it, and the
	// frontend outlives the renderer.
	virtual std::unique_ptr<frame::IRenderFeature> CreateStreamFeature() = 0;
	// The renderer the markers forward to; set once, after it is created.
	virtual void BindRenderer( frame::IRenderer *renderer ) = 0;
	// How many times the material system composed the legacy backend
	// through this frontend.
	virtual std::uint32_t ProviderCreates() const = 0;
	// The capabilities of the backend the material system last created
	// through the frontend; valid while the frontend lives.
	virtual ILegacyCapabilities *Capabilities() = 0;
	// The render call queue the capabilities order their calls on; null
	// (the default) calls the backend directly.
	virtual void BindRenderCallQueue( const RenderCallQueueHost *host ) = 0;
	// The recorder the composition root binds into the backend
	// (NativeVulkanShaderBackend_BindCorePassRecorder), what it records, and
	// how many slots it has recorded.
	virtual ICorePassRecorder *CorePasses() = 0;
	virtual void SetCorePassProbe( CorePassProbe probe ) = 0;
	virtual std::uint64_t CorePassesRecorded() const = 0;
	// The recorder of forwarded tags (kCorePassForwarded), and the backend's
	// slots in frame order for them; null when the backend has none.
	virtual void SetForwardedRecorder( ICorePassRecorder *recorder ) = 0;
	virtual ICorePassSlots *CorePassSlots() = 0;
	// The render core's adapter identity the material system's device facade
	// reports (RFC 0016 legacy device facade, F1): given to every backend the
	// material system creates through this frontend from now on. The root
	// sets it from the device adapter it composed; empty clears it.
	virtual void SetCoreAdapterSource( const LegacyShaderServices::CoreAdapterSource &source ) = 0;
};

// backend: the linked legacy backend the frontend wraps; it outlives the
// frontend. nullptr when the product composes no legacy backend.
std::unique_ptr<ILegacyFrontend> CreateLegacyFrontend( const LegacyShaderProvider *backend );

} // namespace render::legacy

#endif // RENDER_LEGACY_CORE_BACKEND_H
