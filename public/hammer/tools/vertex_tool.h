//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Vertex tool (RFC 0002, hammer.tools; legacy ToolMorph.cpp).
//			It edits the selected solids through their vertices: handles sit at
//			every vertex (ops::SolidVertexList) and edge midpoint
//			(ops::SolidEdgeList) of the solids the selection stands for.
//
//			Handle selection is tool state, not document content. A click on
//			handles selects every handle drawn at the hit spot (in 2D, the
//			vertices stacked along the view's depth axis, as legacy morph);
//			Ctrl toggles them; a click on nothing clears (Ctrl keeps). The
//			handle selection is dropped whenever the document changes other
//			than by this tool's own commit (handles are vertex indices).
//
//			State machine (Left button):
//			  Idle --Down on handles--> Grab (unselected handles become the
//			      selection, or are added with Ctrl, until release)
//			  Idle --Down elsewhere--> Empty press
//			  Grab --past the drag threshold, 2D only--> Drag: the selected
//			      handles move by the delta that lands the grabbed handle on the
//			      grid (Shift: dominant axis; Alt: free), previewed with
//			      ops::RebuildFromVertices per solid; an invalid solid shows the
//			      Error role
//			  Drag --Up--> one "Move vertices" edit: ops::MoveVertices for every
//			      affected solid inside ONE Execute (one history unit); an edge
//			      handle moves both its vertices. A move a solid cannot take
//			      (concave or flat) returns Failed with the reason and no edit.
//			  Grab/Empty --Up below the threshold--> the click rules above
//			  any --Escape, focus or capture loss, tool switch--> Idle; the
//			      handle selection before the press comes back; no edit
//			3D: handles are picked (selection only); drags make no edit.
//			Escape without a gesture clears the handle selection.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_VERTEX_TOOL_H
#define HAMMER_TOOLS_VERTEX_TOOL_H

#include "hammer/tools/tool.h"

#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hammer::tools
{

class VertexTool final : public ITool
{
public:
	static constexpr std::string_view kName = "vertex";

	struct HandleRef
	{
		scene::ObjectId solid;
		bool edge = false;
		int index = 0; // into SolidVertexList or SolidEdgeList
		friend bool operator==( const HandleRef &, const HandleRef & ) = default;
		friend auto operator<=>( const HandleRef &, const HandleRef & ) = default;
	};

	struct HandleInfo
	{
		HandleRef ref;
		mapgeometry::Vec3d position;
	};

	std::string_view Name() const override { return kName; }
	ViewSet SupportedViews() const override { return ViewSet::All(); }

	ToolResult OnPointer( ToolContext &ctx, const PointerEvent &event ) override;
	ToolResult OnKey( ToolContext &ctx, const KeyEvent &event ) override;
	bool InGesture() const override { return m_gesture.has_value(); }
	void Cancel( CancelReason reason ) override;
	void Deactivate() override;
	OverlayList Overlay( const ToolContext &ctx ) const override;
	Cursor CursorFor( const ToolContext &ctx, double x, double y ) const override;
	std::string Status() const override { return m_status; }

	// Every handle of the selected solids, vertices first per solid.
	static std::vector<HandleInfo> Handles( const ToolContext &ctx );
	// The selected handles, valid for 'ctx's document (empty after an
	// external change).
	std::vector<HandleRef> SelectedHandles( const ToolContext &ctx ) const;

private:
	struct Gesture
	{
		viewport::ViewKind view = viewport::ViewKind::Top;
		viewport::ScreenPoint down;
		viewport::ScreenPoint current;
		Modifiers pressModifiers;
		Modifiers modifiers;
		bool dragged = false;
		std::vector<HandleRef> hit; // handles under the press (empty: empty press)
		std::optional<mapgeometry::Vec3d> grabbed;
		mapgeometry::Vec3d delta;      // as of the last move (2D drags)
		std::vector<HandleRef> before; // handle selection before the press
	};

	std::vector<HandleRef> HitHandles( const ToolContext &ctx, double x, double y ) const;
	mapgeometry::Vec3d DragDeltaWorld( const ToolContext &ctx, const Gesture &gesture ) const;
	// Per solid: the vertex indices the selected handles move.
	std::vector<std::pair<scene::ObjectId, std::vector<int>>> MovingVertices(
	    const ToolContext &ctx ) const;
	void Select( const ToolContext &ctx, std::vector<HandleRef> handles );

	std::vector<HandleRef> m_selected;
	std::uint64_t m_revision = 0; // the session revision m_selected belongs to
	std::uint32_t m_serial = 0;   // the document serial it belongs to
	std::optional<Gesture> m_gesture;
	std::string m_status;
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_VERTEX_TOOL_H
