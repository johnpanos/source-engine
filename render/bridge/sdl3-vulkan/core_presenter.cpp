//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SDL3-Vulkan bridge's presentation for the desktop core shader
//			API (core_presenter.h).
//
//===========================================================================//

#include "core_presenter.h"

#include "sdl3_vulkan_presentation.h"
#include "../../device/vulkan/backend_v1/render_backend_v1.h"
#include "../../device/vulkan/host_device.h"
#include "../../../platform/sdl3/render_surface/sdl3_render_surfaces.h"
#include "render/device/device.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <mutex>
#include <string>
#include <vector>

namespace render_vulkan
{

namespace
{

using render::RenderPresentStatus;
using render::RenderResourceHandle;
using render::kInvalidResource;
namespace device = render::device;

void Copy( const std::string &message, char *error, std::size_t errorSize )
{
	if ( error && errorSize )
		std::snprintf( error, errorSize, "%s", message.c_str() );
}

class CorePresenter final : public ICorePresenter
{
public:
	~CorePresenter() override
	{
		if ( m_Watching )
			SDL_RemoveEventWatch( &CorePresenter::OnEvent, this );
		ReleaseWindow();
		if ( m_Device )
		{
			m_Bridge->ReleaseDevice( *m_Device );
			m_Provider->DestroyDevice( m_Device );
		}
		m_Bridge.reset();
		m_Surfaces.reset();
		m_Provider.reset();
		if ( m_Video )
			SDL_QuitSubSystem( SDL_INIT_VIDEO );
	}

	bool Create( bool validation, std::string *error )
	{
		if ( !SDL_InitSubSystem( SDL_INIT_VIDEO ) )
		{
			*error = std::string( "SDL video: " ) + SDL_GetError();
			return false;
		}
		m_Video = true;
		const char *driver = SDL_GetCurrentVideoDriver();
		if ( driver && !SDL_strcmp( driver, "offscreen" ) )
		{
			*error = "the offscreen video driver has no window system to present to";
			return false;
		}
		VulkanProviderOptions options;
		if ( !Sdl3VulkanInstanceExtensions( &options.instanceExtensions, error ) )
			return false;
		options.enableSwapchain = true;
		m_Provider = MakeVulkanRenderBackend( options, error );
		if ( !m_Provider )
			return false;
		m_Surfaces = std::make_unique<platform_sdl3::Sdl3RenderSurfaces>();
		m_Bridge = std::make_unique<Sdl3VulkanPresentationBridge>( *m_Provider, *m_Surfaces, 1 );
		render::RenderDeviceRequest request;
		request.enableValidation = validation;
		m_Device = m_Provider->CreateDevice( request, nullptr );
		m_Endpoint = m_Device ? m_Provider->FindDevice( *m_Device ) : nullptr;
		if ( !m_Endpoint )
		{
			*error = "no Vulkan device from the render.backend.v1 provider";
			return false;
		}
		m_Port = &m_Endpoint->Host().Port();
		return true;
	}

	device::IRenderDevice2 &Port() override { return *m_Port; }

	bool PresentFrame( void *legacyWindowRef, std::uint32_t width, std::uint32_t height, bool vsync,
	    Record record, void *user, device::CompletionToken *submitted, char *error,
	    std::size_t errorSize ) override
	{
		Copy( "", error, errorSize );
		if ( !legacyWindowRef || width == 0 || height == 0 || !record )
			return false;
		std::lock_guard<std::mutex> lock( m_Lock );
		if ( m_Closed )
			return false; // the window is gone
		std::string reason;
		if ( !Open( static_cast<SDL_Window *>( legacyWindowRef ), width, height, vsync, &reason ) )
		{
			Copy( reason, error, errorSize );
			return false;
		}
		RenderResourceHandle back = kInvalidResource;
		const RenderPresentStatus begin = m_Presentation->BeginFrame( &back );
		if ( begin == RenderPresentStatus::kSuspended ||
		     begin == RenderPresentStatus::kRecoverable )
			return false;
		if ( begin != RenderPresentStatus::kOk )
		{
			Copy( "the presentation could not open a frame", error, errorSize );
			return false;
		}
		if ( !Import( back, &reason ) )
		{
			(void)m_Presentation->CancelFrame();
			Copy( reason, error, errorSize );
			return false;
		}
		auto encoder = m_Port->BeginEncoder( device::QueueKind::kGraphics );
		Target target;
		target.texture = m_Imported;
		target.format = kFormat;
		target.usage = device::ResourceUsage::kExternal;
		target.width = width;
		target.height = height;
		if ( !encoder || !record( user, encoder.Value(), target ) )
		{
			(void)m_Presentation->CancelFrame();
			Copy( "the frame's copy was not recorded", error, errorSize );
			return false;
		}
		auto token = m_Port->Submit( device::QueueKind::kGraphics, { &encoder.Value(), 1 }, {} );
		if ( !token )
		{
			(void)m_Presentation->CancelFrame();
			Copy( "the frame's copy was not submitted", error, errorSize );
			return false;
		}
		m_LastToken = token.Value();
		if ( submitted )
			*submitted = m_LastToken;
		(void)m_Port->Poll();
		const bool capture = !m_CapturePath.empty() && m_Bridge->RequestCapture( *m_Presentation );
		const RenderPresentStatus present = m_Presentation->Present();
		if ( capture )
			WriteCapture();
		if ( present != RenderPresentStatus::kOk && present != RenderPresentStatus::kRecoverable )
		{
			Copy( "the presentation refused the frame", error, errorSize );
			return false;
		}
		return present == RenderPresentStatus::kOk;
	}

