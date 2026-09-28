//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Entity tool (RFC 0002, hammer.tools; legacy ToolEntity.cpp and
//			Source 2's click-to-place). It places a point entity of
//			EditorSettings::entityClass with ops::PlaceEntity (class defaults
//			from the catalog) as one "Place entity" edit that selects it.
//
//			State machine (Left button):
//			  Idle --Down--> Pressed (the marker follows the pointer as Pending)
//			  Pressed --Up--> place at the release position (below the drag
//			      threshold: the press position), select it, Idle
//			  Pressed --Escape / focus or capture loss / tool switch--> Idle,
//			      nothing placed
//			Hover (no button) shows the marker where a click would place it.
//
//			Placement:
//			  2D: the grid-snapped plane point; the free (depth) axis takes the
//			      coordinate of the last 3D placement on that axis, else 0.
//			  3D: the first solid surface PickRay hits (markers ignored); the
//			      hit point is snapped on the two axes other than the normal's
//			      dominant axis, then moved along the normal so the class's
//			      marker box (viewport::EntityMarkerBox: catalog box, else
//			      +/- pointHalfSize) just touches the surface. No surface under
//			      the pointer: Failed, nothing placed.
//
//			Failure: no class set, an unknown or brush class (with a catalog)
//			and no surface return Failed with the reason and no edit.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_ENTITY_TOOL_H
#define HAMMER_TOOLS_ENTITY_TOOL_H

#include "hammer/tools/tool.h"

#include <optional>
#include <string>

namespace hammer::tools
{

class EntityTool final : public ITool
{
public:
	static constexpr std::string_view kName = "entity";

	std::string_view Name() const override { return kName; }
	ViewSet SupportedViews() const override { return ViewSet::All(); }

	ToolResult OnPointer( ToolContext &ctx, const PointerEvent &event ) override;
	ToolResult OnKey( ToolContext &ctx, const KeyEvent &event ) override;
	bool InGesture() const override { return m_gesture.has_value(); }
	void Cancel( CancelReason reason ) override;
	OverlayList Overlay( const ToolContext &ctx ) const override;
	Cursor CursorFor( const ToolContext &ctx, double x, double y ) const override;
	std::string Status() const override { return m_status; }

	// Where a click at (x, y) in ctx's view would place the entity; 'why'
	// receives the reason when nothing can be placed.
	std::optional<mapgeometry::Vec3d> PlacementAt( const ToolContext &ctx, double x, double y,
	    Modifiers modifiers, std::string *why = nullptr ) const;

	const std::optional<mapgeometry::Vec3d> &Last3DPlacement() const { return m_last3D; }

private:
	struct Gesture
	{
		viewport::ViewKind view = viewport::ViewKind::Top;
		viewport::ScreenPoint down;
		viewport::ScreenPoint current;
		Modifiers modifiers;
		bool dragged = false;
		std::optional<mapgeometry::Vec3d> preview;
	};
	struct Hover
	{
		viewport::ViewKind view = viewport::ViewKind::Top;
		std::optional<mapgeometry::Vec3d> at;
	};

	void AppendMarker( OverlayList &out, const ToolContext &ctx, const mapgeometry::Vec3d &origin,
	    OverlayRole role ) const;

	std::optional<Gesture> m_gesture;
	std::optional<Hover> m_hover;
	std::optional<mapgeometry::Vec3d> m_last3D;
	std::string m_status;
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_ENTITY_TOOL_H
