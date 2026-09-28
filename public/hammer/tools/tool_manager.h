//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The tool manager (RFC 0002, hammer.tools): owns the tools, the
//			active tool and the pointer-capture bookkeeping, and routes
//			normalized input to the active tool. It replaces the per-view input
//			transport legacy CBaseTool repeated for 2D, 3D and logical views.
//
//			Routing rules (tested state machine):
//			  * An event whose view does not match the context's view, or a
//			    context without a camera, is Failed (a host bug), unrouted.
//			  * Without capture, pointer, key and wheel events for a view the
//			    active tool does not support are Ignored before the tool sees
//			    them. With no active tool everything is Ignored.
//			  * A tool that returns CaptureRequested holds the capture: it
//			    receives every pointer event of the captured view until the
//			    gesture ends; pointer events from other views are Ignored while
//			    it is held. The capture ends when the tool reports that its
//			    gesture is over (CaptureReleased, or Failed on release); the host
//			    releases its native grab when HasCapture() turns false.
//			  * Focus loss, capture loss (the host lost its grab) and switching
//			    tools cancel the gesture in progress (no edit) and drop capture.
//			    Switching also deactivates the old tool (pending state dropped).
//			  * Global keys that are not tool-specific (save, undo, grid size,
//			    tool selection) are NOT handled here; they belong to the action
//			    catalog. Keys reach the active tool only.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_TOOL_MANAGER_H
#define HAMMER_TOOLS_TOOL_MANAGER_H

#include "hammer/tools/tool.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::tools
{

class ToolManager
{
public:
	ToolManager() = default;
	ToolManager( const ToolManager & ) = delete;
	ToolManager &operator=( const ToolManager & ) = delete;

	// Takes ownership. Returns the tool, or nullptr (and drops it) when a tool
	// with the same name is already registered or 'tool' is null.
	ITool *Add( std::unique_ptr<ITool> tool );
	ITool *Find( std::string_view name ) const;
	std::vector<std::string_view> Names() const;

	ITool *Active() const { return m_active; }
	// Makes 'name' active; the previous tool's gesture is cancelled and it is
	// deactivated. Returns false (nothing changes) for an unknown name.
	// Activating the active tool again changes nothing.
	bool Activate( std::string_view name );
	// No active tool (the previous one is deactivated).
	void DeactivateAll();

	ToolResult OnPointer( ToolContext &ctx, const PointerEvent &event );
	ToolResult OnKey( ToolContext &ctx, const KeyEvent &event );
	ToolResult OnWheel( ToolContext &ctx, const WheelEvent &event );

	void OnFocusLost();
	void OnCaptureLost();
	// Cancels the active tool's gesture for 'reason' (e.g. DocumentReplaced).
	void CancelGesture( CancelReason reason );

	bool HasCapture() const { return m_capture.has_value(); }
	std::optional<viewport::ViewKind> CaptureView() const { return m_capture; }

	OverlayList Overlay( const ToolContext &ctx ) const;
	Cursor CursorFor( const ToolContext &ctx, double x, double y ) const;
	std::string Status() const;

private:
	ToolResult Finish( ToolResult result, viewport::ViewKind view );

	std::vector<std::unique_ptr<ITool>> m_tools;
	ITool *m_active = nullptr;
	std::optional<viewport::ViewKind> m_capture;
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_TOOL_MANAGER_H
