//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's presenting host (RFC 0016 K11); see lab_app.h.
//
//=============================================================================//

#include "lab_app.h"

#include "../../bridge/sdl3-vulkan/sdl3_vulkan_presentation.h"
#include "../../device/vulkan/backend_v1/render_backend_v1.h"
#include "../../device/vulkan/host_device.h"
#include "../../../platform/sdl3/render_surface/sdl3_render_surfaces.h"
#include "render/device/device.h"
#include "render/graph/executor.h"
#include "render/pass/output/output.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <span>
#include <thread>

namespace render::lab::app
{

namespace
{

// IEEE 754 binary16 of a finite value, rounded to nearest even.
std::uint16_t ToHalf( float value )
{
	std::uint32_t bits;
	std::memcpy( &bits, &value, sizeof( bits ) );
	const std::uint32_t sign = ( bits >> 16 ) & 0x8000u;
	const std::uint32_t magnitude = bits & 0x7fffffffu;
	if ( magnitude >= 0x477ff000u ) // 65520 and above: infinity
		return std::uint16_t( sign | 0x7c00u );
	if ( magnitude < 0x38800000u ) // below 2^-14: a subnormal half
	{
		float scaled;
		const std::uint32_t absBits = magnitude;
		std::memcpy( &scaled, &absBits, sizeof( scaled ) );
		return std::uint16_t( sign | std::uint32_t( std::nearbyint( scaled * 16777216.0f ) ) );
	}
	const std::uint32_t rounded = magnitude - 0x38000000u + 0xfffu + ( ( magnitude >> 13 ) & 1u );
	return std::uint16_t( sign | ( rounded >> 13 ) );
}

float FromHalf( std::uint16_t half )
{
	const int exponent = ( half >> 10 ) & 31;
	const int mantissa = half & 1023;
	const float value = exponent == 0 ? std::ldexp( float( mantissa ), -24 )
	                                  : std::ldexp( float( mantissa | 1024 ), exponent - 25 );
	return ( half & 0x8000 ) ? -value : value;
}

float Stored( float value )
{
	return FromHalf( ToHalf( value ) );
}

constexpr float kPatchValues[] = { 0.18f, 0.5f, 1.0f, 1.5f, 2.0f, 4.0f, 8.0f, 16.0f };
constexpr std::array<float, 3> kWarm = { 1.0f, 0.6f, 0.3f };
constexpr std::array<std::array<float, 3>, 6> kSaturated = {
    { { 4, 0, 0 }, { 0, 4, 0 }, { 0, 0, 4 }, { 4, 4, 0 }, { 0, 4, 4 }, { 4, 0, 4 } } };

// The chart before half rounding: four bands of rows.
std::array<float, 3> ChartValue( float u, float v )
{
	if ( v < 0.25f )
	{
		const float gray = std::exp2( -6.0f + 10.0f * u ); // 1/64 to 16
		return { gray, gray, gray };
	}
	if ( v < 0.5f )
	{
		const float gray = kPatchValues[std::min( int( u * 8.0f ), 7 )];
		return { gray, gray, gray };
	}
	if ( v < 0.75f )
		return kSaturated[std::min( int( u * 6.0f ), 5 )];
	const float warm = kPatchValues[std::min( int( u * 8.0f ), 7 )];
	return { warm * kWarm[0], warm * kWarm[1], warm * kWarm[2] };
}

void WaitComplete( device::IRenderDevice2 &port, device::CompletionToken token )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	while ( !port.IsComplete( token ) && std::chrono::steady_clock::now() < deadline )
	{
		(void)port.Poll();
		std::this_thread::yield();
	}
	(void)port.Poll();
}

} // namespace

std::array<float, 3> ChartTexel(
    std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height )
{
	const std::array<float, 3> value = ChartValue(
	    ( float( x ) + 0.5f ) / float( width ), ( float( y ) + 0.5f ) / float( height ) );
	return { Stored( value[0] ), Stored( value[1] ), Stored( value[2] ) };
}

