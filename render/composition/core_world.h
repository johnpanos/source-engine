//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): IRenderCoreWorld for the
//			engine over render.pass.world, and the legacy frontend's
//			forwarded recorder that draws its slots. It translates the engine's
//			plain arrays and material system textures (handles through the
//			render call queue host) into the pass's data, marks each view's
//			slot through the frontend's frame-ordered slots, and hands the
//			backend's slot targets and textures to the pass. Private to the
//			composition.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_CORE_WORLD_H
#define RENDER_COMPOSITION_CORE_WORLD_H

#include "core_output.h"
#include "render/composition/render_core.h"
#include "render/legacy/core_backend.h"
#include "render/frame/renderer.h"
#include "render/legacy/core_passes.h"
#include "render/pass/debug/debug_overlays.h"
#include "render/pass/world/world_pass.h"

#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

namespace render::composition
{

class CoreWorld final : public IRenderCoreWorld,
                        public legacy::ICorePassRecorder,
                        public frame::IRenderStageHooks
{
public:
	CoreWorld( legacy::ILegacyFrontend &frontend, const frame::IRenderer &renderer )
	    : m_Frontend( frontend ), m_Renderer( renderer )
	{
	}

	void BindHost( const legacy::RenderCallQueueHost *host ) { m_Host = host; }

	// IRenderCoreWorld (the engine, main thread).
	void SetWorld( const RenderCoreWorldVertex *vertices, unsigned int vertexCount,
	    const unsigned int *indices, unsigned int indexCount,
	    const RenderCoreWorldSurface *surfaces, unsigned int surfaceCount,
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount ) override;
	void ClearWorld() override { m_Pass.ClearWorld(); }
	bool Draws( unsigned int material ) const override { return m_Pass.Draws( material ); }
	bool DrawView( const unsigned int *surfaces, unsigned int count, const float worldToClip[16],
	    const float viewport[6], unsigned long long hostFrame ) override;
	void BeginFrame() override;
	void EndFrame() override;
	// frame::IRenderStageHooks (the main thread): the open views' depth, so a
	// queued view knows whether it is the frame's top level (not a portal,
	// mirror or monitor view inside another).
	void OnStage( frame::Stage stage, std::uint32_t depth ) override;
	unsigned long long Failures() const override;
	void GetStats( RenderCoreWorldStats *out ) const override;

	// legacy::ICorePassRecorder (the backend, render sequence).
	std::uint32_t SlotStages() const override { return 0; }
	void RecordSlot( std::uint32_t tag, device::CommandEncoder &encoder,
	    const legacy::CorePassTarget &target ) override;
	bool RecordOutput( device::CommandEncoder &encoder,
	    const legacy::CoreOutputTargets &targets ) override
	{
		return m_Output.Record( encoder, targets );
	}
	void ReleaseDevice( device::IRenderDevice2 &device ) override
	{
		m_Pass.ReleaseDevice( device );
		m_Overlays.ReleaseDevice( device );
		m_Output.ReleaseDevice( device );
	}

private:
	legacy::ILegacyFrontend &m_Frontend;
	const frame::IRenderer &m_Renderer;
	const legacy::RenderCallQueueHost *m_Host = nullptr;
	pass::world::WorldPass m_Pass;
	pass::debug::DebugOverlays m_Overlays;
	// cl_render_debug_legacy 1: the tags of top-level views (main thread
	// writes, render sequence reads), and per recorded frame serial the
	// top-level world slots it recorded, with their color targets (render
	// sequence). A frame recorded again for a capture records its slots
	// again, under the same or a higher serial, so nothing is consumed.
	std::uint32_t m_ViewDepth = 0;
	std::mutex m_TopLevelLock;
	std::set<std::uint32_t> m_TopLevel;
	std::map<std::uint64_t, std::vector<std::pair<std::uint32_t, device::TextureId>>> m_FrameViews;
	std::atomic<unsigned long long> m_Hatches{ 0 };
	std::atomic<unsigned long long> m_Tints{ 0 };
	std::atomic<unsigned long long> m_Redrawn{ 0 };
	CoreOutput m_Output;
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_WORLD_H