	bool CaptureNextPresented( const char *path ) override
	{
		std::lock_guard<std::mutex> lock( m_Lock );
		if ( !path || !*path )
			return false;
		m_CapturePath = path;
		return true;
	}

private:
	static constexpr device::Format kFormat = device::Format::kRGBA8Unorm;

	void WriteCapture()
	{
		std::vector<uint8_t> rgba;
		std::uint32_t width = 0, height = 0;
		const bool read = m_Bridge->ReadCapture( *m_Presentation, &rgba, &width, &height );
		FILE *file = read ? std::fopen( m_CapturePath.c_str(), "wb" ) : nullptr;
		std::printf( "core presenter: capture %s %s (%ux%u)\n", m_CapturePath.c_str(),
		    file ? "ok" : "failed", width, height );
		m_CapturePath.clear();
		if ( !file )
			return;
		std::fprintf( file, "P6\n%u %u\n255\n", width, height );
		for ( std::size_t i = 0; i < std::size_t( width ) * height; ++i )
			std::fwrite( &rgba[i * 4], 1, 3, file );
		std::fclose( file );
	}

	// SDL sends SDL_EVENT_WINDOW_DESTROYED to watches before the window
	// goes: its presentation and surface release every native object on it
	// then (the engine destroys its window before the root destroys this).
	static bool SDLCALL OnEvent( void *user, SDL_Event *event )
	{
		CorePresenter *self = static_cast<CorePresenter *>( user );
		if ( event->type == SDL_EVENT_WINDOW_DESTROYED && self->m_Window &&
		     event->window.windowID == SDL_GetWindowID( self->m_Window ) )
			self->ReleaseWindow();
		return true;
	}

	void ReleaseWindow()
	{
		std::lock_guard<std::mutex> lock( m_Lock );
		if ( m_Port )
		{
			if ( m_Imported.IsValid() )
				(void)m_Port->Release( m_Imported, m_LastToken );
			m_Imported = {};
			m_ImportedHandle = kInvalidResource;
			(void)m_Port->WaitIdle(); // reviewed idle wait: the window's teardown
			(void)m_Port->Poll();
		}
		if ( m_Presentation )
			m_Bridge->DestroyPresentation( m_Presentation );
		m_Presentation = nullptr;
		if ( m_Surface )
		{
			m_Surfaces->InvalidateWindow( *m_Surface );
			m_Surfaces->Destroy( m_Surface );
		}
		m_Surface = nullptr;
		m_Window = nullptr;
		m_Closed = true;
	}

