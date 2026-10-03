//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition (RFC 0016); see render_core.h.
//
//=============================================================================//

#include "render/composition/render_core.h"

#include "jobsystem/pooled_executor.h"
#include "render/device/null/provider.h"
#include "core_panels.h"
#include "core_world.h"
#include "render/legacy/core_backend.h"
#include "render/legacy/frame_source.h"
#include "render/pass/indirect/port_compute.h"
#include "render/pass/present/feature.h"
#include "render/material/program_resolver.h"
#include "render/renderer/renderer_factory.h"
#include "negotiation.h"
#if defined( RENDER_CORE_VULKAN )
#include "render/device/vulkan/provider.h"
#endif
#if defined( RENDER_CORE_GL )
#include "render/device/gl/provider.h"
#endif

#include <cstdio>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct RenderCore
{
	std::unique_ptr<render::device::IRenderDevice2> device;
	std::unique_ptr<render::legacy::ILegacyFrontend> frontend;
	// The BSP world drawn by the core; the frontend forwards its slots to it.
	std::unique_ptr<render::composition::CoreWorld> world;
	// In-world panels (render.pass.panels): their tags reach them through
	// the router, which sends every other forwarded slot to the world.
	std::unique_ptr<render::composition::CorePanels> panels;
	std::unique_ptr<render::composition::ForwardedSlots> forwarded;
	std::unique_ptr<render::frame::IRenderer> renderer;
	std::string deviceName;
	RenderCoreBinding binding;
	// Pooled culling over the root's compute workers (RenderCoreConfig::
	// computeWorkers); inline on the caller when the root gave none.
	std::unique_ptr<jobsystem::PooledExecutor> cullJobs;
	// The indirect-light producers' compute (render.pass.indirect), flushed
	// ahead of each legacy frame.
	std::unique_ptr<render::pass::indirect::PortCompute> compute;
	struct ComputeFlush final : render::legacy::ILegacyFrameWork
	{
		render::pass::indirect::PortCompute *compute = nullptr;
		void BeforeFrame( render::device::IRenderDevice2 &device ) override
		{
			(void)compute->Flush( device );
		}
	} computeFlush;
	bool computeBound = false;

	~RenderCore()
	{
		if ( computeBound )
			render::legacy::LegacyFrameExecutor().SetFrameWork( nullptr );
		// Borrowers first: the markers forward to the renderer, and the
		// renderer's frames reference the device.
		if ( frontend )
		{
			frontend->BindRenderer( nullptr );
			frontend->SetForwardedRecorder( nullptr );
		}
		if ( renderer && world )
			renderer->RemoveStageHooks( world.get() );
		forwarded.reset();
		panels.reset();
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
#if defined( RENDER_CORE_GL )
	    &render::device::gl::Describe(),
#endif
	};
	for ( const render::device::DeviceProviderDescriptor *descriptor : linked )
	{
		if ( descriptor->id == name )
			return descriptor;
	}
	return nullptr;
}

// The capabilities named in a comma-separated list; nullopt, with the unknown
// name, when one is not a capability.
std::optional<render::device::CapabilitySet> ParseCapabilities(
    std::string_view list, std::string &unknown )
{
	render::device::CapabilitySet set;
	while ( !list.empty() )
	{
		const std::size_t comma = list.find( ',' );
		const std::string_view name = list.substr( 0, comma );
		list = comma == std::string_view::npos ? std::string_view() : list.substr( comma + 1 );
		if ( name.empty() )
			continue;
		bool found = false;
		for ( std::uint32_t bit = 0;
		    bit < static_cast<std::uint32_t>( render::device::Capability::kCount ); ++bit )
		{
			const auto capability = static_cast<render::device::Capability>( bit );
			if ( name == render::device::CapabilityName( capability ) )
			{
				set.Add( capability );
				found = true;
			}
		}
		if ( !found )
		{
			unknown = std::string( name );
			return std::nullopt;
		}
	}
	return set;
}

