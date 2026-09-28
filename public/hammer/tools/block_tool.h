//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Block tool (RFC 0002, hammer.tools; legacy ToolBlock.cpp and
//			Source 2's Block tool). It edits a pending box -- tool state, not
//			document content -- and creates the primitive (or arch) only when
//			the box is committed with Enter.
//
//			2D (Left button):
//			  Idle/HasBox --Down on a handle of the pending box--> Resize
//			  Idle/HasBox --Down inside the pending box--> MoveBox
//			  Idle/HasBox --Down elsewhere--> NewBox
//			  NewBox: the two snapped plane corners give the box in the view's
//			      two axes. The free axis (depth) extent is taken from, in order:
//			      the current or previous pending box, the selection bounds, or
//			      [0, grid size] (the default depth is one grid step).
//			  Resize: box_handles.h (edges snap). MoveBox: the delta snaps the
//			      box's minimum corner; Shift constrains; Alt frees.
//			  --Up past the drag threshold with a box of positive size--> HasBox;
//			  --Up below the threshold (a click) or with a flat box--> the
//			      previous state (nothing changes).
//
//			3D (Left button):
//			  No box, or a press whose ray misses the pending box --> Base drag
//			      on the workplane: the plane perpendicular to the dominant axis
//			      of the surface normal PickRay hits, at the hit's snapped
//			      coordinate on that axis (no hit: the z = 0 plane, facing +Z).
//			      The box grows from the workplane toward the surface normal,
//			      one grid step tall.
//			  A press whose ray hits the pending box --> Height drag: the far
//			      face moves along the box's height axis by the pointer ray's
//			      motion along that axis (snapped); a height crossing the base
//			      flips the box to the other side of the base.
//			  Up past the threshold --> HasBox; a click --> the previous state.
//
//			Keys: Enter commits (ExecuteSelecting "Create block" / "Create
//			arch", the new object selected, the pending box cleared and
//			remembered for the depth rule); Escape cancels a drag (the pending
//			box before the drag comes back) or, without a drag, discards the
//			pending box. Deactivation discards the pending box.
//
//			The primitive's height axis (PrimitiveSpec::axis) is the depth axis
//			of the 2D view that drew the box, or the workplane axis in 3D.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_BLOCK_TOOL_H
#define HAMMER_TOOLS_BLOCK_TOOL_H

#include "hammer/tools/box_handles.h"
#include "hammer/tools/tool.h"

#include <optional>
#include <string>

namespace hammer::tools
{

class BlockTool final : public ITool
{
public:
	static constexpr std::string_view kName = "block";

	struct PendingBox
	{
		scene::Box box;
		int axis = 2;          // the height axis
		bool baseAtMin = true; // the base is the box's minimum face on 'axis'
		friend bool operator==( const PendingBox &, const PendingBox & ) = default;
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

	const std::optional<PendingBox> &Pending() const { return m_pending; }
	// Commits the pending box (what Enter does).
	ToolResult Commit( ToolContext &ctx );

private:
	enum class Kind
	{
		NewBox,
		Resize,
		MoveBox,
		Base,
		Height,
	};
	struct Gesture
	{
		Kind kind = Kind::NewBox;
		viewport::ViewKind view = viewport::ViewKind::Top;
		viewport::ScreenPoint down;
		viewport::ScreenPoint current;
		Modifiers modifiers;
		bool dragged = false;
		std::optional<PendingBox> before;
		std::optional<PendingBox> preview; // the box the drag shows now
		BoxHandle handle;
		viewport::PlanePoint start; // NewBox: the snapped first corner
		// 3D workplane
		int planeAxis = 2;
		double planeCoord = 0.0;
		double planeSign = 1.0;
		mapgeometry::Vec3d baseStart; // snapped first corner on the workplane
		double lineDown = 0.0;        // Height: pointer parameter along the axis at the press
	};

	std::optional<PendingBox> Update2D( const ToolContext &ctx, const Gesture &gesture ) const;
	std::optional<PendingBox> Update3D( const ToolContext &ctx, const Gesture &gesture ) const;
	void DepthExtent( const ToolContext &ctx, int axis, double &lo, double &hi ) const;
	void SetStatusFor( const std::optional<PendingBox> &box );

	std::optional<PendingBox> m_pending;
	std::optional<scene::Box> m_last; // the previous committed or discarded box
	std::optional<Gesture> m_gesture;
	std::string m_status;
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_BLOCK_TOOL_H
