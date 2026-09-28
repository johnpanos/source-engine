//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/camera_controller.h.
//
//=============================================================================//

#include "hammer/tools/camera_controller.h"

#include "hammer/scene/map_queries.h"
#include "hammer/tools/interaction_policy.h"
#include "mapgeometry/vec3.h"

#include <cmath>

namespace hammer::tools
{

using mapgeometry::Vec3d;

ToolResult CameraController::OnPointer( viewport::Camera2D &camera, const PointerEvent &event )
{
	const double dx = event.x - m_last.x;
	const double dy = event.y - m_last.y;
	switch ( event.phase )
	{
	case PointerPhase::Down:
		if ( m_drag != Drag::None )
			return ToolResult::Ignore();
		if ( event.button == PointerButton::Middle ||
		     ( event.button == PointerButton::Left && m_space ) )
		{
			m_drag = Drag::Pan2D;
			m_button = event.button;
			m_view = event.view;
			m_down = m_last = event.Point();
			return ToolResult::Capture();
		}
		return ToolResult::Ignore();
	case PointerPhase::Move:
		if ( m_drag != Drag::Pan2D || event.view != m_view )
			return ToolResult::Ignore();
		camera.PanPixels( dx, dy );
		m_last = event.Point();
		return ToolResult::Handle();
	case PointerPhase::Up:
		if ( m_drag != Drag::Pan2D || event.button != m_button )
			return ToolResult::Ignore();
		camera.PanPixels( dx, dy );
		m_drag = Drag::None;
		return ToolResult::Release();
	}
	return ToolResult::Ignore();
}

ToolResult CameraController::OnPointer( viewport::Camera3D &camera, const PointerEvent &event )
{
	const double dx = event.x - m_last.x;
	const double dy = event.y - m_last.y;
	const auto apply = [&]()
	{
		switch ( m_drag )
		{
		case Drag::Look:
			camera.Look(
			    -dx * m_settings.lookDegreesPerPixel, dy * m_settings.lookDegreesPerPixel );
			break;
		case Drag::Orbit:
			camera.Orbit( m_dragPivot, -dx * m_settings.orbitDegreesPerPixel,
			    dy * m_settings.orbitDegreesPerPixel );
			break;
		case Drag::Pan3D:
			camera.Fly(
			    0.0, -dx * m_settings.panUnitsPerPixel3D, dy * m_settings.panUnitsPerPixel3D );
			break;
		default:
			break;
		}
		m_last = event.Point();
	};
	switch ( event.phase )
	{
	case PointerPhase::Down:
	{
		if ( m_drag != Drag::None )
			return ToolResult::Ignore();
		m_button = event.button;
		m_view = event.view;
		m_down = m_last = event.Point();
		if ( event.button == PointerButton::Left && event.modifiers.Alt() )
		{
			m_drag = Drag::Orbit;
			m_dragPivot =
			    m_pivot ? *m_pivot
			            : camera.Position() + camera.Forward() * m_settings.orbitDefaultDistance;
			return ToolResult::Capture();
		}
		if ( event.button == PointerButton::Left && m_space )
		{
			m_drag = Drag::Look;
			return ToolResult::Capture();
		}
		if ( event.button != PointerButton::None && event.button != PointerButton::Left &&
		     event.button == m_settings.lookButton )
		{
			m_drag = Drag::LookArmed;
			return ToolResult::Ignore(); // the press also belongs to the tool until it drags
		}
		if ( event.button == PointerButton::Middle )
		{
			m_drag = Drag::Pan3D;
			return ToolResult::Capture();
		}
		return ToolResult::Ignore();
	}
	case PointerPhase::Move:
		if ( m_drag == Drag::None || event.view != m_view )
			return ToolResult::Ignore();
		if ( m_drag == Drag::LookArmed )
		{
			if ( !ExceedsDragThreshold( m_down, event.Point() ) )
				return ToolResult::Ignore();
			m_drag = Drag::Look;
			apply();
			return ToolResult::Capture();
		}
		apply();
		return ToolResult::Handle();
	case PointerPhase::Up:
		if ( m_drag == Drag::None || event.button != m_button )
			return ToolResult::Ignore();
		if ( m_drag == Drag::LookArmed )
		{
			m_drag = Drag::None;
			return ToolResult::Ignore(); // a click: the tool's
		}
		apply();
		m_drag = Drag::None;
		return ToolResult::Release();
	}
	return ToolResult::Ignore();
}

ToolResult CameraController::OnWheel( viewport::Camera2D &camera, const WheelEvent &event )
{
	if ( event.steps == 0.0 ||
	     !camera.ZoomAt( event.x, event.y, std::pow( m_settings.wheelZoomFactor, event.steps ) ) )
		return ToolResult::Ignore();
	return ToolResult::Handle();
}

ToolResult CameraController::OnWheel( viewport::Camera3D &camera, const WheelEvent &event )
{
	if ( event.steps == 0.0 || !camera.Fly( event.steps * m_settings.wheelDollyUnits, 0.0, 0.0 ) )
		return ToolResult::Ignore();
	return ToolResult::Handle();
}

ToolResult CameraController::OnKey( const KeyEvent &event )
{
	const bool press = event.IsPress();
	m_fast = event.modifiers.Shift();
	if ( event.key == Key::Space )
	{
		m_space = press;
		return ToolResult::Handle();
	}
	if ( event.key != Key::Character )
		return ToolResult::Ignore();
	std::uint8_t bit = 0;
	switch ( event.character )
	{
	case 'w':
		bit = kForward;
		break;
	case 's':
		bit = kBack;
		break;
	case 'a':
		bit = kLeft;
		break;
	case 'd':
		bit = kRight;
		break;
	case 'e':
		bit = kUp;
		break;
	case 'q':
		bit = kDown;
		break;
	default:
		return ToolResult::Ignore();
	}
	m_keys = press ? static_cast<std::uint8_t>( m_keys | bit )
	               : static_cast<std::uint8_t>( m_keys & ~bit );
	return ToolResult::Handle();
}

ToolResult CameraController::OnKey( viewport::Camera2D &camera, const KeyEvent &event )
{
	if ( event.IsPress() && ( event.IsChar( '=' ) || event.IsChar( '-' ) ) )
	{
		const double factor =
		    event.IsChar( '=' ) ? m_settings.keyZoomFactor : 1.0 / m_settings.keyZoomFactor;
		camera.ZoomAt( camera.Width() * 0.5, camera.Height() * 0.5, factor );
		return ToolResult::Handle();
	}
	return OnKey( event );
}

bool CameraController::Advance( viewport::Camera3D &camera, double seconds )
{
	if ( m_keys == 0 || !( seconds > 0.0 ) )
		return false;
	const auto axis = [this]( std::uint8_t plus, std::uint8_t minus )
	{
		return ( ( m_keys & plus ) ? 1.0 : 0.0 ) - ( ( m_keys & minus ) ? 1.0 : 0.0 );
	};
	const double step =
	    m_settings.flySpeed * ( m_fast ? m_settings.fastMultiplier : 1.0 ) * seconds;
	const double forward = axis( kForward, kBack ) * step;
	const double right = axis( kRight, kLeft ) * step;
	const double up = axis( kUp, kDown ) * step;
	if ( forward == 0.0 && right == 0.0 && up == 0.0 )
		return false;
	return camera.Fly( forward, right, up );
}

void CameraController::OnFocusLost()
{
	m_drag = Drag::None;
	m_space = false;
	m_fast = false;
	m_keys = 0;
}

std::optional<scene::Box> CameraController::SelectionBounds(
    const app::EditSession &session, double pointHalfSize )
{
	const app::Selection &selection = session.CurrentSelection();
	std::vector<scene::ObjectId> ids = selection.objects;
	if ( ids.empty() )
		for ( const scene::FaceRef &face : selection.faces )
			ids.push_back( face.solid );
	if ( ids.empty() )
		return std::nullopt;
	return scene::ObjectsBounds( session.Document(), ids, pointHalfSize );
}

bool CameraController::FrameSelection(
    viewport::Camera2D &camera, const app::EditSession &session ) const
{
	const std::optional<scene::Box> box = SelectionBounds( session );
	return box && camera.Frame( *box, m_settings.frameMarginPixels );
}

bool CameraController::FrameSelection(
    viewport::Camera3D &camera, const app::EditSession &session ) const
{
	const std::optional<scene::Box> box = SelectionBounds( session );
	return box && camera.Frame( *box );
}

} // namespace hammer::tools