std::vector<ChartPatch> ChartPatches()
{
	static const char *const kGrayNames[] = {
	    "gray-0.18", "gray-0.5", "gray-1", "gray-1.5", "gray-2", "gray-4", "gray-8", "gray-16" };
	static const char *const kWarmNames[] = {
	    "warm-0.18", "warm-0.5", "warm-1", "warm-1.5", "warm-2", "warm-4", "warm-8", "warm-16" };
	static const char *const kSaturatedNames[] = {
	    "red-4", "green-4", "blue-4", "yellow-4", "cyan-4", "magenta-4" };
	std::vector<ChartPatch> patches;
	for ( int i = 0; i < 8; ++i )
	{
		const float u = ( float( i ) + 0.5f ) / 8.0f;
		const std::array<float, 3> gray = ChartValue( u, 0.375f );
		const std::array<float, 3> warm = ChartValue( u, 0.875f );
		patches.push_back( { kGrayNames[i], u, 0.375f,
		    { Stored( gray[0] ), Stored( gray[1] ), Stored( gray[2] ) } } );
		patches.push_back( { kWarmNames[i], u, 0.875f,
		    { Stored( warm[0] ), Stored( warm[1] ), Stored( warm[2] ) } } );
	}
	for ( int i = 0; i < 6; ++i )
	{
		const float u = ( float( i ) + 0.5f ) / 6.0f;
		patches.push_back( { kSaturatedNames[i], u, 0.625f, kSaturated[i] } );
	}
	return patches;
}

std::array<float, 3> LabCapture::At( float u, float v ) const
{
	const std::uint32_t x = std::min( width - 1, std::uint32_t( u * float( width ) ) );
	const std::uint32_t y = std::min( height - 1, std::uint32_t( v * float( height ) ) );
	const std::size_t i = ( std::size_t( y ) * width + x ) * 4;
	if ( !linear.empty() )
		return { linear[i], linear[i + 1], linear[i + 2] };
	if ( !rgba8.empty() )
		return { float( rgba8[i] ), float( rgba8[i + 1] ), float( rgba8[i + 2] ) };
	return { -1.0f, -1.0f, -1.0f };
}

struct LabApp::State
{
	std::unique_ptr<render_vulkan::VulkanRenderBackend> provider;
	std::unique_ptr<platform_sdl3::Sdl3RenderSurfaces> surfaces;
	std::unique_ptr<render_vulkan::Sdl3VulkanPresentationBridge> bridge;
	IRenderDevice *device = nullptr;
	render_vulkan::VulkanDeviceEndpoint *endpoint = nullptr;
	device::IRenderDevice2 *port = nullptr;
	SDL_Window *window = nullptr;
	IRenderSurface *surface = nullptr;
	IRenderPresentation *presentation = nullptr;
	RenderDynamicRange range = RenderDynamicRange::kStandard;
	device::Format targetFormat = device::Format::kUnknown;
	std::unique_ptr<pass::output::OutputRenderer> output;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	device::TextureId scene;
	device::TextureDesc sceneDesc;
	// The back buffer imported as a port texture, and the handle it came from.
	RenderResourceHandle importedHandle = kInvalidResource;
	device::TextureId imported;
	device::TextureDesc importedDesc;
	device::CompletionToken lastToken;
	bool closing = false;

	~State()
	{
		if ( port )
		{
			output.reset();
			if ( imported.IsValid() )
				(void)port->Release( imported, lastToken );
			if ( scene.IsValid() )
				(void)port->Release( scene, lastToken );
			(void)port->WaitIdle();
			(void)port->Poll();
		}
		if ( presentation )
			bridge->DestroyPresentation( presentation );
		if ( device )
			while ( device->PollCompletion() > 0 )
			{
			}
		// The window goes after its surface releases every native object on it.
		if ( surface )
			surfaces->InvalidateWindow( *surface );
		if ( window )
			SDL_DestroyWindow( window );
		if ( surface )
			surfaces->Destroy( surface );
		if ( device )
		{
			bridge->ReleaseDevice( *device );
			provider->DestroyDevice( device );
		}
		bridge.reset();
		surfaces.reset();
		provider.reset();
	}

