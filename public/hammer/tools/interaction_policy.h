//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The one owner of the interaction policies every tool shares (RFC
//			0002, hammer.tools: "Shared snapping, drag thresholds, transform
//			constraints and hit ordering have one policy owner and are reused
//			across tools"). Tools call these functions; none re-derives them.
//
//			Drag threshold. A press becomes a drag once the pointer has moved
//			more than kDragThresholdPixels (3) on either screen axis (legacy
//			Tool3D DRAG_THRESHHOLD per-axis rule). A press released before that
//			is a click, whatever small motion happened in between.
//
//			Snapping. Positions snap through viewport::GridPolicy (the grid
//			size, ties away from zero). Snapping is active when the grid's snap
//			is enabled and Alt is NOT held (legacy Tool3D::GetConstraints: Alt
//			disables snap). A drag snaps its reference point, not the raw delta:
//			the delta is chosen so reference + delta lands on the grid, so an
//			off-grid object or vertex lands on the grid when moved.
//
//			Constraint. With Shift held, a 2D drag delta keeps only its dominant
//			view axis (the larger magnitude; a tie keeps the horizontal axis) --
//			Source 2 and legacy Hammer's axis constraint.
//
//			Rotation. Angles step by kRotateFineDegrees (0.5, legacy's
//			unconstrained step); Shift steps by kRotateSnapDegrees (15, legacy's
//			"RotateConstrain" step, bound to Shift here); Alt leaves the angle
//			unrounded. (Legacy inverted Shift when its RotateConstrain option was
//			on; this editor has one fixed binding.)
//
//			Nudge. Arrow keys move along the view's axes (Right +u, Up +v; the
//			3D view uses world X/Y, viewport::AxesOf(Camera3D)). The step is the
//			grid size when NudgeStep::Grid, the grid snap is enabled and Ctrl is
//			not held; otherwise one unit (legacy GetNudgeVector).
//
//			Hit ordering. The first hit of viewport::Pick2D / PickRay (the
//			viewport owns hit order) resolved through app::ResolvePick with the
//			selection granularity. Marquee results resolve the same way.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_INTERACTION_POLICY_H
#define HAMMER_TOOLS_INTERACTION_POLICY_H

#include "hammer/app/selection.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/map_document.h"
#include "hammer/tools/input.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/grid.h"
#include "hammer/viewport/picking.h"

#include <optional>
#include <vector>

namespace hammer::tools
{

constexpr double kDragThresholdPixels = 3.0;
constexpr double kHandleRadiusPixels = 4.0; // legacy HANDLE_RADIUS
constexpr double kHandleOffsetPixels = 6.0; // legacy HANDLE_OFFSET (2D box handles)
constexpr double kRotateSnapDegrees = 15.0;
constexpr double kRotateFineDegrees = 0.5;

// --- Drag threshold ------------------------------------------------------------------

bool ExceedsDragThreshold( viewport::ScreenPoint down, viewport::ScreenPoint now,
    double thresholdPixels = kDragThresholdPixels );

// True when 'point' is within 'radius' pixels of 'center' on both axes (the
// legacy square handle hit test).
bool WithinHandle( viewport::ScreenPoint center, viewport::ScreenPoint point,
    double radius = kHandleRadiusPixels );

// The larger of the two axis distances (the metric WithinHandle uses).
double HandleDistance( viewport::ScreenPoint a, viewport::ScreenPoint b );

// --- Snapping and constraints -----------------------------------------------------------

bool SnapActive( const viewport::GridPolicy &grid, Modifiers modifiers );
double SnapValue( const viewport::GridPolicy &grid, double value, Modifiers modifiers );
viewport::PlanePoint SnapPlanePoint(
    const viewport::GridPolicy &grid, viewport::PlanePoint point, Modifiers modifiers );
mapgeometry::Vec3d SnapWorldPoint( const viewport::GridPolicy &grid,
    const mapgeometry::Vec3d &point, Modifiers modifiers, viewport::AxisMask mask = {} );

viewport::PlanePoint ConstrainToDominantAxis( viewport::PlanePoint delta );

// The drag delta for a raw pointer delta: Shift constraint first, then the
// snap of 'reference' + delta (a constrained-away axis stays zero).
viewport::PlanePoint DragDelta( const viewport::GridPolicy &grid, viewport::PlanePoint reference,
    viewport::PlanePoint rawDelta, Modifiers modifiers );

// A view-plane delta as a world delta (zero on the depth axis).
mapgeometry::Vec3d PlaneDeltaToWorld( viewport::ViewAxes axes, viewport::PlanePoint delta );

double SnapAngle( double degrees, Modifiers modifiers );

// --- Nudge -------------------------------------------------------------------------------

enum class NudgeStep
{
	Grid,
	Unit,
};

// The world delta an arrow key nudges by in 'view'; nothing for other keys.
std::optional<mapgeometry::Vec3d> NudgeDelta( viewport::ViewKind view, Key key, Modifiers modifiers,
    const viewport::GridPolicy &grid, NudgeStep step );

// --- Hit ordering ----------------------------------------------------------------------------

struct PickSettings
{
	app::SelectionGranularity granularity = app::SelectionGranularity::Objects;
	const ports::IEntityCatalog *catalog = nullptr;
	double pointHalfSize = viewport::kDefaultPointHalfSize;
	bool solids = true;
	bool entities = true;
};

struct ObjectPick
{
	scene::ObjectId target; // what a click selects (ResolvePick of 'object')
	scene::ObjectId object; // the solid or point entity hit
	scene::FaceRef face;    // 3D solid hits only
	bool hasPoint = false;  // 3D only
	mapgeometry::Vec3d point;
	mapgeometry::Vec3d normal;
};

std::optional<ObjectPick> PickObject2D( const scene::DocumentReader &doc,
    const viewport::Camera2D &camera, double x, double y, const PickSettings &settings );
std::optional<ObjectPick> PickObject3D( const scene::DocumentReader &doc,
    const viewport::Camera3D &camera, double x, double y, const PickSettings &settings );

// The objects a marquee selects, resolved per granularity, unique, in id order.
std::vector<scene::ObjectId> MarqueeTargets( const scene::DocumentReader &doc,
    const viewport::Camera2D &camera, const viewport::ScreenRect &rect, viewport::MarqueeMode mode,
    const PickSettings &settings );

} // namespace hammer::tools

#endif // HAMMER_TOOLS_INTERACTION_POLICY_H
