//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The tool contract (RFC 0002, hammer.tools; "Input and tool
//			contracts"). A tool is a gesture state machine: it turns normalized
//			input (input.h) into edits through the EditSession, and describes an
//			overlay, a cursor intent and a status line. It never paints, opens
//			dialogs or reads global key state.
//
//			Per-call context. Tools receive a ToolContext on every call: the
//			session they edit, the view the event belongs to (its camera), the
//			host-owned app::EditorSettings (and the grid policy derived from
//			them), the tool-only ToolSettings and the optional catalog and
//			material ports. Tools keep no pointer to any of them between
//			calls, so the host may swap documents, cameras or settings freely.
//
//			Gesture rules shared by every tool (tested per tool):
//			  * a tool mutates the session only when a gesture completes; during
//			    a gesture it changes nothing and previews through the overlay,
//			    so cancelling (Escape, focus loss, capture loss, tool switch)
//			    restores the pre-gesture state by discarding tool state and
//			    makes no edit;
//			  * one completed gesture is at most one EditSession::Execute, one
//			    history unit;
//			  * a refused edit returns Failed with the reason and changes
//			    nothing;
//			  * events for views the tool does not support are Ignored (the
//			    ToolManager filters them before the tool sees them).
//
//=============================================================================//

#ifndef HAMMER_TOOLS_TOOL_H
#define HAMMER_TOOLS_TOOL_H

#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/app/ops/create_ops.h"
#include "hammer/app/selection.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/ports/material_info.h"
#include "hammer/scene/map_objects.h"
#include "hammer/tools/input.h"
#include "hammer/tools/interaction_policy.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/grid.h"
#include "hammer/viewport/picking.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace hammer::tools
{

// A set of view kinds (the interaction contexts a tool supports).
struct ViewSet
{
	std::uint8_t bits = 0;

	static constexpr std::uint8_t Bit( viewport::ViewKind kind )
	{
		return static_cast<std::uint8_t>( 1u << static_cast<int>( kind ) );
	}
	static constexpr ViewSet All() { return { 0x0f }; }
	static constexpr ViewSet TwoD()
	{
		return { static_cast<std::uint8_t>( Bit( viewport::ViewKind::Top ) |
		                                    Bit( viewport::ViewKind::Front ) |
		                                    Bit( viewport::ViewKind::Side ) ) };
	}
	static constexpr ViewSet Only( viewport::ViewKind kind ) { return { Bit( kind ) }; }

	constexpr bool Contains( viewport::ViewKind kind ) const { return ( bits & Bit( kind ) ) != 0; }
};

// The view an event belongs to: its kind and camera (exactly one camera).
struct ViewRef
{
	viewport::ViewKind kind = viewport::ViewKind::Top;
	const viewport::Camera2D *camera2D = nullptr;
	const viewport::Camera3D *camera3D = nullptr;

	static ViewRef Of( const viewport::Camera2D &camera )
	{
		return { camera.Kind(), &camera, nullptr };
	}
	static ViewRef Of( const viewport::Camera3D &camera )
	{
		return { viewport::ViewKind::Camera3D, nullptr, &camera };
	}

	bool Is2D() const { return kind != viewport::ViewKind::Camera3D && camera2D != nullptr; }
	bool Is3D() const { return kind == viewport::ViewKind::Camera3D && camera3D != nullptr; }
	bool Valid() const { return Is2D() || Is3D(); }
	// The world point's pixel in this view; nothing behind a 3D camera.
	std::optional<viewport::ScreenPoint> Project( const mapgeometry::Vec3d &world ) const;
};

// Tool-only settings (the Tool Properties panel's extras). Editing settings
// shared with menus and scripts -- the face texture, entity class, primitive,
// arch, grid, snap, texture lock and granularity -- are app::EditorSettings,
// their one owner; tools read them through ToolContext::editor.
struct ToolSettings
{
	bool makeArch = false; // the Block tool makes EditorSettings::arch arches
	viewport::MarqueeMode marquee = viewport::MarqueeMode::Inside;
	NudgeStep nudge = NudgeStep::Grid;
	double pointHalfSize = viewport::kDefaultPointHalfSize;
};

// The grid policy EditorSettings describes: its size (rounded to the nearest
// valid power of two in [1, 1024]) and whether snapping is on.
viewport::GridPolicy GridFrom( const app::EditorSettings &editor );

struct ToolContext
{
	app::EditSession &session;
	ViewRef view;
	const app::EditorSettings &editor;
	const ToolSettings &tool;
	const ports::IEntityCatalog *catalog = nullptr;
	const ports::IMaterialInfo *materials = nullptr;
	viewport::GridPolicy grid; // derived from 'editor' (GridFrom); not a second owner

	static ToolContext Make( app::EditSession &session, ViewRef view,
	    const app::EditorSettings &editor, const ToolSettings &tool,
	    const ports::IEntityCatalog *catalog = nullptr,
	    const ports::IMaterialInfo *materials = nullptr )
	{
		return ToolContext{ session, view, editor, tool, catalog, materials, GridFrom( editor ) };
	}

	PickSettings Picking() const
	{
		return { editor.granularity, catalog, tool.pointHalfSize, true, true };
	}
};

// 'success' when a session call succeeded, otherwise Failed with its reason.
template <typename T>
ToolResult ResultOf( const foundation::Expected<T, app::EditError> &result, ToolResult success )
{
	if ( result )
		return success;
	return ToolResult::Fail( result.Error().message );
}

class ITool
{
public:
	virtual ~ITool() = default;

	virtual std::string_view Name() const = 0;
	virtual ViewSet SupportedViews() const = 0;

	virtual ToolResult OnPointer( ToolContext &ctx, const PointerEvent &event ) = 0;
	virtual ToolResult OnKey( ToolContext &ctx, const KeyEvent &event ) = 0;
	virtual ToolResult OnWheel( ToolContext &ctx, const WheelEvent &event )
	{
		(void)ctx;
		(void)event;
		return ToolResult::Ignore();
	}

	// True while a gesture (a press not yet released) is in progress.
	virtual bool InGesture() const = 0;
	// Abandons the gesture in progress: no edit, pre-gesture tool state back.
	virtual void Cancel( CancelReason reason ) = 0;
	// Called when another tool becomes active: cancels any gesture and drops
	// pending, uncommitted tool state (a pending block, a clip line).
	virtual void Deactivate() { Cancel( CancelReason::ToolSwitched ); }

	virtual OverlayList Overlay( const ToolContext &ctx ) const = 0;
	virtual Cursor CursorFor( const ToolContext &ctx, double x, double y ) const = 0;
	virtual std::string Status() const = 0;
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_TOOL_H
