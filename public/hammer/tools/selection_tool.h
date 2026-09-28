//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Selection tool (RFC 0002, hammer.tools; legacy
//			ToolSelection.cpp / Box3D.cpp). State machine, Left button only:
//
//			  Idle --Down--> Pressed(on handle | on object | on empty)
//			  Pressed --move past the drag threshold--> Dragging:
//			      on handle           -> Scale or Rotate (the handle mode)
//			      on object, no Ctrl  -> Move (the selection when the object is
//			                             selected, else just that object)
//			      on object with Ctrl, or on empty (2D) -> Marquee
//			      3D: no drag gesture (a drag is swallowed, no edit)
//			  Pressed --Up--> click:
//			      object, Ctrl        -> toggle it in the selection
//			      object, selected    -> cycle the 2D handle mode Scale <-> Rotate
//			      object, unselected  -> select it (Replace), mode Scale
//			      empty, no Ctrl      -> clear the selection
//			  Dragging --Up--> commit: Move = one Translate "Move"; Scale = one
//			      ScaleToBox "Scale"; Rotate = one Rotate "Rotate"; Marquee =
//			      select the objects (settings' marquee mode; Ctrl adds). A zero
//			      move/scale/rotation makes no edit. Moving an unselected object
//			      selects it with the edit (one history unit).
//			  Pressed/Dragging --Escape, focus/capture loss, tool switch--> Idle,
//			      no edit, selection untouched (nothing changes before release).
//
//			Selection changes happen on release, never on press: the object
//			under a press is shown as Hover in the overlay until then.
//
//			Handles (2D only): on the selection bounds (box_handles.h). Scale
//			mode: 8 handles, preview then ScaleToBox. Rotate mode: 4 corners,
//			rotating about the bounds center in the view plane: 0.5 degree
//			steps, Shift 15 degrees, Alt free (interaction_policy.h). The mode
//			resets to Scale whenever the selection changes. Legacy's Shear mode
//			is not offered.
//
//			Move: the delta snaps so the bounds' minimum corner lands on the
//			grid; Shift keeps the dominant axis; Alt disables snapping.
//
//			Keys (Press): arrows nudge the selection (NudgeDelta: grid size, Ctrl
//			one unit; view axes, world X/Y in 3D) as one "Nudge" edit; Delete
//			deletes the selection ("Delete", ops::DeleteObjects); Escape cancels
//			a gesture, else clears the selection.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_SELECTION_TOOL_H
#define HAMMER_TOOLS_SELECTION_TOOL_H

#include "hammer/tools/box_handles.h"
#include "hammer/tools/tool.h"

#include <optional>
#include <string>
#include <vector>

namespace hammer::tools
{

class SelectionTool final : public ITool
{
public:
	static constexpr std::string_view kName = "selection";

	std::string_view Name() const override { return kName; }
	ViewSet SupportedViews() const override { return ViewSet::All(); }

	ToolResult OnPointer( ToolContext &ctx, const PointerEvent &event ) override;
	ToolResult OnKey( ToolContext &ctx, const KeyEvent &event ) override;
	bool InGesture() const override { return m_gesture.has_value(); }
	void Cancel( CancelReason reason ) override;
	OverlayList Overlay( const ToolContext &ctx ) const override;
	Cursor CursorFor( const ToolContext &ctx, double x, double y ) const override;
	std::string Status() const override { return m_status; }

	// The handle mode that applies to 'selection' (Scale unless the selection
	// was clicked again since it last changed).
	BoxHandleMode HandleMode( const app::Selection &selection ) const;

private:
	enum class Press
	{
		Handle,
		Object,
		Empty,
	};
	enum class Drag
	{
		None,
		Move,
		Marquee,
		Scale,
		Rotate,
	};
	struct DragEdit
	{
		std::string label;
		app::EditSession::Operation operation;
		std::optional<app::Selection> selectionAfter;
	};
	struct Gesture
	{
		Press press = Press::Empty;
		Drag drag = Drag::None;
		bool dragged = false; // past the threshold (3D drags have Drag::None)
		viewport::ViewKind view = viewport::ViewKind::Top;
		viewport::ScreenPoint down;
		viewport::ScreenPoint current;
		Modifiers pressModifiers;
		Modifiers modifiers;
		scene::ObjectId target;
		bool targetSelected = false;
		BoxHandle handle;
		BoxHandleMode handleMode = BoxHandleMode::Scale;
		scene::Box bounds;
		bool hasBounds = false;
		std::vector<scene::ObjectId> moving;
		std::optional<DragEdit> edit; // as of the last move (the preview)
	};

	ToolResult OnDown( ToolContext &ctx, const PointerEvent &event );
	ToolResult OnUp( ToolContext &ctx );
	ToolResult Click( ToolContext &ctx, const Gesture &gesture );
	// The edit the current drag would commit; nothing when it changes nothing.
	std::optional<DragEdit> EditFor( const ToolContext &ctx, const Gesture &gesture ) const;
	std::optional<BoxHandle> HandleUnder( const ToolContext &ctx, double x, double y ) const;
	void SetMode( BoxHandleMode mode, const app::Selection &selection );

	std::optional<Gesture> m_gesture;
	BoxHandleMode m_mode = BoxHandleMode::Scale;
	std::vector<scene::ObjectId> m_modeObjects;
	std::optional<BoxHandle> m_hot;
	viewport::ViewKind m_hotView = viewport::ViewKind::Top;
	std::string m_status;
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_SELECTION_TOOL_H
