//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition (RFC 0016); see render_core.h.
//
//=============================================================================//

#include "render/composition/render_core.h"

#include "jobsystem/pooled_executor.h"
#include "render/device/null/provider.h"
#include "core_world.h"
#include "render/legacy/core_backend.h"
#include "render/pass/present/feature.h"
#include "render/material/program_resolver.h"
#include "render/renderer/renderer_factory.h"
#if defined( RENDER_CORE_VULKAN )
#include "render/device/vulkan/provider.h"
#endif

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

struct RenderCore
{
	std::unique_ptr<render::device::IRenderDevice2> device;
	std::unique_ptr<render::legacy::ILegacyFrontend> frontend;
	// The BSP world drawn by the core; the frontend forwards its slots to it.
	std::unique_ptr<render::composition::CoreWorld> world;
	std::unique_ptr<render::frame::IRenderer> renderer;
	std::string deviceName;
	RenderCoreBinding binding;
	// Pooled culling over the root's compute workers (RenderCoreConfig::
	// computeWorkers); inline on the caller when the root gave none.
	std::unique_ptr<jobsystem::PooledExecutor> cullJobs;

	~RenderCore()
	{
		// Borrowers first: the markers forward to the renderer, and the
		// renderer's frames reference the device.
		if ( frontend )
		{
			frontend->BindRenderer( nullptr );
			frontend->SetForwardedRecorder( nullptr );
		}
		if ( renderer && world )
			renderer->RemoveStageHooks( world.get() );
		world.reset();
		renderer.reset();
		cullJobs.reset();
		if ( device )
			(void)device->WaitIdle(); // reviewed idle wait: teardown
		frontend.reset();
		device.reset();
	}
};

namespace
{

foundation::Expected<render::scene::DrawList, render::scene::CullStatus> BuildDrawListPooledOnCore(
    void *context, const render::scene::SceneSnapshot &snapshot,
    const render::scene::SceneView &view, render::scene::IVisibilityProvider *provider )
{
	RenderCore &core = *static_cast<RenderCore *>( context );
	return render::scene::BuildDrawListPooled( snapshot, view, *core.cullJobs, provider );
}

const render::device::DeviceProviderDescriptor *FindDevice( std::string_view name )
{
	const render::device::DeviceProviderDescriptor *linked[] = {
	    &render::device::null::Describe(),
#if defined( RENDER_CORE_VULKAN )
	    &render::device::vulkan::Describe(),
#endif
	};
	for ( const render::device::DeviceProviderDescriptor *descriptor : linked )
	{
		if ( descriptor->id == name )
			return descriptor;
	}
	return nullptr;
}

RenderCore *Fail( RenderCoreResult *result, RenderCoreStatus status, const std::string &message )
{
	if ( result )
	{
		result->status = status;
		std::snprintf( result->message, sizeof( result->message ), "%s", message.c_str() );
	}
	return nullptr;
}

} // namespace

// feature_catalog.cpp
std::unique_ptr<render::frame::IRenderFeature> RenderCore_CreateFeature(
    std::string_view name, render::legacy::ILegacyFrontend &frontend );

