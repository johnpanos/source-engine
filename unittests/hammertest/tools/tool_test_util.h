//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared rig for the hammer.tools suites: a real EditSession, one
//			camera per view (800 x 600 pixels, 2D zoom 1 pixel per unit centred
//			on the origin, so plane (u, v) is pixel (400 + u, 300 - v)), editor
//			settings (grid 16, Objects granularity), tool settings, and helpers that build normalized pointer and
//			key events from plane coordinates. Tools are driven through a
//			ToolManager, as a host drives them.
//
//=============================================================================//

#ifndef HAMMERTEST_TOOLS_TOOL_TEST_UTIL_H
#define HAMMERTEST_TOOLS_TOOL_TEST_UTIL_H

#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/scene/change_set.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "hammer/tools/tool.h"
#include "hammer/tools/tool_manager.h"
#include "mapgeometry/vec3.h"

#include <memory>
#include <string>
#include <utility>

namespace tooltest
{

using namespace hammer;
using mapgeometry::Vec3d;
using tools::Mods;
using tools::PointerButton;
using tools::PointerPhase;
using viewport::ViewKind;

inline scene::FaceTexture DevTexture()
{
	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	return tex;
}

// A document builder: add objects, then hand the document to a Rig.
struct DocBuilder
{
	scene::MapDocument doc;

	scene::ObjectId Box( Vec3d mins, Vec3d maxs )
	{
		scene::DocumentEdit edit( doc );
		const scene::ObjectId id = edit.Add( scene::MakeBoxSolid( { mins, maxs }, DevTexture() ) );
		scene::CommitEdit( doc, edit );
		return id;
	}
	scene::ObjectId Point( const char *classname, Vec3d origin )
	{
		scene::DocumentEdit edit( doc );
		scene::Entity e;
		e.classname = classname;
		e.SetOrigin( origin );
		const scene::ObjectId id = edit.Add( e );
		scene::CommitEdit( doc, edit );
		return id;
	}
};

struct Rig
{
	app::EditSession session;
	viewport::Camera2D top;
	viewport::Camera2D front;
	viewport::Camera2D side;
	viewport::Camera3D camera;
	app::EditorSettings editor;
	tools::ToolSettings tool;
	const ports::IEntityCatalog *catalog = nullptr;
	const ports::IMaterialInfo *materials = nullptr;
	tools::ToolManager manager;

	explicit Rig( scene::MapDocument doc = scene::MapDocument() ) : session( std::move( doc ) )
	{
		top.SetKind( ViewKind::Top );
		front.SetKind( ViewKind::Front );
		side.SetKind( ViewKind::Side );
		for ( viewport::Camera2D *c : { &top, &front, &side } )
		{
			c->SetViewport( 800, 600 );
			c->SetZoom( 1.0 );
		}
		camera.SetViewport( 800, 600 );
		editor.gridSize = 16.0;
		editor.granularity = app::SelectionGranularity::Objects;
		editor.faceTexture = DevTexture();
	}

	const viewport::Camera2D &Cam( ViewKind kind ) const
	{
		return kind == ViewKind::Front ? front : ( kind == ViewKind::Side ? side : top );
	}

	tools::ToolContext Ctx( ViewKind kind )
	{
		const tools::ViewRef view = kind == ViewKind::Camera3D ? tools::ViewRef::Of( camera )
		                                                       : tools::ViewRef::Of( Cam( kind ) );
		return tools::ToolContext::Make( session, view, editor, tool, catalog, materials );
	}

	template <typename T> T *Use( std::unique_ptr<T> tool )
	{
		T *raw = tool.get();
		manager.Add( std::move( tool ) );
		manager.Activate( raw->Name() );
		return raw;
	}

	// Pixel of plane point (u, v) in a 2D view.
	viewport::ScreenPoint Px( ViewKind kind, double u, double v ) const
	{
		return Cam( kind ).PlaneToScreen( { u, v } );
	}

	tools::ToolResult Pointer( ViewKind kind, PointerPhase phase, viewport::ScreenPoint p,
	    tools::Modifiers mods = {}, PointerButton button = PointerButton::Left )
	{
		tools::PointerEvent e;
		e.phase = phase;
		e.button = button;
		e.modifiers = mods;
		e.x = p.x;
		e.y = p.y;
		e.view = kind;
		tools::ToolContext ctx = Ctx( kind );
		return manager.OnPointer( ctx, e );
	}
	// Plane-coordinate variants for 2D views.
	tools::ToolResult Down( ViewKind k, double u, double v, tools::Modifiers m = {},
	    PointerButton b = PointerButton::Left )
	{
		return Pointer( k, PointerPhase::Down, Px( k, u, v ), m, b );
	}
	tools::ToolResult Move( ViewKind k, double u, double v, tools::Modifiers m = {},
	    PointerButton b = PointerButton::Left )
	{
		return Pointer( k, PointerPhase::Move, Px( k, u, v ), m, b );
	}
	tools::ToolResult Up( ViewKind k, double u, double v, tools::Modifiers m = {},
	    PointerButton b = PointerButton::Left )
	{
		return Pointer( k, PointerPhase::Up, Px( k, u, v ), m, b );
	}
	// Down, a move to the end, and Up (a drag, or a click when short).
	tools::ToolResult Drag(
	    ViewKind k, double u0, double v0, double u1, double v1, tools::Modifiers m = {} )
	{
		Down( k, u0, v0, m );
		Move( k, ( u0 + u1 ) * 0.5, ( v0 + v1 ) * 0.5, m );
		Move( k, u1, v1, m );
		return Up( k, u1, v1, m );
	}
	tools::ToolResult Key( ViewKind k, tools::KeyEvent e )
	{
		tools::ToolContext ctx = Ctx( k );
		return manager.OnKey( ctx, e );
	}
	tools::ToolResult Key( ViewKind k, tools::Key key, tools::Modifiers m = {} )
	{
		return Key( k, tools::KeyEvent::Press( key, m ) );
	}
	tools::OverlayList Overlay( ViewKind k )
	{
		const tools::ToolContext ctx = Ctx( k );
		return manager.Overlay( ctx );
	}
	tools::Cursor CursorAt( ViewKind k, viewport::ScreenPoint p )
	{
		const tools::ToolContext ctx = Ctx( k );
		return manager.CursorFor( ctx, p.x, p.y );
	}

	// Applied history units (the undo position).
	std::size_t HistorySize() const { return session.History().Position(); }
	// The label of the entry an undo would revert.
	std::string LastLabel() const
	{
		const app::HistoryEntry *entry = session.History().UndoEntry();
		return entry ? entry->label : std::string();
	}
};

inline bool Is( const tools::ToolResult &r, tools::ToolResultKind k )
{
	return r.kind == k;
}

inline std::optional<scene::Box> BoundsOf( const app::EditSession &s, scene::ObjectId id )
{
	return scene::ObjectBounds( s.Document(), id );
}

} // namespace tooltest

#endif // HAMMERTEST_TOOLS_TOOL_TEST_UTIL_H
