//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SDL3-Vulkan bridge's presentation state for the legacy
//			backend's window; see legacy_presentation.h.
//
//=============================================================================//

#include "legacy_presentation.h"

#include "sdl3_vulkan_surface_host.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace render_vulkan
{
namespace
{

class Sdl3LegacyPresentation final : public ILegacyPresentation
{
public:
	IVulkanSurfaceHost *Open( void *legacyWindowRef, char *error, std::size_t errorSize ) override
	{
		if ( error && errorSize )
			error[0] = '\0';
		if ( m_Host )
			return m_Host.get();
		std::string reason;
		m_Host = MakeSdl3LegacySurfaceHost( legacyWindowRef, &reason );
		if ( !m_Host && error && errorSize )
			std::snprintf( error, errorSize, "%s", reason.c_str() );
		return m_Host.get();
	}
	IVulkanSurfaceHost *SurfaceHost() const override { return m_Host.get(); }
	void Close() override { m_Host.reset(); }

	void RequestMode( const Mode &mode ) override
	{
		std::lock_guard<std::mutex> lock( m_Mutex );
		m_Mode = mode;
		++m_ModeRevision;
	}
	Mode GetMode() const override
	{
		std::lock_guard<std::mutex> lock( m_Mutex );
		return m_Mode;
	}
	std::uint64_t ModeRevision() const override
	{
		std::lock_guard<std::mutex> lock( m_Mutex );
		return m_ModeRevision;
	}

	// Any thread may set the ramp (mat_monitorgamma); presents read it.
	void SetGammaRamp( const render::GammaRamp16 &ramp ) override
	{
		std::lock_guard<std::mutex> lock( m_Mutex );
		m_Gamma = ramp;
		++m_GammaRevision;
	}
	bool GetGammaRamp( render::GammaRamp16 *ramp, std::uint64_t *revision ) const override
	{
		std::lock_guard<std::mutex> lock( m_Mutex );
		if ( !m_GammaRevision )
			return false;
		*ramp = m_Gamma;
		*revision = m_GammaRevision;
		return true;
	}

	void AddModeChangeCallback( void ( *callback )() ) override
	{
		if ( callback &&
		     std::find( m_Callbacks.begin(), m_Callbacks.end(), callback ) == m_Callbacks.end() )
			m_Callbacks.push_back( callback );
	}
	void RemoveModeChangeCallback( void ( *callback )() ) override
	{
		m_Callbacks.erase(
		    std::remove( m_Callbacks.begin(), m_Callbacks.end(), callback ), m_Callbacks.end() );
	}
	void ModeChanged() override { m_PendingChange = true; }
	bool DispatchModeChange() override
	{
		if ( !m_PendingChange )
			return false;
		m_PendingChange = false;
		// A callback may add or remove callbacks; run the ones registered now.
		const std::vector<void ( * )()> callbacks = m_Callbacks;
		for ( void ( *callback )() : callbacks )
			callback();
		return true;
	}

private:
	mutable std::mutex m_Mutex; // the mode and the ramp
	std::unique_ptr<IVulkanSurfaceHost> m_Host;
	Mode m_Mode;
	std::uint64_t m_ModeRevision = 0;
	render::GammaRamp16 m_Gamma = {};
	std::uint64_t m_GammaRevision = 0;
	std::vector<void ( * )()> m_Callbacks;
	bool m_PendingChange = false;
};

} // namespace

ILegacyPresentation *CreateSdl3LegacyPresentation()
{
	return new Sdl3LegacyPresentation();
}

void DestroySdl3LegacyPresentation( ILegacyPresentation *presentation )
{
	delete static_cast<Sdl3LegacyPresentation *>( presentation );
}

} // namespace render_vulkan