	bool UploadScene( std::string *error )
	{
		sceneDesc.format = device::Format::kRGBA16Float;
		sceneDesc.width = width;
		sceneDesc.height = height;
		sceneDesc.usages = {
		    device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
		sceneDesc.debugName = "render_lab.chart";
		auto texture = port->CreateTexture( sceneDesc );
		std::vector<std::uint16_t> texels( std::size_t( width ) * height * 4 );
		for ( std::uint32_t y = 0; y < height; ++y )
			for ( std::uint32_t x = 0; x < width; ++x )
			{
				const std::array<float, 3> value =
				    ChartValue( ( float( x ) + 0.5f ) / float( width ),
				        ( float( y ) + 0.5f ) / float( height ) );
				std::uint16_t *out = &texels[( std::size_t( y ) * width + x ) * 4];
				out[0] = ToHalf( value[0] );
				out[1] = ToHalf( value[1] );
				out[2] = ToHalf( value[2] );
				out[3] = ToHalf( 1.0f );
			}
		device::BufferDesc stagingDesc;
		stagingDesc.size = texels.size() * sizeof( std::uint16_t );
		stagingDesc.usages = {
		    device::ResourceUsage::kCopyDestination, device::ResourceUsage::kCopySource };
		auto staging = port->CreateBuffer( stagingDesc );
		if ( !texture || !staging )
		{
			*error = "the chart's texture or staging buffer";
			return false;
		}
		scene = texture.Value();
		// The adapter's upload ring holds 4 MB: the chart is written in 1 MB
		// submissions, then copied in one.
		const std::span<const std::byte> bytes = std::as_bytes( std::span( texels ) );
		constexpr std::size_t kChunk = std::size_t( 1 ) << 20;
		for ( std::size_t offset = 0; offset < bytes.size(); offset += kChunk )
		{
			auto encoder = port->BeginEncoder( device::QueueKind::kGraphics );
			if ( !encoder )
			{
				*error = "an encoder for the chart";
				return false;
			}
			if ( offset == 0 )
				encoder.Value().TransitionBuffer( staging.Value(),
				    device::ResourceUsage::kUndefined, device::ResourceUsage::kCopyDestination );
			encoder.Value().WriteBuffer( staging.Value(), offset,
			    bytes.subspan( offset, std::min( kChunk, bytes.size() - offset ) ) );
			device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
			auto token = port->Submit( device::QueueKind::kGraphics, encoders, {} );
			if ( !token )
			{
				*error = "the chart's upload submission";
				return false;
			}
			WaitComplete( *port, token.Value() );
		}
		auto encoder = port->BeginEncoder( device::QueueKind::kGraphics );
		if ( !encoder )
		{
			*error = "an encoder for the chart copy";
			return false;
		}
		encoder.Value().TransitionBuffer( staging.Value(), device::ResourceUsage::kCopyDestination,
		    device::ResourceUsage::kCopySource );
		encoder.Value().TransitionTexture(
		    scene, device::ResourceUsage::kUndefined, device::ResourceUsage::kCopyDestination );
		encoder.Value().CopyBufferToTexture( staging.Value(), scene, { 0, 0, 0, width, height } );
		encoder.Value().TransitionTexture(
		    scene, device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled );
		device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
		auto token = port->Submit( device::QueueKind::kGraphics, encoders, {} );
		if ( !token )
		{
			*error = "the chart's copy submission";
			return false;
		}
		WaitComplete( *port, token.Value() );
		(void)port->Release( staging.Value(), token.Value() );
		return true;
	}

	// The back buffer behind 'handle' as a port texture (imported once).
	bool Import( RenderResourceHandle handle, std::string *error )
	{
		if ( handle == importedHandle && imported.IsValid() )
			return true;
		if ( imported.IsValid() )
			(void)port->Release( imported, lastToken );
		imported = {};
		importedHandle = kInvalidResource;
		importedDesc = {};
		importedDesc.format = targetFormat;
		importedDesc.width = width;
		importedDesc.height = height;
		importedDesc.usages = {
		    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kExternal };
		importedDesc.debugName = "render_lab.back-buffer";
		// The bridge keeps its back buffer in GENERAL, the layout kExternal
		// names; it blits from it after the port's writes.
		if ( !endpoint->Host().ImportImage( endpoint->Image( handle ), importedDesc,
		         device::ResourceUsage::kExternal, &imported ) )
		{
			*error = "the back buffer could not be imported as a port texture";
			return false;
		}
		importedHandle = handle;
		return true;
	}
};

LabApp::LabApp( std::unique_ptr<State> state ) : m_State( std::move( state ) )
{
}

LabApp::~LabApp() = default;

std::unique_ptr<LabApp> LabApp::Create( const LabAppOptions &options, std::string *error )
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		*error = std::string( "SDL_Init: " ) + SDL_GetError();
		return nullptr;
	}
	auto state = std::make_unique<State>();
	render_vulkan::VulkanProviderOptions providerOptions;
	if ( !render_vulkan::Sdl3VulkanInstanceExtensions(
	         &providerOptions.instanceExtensions, error ) )
		return nullptr;
	providerOptions.enableSwapchain = true;
	state->provider = render_vulkan::MakeVulkanRenderBackend( providerOptions, error );
	if ( !state->provider )
		return nullptr;
	state->surfaces = std::make_unique<platform_sdl3::Sdl3RenderSurfaces>();
	state->bridge = std::make_unique<render_vulkan::Sdl3VulkanPresentationBridge>(
	    *state->provider, *state->surfaces, 1 );
	RenderDeviceRequest request;
	state->device = state->provider->CreateDevice( request, nullptr );
	state->endpoint = state->device ? state->provider->FindDevice( *state->device ) : nullptr;
	if ( !state->endpoint )
	{
		*error = "no Vulkan device from the render.backend.v1 provider";
		return nullptr;
	}
	state->port = &state->endpoint->Host().Port();

	SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if ( options.fullscreen )
		flags |= SDL_WINDOW_FULLSCREEN;
	state->window = SDL_CreateWindow( "render_lab", 1280, 720, flags );
	if ( !state->window )
	{
		*error = std::string( "SDL_CreateWindow: " ) + SDL_GetError();
		return nullptr;
	}
	SDL_SyncWindow( state->window );
	state->surface = state->surfaces->Adopt( state->window );
	int pixelsWide = 0, pixelsHigh = 0;
	SDL_GetWindowSizeInPixels( state->window, &pixelsWide, &pixelsHigh );
	if ( !state->surface || pixelsWide <= 0 || pixelsHigh <= 0 )
	{
		*error = "no render surface for the window";
		return nullptr;
	}
	const float scale = std::min( 1.0f, float( options.maxWidth ) / float( pixelsWide ) );
	state->width = std::max( 1u, std::uint32_t( std::lround( pixelsWide * scale ) ) );
	state->height = std::max( 1u, std::uint32_t( std::lround( pixelsHigh * scale ) ) );

	RenderPresentationConfig config;
	config.extent = { state->width, state->height };
	RenderCreateError createError;
	if ( options.extended )
	{
		config.format = RenderColorFormat::kRGBA16Float;
		config.dynamicRange = RenderDynamicRange::kExtendedLinear;
		state->presentation = state->bridge->CreatePresentation(
		    *state->device, *state->surface, config, &createError );
		if ( state->presentation )
		{
			state->range = RenderDynamicRange::kExtendedLinear;
			state->targetFormat = device::Format::kRGBA16Float;
		}
		else if ( createError.status != RenderCreateStatus::kSurfaceIncompatible )
		{
			*error = std::string( "the extended-linear presentation: " ) + createError.message;
			return nullptr;
		}
	}
	if ( !state->presentation )
	{
		config.format = RenderColorFormat::kRGBA8Unorm;
		config.dynamicRange = RenderDynamicRange::kStandard;
		state->presentation = state->bridge->CreatePresentation(
		    *state->device, *state->surface, config, &createError );
		if ( !state->presentation )
		{
			*error = std::string( "the standard presentation: " ) + createError.message;
			return nullptr;
		}
		state->range = RenderDynamicRange::kStandard;
		state->targetFormat = device::Format::kRGBA8Unorm;
	}
	auto output = pass::output::OutputRenderer::Create( *state->port, state->targetFormat );
	if ( !output )
	{
		*error = "render.pass.output refused the back buffer's format";
		return nullptr;
	}
	state->output = std::move( output ).Value();
	if ( !state->UploadScene( error ) )
		return nullptr;
	return std::unique_ptr<LabApp>( new LabApp( std::move( state ) ) );
}

