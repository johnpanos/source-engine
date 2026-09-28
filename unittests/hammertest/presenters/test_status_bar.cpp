//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters status bar (RFC 0002, R08 domain logic) over a
//			real EditSession: the selection summary (kinds, plurals, one-class
//			entity naming), the size text of objects and faces, pointer text
//			per view kind with rounding, grid and snap read live from
//			app::EditorSettings, the tool name, and the transient message slot
//			(errors, clearing on the next committed change). Negative checks:
//			empty selection, faces-only and extent-less selections, a success
//			leaves the message alone, destruction after the session.
//
//=============================================================================//

#include "hammer/presenters/status_bar.h"

#include "hammer/app/edit_session.h"
#include "hammer/app/ops/entity_ops.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <memory>

using namespace hammer;
using namespace hammer::presenters;
using app::SelectMode;
using mapgeometry::Vec3d;
using scene::ObjectId;

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "dev/dev_measuregeneric01b";
	scene::MapDocument doc;
	ObjectId a, b, lamp, lamp2, relay, group;
	{
		scene::DocumentEdit edit( doc );
		a = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 16, 32 ) }, tex ) );
		b = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) }, tex ) );
		scene::Entity e;
		e.classname = "light";
		e.SetOrigin( Vec3d( 100, 0, 0 ) );
		lamp = edit.Add( e );
		lamp2 = edit.Add( e );
		scene::Entity r;
		r.classname = "logic_relay";
		relay = edit.Add( r );
		group = edit.Add( scene::Group{} );
		scene::CommitEdit( doc, edit );
	}
	const std::uint32_t topSide = doc.FindSolid( a )->sides[0].vmfId;

	auto session = std::make_unique<app::EditSession>( doc );
	app::EditorSettings settings;
	auto bar = std::make_unique<StatusBar>( *session, settings );

	checks.That( bar->SelectionText() == "No selection" && bar->SizeText().empty(),
	    "empty selection (negative)" );

	const std::uint64_t rev = bar->Revision();
	checks.That(
	    session->SelectObjects( { a, b, lamp }, SelectMode::Replace ).HasValue(), "select" );
	checks.That( bar->Revision() > rev, "revision moves" );
	checks.Equal( bar->SelectionText(), std::string( "2 solids, 1 entity (light)" ), "summary" );
	checks.Equal( bar->SizeText(), std::string( "w 108 h 40 d 24" ),
	    "size: x, z, y extents (the light adds a +/-8 box)" );
	checks.That( session->SelectObjects( { a }, SelectMode::Replace ).HasValue() &&
	                 bar->SelectionText() == "1 solid" && bar->SizeText() == "w 64 h 32 d 16",
	    "one solid" );
	checks.That( session->SelectObjects( { lamp, lamp2 }, SelectMode::Replace ).HasValue() &&
	                 bar->SelectionText() == "2 entities (light)",
	    "plural with one class" );
	checks.That( session->SelectObjects( { relay }, SelectMode::Add ).HasValue() &&
	                 bar->SelectionText() == "3 entities",
	    "mixed classes name none" );
	checks.That( session->SetSelection(
	                        app::Selection{ { group }, { scene::FaceRef{ a, topSide } }, group } )
	                     .HasValue() &&
	                 bar->SelectionText() == "1 group, 1 face",
	    "groups and faces" );
	checks.Equal( bar->SizeText(), std::string( "w 64 h 0 d 16" ),
	    "a face's polygon extent; an empty group none" );
	checks.That( session->SelectObjects( { relay }, SelectMode::Replace ).HasValue() &&
	                 bar->SizeText().empty() == false,
	    "a point entity without origin still has a box" );

	// Pointer.
	bar->SetPointer( viewport::ViewKind::Top, Vec3d( 12.345, -4, 7 ) );
	checks.Equal( bar->PointerText(), std::string( "x 12.35 y -4" ), "top view: x y, rounded" );
	bar->SetPointer( viewport::ViewKind::Front, Vec3d( 1, 2, 3 ) );
	checks.Equal( bar->PointerText(), std::string( "x 1 z 3" ), "front view: x z" );
	bar->SetPointer( viewport::ViewKind::Side, Vec3d( 1, 2, 3 ) );
	checks.Equal( bar->PointerText(), std::string( "y 2 z 3" ), "side view: y z" );
	bar->SetPointer( viewport::ViewKind::Camera3D, Vec3d( 1, 2, 3 ) );
	checks.Equal( bar->PointerText(), std::string( "x 1 y 2 z 3" ), "3D view: all three" );
	bar->ClearPointer();
	checks.That( bar->PointerText().empty(), "cleared" );

	// Grid, snap, tool.
	checks.That( bar->GridText() == "Grid 64" && bar->SnapText() == "Snap on",
	    "defaults from the settings owner" );
	settings.gridSize = 8;
	settings.snapToGrid = false;
	checks.That( bar->GridText() == "Grid 8" && bar->SnapText() == "Snap off", "read live" );
	bar->SetTool( "Block" );
	checks.Equal( bar->ToolText(), std::string( "Block" ), "tool" );

	// Messages.
	auto refused = session->Execute( "Nothing",
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::SetKey( edit, { a }, "k", "v" );
	    } );
	checks.That( !refused, "a refused operation" );
	bar->Report( foundation::Expected<void, app::EditError>(
	    foundation::MakeUnexpected( refused.Error() ) ) );
	checks.That(
	    !bar->Message().empty() && bar->Message() == refused.Error().message, "error shown" );
	bar->Report( foundation::Expected<void, app::EditError>() );
	checks.That( !bar->Message().empty(), "a success leaves the message (negative)" );
	checks.That(
	    session->SelectObjects( { a }, SelectMode::Replace ).HasValue() && !bar->Message().empty(),
	    "a selection change keeps it" );
	auto done = session->Execute( "Name",
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::SetKey( edit, { lamp }, "targetname", "l" );
	    } );
	checks.That( done.HasValue() && bar->Message().empty(), "the next committed change clears it" );
	bar->ShowMessage( "Saved" );
	bar->ClearMessage();
	checks.That( bar->Message().empty(), "explicit clear" );

	session.reset();
	bar.reset();
	checks.That( true, "destroyed after the session" );
	return checks.Report();
}
