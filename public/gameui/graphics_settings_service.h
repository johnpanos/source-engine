//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A menu-independent transaction for graphics settings.
//=============================================================================//

#ifndef GAMEUI_GRAPHICS_SETTINGS_SERVICE_H
#define GAMEUI_GRAPHICS_SETTINGS_SERVICE_H

#include <cmath>

namespace gameui
{

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
	float temporalScale = 0.0f; // 0 disables the selected temporal provider

	bool operator==( const GraphicsSettings &other ) const
	{
		return width == other.width && height == other.height && windowed == other.windowed &&
		       borderless == other.borderless && vrEnabled == other.vrEnabled &&
		       displayIndex == other.displayIndex && uiScale == other.uiScale &&
		       powerSaving == other.powerSaving && temporalScale == other.temporalScale;
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
		if ( m_state == State::Uninitialized || draft.width <= 0 || draft.height <= 0 ||
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

} // namespace gameui

#endif // GAMEUI_GRAPHICS_SETTINGS_SERVICE_H
