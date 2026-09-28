//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Clip tool (RFC 0002, hammer.tools; legacy ToolClipper.cpp). 2D
//			views only; the 3D view is not a supported context.
//
//			Clip line: two grid-snapped plane points in one 2D view. The clip
//			plane contains the line and the view's free axis. Its normal is
//			Normalize( (b - a) x viewDirection ), so FRONT is the half-space to
//			the LEFT of the line from its first to its second point as seen in
//			the view that drew it.
//
//			State machine (Left button):
//			  NoLine/HasLine --Down on a point handle of the line (this view
//			      kind)--> MovePoint: the point follows the snapped pointer
//			  NoLine/HasLine --Down elsewhere--> NewLine from the snapped press
//			      point to the snapped pointer
//			  --Up past the drag threshold with two distinct points--> HasLine
//			  --Up below the threshold, or with coincident points--> the
//			      previous state (a click changes nothing)
//			  Escape / focus or capture loss / tool switch during a drag --> the
//			      previous line
//
//			Keys (Press): Shift+X cycles what is kept, Front -> Back -> Both ->
//			Front (legacy order, starting at Front); Enter applies the clip to
//			the selected solids (ops::ClipSolids, cap faces take EditorSettings'
//			face texture) as one "Clip" edit that keeps the pieces selected, and
//			clears the line; Escape without a drag clears the line.
//
//			Preview: for every selected solid the plane splits
//			(ops::SplitSolid), the kept pieces as Clip-role edges; the line and
//			its point handles in the view kind that drew it.
//
//			Failure: Enter with a line but no selected solid, or with a plane
//			that crosses none of them, returns Failed with no edit.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_CLIP_TOOL_H
#define HAMMER_TOOLS_CLIP_TOOL_H

#include "hammer/app/ops/csg_ops.h"
#include "hammer/tools/tool.h"

#include <optional>
#include <string>

namespace hammer::tools
{

class ClipTool final : public ITool
{
public:
	static constexpr std::string_view kName = "clip";

	struct ClipLine
	{
		viewport::ViewKind view = viewport::ViewKind::Top;
		viewport::PlanePoint a;
		viewport::PlanePoint b;
		friend bool operator==( const ClipLine &, const ClipLine & ) = default;
	};

	std::string_view Name() const override { return kName; }
	ViewSet SupportedViews() const override { return ViewSet::TwoD(); }

	ToolResult OnPointer( ToolContext &ctx, const PointerEvent &event ) override;
	ToolResult OnKey( ToolContext &ctx, const KeyEvent &event ) override;
	bool InGesture() const override { return m_gesture.has_value(); }
	void Cancel( CancelReason reason ) override;
	void Deactivate() override;
	OverlayList Overlay( const ToolContext &ctx ) const override;
	Cursor CursorFor( const ToolContext &ctx, double x, double y ) const override;
	std::string Status() const override;

	const std::optional<ClipLine> &Line() const { return m_line; }
	app::ops::ClipKeep Keep() const { return m_keep; }
	// The plane of 'line' (nothing for coincident points).
	static std::optional<mapgeometry::Plane> PlaneOf( const ClipLine &line );
	// Applies the clip (what Enter does).
	ToolResult Apply( ToolContext &ctx );

private:
	struct Gesture
	{
		viewport::ViewKind view = viewport::ViewKind::Top;
		viewport::ScreenPoint down;
		bool dragged = false;
		int point = -1; // 0/1 = moving that point; -1 = a new line
		std::optional<ClipLine> before;
		ClipLine line;
	};

	std::optional<int> PointUnder( const ToolContext &ctx, double x, double y ) const;

	std::optional<ClipLine> m_line;
	std::optional<Gesture> m_gesture;
	app::ops::ClipKeep m_keep = app::ops::ClipKeep::Front;
	std::string m_message;
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_CLIP_TOOL_H