	// The presentation of the window at the frame's size: made once, resized
	// when the frame's size changes (the bridge scales it to the window).
	bool Open( SDL_Window *window, std::uint32_t width, std::uint32_t height, bool vsync,
	    std::string *error )
	{
		if ( m_Window && window != m_Window )
		{
			*error = "the engine's window changed under its presentation";
			return false;
		}
		if ( m_Presentation && vsync != m_Vsync )
		{
			// The present mode is fixed per presentation: it is made again.
			if ( m_Imported.IsValid() )
				(void)m_Port->Release( m_Imported, m_LastToken );
			m_Imported = {};
			m_ImportedHandle = kInvalidResource;
			(void)m_Port->WaitIdle(); // reviewed idle wait: a present-mode change
			m_Bridge->DestroyPresentation( m_Presentation );
			m_Presentation = nullptr;
		}
		if ( !m_Presentation )
		{
			if ( !m_Watching )
				m_Watching = SDL_AddEventWatch( &CorePresenter::OnEvent, this );
			m_Window = window;
			if ( !m_Surface )
				m_Surface = m_Surfaces->Adopt( window );
			if ( !m_Surface )
			{
				*error = "no render surface for the engine's window";
				return false;
			}
			render::RenderPresentationConfig config;
			config.extent = { width, height };
			config.format = render::RenderColorFormat::kRGBA8Unorm;
			config.vsync = vsync;
			m_Vsync = vsync;
			config.dynamicRange = render::RenderDynamicRange::kStandard;
			render::RenderCreateError createError;
			m_Presentation =
			    m_Bridge->CreatePresentation( *m_Device, *m_Surface, config, &createError );
			if ( !m_Presentation )
			{
				*error = std::string( "the presentation: " ) + createError.message;
				return false;
			}
		}
		const render::RenderExtent extent = m_Presentation->GetExtent();
		if ( extent.width != width || extent.height != height )
		{
			if ( !m_Presentation->ResizeTo( { width, height } ) )
			{
				*error = "the presentation could not resize";
				return false;
			}
			if ( m_Imported.IsValid() )
				(void)m_Port->Release( m_Imported, m_LastToken );
			m_Imported = {};
			m_ImportedHandle = kInvalidResource;
		}
		m_Width = width;
		m_Height = height;
		return true;
	}

	// The back buffer as a port texture: imported once per back buffer.
	bool Import( RenderResourceHandle handle, std::string *error )
	{
		if ( handle == m_ImportedHandle && m_Imported.IsValid() )
			return true;
		if ( m_Imported.IsValid() )
			(void)m_Port->Release( m_Imported, m_LastToken );
		m_Imported = {};
		m_ImportedHandle = kInvalidResource;
		device::TextureDesc desc;
		desc.format = kFormat;
		desc.width = m_Width;
		desc.height = m_Height;
		desc.usages = { device::ResourceUsage::kColorAttachment, device::ResourceUsage::kExternal };
		desc.debugName = "core-presenter.back-buffer";
		// The bridge keeps its back buffer in GENERAL, the layout kExternal
		// names, and blits from it after the port's writes.
		if ( !m_Endpoint->Host().ImportImage( m_Endpoint->Image( handle ), desc,
		         device::ResourceUsage::kExternal, &m_Imported ) )
		{
			*error = "the back buffer could not be imported as a port texture";
			return false;
		}
		m_ImportedHandle = handle;
		return true;
	}

	bool m_Video = false;
	std::mutex m_Lock;
	bool m_Watching = false;
	bool m_Closed = false;
	std::unique_ptr<VulkanRenderBackend> m_Provider;
	std::unique_ptr<platform_sdl3::Sdl3RenderSurfaces> m_Surfaces;
	std::unique_ptr<Sdl3VulkanPresentationBridge> m_Bridge;
	render::IRenderDevice *m_Device = nullptr;
	VulkanDeviceEndpoint *m_Endpoint = nullptr;
	device::IRenderDevice2 *m_Port = nullptr;
	SDL_Window *m_Window = nullptr;
	render::IRenderSurface *m_Surface = nullptr;
	render::IRenderPresentation *m_Presentation = nullptr;
	bool m_Vsync = true;
	std::string m_CapturePath;
	std::uint32_t m_Width = 0;
	std::uint32_t m_Height = 0;
	RenderResourceHandle m_ImportedHandle = kInvalidResource;
	device::TextureId m_Imported;
	device::CompletionToken m_LastToken;
};

} // namespace

std::unique_ptr<ICorePresenter> CreateSdl3CorePresenter(
    bool validation, char *error, std::size_t errorSize )
{
	auto presenter = std::make_unique<CorePresenter>();
	std::string reason;
	if ( !presenter->Create( validation, &reason ) )
	{
		Copy( reason, error, errorSize );
		return nullptr;
	}
	return presenter;
}

} // namespace render_vulkan
