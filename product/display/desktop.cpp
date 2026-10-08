//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.display.desktop: user, none and private sessions.
//
//=============================================================================//

#include "product/display_desktop.h"

#include <cstdio>
#include <fstream>

namespace product
{

namespace
{

namespace fs = std::filesystem;

// D-Bus services a private session must never start: each owns state in the
// shared runtime directory.
constexpr const char *kBlockedServices[] = { "org.freedesktop.portal.Documents" };

bool Cancelled( const DisplayRequest &request )
{
	return request.cancel && request.cancel->IsCancelled();
}

class UserSession final : public IDisplaySession
{
public:
	std::string_view Name() const noexcept override { return "user"; }
	bool ClaimsIsolation() const noexcept override { return false; }
	foundation::Expected<DisplayEnvironment, ProviderError> Open(
	    const DisplayRequest &request ) override
	{
		if ( Cancelled( request ) )
			return foundation::MakeUnexpected( ProviderError{ std::string( kCancelled ), "" } );
		m_Open = true;
		return DisplayEnvironment{};
	}
	void Close() noexcept override { m_Open = false; }
	bool IsOpen() const noexcept override { return m_Open; }

private:
	bool m_Open = false;
};

class HeadlessSession final : public IDisplaySession
{
public:
	std::string_view Name() const noexcept override { return "none"; }
	bool ClaimsIsolation() const noexcept override { return true; }
	foundation::Expected<DisplayEnvironment, ProviderError> Open(
	    const DisplayRequest &request ) override
	{
		if ( Cancelled( request ) )
			return foundation::MakeUnexpected( ProviderError{ std::string( kCancelled ), "" } );
		m_Open = true;
		DisplayEnvironment environment;
		environment.environment = { { "SDL_VIDEODRIVER", std::string( "offscreen" ) },
		    { "SDL_VIDEO_DRIVER", std::string( "offscreen" ) }, { "DISPLAY", std::nullopt },
		    { "WAYLAND_DISPLAY", std::nullopt } };
		environment.isolated = true;
		environment.headless = true;
		return environment;
	}
	void Close() noexcept override { m_Open = false; }
	bool IsOpen() const noexcept override { return m_Open; }

private:
	bool m_Open = false;
};

class PrivateSession final : public IDisplaySession
{
public:
	explicit PrivateSession( std::vector<fs::path> configs ) : m_Configs( std::move( configs ) ) {}
	std::string_view Name() const noexcept override { return "private"; }
	bool ClaimsIsolation() const noexcept override { return true; }
	foundation::Expected<DisplayEnvironment, ProviderError> Open(
	    const DisplayRequest &request ) override
	{
		if ( Cancelled( request ) )
			return foundation::MakeUnexpected( ProviderError{ std::string( kCancelled ), "" } );
		std::error_code ec;
		fs::path system;
		for ( const fs::path &config : m_Configs )
		{
			if ( fs::is_regular_file( config, ec ) )
			{
				system = config;
				break;
			}
		}
		if ( system.empty() )
			return foundation::MakeUnexpected( ProviderError{
			    std::string( kUnavailable ), "no D-Bus session.conf for a private bus" } );
		if ( request.scratch.empty() )
			return foundation::MakeUnexpected(
			    ProviderError{ "invalid-request", "a private session needs a scratch directory" } );
		const fs::path scratch = fs::absolute( request.scratch, ec );
		const fs::path services = scratch / "services";
		fs::create_directories( services, ec );
		for ( const char *name : kBlockedServices )
		{
			// The first <servicedir> wins, so this shadows the system file; the
			// activation fails and nothing claims the shared mount.
			std::ofstream( services / ( std::string( name ) + ".service" ) )
			    << "[D-BUS Service]\nName=" << name << "\nExec=/bin/false\n";
		}
		const fs::path config = scratch / "session.conf";
		std::ofstream( config )
		    << "<!DOCTYPE busconfig PUBLIC \"-//freedesktop//DTD D-Bus Bus Configuration "
		       "1.0//EN\"\n"
		       " \"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">\n"
		       "<busconfig>\n"
		    << "  <servicedir>" << services.string() << "</servicedir>\n"
		    << "  <include>" << system.string() << "</include>\n"
		    << "</busconfig>\n";
		char monitor[64];
		std::snprintf( monitor, sizeof( monitor ), "%dx%d@%g", request.width, request.height,
		    request.refreshHz );
		DisplayEnvironment environment;
		environment.commandPrefix = { "dbus-run-session", "--config-file=" + config.string(), "--",
		    "mutter", "--headless", "--virtual-monitor", monitor, "--wayland-display",
		    "kiln-" + scratch.filename().string(), "--" };
		environment.environment = { { "DISPLAY", std::nullopt },
		    { "WAYLAND_DISPLAY", std::nullopt }, { "SDL_VIDEODRIVER", std::string( "wayland" ) },
		    { "SDL_VIDEO_DRIVER", std::string( "wayland" ) } };
		environment.isolated = true;
		m_Open = true;
		return environment;
	}
	void Close() noexcept override { m_Open = false; }
	bool IsOpen() const noexcept override { return m_Open; }

private:
	std::vector<fs::path> m_Configs;
	bool m_Open = false;
};

} // namespace

std::unique_ptr<IDisplaySession> CreateUserDisplaySession()
{
	return std::make_unique<UserSession>();
}

std::unique_ptr<IDisplaySession> CreateHeadlessDisplaySession()
{
	return std::make_unique<HeadlessSession>();
}

std::unique_ptr<IDisplaySession> CreatePrivateDisplaySession( std::vector<fs::path> sessionConfigs )
{
	return std::make_unique<PrivateSession>( std::move( sessionConfigs ) );
}

} // namespace product
