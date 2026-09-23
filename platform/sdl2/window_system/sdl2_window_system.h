//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDL2 window/input provider (RFC 0001 rank 7, roadmap R14). Exports
//			the platform.window.v1 capabilities over SDL2: IWindowSystem,
//			IEventSource, ICursor, IClipboard, IMessageBox and IGamepads.
//			Native events are decoded here and normalized by the shared
//			platform/window InputNormalizer, so this provider and the headless
//			one apply the same rules; surfaces come from the shared registry.
//
//			SDL keeps process-global state, so at most one instance may be
//			connected at a time; a second Connect fails structurally. No SDL
//			type appears in this header. NativeWindowId is the private endpoint
//			for this provider's native tests and future SDL2 bridges.
//
//=============================================================================//

#ifndef PLATFORM_SDL2_WINDOW_SYSTEM_H
#define PLATFORM_SDL2_WINDOW_SYSTEM_H

#include "../../window/input_normalizer.h"
#include "platform/composition.h"
#include "platform/window/window_system.h"

#include <cstdint>
#include <memory>

namespace platform_sdl2
{

struct Sdl2WindowSystemConfig
{
	platform::window::NormalizerPolicy policy;
	bool gamepads = true; // initialize SDL's game-controller subsystem
};

class Sdl2WindowSystem final : public platform::IProviderLifecycle,
                               public platform::window::IWindowSystem,
                               public platform::window::IEventSource,
                               public platform::window::ICursor,
                               public platform::window::IClipboard,
                               public platform::window::IMessageBox,
                               public platform::window::IGamepads
{
public:
	explicit Sdl2WindowSystem( Sdl2WindowSystemConfig config = Sdl2WindowSystemConfig{} );
	~Sdl2WindowSystem() override;

	Sdl2WindowSystem( const Sdl2WindowSystem & ) = delete;
	Sdl2WindowSystem &operator=( const Sdl2WindowSystem & ) = delete;

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

private:
	struct State;
	std::unique_ptr<State> m_State;
};

} // namespace platform_sdl2

#endif // PLATFORM_SDL2_WINDOW_SYSTEM_H