extern "C" RenderCore *RenderCore_Create( const RenderCoreConfig *config, RenderCoreResult *result )
{
	if ( result )
		*result = RenderCoreResult();
	if ( !config || !config->device || !config->features )
		return Fail( result, RENDER_CORE_INVALID_CONFIG, "no configuration" );

	const render::device::DeviceProviderDescriptor *descriptor = FindDevice( config->device );
	if ( !descriptor )
		return Fail( result, RENDER_CORE_UNKNOWN_DEVICE,
		    std::string( "render device '" ) + config->device + "' is not linked in this product" );

	auto core = std::make_unique<RenderCore>();
	// The core starts no threads: compute work runs on the workers the root
	// lends it (RFC 0003 "capacity policy").
	core->cullJobs = std::make_unique<jobsystem::PooledExecutor>( config->computeWorkers );
	render::device::DeviceRequest request;
	request.validation = config->validation;
	auto device = descriptor->create( request );
	if ( !device )
		return Fail( result, RENDER_CORE_DEVICE_FAILED,
		    std::string( "render device '" ) + config->device +
		        "' failed: " + render::device::DescribeStatus( device.Error().status ) + " in " +
		        render::device::DescribeOperation( device.Error().operation ) );
	core->device = std::move( device ).Value();
	core->deviceName = std::string( descriptor->id );
	core->frontend = render::legacy::CreateLegacyFrontend( config->legacyBackend );

	render::renderer::RendererDeps deps;
	deps.device = core->device.get();
	std::string_view features = config->features;
	while ( !features.empty() )
	{
		const std::size_t comma = features.find( ',' );
		const std::string_view name = features.substr( 0, comma );
		features =
		    comma == std::string_view::npos ? std::string_view() : features.substr( comma + 1 );
		if ( name.empty() )
			continue;
		std::unique_ptr<render::frame::IRenderFeature> feature =
		    RenderCore_CreateFeature( name, *core->frontend );
		if ( !feature )
			return Fail( result, RENDER_CORE_UNKNOWN_FEATURE,
			    "render feature '" + std::string( name ) + "' is not linked in this product" );
		deps.features.push_back( std::move( feature ) );
	}
	// The programs cl_render_debug_view_program may name: the world pass's
	// (RFC 0014), through the one resolver.
	for ( std::string_view program : render::material::ProgramResolver::ProgramNames() )
		deps.debugPrograms.emplace_back( program );
	auto renderer = render::renderer::CreateRenderer( std::move( deps ) );
	if ( !renderer )
		return Fail( result, RENDER_CORE_MISSING_CAPABILITY,
		    std::string( "render feature '" ) +
		        ( renderer.Error().feature ? renderer.Error().feature : "?" ) + "' requires " +
		        render::device::CapabilityName( renderer.Error().capability ) + ", which device '" +
		        core->deviceName + "' lacks" );
	core->renderer = std::move( renderer ).Value();
	core->frontend->BindRenderer( core->renderer.get() );

	core->binding.device = core->device.get();
	core->binding.renderer = core->renderer.get();
	core->binding.sceneFactory.create = &render::scene::CreateRenderScene;
	core->binding.sceneFactory.makeView = &render::scene::MakeView;
	core->binding.sceneFactory.buildDrawList = &render::scene::BuildDrawList;
	core->binding.sceneFactory.context = core.get();
	core->binding.sceneFactory.buildDrawListPooled = &BuildDrawListPooledOnCore;
	core->binding.stageMarkers = core->frontend->Markers();
	core->binding.materialBlocks = core->frontend->MaterialBlocks();
	core->binding.capabilities = core->frontend->Capabilities();
	const std::string_view probe = config->corePasses ? config->corePasses : "";
	if ( probe == "empty" )
		core->frontend->SetCorePassProbe( render::legacy::CorePassProbe::kEmpty );
	else if ( probe == "seeded-clear" )
		core->frontend->SetCorePassProbe( render::legacy::CorePassProbe::kSeededClear );
	else if ( !probe.empty() )
		return Fail( result, RENDER_CORE_INVALID_CONFIG,
		    "core passes '" + std::string( probe ) + "' are not empty or seeded-clear" );
	core->binding.corePasses = core->frontend->CorePasses();
	core->world =
	    std::make_unique<render::composition::CoreWorld>( *core->frontend, *core->renderer );
	core->renderer->AddStageHooks( core->world.get() );
	core->frontend->SetForwardedRecorder( core->world.get() );
	core->binding.world = core->world.get();
	core->binding.deviceName = core->deviceName.c_str();
	return core.release();
}

extern "C" void RenderCore_Destroy( RenderCore *core )
{
	delete core;
}

extern "C" const RenderCoreBinding *RenderCore_GetBinding( const RenderCore *core )
{
	return core ? &core->binding : nullptr;
}

extern "C" const render::LegacyShaderProvider *RenderCore_GetLegacyProvider(
    const RenderCore *core )
{
	return core ? core->frontend->Provider() : nullptr;
}

extern "C" unsigned int RenderCore_GetLegacyProviderCreates( const RenderCore *core )
{
	return core ? core->frontend->ProviderCreates() : 0u;
}

extern "C" void RenderCore_BindRenderCallQueue(
    RenderCore *core, const render::legacy::RenderCallQueueHost *host )
{
	if ( core )
	{
		core->frontend->BindRenderCallQueue( host );
		core->world->BindHost( host );
	}
}
