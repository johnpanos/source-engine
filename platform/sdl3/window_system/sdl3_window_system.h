//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDL3 window/input provider (RFC 0001 rank 7, roadmap R14). Exports
//			the platform.window.v1 capabilities over SDL3: IWindowSystem,
//			IEventSource, ICursor, IClipboard, IMessageBox and IGamepads.
//			Native events are decoded here and normalized by the shared
//			platform/window InputNormalizer, so this provider and the headless
//			one apply the same rules. Surfaces are platform/sdl3/render_surface
//			objects, so the SDL3 presentation bridges (sdl3-vulkan)
//			present to this provider's windows through RenderSurfaces().
//
//			SDL keeps process-global state, so at most one instance may be
//			connected at a time; a second Connect fails structurally. No SDL
//			type appears in this header. NativeWindowId and RenderSurfaces are
//			the private endpoints for native tests and SDL3 bridges.
//
//=============================================================================//

#ifndef PLATFORM_SDL3_WINDOW_SYSTEM_H
#define PLATFORM_SDL3_WINDOW_SYSTEM_H

#include "../../window/input_normalizer.h"
#include "platform/composition.h"
#include "platform/window/window_system.h"

#include <cstdint>
#include <memory>

namespace platform_sdl3
{

class Sdl3RenderSurfaces;

struct Sdl3WindowSystemConfig
{
	platform::window::NormalizerPolicy policy;
	bool gamepads = true; // initialize SDL's game-controller subsystem
	// Gamepad input while none of the process's windows has focus. Off is the
	// SDL3 launcher's behavior (SDL's default): a game ignores the pad while
	// the user is in another application.
	bool gamepadsWithoutFocus = false;
};

class Sdl3WindowSystem final : public platform::IProviderLifecycle,
                               public platform::window::IWindowSystem,
                               public platform::window::IEventSource,
                               public platform::window::ICursor,
                               public platform::window::IClipboard,
                               public platform::window::IMessageBox,
                               public platform::window::IGamepads
{
public:
	explicit Sdl3WindowSystem( Sdl3WindowSystemConfig config = Sdl3WindowSystemConfig{} );
	~Sdl3WindowSystem() override;

	Sdl3WindowSystem( const Sdl3WindowSystem & ) = delete;
	Sdl3WindowSystem &operator=( const Sdl3WindowSystem & ) = delete;

	// IProviderLifecycle
	foundation::Expected<void, platform::ProviderError> Connect() override;
	foundation::Expected<void, platform::ProviderError> Initialize() override;
	void Shutdown() noexcept override;
	void Disconnect() noexcept override;

	// IWindowSystem
	foundation::Expected<platform::window::WindowId, platform::window::WindowError> Create(
	    const platform::window::WindowDesc &desc ) override;
	foundation::Expected<void, platform::window::WindowError> Destroy(
	    platform::window::WindowId window ) override;
	foundation::Expected<platform::window::SizeEvent, platform::window::WindowError> GetSize(
	    platform::window::WindowId window ) const override;
	foundation::Expected<platform::window::SizeEvent, platform::window::WindowError> GetPixelSize(
	    platform::window::WindowId window ) const override;
	foundation::Expected<void, platform::window::WindowError> SetSize(
	    platform::window::WindowId window, std::int32_t width, std::int32_t height ) override;
	foundation::Expected<void, platform::window::WindowError> SetTitle(
	    platform::window::WindowId window, std::string_view title ) override;
	render::IRenderSurface *Surface( platform::window::WindowId window ) override;

	// IEventSource
	std::size_t Poll( std::span<platform::window::Event> out ) override;
	bool Wait( std::uint32_t timeoutMs ) override;

	// ICursor
	foundation::Expected<void, platform::window::WindowError> SetVisible( bool visible ) override;
	bool IsVisible() const override;
	foundation::Expected<void, platform::window::WindowError> SetRelativeMode(
	    platform::window::WindowId window, bool enabled ) override;
	foundation::Expected<void, platform::window::WindowError> Warp(
	    platform::window::WindowId window, std::int32_t x, std::int32_t y ) override;
	foundation::Expected<void, platform::window::WindowError> SetShape(
	    platform::window::CursorShape shape ) override;

	// IClipboard
	foundation::Expected<void, platform::window::WindowError> SetText(
	    std::string_view utf8 ) override;
	foundation::Expected<std::string, platform::window::WindowError> GetText() override;

	// IMessageBox
	foundation::Expected<void, platform::window::WindowError> ShowError( std::string_view title,
	    std::string_view message, platform::window::WindowId parent ) override;

	// IGamepads
	std::size_t ConnectedCount() const override;
	foundation::Expected<void, platform::window::WindowError> Rumble( std::uint32_t instance,
	    float lowFrequency, float highFrequency, std::uint32_t durationMs ) override;

	// Private endpoint: the SDL window ID behind a live window, or 0.
	std::uint32_t NativeWindowId( platform::window::WindowId window ) const;

	// Private endpoint for SDL3 presentation bridges: the registry that owns this
	// provider's surfaces. Valid for the provider's lifetime.
	Sdl3RenderSurfaces &RenderSurfaces();

private:
	struct State;
	std::unique_ptr<State> m_State;
};

} // namespace platform_sdl3

#endif // PLATFORM_SDL3_WINDOW_SYSTEM_H
