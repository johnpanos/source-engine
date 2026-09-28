//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters editor workspace (RFC 0002, R08 domain logic),
//			driven headlessly the way a UI shell drives it: a new map, the
//			Block tool by its shortcut, a drag in the Top view and Enter, the
//			snapshot showing the block selected, Ctrl+Z / Ctrl+Y, an entity
//			placed by the Entity tool in the 3D view, the inspector showing its
//			class, a key drafted and committed by a selection change, save and
//			reopen through the fake codec and file store, the problems panel
//			listing "no player start" until one is placed, camera pan and zoom,
//			focus loss mid-drag, 3D fly, grid lines, host requests (dialogs,
//			run map) and a build through a fake builder. Negative checks:
//			disabled shortcuts, a failed open, missing services, a gesture
//			cancelled by document replacement.
//
//=============================================================================//

#include "hammer/presenters/editor_workspace.h"

#include "hammer/scene/map_queries.h"
#include "testing/checks.h"

#include "app/fake_file_store.h"
#include "fakes/fake_entity_catalog.h"
#include "fakes/fake_map_codec.h"
#include "fakes/fake_material_info.h"

#include <algorithm>

using namespace hammer;
using namespace hammer::presenters;
using mapgeometry::Vec3d;
using tools::KeyEvent;
using tools::Mods;
using tools::PointerButton;
using tools::PointerEvent;
using tools::PointerPhase;
using viewport::ViewKind;
using C = hammertest::FakeEntityCatalog;

namespace
{

class FakeBuilder final : public ports::IMapBuilder
{
public:
	int builds = 0;
	ports::MapBuildResult Build( const ports::MapBuildRequest &request ) override
	{
		++builds;
		lastPublish = request.publish;
		ports::MapBuildResult result;
		result.ok = true;
		result.status = "pass";
		return result;
	}
	bool lastPublish = false;
};

PointerEvent Ptr( PointerPhase phase, PointerButton button, viewport::ScreenPoint p,
    tools::Modifiers modifiers = {} )
{
	PointerEvent e;
	e.phase = phase;
	e.button = button;
	e.x = p.x;
	e.y = p.y;
	e.modifiers = modifiers;
	return e;
}

// A left click (press and release in place).
void Click( EditorWorkspace &ws, ViewKind view, viewport::ScreenPoint p )
{
	ws.OnPointer( view, Ptr( PointerPhase::Down, PointerButton::Left, p ) );
	ws.OnPointer( view, Ptr( PointerPhase::Up, PointerButton::Left, p ) );
}

// A left drag from 'a' to 'b' in steps.
void Drag( EditorWorkspace &ws, ViewKind view, viewport::ScreenPoint a, viewport::ScreenPoint b,
    bool release = true )
{
	ws.OnPointer( view, Ptr( PointerPhase::Down, PointerButton::Left, a ) );
	for ( int i = 1; i <= 4; ++i )
	{
		const viewport::ScreenPoint p{ a.x + ( b.x - a.x ) * i / 4, a.y + ( b.y - a.y ) * i / 4 };
		ws.OnPointer( view, Ptr( PointerPhase::Move, PointerButton::Left, p ) );
	}
	if ( release )
	{
		ws.OnPointer( view, Ptr( PointerPhase::Up, PointerButton::Left, b ) );
	}
}

bool HasProblem( EditorWorkspace &ws, const std::string &code )
{
	const auto &rows = ws.Problems().Rows();
	return std::any_of( rows.begin(), rows.end(),
	    [&]( const ProblemRow &row )
	    {
		    return row.codeName == code;
	    } );
}

} // namespace

