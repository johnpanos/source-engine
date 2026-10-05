//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A menu-independent transaction for graphics settings.
//=============================================================================//

#ifndef GAMEUI_GRAPHICS_SETTINGS_SERVICE_H
#define GAMEUI_GRAPHICS_SETTINGS_SERVICE_H

#include <cmath>
#include <charconv>
#include <string_view>

namespace gameui
{

struct HdrSettings
{
	bool automatic = true;
	// 0 takes each from the display (mat_hdr_exposure, mat_hdr_peak_nits).
	float exposure = 0.0f;
	int peakNits = 0;
	bool operator==( const HdrSettings & ) const = default;
};

struct GraphicsSettings
{
	int width = 0;
	int height = 0;
	bool windowed = false;
	bool borderless = false;
	bool vrEnabled = false;
	int displayIndex = 0;
	float uiScale = 0.0f;
	int powerSaving = 0;
	HdrSettings hdr;
	float temporalScale = 0.0f; // 0 disables the selected temporal provider

	bool operator==( const GraphicsSettings &other ) const
	{
		return width == other.width && height == other.height && windowed == other.windowed &&
		       borderless == other.borderless && vrEnabled == other.vrEnabled &&
		       displayIndex == other.displayIndex && uiScale == other.uiScale &&
		       powerSaving == other.powerSaving && temporalScale == other.temporalScale && hdr == other.hdr;
	}
};

// The menu owns the backend and the session. A backend returns false when it
// cannot submit a complete change or persist it; the draft remains retryable.
class IGraphicsSettingsBackend
{
public:
	virtual ~IGraphicsSettingsBackend() = default;
	virtual bool Apply( const GraphicsSettings &from, const GraphicsSettings &to ) = 0;
	virtual bool Save() = 0;
};

class GraphicsSettingsService
{
public:
	enum class State
	{
		Uninitialized,
		Saved,
		Editing,
		Applied
	};

	void Begin( const GraphicsSettings &current )
	{
		m_applied = current;
		m_draft = current;
		m_cleanState = State::Saved;
		m_state = State::Saved;
	}

	bool Stage( const GraphicsSettings &draft )
	{
		if ( m_state == State::Uninitialized || !std::isfinite( draft.hdr.exposure ) ||
		     ( draft.hdr.exposure != 0.0f && draft.hdr.exposure < 0.25f ) ||
		     draft.hdr.exposure > 4.0f || ( draft.hdr.peakNits != 0 && draft.hdr.peakNits < 203 ) ||
		     draft.hdr.peakNits > 10000 || draft.width <= 0 || draft.height <= 0 ||
		     ( draft.borderless && !draft.windowed ) || !std::isfinite( draft.uiScale ) ||
		     draft.uiScale < 0.0f || !std::isfinite( draft.temporalScale ) ||
		     ( draft.temporalScale != 0.0f && draft.temporalScale < 0.5f ) ||
		     draft.temporalScale > 1.0f || ( draft.powerSaving != 0 && draft.powerSaving != 1 ) )
			return false;

		m_draft = draft;
		m_state = m_draft == m_applied ? m_cleanState : State::Editing;
		return true;
	}

	bool Apply( IGraphicsSettingsBackend &backend )
	{
		if ( m_state == State::Uninitialized )
			return false;
		if ( m_state != State::Editing )
			return true;
		if ( !backend.Apply( m_applied, m_draft ) )
			return false;

		m_applied = m_draft;
		m_cleanState = State::Applied;
		m_state = State::Applied;
		return true;
	}

	bool Save( IGraphicsSettingsBackend &backend )
	{
		if ( m_state != State::Applied && m_state != State::Saved )
			return false;
		if ( !backend.Save() )
			return false;
		m_cleanState = State::Saved;
		m_state = State::Saved;
		return true;
	}

	void Cancel()
	{
		if ( m_state == State::Uninitialized )
			return;
		m_draft = m_applied;
		m_state = m_cleanState;
	}

	State GetState() const { return m_state; }
	const GraphicsSettings &Draft() const { return m_draft; }
	const GraphicsSettings &Applied() const { return m_applied; }

private:
	GraphicsSettings m_applied;
	GraphicsSettings m_draft;
	State m_state = State::Uninitialized;
	State m_cleanState = State::Saved;
};

// The HDR submenu borrows the Video transaction. Its snapshot restores only
// this page's edits on Back, preserving pending resolution/FSR/UI changes.
class HdrSettingsMenu
{
public:
	enum class Action { Changed, Apply, Cancel, Invalid };
	explicit HdrSettingsMenu( GraphicsSettingsService &session )
	    : m_session( session ), m_original( session.Draft().hdr ) {}

	Action Command( std::string_view command )
	{
		if ( command == "ApplyHDR" )
			return Action::Apply;
		if ( command == "Back" || command == "Cancel" )
		{
			Cancel();
			return Action::Cancel;
		}
		auto draft = m_session.Draft();
		int value = 0;
		const auto read = [&]( std::string_view prefix )
		{
			if ( !command.starts_with( prefix ) )
				return false;
			const auto suffix = command.substr( prefix.size() );
			const auto result = std::from_chars( suffix.data(), suffix.data() + suffix.size(), value );
			return !suffix.empty() && result.ec == std::errc{} &&
			       result.ptr == suffix.data() + suffix.size();
		};
		if ( read( "HdrMode" ) && ( value == 0 || value == 1 ) )
			draft.hdr.automatic = value == 1;
		else if ( read( "HdrExposure" ) )
			draft.hdr.exposure = value * 0.01f;
		else if ( read( "HdrPeak" ) )
			draft.hdr.peakNits = value;
		else
			return Action::Invalid;
		return m_session.Stage( draft ) ? Action::Changed : Action::Invalid;
	}

	void Cancel()
	{
		auto draft = m_session.Draft();
		draft.hdr = m_original;
		(void)m_session.Stage( draft );
	}
	void Commit() { m_original = m_session.Applied().hdr; }
	const HdrSettings &Draft() const { return m_session.Draft().hdr; }

private:
	GraphicsSettingsService &m_session;
	HdrSettings m_original;
};

} // namespace gameui

#endif // GAMEUI_GRAPHICS_SETTINGS_SERVICE_H