// The device, with the profile's masked capabilities never claimed. Masking
// is an adapter option, so only adapters that offer it take a mask; another
// fails kUnsupported.
render::device::DeviceResult<std::unique_ptr<render::device::IRenderDevice2>> CreateDevice(
    const render::device::DeviceProviderDescriptor &descriptor,
    const render::device::DeviceRequest &request, render::device::CapabilitySet masked )
{
	if ( masked.Bits() == 0 )
		return descriptor.create( request );
	auto allow = [&]( render::device::CapabilitySet set )
	{
		for ( std::uint32_t bit = 0;
		    bit < static_cast<std::uint32_t>( render::device::Capability::kCount ); ++bit )
		{
			if ( masked.Has( static_cast<render::device::Capability>( bit ) ) )
				set.Remove( static_cast<render::device::Capability>( bit ) );
		}
		return set;
	};
	if ( descriptor.id == "null" )
	{
		render::device::null::NullOptions options;
		options.capabilities = allow( options.capabilities );
		return render::device::null::Create( options );
	}
#if defined( RENDER_CORE_GL )
	if ( descriptor.id == "gl" )
	{
		render::device::gl::GlAdapterOptions options;
		options.validation = request.validation;
		options.allowed = allow( options.allowed );
		return render::device::gl::Create( options );
	}
#endif
	return foundation::MakeUnexpected(
	    render::device::DeviceError{ render::device::DeviceStatus::kUnsupported,
	        render::device::DeviceOperation::kCreateDevice, 0 } );
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

	std::string unknownCapability;
	const std::optional<render::device::CapabilitySet> masked = ParseCapabilities(
	    config->maskedCapabilities ? config->maskedCapabilities : "", unknownCapability );
	if ( !masked )
		return Fail( result, RENDER_CORE_INVALID_CONFIG,
		    "masked capability '" + unknownCapability + "' is not a render device capability" );

	auto core = std::make_unique<RenderCore>();
	// The core starts no threads: compute work runs on the workers the root
	// lends it (RFC 0003 "capacity policy").
	core->cullJobs = std::make_unique<jobsystem::PooledExecutor>( config->computeWorkers );
	render::device::DeviceRequest request;
	request.validation = config->validation;
	auto device = CreateDevice( *descriptor, request, *masked );
	if ( !device )
		return Fail( result, RENDER_CORE_DEVICE_FAILED,
		    std::string( "render device '" ) + config->device + "' failed" +
		        ( masked->Bits() ? " with capabilities masked" : "" ) + ": " +
		        render::device::DescribeStatus( device.Error().status ) + " in " +
		        render::device::DescribeOperation( device.Error().operation ) );
	core->device = std::move( device ).Value();
	core->deviceName = std::string( descriptor->id );
	core->frontend = render::legacy::CreateLegacyFrontend( config->legacyBackend );

	render::renderer::RendererDeps deps;
	deps.device = core->device.get();
	// Capability negotiation (RFC 0016 K10): declared fallbacks only, each
	// reported by name.
	render::composition::Negotiation negotiated = render::composition::Negotiate( config->features,
	    config->fallbacks ? config->fallbacks : "", core->device->Facts().capabilities,
	    [&core]( std::string_view name )
	    {
		    return RenderCore_CreateFeature( name, *core->frontend );
	    } );
	switch ( negotiated.status )
	{
	case render::composition::NegotiationStatus::kOk:
		break;
	case render::composition::NegotiationStatus::kUnknownFeature:
		return Fail( result, RENDER_CORE_UNKNOWN_FEATURE, negotiated.message );
	case render::composition::NegotiationStatus::kMissingCapability:
		return Fail( result, RENDER_CORE_MISSING_CAPABILITY,
		    negotiated.message + " (device '" + core->deviceName + "')" );
	case render::composition::NegotiationStatus::kUndeclaredFallback:
		return Fail( result, RENDER_CORE_UNDECLARED_FALLBACK,
		    negotiated.message + " (device '" + core->deviceName + "')" );
	}
	deps.features = std::move( negotiated.features );
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
	// RFC 0016 K12: the indirect-light producers' GPU work runs on the core,
	// flushed on the render sequence ahead of each frame the legacy frame
	// executor runs; only the native Vulkan backend runs its frames there.
	if ( config->legacyBackend && config->legacyBackend->id &&
	     std::string_view( config->legacyBackend->id ) == "native-vulkan" )
	{
		core->compute = render::pass::indirect::PortCompute::Create();
		core->computeFlush.compute = core->compute.get();
		render::legacy::LegacyFrameExecutor().SetFrameWork( &core->computeFlush );
		core->computeBound = true;
		core->world->BindCompute( core->compute.get() );
		core->binding.gpuCompute = core->compute.get();
	}
	core->panels =
	    std::make_unique<render::composition::CorePanels>( *core->frontend, *core->renderer );
	core->forwarded =
	    std::make_unique<render::composition::ForwardedSlots>( *core->world, *core->panels );
	core->world->EnableTemporal( config->temporal, config->temporalAssets );
	core->binding.temporal.context = core->world.get();
	core->binding.temporal.setEnabled = []( void *context, bool enabled )
	{
		return static_cast<render::composition::CoreWorld *>( context )->SetTemporalEnabled(
		    enabled );
	};
	core->binding.temporal.available = []( const void *context )
	{
		return static_cast<const render::composition::CoreWorld *>( context )->TemporalAvailable();
	};
	core->renderer->AddStageHooks( core->world.get() );
	core->frontend->SetForwardedRecorder( core->forwarded.get() );
	core->binding.world = core->world.get();
	core->binding.panels = core->panels.get();
	core->binding.deviceName = core->deviceName.c_str();
	if ( result )
	{
		std::snprintf( result->substitutions, sizeof( result->substitutions ), "%s",
		    render::composition::DescribeSubstitutions( negotiated.substitutions ).c_str() );
		result->substitutionCount = static_cast<unsigned int>( negotiated.substitutions.size() );
	}
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
		core->panels->BindHost( host );
	}
}