int main()
{
	testing::Checks checks;

	C catalog;
	catalog
	    .AddPoint(
	        "light", { C::Key( "targetname", "target_source" ), C::Key( "_light", "color255" ) } )
	    .Box( { -8, -8, -8 }, { 8, 8, 8 } );
	catalog.AddPoint( "info_player_start", { C::Key( "angles", "angle", "0 0 0" ) } )
	    .Box( { -16, -16, 0 }, { 16, 16, 72 } );
	hammertest::FakeMaterialInfo materials;
	materials.Add( "DEV/DEV_MEASUREGENERIC01B", 128, 128 );
	hammertest::FakeMapCodec codec;
	hammertest::InMemoryFileStore store;
	FakeBuilder builder;

	WorkspaceServices services;
	services.codec = &codec;
	services.store = &store;
	services.builder = &builder;
	services.catalog = &catalog;
	services.materials = &materials;
	EditorWorkspace ws( services );
	ws.SetViewportSize( ViewKind::Top, 800, 600 );
	ws.SetViewportSize( ViewKind::Front, 800, 600 );
	ws.SetViewportSize( ViewKind::Side, 800, 600 );
	ws.SetViewportSize( ViewKind::Camera3D, 640, 480 );
	checks.That( ws.Classes() && ws.Materials(), "optional presenters exist with their ports" );
	checks.That( ws.Tools().Active() && ws.Tools().Active()->Name() == "selection" &&
	                 ws.Tools().Names().size() == 6,
	    "every tool registered; selection active" );

	// --- New map, block by shortcut and drag ---------------------------------------------
	checks.That(
	    ws.New().HasValue() && ws.MapPath().empty() && ws.Snapshot().solids.empty(), "new map" );
	InputOutcome key = ws.OnKey( ViewKind::Top, KeyEvent::Char( 'b', Mods( tools::kShift ) ) );
	checks.That( key.handled && key.actionId == "tools.block" &&
	                 ws.Tools().Active()->Name() == "block" && ws.Status().ToolText() == "block",
	    "Shift+B activates the block tool" );
	viewport::Camera2D &top = ws.Camera2DFor( ViewKind::Top );
	const viewport::ScreenPoint p0 = top.WorldToScreen( Vec3d( 0, 0, 0 ) );
	const viewport::ScreenPoint p1 = top.WorldToScreen( Vec3d( 128, 64, 0 ) );
	Drag( ws, ViewKind::Top, p0, p1 );
	checks.That( ws.Overlay( ViewKind::Top ).CountRole( tools::OverlayRole::Pending ) > 0 &&
	                 ws.Session().Document().Solids().empty(),
	    "a pending box previews; nothing created yet" );
	InputOutcome enter = ws.OnKey( ViewKind::Top, KeyEvent::Press( tools::Key::Enter ) );
	checks.That( enter.handled && enter.redraw, "Enter goes to the tool" );
	checks.That( ws.Session().Document().Solids().size() == 1 && ws.Snapshot().solids.size() == 1 &&
	                 ws.Snapshot().solids[0].selected,
	    "the block exists and the snapshot shows it selected" );
	checks.That( ws.Snapshot().bounds && ws.Snapshot().bounds->Size().x == 128 &&
	                 ws.Snapshot().bounds->Size().y == 64,
	    "the snapshot has the dragged extent" );
	checks.That( ws.Status().SelectionText() == "1 solid", "status bar summary" );

	// Undo / redo by shortcut.
	InputOutcome undo = ws.OnKey( ViewKind::Top, KeyEvent::Char( 'z', Mods( tools::kCtrl ) ) );
	checks.That( undo.actionId == "edit.undo" && ws.Snapshot().solids.empty() &&
	                 ws.History().Position() == 0,
	    "Ctrl+Z empties the snapshot again" );
	checks.That( ws.OnKey( ViewKind::Top, KeyEvent::Char( 'y', Mods( tools::kCtrl ) ) ).actionId ==
	                     "edit.redo" &&
	                 ws.Snapshot().solids.size() == 1,
	    "Ctrl+Y restores it" );

	// --- Entity in 3D ----------------------------------------------------------------------
	checks.That(
	    ws.Classes()->SetActive( "light" ).HasValue() && ws.Settings().entityClass == "light",
	    "choose the class" );
	ws.OnKey( ViewKind::Camera3D, KeyEvent::Char( 'e', Mods( tools::kShift ) ) );
	checks.That( ws.Tools().Active()->Name() == "entity", "Shift+E" );
	viewport::Camera3D &cam3 = ws.Camera3DView();
	cam3.SetPosition( Vec3d( 64, -300, 300 ) );
	cam3.LookAt( Vec3d( 64, 32, 64 ) );
	Click( ws, ViewKind::Camera3D, { 320, 240 } );
	checks.Equal(
	    ws.Session().Document().Entities().size(), std::size_t( 1 ), "entity placed in 3D" );
	const scene::ObjectId lamp = ws.Session().Document().Entities().empty()
	                                 ? scene::ObjectId{}
	                                 : ws.Session().Document().Entities().begin()->first;
	checks.That( ws.Inspector().Class() == app::PropertyValue::Single( "light" ) &&
	                 ws.Snapshot().entities.size() == 1 && ws.Snapshot().entities[0].selected,
	    "the inspector shows its class; the snapshot shows it selected" );
	const std::optional<Vec3d> origin =
	    lamp.IsValid() ? ws.Session().Document().FindEntity( lamp )->Origin() : std::nullopt;
	checks.That( origin && origin->z >= 64, "placed on top of the block" );

	// Draft, then a selection change commits it.
	checks.That( ws.Inspector().SetDraft( "targetname", "lamp" ).HasValue(), "draft a key" );
	ws.OnKey( ViewKind::Top, KeyEvent::Char( 's', Mods( tools::kShift ) ) );
	Click( ws, ViewKind::Top, top.WorldToScreen( Vec3d( 2000, 2000, 0 ) ) );
	checks.That(
	    ws.Session().CurrentSelection().Empty(), "clicking empty space cleared the selection" );
	checks.That( ws.Session().Document().FindEntity( lamp ) &&
	                 ws.Session().Document().FindEntity( lamp )->Name() == "lamp" &&
	                 ws.History().Rows().back().label == "Edit properties",
	    "the selection change committed the draft as one step" );

	// --- Problems, save and reopen -------------------------------------------------------------
	checks.That( HasProblem( ws, "no-player-start" ), "no player start listed" );
	checks.That( ws.Save( "maps/room.vmf" ).HasValue() && store.Exists( "maps/room.vmf" ) &&
	                 ws.MapPath() == "maps/room.vmf" && !ws.Session().IsModified(),
	    "save" );
	checks.That( ws.Open( "maps/room.vmf" ).HasValue() && ws.History().Rows().empty() &&
	                 ws.Snapshot().solids.size() == 1 && ws.Snapshot().entities.size() == 1,
	    "reopen: the snapshot is rebuilt" );
	checks.That( !scene::FindEntitiesByName( ws.Session().Document(), "lamp" ).empty(),
	    "the edited key survived the round trip" );
	checks.That( HasProblem( ws, "no-player-start" ), "still no player start" );
	checks.That(
	    ws.Classes()->SetActive( "info_player_start" ).HasValue(), "choose the spawn class" );
	ws.OnKey( ViewKind::Top, KeyEvent::Char( 'e', Mods( tools::kShift ) ) );
	Click( ws, ViewKind::Top, top.WorldToScreen( Vec3d( 256, 256, 0 ) ) );
	checks.That(
	    !HasProblem( ws, "no-player-start" ) &&
	        !scene::FindEntitiesByClass( ws.Session().Document(), "info_player_start" ).empty(),
	    "placing one clears the problem" );

	// Shortcuts that need the host.
	checks.That(
	    ws.OnKey( ViewKind::Top, KeyEvent::Char( 'o', Mods( tools::kCtrl ) ) ).hostRequest ==
	        "open_dialog",
	    "Ctrl+O asks the host for a file" );
	store.files.erase( "maps/room.vmf" );
	InputOutcome saved = ws.OnKey( ViewKind::Top, KeyEvent::Char( 's', Mods( tools::kCtrl ) ) );
	checks.That( saved.actionId == "file.save" && store.Exists( "maps/room.vmf" ) &&
	                 !ws.Session().IsModified(),
	    "Ctrl+S saves to the current path" );
	InputOutcome built =
	    ws.OnKey( ViewKind::Top, KeyEvent::Press( tools::Key::F9, Mods( tools::kShift ) ) );
	checks.That( built.hostRequest == "run_map" && builder.builds == 1 && builder.lastPublish,
	    "Shift+F9 builds, publishes and asks the host to run" );
	std::string request;
	checks.That( ws.Build( "maps/room.vmf", false, &request ).HasValue() && request.empty() &&
	                 builder.builds == 2 && !builder.lastPublish,
	    "a plain build" );

	// --- Cameras -------------------------------------------------------------------------------
	const viewport::PlanePoint center = top.Center();
	ws.OnPointer( ViewKind::Top, Ptr( PointerPhase::Down, PointerButton::Middle, { 100, 100 } ) );
	InputOutcome pan = ws.OnPointer(
	    ViewKind::Top, Ptr( PointerPhase::Move, PointerButton::Middle, { 160, 100 } ) );
	ws.OnPointer( ViewKind::Top, Ptr( PointerPhase::Up, PointerButton::Middle, { 160, 100 } ) );
	checks.That(
	    pan.handled && pan.redraw && !( top.Center() == center ), "middle drag pans the 2D view" );
	const double zoom = top.Zoom();
	tools::WheelEvent wheel;
	wheel.steps = 1;
	wheel.x = 400;
	wheel.y = 300;
	checks.That(
	    ws.OnWheel( ViewKind::Top, wheel ).handled && top.Zoom() > zoom, "the wheel zooms" );
	const double zoomedIn = top.Zoom();
	ws.FrameDocument(); // a host's "reset views"
	checks.That( ws.Snapshot().bounds &&
	                 top.Center() == top.ToPlane( ws.Snapshot().bounds->Center() ) &&
	                 top.Zoom() < zoomedIn,
	    "FrameDocument re-frames a panned and zoomed view on the document" );
	const Vec3d eye = cam3.Position();
	ws.OnKey( ViewKind::Camera3D, KeyEvent::Char( 'w' ) );
	checks.That( ws.Advance( 0.5 ).redraw && !( cam3.Position() == eye ), "W flies the 3D camera" );
	ws.OnKey( ViewKind::Camera3D, KeyEvent::Char( 'w', {}, tools::KeyPhase::Release ) );
	checks.That( !ws.Advance( 0.5 ).redraw, "released: no motion" );
	checks.That(
	    !ws.GridLines( ViewKind::Top ).empty() && ws.GridLines( ViewKind::Camera3D ).empty(),
	    "grid lines for 2D views only" );

	// --- Focus loss mid-drag, replacement mid-drag (negative) -------------------------------------
	ws.OnKey( ViewKind::Top, KeyEvent::Char( 'b', Mods( tools::kShift ) ) );
	std::uint64_t revision = ws.Session().Revision();
	const std::size_t solids = ws.Session().Document().Solids().size();
	Drag( ws, ViewKind::Top, top.WorldToScreen( Vec3d( 512, 0, 0 ) ),
	    top.WorldToScreen( Vec3d( 640, 64, 0 ) ), false );
	checks.That( ws.Tools().Active()->InGesture(), "mid-drag" );
	checks.That(
	    ws.OnFocusLost().redraw && !ws.Tools().Active()->InGesture(), "focus loss cancels" );
	ws.OnKey( ViewKind::Top, KeyEvent::Press( tools::Key::Enter ) );
	checks.That(
	    ws.Session().Revision() == revision && ws.Session().Document().Solids().size() == solids,
	    "no edit after focus loss" );
	Drag( ws, ViewKind::Top, top.WorldToScreen( Vec3d( 512, 0, 0 ) ),
	    top.WorldToScreen( Vec3d( 640, 64, 0 ) ), false );
	checks.That( ws.New().HasValue() && !ws.Tools().Active()->InGesture(),
	    "replacement cancels the gesture" );
	ws.OnKey( ViewKind::Top, KeyEvent::Press( tools::Key::Enter ) );
	checks.That( ws.Session().Document().Solids().empty() && ws.Snapshot().solids.empty(),
	    "and nothing is created in the new map" );
	checks.That( top.Center() == viewport::PlanePoint{}, "an empty map centres the views" );

	// --- Other negatives ---------------------------------------------------------------------
	InputOutcome disabled = ws.OnKey( ViewKind::Top, KeyEvent::Char( 'z', Mods( tools::kCtrl ) ) );
	checks.That( !disabled.handled && disabled.status == "Undo is not available now" &&
	                 ws.Status().Message() == "Undo is not available now",
	    "a disabled shortcut is reported, not run" );
	revision = ws.Session().Revision();
	checks.That( !ws.Open( "maps/missing.vmf" ) && ws.Session().Revision() == revision &&
	                 ws.MapPath().empty() && !ws.Status().Message().empty(),
	    "a failed open changes nothing" );
	checks.That( ws.RunAction( "tools.clip" ).handled && ws.Tools().Active()->Name() == "clip",
	    "menus run actions through the same path" );
	checks.That( !ws.RunAction( "no.such" ).handled, "unknown action (negative)" );

	{
		EditorWorkspace bare( WorkspaceServices{} );
		checks.That( !bare.Classes() && !bare.Materials(),
		    "no catalog or materials: no palette or browser" );
		checks.That( !bare.Save( "a.vmf" ) && !bare.Status().Message().empty(),
		    "save without a codec fails" );
		checks.That(
		    bare.OnKey( ViewKind::Top, KeyEvent::Char( 's', Mods( tools::kCtrl ) ) ).hostRequest ==
		        "save_dialog",
		    "Ctrl+S without a path asks for one" );
		checks.That( !bare.Build( "a.vmf", true ), "build without a builder fails" );
	}

	return checks.Report();
}
