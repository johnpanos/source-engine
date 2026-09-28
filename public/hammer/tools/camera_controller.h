//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Camera navigation for editor hosts (RFC 0002, hammer.tools). Pure
//			camera control: it moves the viewport::Camera2D / Camera3D values the
//			host owns through their own navigation functions (PanPixels, ZoomAt,
//			Fly, Look, Orbit, Frame) and never touches the document.
//
//			Bindings (NavigationSettings holds the tunables):
//			  2D  pan:   Middle drag, or Left drag while Space is held
//			      zoom:  wheel about the cursor (wheelZoomFactor per notch);
//			             '=' / '-' about the view centre (keyZoomFactor)
//			  3D  look:  a drag with lookButton (default Right, Source 2) or
//			             Left while Space is held; yaw/pitch per pixel
//			      orbit: Alt + Left drag about the orbit pivot (SetOrbitPivot;
//			             default 256 units ahead of the eye at the press)
//			      pan:   Middle drag (right/up in the view plane)
//			      fly:   W/S forward/back, A/D left/right, E/Q up/down along
//			             world Z while held; Advance(seconds) moves the camera
//			             at flySpeed (Shift: x fastMultiplier)
//			      dolly: wheel, wheelDollyUnits per notch
//			  Framing: FrameSelection centres (2D) or backs off (3D) to the
//			      selection bounds.
//
//			Arbitration with tools. A host offers each pointer event to the
//			controller first and to the ToolManager only when the controller
//			returns Ignored. A press with lookButton is only ARMED (Ignored), so
//			a click with that button still reaches the active tool (the Face
//			tool's right-click apply); when the pointer then moves past the
//			drag threshold the controller returns CaptureRequested and the host
//			calls ToolManager::OnCaptureLost(), cancelling the tool's press.
//			A release of an armed press that never dragged is Ignored (it goes
//			to the tool). Space and the fly keys are Handled (consumed).
//
//=============================================================================//

#ifndef HAMMER_TOOLS_CAMERA_CONTROLLER_H
#define HAMMER_TOOLS_CAMERA_CONTROLLER_H

#include "hammer/app/edit_session.h"
#include "hammer/tools/input.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/view_policy.h"

#include <cstdint>
#include <optional>

namespace hammer::tools
{

struct NavigationSettings
{
	double flySpeed = 512.0; // units per second
	double fastMultiplier = 4.0;
	double lookDegreesPerPixel = 0.25;
	double orbitDegreesPerPixel = 0.5;
	double panUnitsPerPixel3D = 1.0;
	double wheelZoomFactor = 1.25;
	double keyZoomFactor = 2.0;
	double wheelDollyUnits = 64.0;
	double orbitDefaultDistance = 256.0;
	double frameMarginPixels = 32.0;
	PointerButton lookButton = PointerButton::Right;
};

class CameraController
{
public:
	explicit CameraController( NavigationSettings settings = {} ) : m_settings( settings ) {}

	NavigationSettings &Settings() { return m_settings; }
	const NavigationSettings &Settings() const { return m_settings; }

	ToolResult OnPointer( viewport::Camera2D &camera, const PointerEvent &event );
	ToolResult OnPointer( viewport::Camera3D &camera, const PointerEvent &event );
	ToolResult OnWheel( viewport::Camera2D &camera, const WheelEvent &event );
	ToolResult OnWheel( viewport::Camera3D &camera, const WheelEvent &event );
	// Held-key tracking (Space, fly keys). Handled for the keys it tracks.
	ToolResult OnKey( const KeyEvent &event );
	// OnKey plus the 2D zoom keys.
	ToolResult OnKey( viewport::Camera2D &camera, const KeyEvent &event );

	// Applies held fly keys for 'seconds'. Returns whether the camera moved.
	bool Advance( viewport::Camera3D &camera, double seconds );

	void SetOrbitPivot( std::optional<mapgeometry::Vec3d> pivot ) { m_pivot = pivot; }
	const std::optional<mapgeometry::Vec3d> &OrbitPivot() const { return m_pivot; }

	// Releases held keys and ends any drag (the host lost focus).
	void OnFocusLost();

	bool HasCapture() const { return m_drag != Drag::None && m_drag != Drag::LookArmed; }
	bool Flying() const { return m_keys != 0; }

	// The selection's bounds (objects, else the solids of selected faces).
	static std::optional<scene::Box> SelectionBounds(
	    const app::EditSession &session, double pointHalfSize = viewport::kDefaultPointHalfSize );
	// Returns false (camera unchanged) with nothing selected.
	bool FrameSelection( viewport::Camera2D &camera, const app::EditSession &session ) const;
	bool FrameSelection( viewport::Camera3D &camera, const app::EditSession &session ) const;

private:
	enum class Drag
	{
		None,
		Pan2D,
		Pan3D,
		Look,
		LookArmed,
		Orbit,
	};

	enum FlyKey : std::uint8_t
	{
		kForward = 1u << 0,
		kBack = 1u << 1,
		kLeft = 1u << 2,
		kRight = 1u << 3,
		kUp = 1u << 4,
		kDown = 1u << 5,
	};

	NavigationSettings m_settings;
	Drag m_drag = Drag::None;
	PointerButton m_button = PointerButton::None;
	viewport::ViewKind m_view = viewport::ViewKind::Top;
	viewport::ScreenPoint m_down;
	viewport::ScreenPoint m_last;
	mapgeometry::Vec3d m_dragPivot;
	std::optional<mapgeometry::Vec3d> m_pivot;
	bool m_space = false;
	bool m_fast = false;
	std::uint8_t m_keys = 0;
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_CAMERA_CONTROLLER_H
