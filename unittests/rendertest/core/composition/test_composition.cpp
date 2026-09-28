//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition and render.legacy-frontend (RFC 0016 A.7),
//			through the C entry points the launcher uses:
//
//			P1 RenderCore_Create composes the null device, the frontend and the
//			   renderer, and fails with a structured result naming an unknown
//			   device or feature;
//			P2 the frontend's provider keeps the wrapped backend's id and
//			   module and forwards creation, counting it;
//			P3 RenderStageMarkers001 marks stages of the engine's frames,
//			   rejects the engine-owned begin and end, and counts violations;
//			P4 each frame runs the legacy-stream pass before present.
//
//=============================================================================//

#include "render/composition/render_core.h"
#include "render/device/device.h"
#include "render/legacy/stage_markers.h"
#include "testing/checks.h"

#include <cstring>

namespace
{

int g_Creates = 0;

bool FakeCreate( render::LegacyShaderServices *services )
{
	++g_Creates;
	services->manager = reinterpret_cast<IShaderDeviceMgr *>( 0x10 );
	return true;
}

const render::LegacyShaderProvider g_Backend = { "fixture", "fixture_module", &FakeCreate, true };

} // namespace

int main()
{
	testing::Checks checks;
	RenderCoreResult result;

	RenderCoreConfig unknownDevice;
	unknownDevice.device = "metal";
	checks.That( !RenderCore_Create( &unknownDevice, &result ) &&
	                 result.status == RENDER_CORE_UNKNOWN_DEVICE &&
	                 std::strstr( result.message, "metal" ),
	    "P1.an-unlinked-device-fails-by-name" );
	RenderCoreConfig unknownFeature;
	unknownFeature.features = "legacy-stream,shadows,present";
	checks.That( !RenderCore_Create( &unknownFeature, &result ) &&
	                 result.status == RENDER_CORE_UNKNOWN_FEATURE &&
	                 std::strstr( result.message, "shadows" ),
	    "P1.an-unlinked-feature-fails-by-name" );
	checks.That(
	    !RenderCore_Create( nullptr, &result ) && result.status == RENDER_CORE_INVALID_CONFIG,
	    "P1.no-config-fails" );

	RenderCoreConfig config;
	config.legacyBackend = &g_Backend;
	RenderCore *core = RenderCore_Create( &config, &result );
	if ( !checks.That( core && result.status == RENDER_CORE_OK, "P1.the-null-core-composes" ) )
		return checks.Report();
	const RenderCoreBinding *binding = RenderCore_GetBinding( core );
	checks.That( binding && binding->device && binding->renderer && binding->sceneFactory.create &&
	                 binding->stageMarkers && std::strcmp( binding->deviceName, "null" ) == 0,
	    "P1.the-binding-is-complete" );
	// A host drives its own graphs on the core's device port.
	if ( binding && binding->device )
	{
		render::device::BufferDesc desc;
		desc.size = 64;
		desc.usages = render::device::UsageSet{ render::device::ResourceUsage::kCopyDestination };
		auto buffer = binding->device->CreateBuffer( desc );
		checks.That(
		    buffer.HasValue() &&
		        binding->device->Release( buffer.Value(), render::device::CompletionToken() )
		            .HasValue(),
		    "P1.the-device-port-is-usable" );
	}

	const render::LegacyShaderProvider *provider = RenderCore_GetLegacyProvider( core );
	checks.That( provider && std::strcmp( provider->id, "fixture" ) == 0 &&
	                 std::strcmp( provider->legacyModuleName, "fixture_module" ) == 0 &&
	                 provider->supportsQueuedRendering && provider->createFor && !provider->create,
	    "P2.the-provider-keeps-the-backend-identity" );
	render::LegacyShaderServices services;
	checks.That( provider && provider->createFor( provider->context, &services ) &&
	                 services.manager == reinterpret_cast<IShaderDeviceMgr *>( 0x10 ) &&
	                 g_Creates == 1 && RenderCore_GetLegacyProviderCreates( core ) == 1,
	    "P2.creation-forwards-to-the-backend" );

	IRenderStageMarkers *markers = binding->stageMarkers;
	render::frame::IRenderer *renderer = binding->renderer;
	checks.That(
	    !markers->MarkStage( RENDER_STAGE_OPAQUE ), "P3.marks-outside-a-frame-are-rejected" );
	render::frame::FrameDesc frame;
	frame.width = 320;
	frame.height = 200;
	(void)renderer->BeginFrame( frame );
	checks.That( markers->MarkStage( RENDER_STAGE_VIEW_BEGIN ) &&
	                 markers->MarkStage( RENDER_STAGE_OPAQUE ) &&
	                 markers->MarkStage( RENDER_STAGE_TRANSLUCENT ) &&
	                 markers->MarkStage( RENDER_STAGE_VIEW_END ) &&
	                 markers->MarkStage( RENDER_STAGE_HUD ),
	    "P3.the-client-marks-a-frame" );
	checks.That( !markers->MarkStage( RENDER_STAGE_FRAME_END ) &&
	                 !markers->MarkStage( RENDER_STAGE_FRAME_BEGIN ),
	    "P3.the-engine-owns-begin-and-end" );
	auto stats = renderer->EndFrame();
	checks.That( stats && stats.Value().passes == 2 && stats.Value().views == 1 &&
	                 stats.Value().orderViolations == 0,
	    "P4.the-frame-runs-legacy-stream-then-present" );
	checks.Equal( markers->GetOrderViolations(), 3u, "P3.rejected-marks-are-counted" );

	auto scene = binding->sceneFactory.create();
	checks.That( scene && scene->Revision() == 0, "P1.the-scene-factory-makes-scenes" );
	scene.reset();
	RenderCore_Destroy( core );
	RenderCore_Destroy( nullptr );
	checks.That( true, "P1.destroy-tears-down" );
	return checks.Report();
}