RenderDynamicRange LabApp::Range() const
{
	return m_State->range;
}

RenderDynamicRangeState LabApp::DynamicRange() const
{
	return m_State->presentation->GetDynamicRange();
}

std::uint32_t LabApp::SceneWidth() const
{
	return m_State->width;
}

std::uint32_t LabApp::SceneHeight() const
{
	return m_State->height;
}

bool LabApp::Pump()
{
	SDL_PumpEvents();
	SDL_Event event;
	while ( SDL_PollEvent( &event ) )
	{
		m_State->surfaces->HandleEvent( event );
		if ( event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED )
			m_State->closing = true;
	}
	return !m_State->closing;
}

bool LabApp::Frame( const LabFrame &frame, float *usedHeadroom, std::string *error )
{
	State &s = *m_State;
	for ( int attempt = 0; attempt < 6; ++attempt )
	{
		Pump();
		RenderResourceHandle back = kInvalidResource;
		const RenderPresentStatus begin = s.presentation->BeginFrame( &back );
		if ( std::getenv( "RENDER_LAB_TRACE" ) )
			std::fprintf( stderr, "render_lab: BeginFrame %u\n", unsigned( begin ) );
		if ( begin == RenderPresentStatus::kRecoverable ||
		     begin == RenderPresentStatus::kSuspended )
			continue;
		if ( begin != RenderPresentStatus::kOk )
		{
			*error = "BeginFrame failed";
			return false;
		}
		if ( !s.Import( back, error ) )
			return false;
		const float headroom =
		    s.range == RenderDynamicRange::kExtendedLinear
		        ? std::max( 1.0f, s.presentation->GetDynamicRange().currentHeadroom )
		        : 1.0f;
		if ( usedHeadroom )
			*usedHeadroom = headroom;

		graph::GraphBuilder builder;
		const graph::ResourceRef sceneRef = builder.ImportTexture( "scene", s.scene, s.sceneDesc,
		    device::ResourceUsage::kSampled, device::ResourceUsage::kSampled );
		const graph::ResourceRef targetRef = builder.ImportTexture( "back-buffer", s.imported,
		    s.importedDesc, device::ResourceUsage::kExternal, device::ResourceUsage::kExternal );
		pass::output::OutputTargets targets;
		targets.scene = sceneRef;
		targets.target = targetRef;
		targets.width = s.width;
		targets.height = s.height;
		pass::output::OutputParams params;
		params.exposure = frame.exposure;
		params.scenePeak = frame.scenePeak;
		params.headroom = headroom;
		params.toneMap = frame.toneMap;
		if ( !s.output->AddPass( builder, targets, params ) )
		{
			*error = "render.pass.output refused the frame";
			return false;
		}
		auto compiled = graph::CompileGraph( std::move( builder ) );
		if ( !compiled )
		{
			*error = "the frame graph did not compile";
			return false;
		}
		graph::SerialGraphExecutor executor;
		auto executed = executor.Execute( compiled.Value(), *s.port );
		if ( !executed )
		{
			*error = "the frame graph did not execute";
			return false;
		}
		s.lastToken = executed.Value().token;
		s.output->Collect( s.lastToken );
		(void)s.port->Poll();
		if ( frame.capture )
			s.bridge->RequestCapture( *s.presentation );
		const RenderPresentStatus status = s.presentation->Present();
		if ( std::getenv( "RENDER_LAB_TRACE" ) )
			std::fprintf( stderr, "render_lab: Present %u\n", unsigned( status ) );
		if ( status == RenderPresentStatus::kOk )
			return s.output->RecordFailures() == 0;
		if ( status != RenderPresentStatus::kRecoverable )
		{
			*error = "Present failed";
			return false;
		}
	}
	*error = "no frame presented in six attempts";
	return false;
}

bool LabApp::ReadCapture( LabCapture &out )
{
	State &s = *m_State;
	out = LabCapture();
	if ( s.range == RenderDynamicRange::kExtendedLinear )
		return s.bridge->ReadCaptureLinear( *s.presentation, &out.linear, &out.width, &out.height );
	return s.bridge->ReadCapture( *s.presentation, &out.rgba8, &out.width, &out.height );
}

bool LabApp::ReadLayer( bool *extended, bool *extendedLinear )
{
	return m_State->bridge->ReadNativeDynamicRange(
	    *m_State->presentation, extended, extendedLinear );
}

LabDisplayMode LabApp::ReadDisplayMode()
{
	LabDisplayMode mode;
	mode.known =
	    m_State->bridge->ReadNativeDisplayMode( *m_State->presentation, &mode.matchingEnabled,
	        &mode.switching, &mode.askedForHdr, &mode.hdrModes, &mode.eligibleForHdr );
	return mode;
}

} // namespace render::lab::app
