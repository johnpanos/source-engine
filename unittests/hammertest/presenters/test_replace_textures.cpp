//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters ReplaceTexturesDialog (RFC 0002, R08): the
//			legacy defaults at Open (current material, selection scope when
//			something is selected, exact, everything else off), validation
//			messages, the live preview and used-material list, Apply through
//			the command layer with the legacy messages and one "Replace
//			Textures" undo step, mark only (solids, or faces with the face
//			tool), and an open/replace/undo/redo/save/reopen round trip
//			through the file-store port. Negative checks: refusals change
//			nothing and record no history.
//
//=============================================================================//

#include "hammer/presenters/replace_textures.h"

#include "app/fake_file_store.h"
#include "fakes/fake_material_info.h"
#include "hammer/app/editor_settings.h"
#include "hammer/formats/vmf_map_codec.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

using namespace hammer;
using presenters::ReplaceTexturesDialog;
using mapgeometry::Vec3d;

namespace
{

std::string FaceMaterial( const app::EditSession &session, scene::ObjectId solid, std::size_t i )
{
	return session.Document().FindSolid( solid )->sides[i].texture.material;
}

} // namespace

int main()
{
	testing::Checks checks;

	hammertest::FakeMaterialInfo materials;
	materials.Add( "dev/dev_measuregeneric01b", 128, 128 ).Add( "brick/brickwall001", 256, 128 );
	formats::VmfMapCodec codec;
	hammertest::InMemoryFileStore store;
	app::EditSession session;
	app::EditorSettings settings;
	app::MapFragment clipboard;
	app::SessionCommands commands(
	    session, settings, { &codec, &store, nullptr, nullptr, &materials, &clipboard } );

	scene::FaceTexture dev;
	dev.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::ObjectId a, b, hidden;
	checks.That(
	    session
	        .Execute( "setup",
	            [&]( scene::DocumentEdit &e ) -> app::EditResult
	            {
		            a = e.Add(
		                scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) }, dev ) );
		            scene::Solid other =
		                scene::MakeBoxSolid( { Vec3d( 128, 0, 0 ), Vec3d( 192, 64, 64 ) }, dev );
		            other.sides[0].texture.material = "brick/brickwall001";
		            b = e.Add( other );
		            scene::Solid h =
		                scene::MakeBoxSolid( { Vec3d( 256, 0, 0 ), Vec3d( 320, 64, 64 ) }, dev );
		            h.hidden = true;
		            hidden = e.Add( h );
		            return {};
	            } )
	        .HasValue(),
	    "setup" );
	session.MarkSaved();

	ReplaceTexturesDialog dialog( session, commands, &materials );

	// Defaults.
	dialog.Open( "dev/dev_measuregeneric01b", false );
	checks.That(
	    dialog.Draft().find == "dev/dev_measuregeneric01b" && dialog.Draft().replace.empty() &&
	        dialog.Draft().scope == ReplaceTexturesDialog::Scope::Everything &&
	        dialog.Draft().match == app::ops::MaterialMatch::Exact && !dialog.Draft().markOnly &&
	        !dialog.Draft().includeHidden && !dialog.Draft().rescale,
	    "Open with nothing selected: legacy defaults, everything" );
	checks.That( !dialog.SelectionScopeAvailable() && dialog.RescaleAvailable(),
	    "selection scope unavailable without a selection" );
	checks.That( dialog.Validate() == "Enter the replacement material.",
	    "an empty replacement is named (negative)" );
	checks.That( !dialog.Apply().applied && !session.IsModified(),
	    "Apply refuses an invalid draft and changes nothing (negative)" );
	checks.That( session.SelectObjects( { a }, app::SelectMode::Replace ).HasValue(), "select" );
	dialog.Open( "dev/dev_measuregeneric01b", false );
	checks.That( dialog.Draft().scope == ReplaceTexturesDialog::Scope::Selection,
	    "Open with a selection: the selection scope" );

	// Preview and used materials.
	dialog.Draft().scope = ReplaceTexturesDialog::Scope::Everything;
	checks.That( dialog.Preview().faces == 11 && dialog.Preview().solids == 2,
	    "preview: visible matching faces and solids" );
	dialog.Draft().includeHidden = true;
	checks.That( dialog.Preview().faces == 17, "preview with hidden objects" );
	dialog.Draft().includeHidden = false;
	const auto used = dialog.UsedMaterials();
	checks.That( used.size() == 2 && used[0].name == "brick/brickwall001" && used[0].faces == 1 &&
	                 used[1].name == "dev/dev_measuregeneric01b" && used[1].faces == 17,
	    "used materials as normalized identities with face counts" );
	checks.That( dialog.Candidates().size() == 2, "candidates come from the material port" );

	// Replace in the selection, then everything; undo, redo.
	dialog.Draft().scope = ReplaceTexturesDialog::Scope::Selection;
	dialog.Draft().replace = "Brick\\BrickWall001";
	checks.That( dialog.Validate().empty(), "a valid draft" );
	const std::size_t setupSteps = session.History().Size();
	const auto replaced = dialog.Apply();
	checks.That( replaced.applied && replaced.done && replaced.message == "6 textures replaced.",
	    "replace within the selection: the legacy message" );
	checks.That( FaceMaterial( session, a, 0 ) == "brick/brickwall001" &&
	                 FaceMaterial( session, b, 1 ) == "DEV/DEV_MEASUREGENERIC01B",
	    "only the selection changed; the stored name is the identity" );
	checks.That( session.History().Size() == setupSteps + 1 &&
	                 session.History().UndoEntry()->label == "Replace Textures",
	    "one Replace Textures undo step" );
	checks.That(
	    session.Undo().HasValue() && FaceMaterial( session, a, 0 ) == "DEV/DEV_MEASUREGENERIC01B",
	    "undo restores the materials" );
	checks.That( session.Redo().HasValue() && FaceMaterial( session, a, 0 ) == "brick/brickwall001",
	    "redo replaces again" );

	// Nothing matched: the legacy message, no history.
	dialog.Draft().find = "nope";
	const auto none = dialog.Apply();
	checks.That( none.done && !none.applied && none.message == "0 textures replaced." &&
	                 session.History().Size() == setupSteps + 1,
	    "no match: \"0 textures replaced.\" and no history (negative)" );

	// Rescale.
	dialog.Draft() = {};
	dialog.Draft().find = "dev/dev_measuregeneric01b";
	dialog.Draft().replace = "brick/brickwall001";
	dialog.Draft().rescale = true;
	const double before = session.Document().FindSolid( b )->sides[1].texture.u.scale;
	checks.That( dialog.Apply().message == "5 textures replaced.", "rescale everywhere visible" );
	checks.Near( session.Document().FindSolid( b )->sides[1].texture.u.scale, before * 0.5, 1e-12,
	    "rescaled to the new width" );
	checks.That( FaceMaterial( session, hidden, 0 ) == "DEV/DEV_MEASUREGENERIC01B",
	    "hidden objects are left alone" );
	dialog.Draft().find = "brick/brickwall001";
	dialog.Draft().replace = "unknown/size";
	const auto refused = dialog.Apply();
	checks.That( !refused.applied && !refused.done &&
	                 refused.message.find( "unknown/size" ) != std::string::npos &&
	                 FaceMaterial( session, b, 0 ) == "brick/brickwall001",
	    "rescale to an unknown size refuses and names it (negative)" );

	// Mark only.
	dialog.Open( "brick/brickwall001", false );
	dialog.Draft().scope = ReplaceTexturesDialog::Scope::Everything;
	dialog.Draft().markOnly = true;
	dialog.Draft().includeHidden = true;
	checks.That( !dialog.HiddenAvailable() && dialog.Validate().empty(),
	    "mark only needs no replacement and never marks hidden objects" );
	const std::size_t history = session.History().Size();
	checks.That( dialog.Apply().message == "2 solids marked." &&
	                 session.CurrentSelection().objects == std::vector<scene::ObjectId>{ a, b } &&
	                 session.History().Size() == history,
	    "mark solids: a selection change, no history" );
	dialog.Open( "brick/brickwall001", true );
	dialog.Draft().scope = ReplaceTexturesDialog::Scope::Selection;
	dialog.Draft().markOnly = true;
	checks.That( dialog.MarksFaces() && dialog.Apply().message == "12 faces marked." &&
	                 session.CurrentSelection().faces.size() == 12,
	    "mark faces within the selection (legacy marked nothing here)" );

	// Save and reopen.
	checks.That( commands.Execute( "save", { { "path", "maps/replaced.vmf" } } ).HasValue() &&
	                 !session.IsModified(),
	    "save" );
	const std::string &vmf = store.files["maps/replaced.vmf"];
	checks.That( vmf.find( "\"material\" \"brick/brickwall001\"" ) != std::string::npos &&
	                 vmf.find( "\"material\" \"DEV/DEV_MEASUREGENERIC01B\"" ) != std::string::npos,
	    "the VMF holds the replaced and the untouched (hidden) materials" );
	checks.That(
	    commands.Execute( "open", { { "path", "maps/replaced.vmf" } } ).HasValue(), "reopen" );
	int bricks = 0;
	for ( scene::ObjectId id : session.Document().SolidIds() )
		for ( const scene::Side &side : session.Document().FindSolid( id )->sides )
			bricks += side.texture.material == "brick/brickwall001";
	checks.That( bricks == 12, "the replacement survives save and reopen" );

	// Without a material port rescaling is unavailable.
	ReplaceTexturesDialog bare( session, commands, nullptr );
	bare.Open( "brick/brickwall001", false );
	bare.Draft().replace = "dev/dev_measuregeneric01b";
	bare.Draft().rescale = true;
	checks.That( !bare.RescaleAvailable() && !bare.Validate().empty() && !bare.Apply().applied,
	    "no material port: rescale refused (negative)" );

	return checks.Report();
}
