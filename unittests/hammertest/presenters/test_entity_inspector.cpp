//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters entity inspector (RFC 0002, R08 domain logic)
//			driven through a real EditSession and the fake catalog: schema
//			rows in catalog order with extras after, Single/Mixed/Unset
//			aggregates and default differences, tri-state flags, outputs with
//			target validity and inputs from other entities, SmartEdit off,
//			mixed-class schema intersection, draft commit as one undo step, the
//			selection-change draft policy (valid drafts commit inside the guard
//			before the selection changes, invalid drafts veto), targetname
//			drafts renaming with reference updates, worldspawn properties when
//			no entity is selected, immediate class/flag/output operations
//			(removing exactly one of two identical outputs). Negative checks:
//			invalid drafts refused with messages and no change, reserved and
//			read-only keys, world-refused operations, drafts discarded when
//			their entities change without the guard, destruction order.
//
//=============================================================================//

#include "hammer/presenters/entity_inspector.h"

#include "hammer/app/edit_session.h"
#include "hammer/app/ops/entity_ops.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include "fakes/fake_entity_catalog.h"

#include <functional>
#include <memory>
#include <vector>

using namespace hammer;
using namespace hammer::presenters;
using app::EditErrorCode;
using app::SelectMode;
using mapgeometry::Vec3d;
using scene::ObjectId;
using C = hammertest::FakeEntityCatalog;

namespace
{

ports::KeyDefinition Choices( std::string key, std::vector<ports::KeyChoice> choices,
    std::string def, std::string type = "choices" )
{
	ports::KeyDefinition k = C::Key( std::move( key ), std::move( type ), std::move( def ) );
	k.choices = std::move( choices );
	return k;
}

const KeyRow *FindRow( const EntityInspector &inspector, const std::string &key )
{
	for ( const KeyRow &row : inspector.Rows() )
	{
		if ( row.key == key )
		{
			return &row;
		}
	}
	return nullptr;
}

} // namespace

int main()
{
	testing::Checks checks;

	C catalog;
	ports::KeyDefinition name = C::Key( "targetname", "target_source" );
	name.displayName = "Name";
	ports::KeyDefinition light = C::Key( "_light", "color255", "255 255 255 200" );
	light.displayName = "Brightness";
	light.help = "Color and brightness";
	ports::KeyDefinition style =
	    Choices( "style", { { "0", "Normal", false }, { "10", "Fluorescent", false } }, "0" );
	ports::KeyDefinition flags = Choices( "spawnflags",
	    { { "1", "Initially dark", false }, { "2", "Start on", true } }, "", "flags" );
	ports::KeyDefinition hint = C::Key( "hint", "string", "fixed" );
	hint.readOnly = true;
	catalog.AddPoint( "light", { name, light, style, flags, hint }, { C::Io( "TurnOn" ) } );
	catalog.AddPoint( "light_spot", { name, light, C::Key( "_cone", "integer", "45" ), flags },
	    { C::Io( "TurnOn" ) } );
	catalog.AddSolid( "func_button", { name }, {}, { C::Io( "OnPressed" ) } );
	ports::KeyDefinition sky = C::Key( "skyname", "string", "sky_day01_01" );
	sky.displayName = "Sky name";
	catalog.AddKind( ports::EntityClassKind::Other, "worldspawn", { sky } );

	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::MapDocument doc;
	ObjectId lamp1, lamp2, spot, button, buttonSolid;
	{
		scene::DocumentEdit edit( doc );
		scene::Entity l;
		l.classname = "light";
		l.SetKey( "targetname", "lamp1" );
		l.SetKey( "_light", "255 255 255 200" );
		l.SetKey( "spawnflags", "3" );
		l.SetKey( "customkey", "x" );
		lamp1 = edit.Add( l );
		scene::Entity l2;
		l2.classname = "light";
		l2.SetKey( "targetname", "lamp2" );
		l2.SetKey( "_light", "255 0 0 200" );
		lamp2 = edit.Add( l2 );
		scene::Entity s;
		s.classname = "light_spot";
		s.SetKey( "targetname", "spot" );
		s.SetKey( "_cone", "30" );
		spot = edit.Add( s );
		scene::Entity b;
		b.classname = "func_button";
		b.connections.push_back( *scene::ParseConnection( "OnPressed", "lamp1,TurnOn,,0,-1" ) );
		b.connections.push_back( *scene::ParseConnection( "OnPressed", "missing,TurnOn,,0,-1" ) );
		b.connections.push_back( *scene::ParseConnection( "OnPressed", "!self,Lock,,0,-1" ) );
		b.connections.push_back( *scene::ParseConnection( "OnDamaged", "spot,Explode,,0,-1" ) );
		button = edit.Add( b );
		scene::Solid bs = scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) }, tex );
		bs.owner = button;
		buttonSolid = edit.Add( bs );
		scene::CommitEdit( doc, edit );
	}

	auto session = std::make_unique<app::EditSession>( doc );
	auto inspector = std::make_unique<EntityInspector>( *session, &catalog );
	checks.That( inspector->Entities().empty() && inspector->IsWorld() &&
	                 inspector->Class() == app::PropertyValue::Single( "worldspawn" ) &&
	                 inspector->Flags().empty() && inspector->Outputs().empty(),
	    "nothing selected: the worldspawn" );
	checks.That( !inspector->Rows().empty() && inspector->Rows()[0].key == "skyname" &&
	                 inspector->Rows()[0].displayName == "Sky name" &&
	                 inspector->Rows()[0].value.IsUnset(),
	    "the worldspawn schema" );

	// --- Aggregates over two lights --------------------------------------------------
	const std::uint64_t rev0 = inspector->Revision();
	checks.That( session->SelectObjects( { lamp1, lamp2 }, SelectMode::Replace ).HasValue(),
	    "select lamps" );
	checks.That( inspector->Revision() > rev0, "a selection change moves the revision" );
	checks.That(
	    inspector->Class().IsSingle() && inspector->Class().Value() == "light", "class Single" );
	checks.That( inspector->ClassInfo() && inspector->ClassInfo()->name == "light", "class info" );
	{
		const auto &rows = inspector->Rows();
		checks.Equal( rows.size(), std::size_t( 5 ), "4 schema rows (flags excluded) + 1 extra" );
		checks.That( rows.size() == 5 && rows[0].key == "targetname" && rows[1].key == "_light" &&
		                 rows[2].key == "style" && rows[3].key == "hint" &&
		                 rows[4].key == "customkey",
		    "catalog order, then extras" );
		const KeyRow *l = FindRow( *inspector, "_light" );
		checks.That( l && l->value.IsMixed() && l->displayName == "Brightness" &&
		                 l->help == "Color and brightness" && l->type == ports::KeyType::Color255 &&
		                 l->differsFromDefault,
		    "_light Mixed, display name, help, type" );
		const KeyRow *st = FindRow( *inspector, "style" );
		checks.That(
		    st && st->value.IsUnset() && !st->differsFromDefault && st->choices.size() == 2,
		    "style Unset, not different" );
		const KeyRow *extra = FindRow( *inspector, "customkey" );
		checks.That(
		    extra && !extra->inSchema && extra->value.IsMixed() && extra->presentCount == 1,
		    "a key on one of two entities is Mixed, not Single" );
		checks.That( !FindRow( *inspector, "spawnflags" ), "flags key hidden in SmartEdit" );
	}
	checks.Equal( inspector->FlagsKey(), std::string( "spawnflags" ), "flags key" );
	checks.That(
	    inspector->Flags().size() == 2 && inspector->Flags()[0].state == FlagState::Mixed &&
	        inspector->Flags()[1].state == FlagState::On && inspector->Flags()[1].defaultOn,
	    "flags tri-state: 3 vs default 2 gives bit1 Mixed, bit2 On" );

	checks.That(
	    session->SelectObjects( { lamp1 }, SelectMode::Replace ).HasValue(), "select lamp1" );
	{
		const KeyRow *l = FindRow( *inspector, "_light" );
		checks.That( l && l->value.IsSingle() && !l->differsFromDefault, "Single at default" );
		const KeyRow *n = FindRow( *inspector, "targetname" );
		checks.That(
		    n && n->value == app::PropertyValue::Single( "lamp1" ) && n->differsFromDefault,
		    "Single differing from default" );
	}
	checks.That( inspector->Inputs().size() == 1 && inspector->Inputs()[0].source == button &&
	                 inspector->Inputs()[0].sourceLabel == "func_button" &&
	                 inspector->Inputs()[0].inputKnown,
	    "input from the button" );

	// SmartEdit off: raw keys, flags key included.
	inspector->SetSmartEdit( false );
	{
		const auto &rows = inspector->Rows();
		checks.That( rows.size() == 4 && rows[0].key == "targetname" &&
		                 rows[0].displayName == "targetname" && rows[2].key == "spawnflags" &&
		                 rows[3].key == "customkey",
		    "raw rows in key order" );
	}
	inspector->SetSmartEdit( true );

	// Mixed classes: schema is the intersection.
	checks.That(
	    session->SelectObjects( { lamp1, spot }, SelectMode::Replace ).HasValue(), "select mixed" );
	checks.That( inspector->Class().IsMixed() && !inspector->ClassInfo(), "class Mixed" );
	{
		const auto &rows = inspector->Rows();
		checks.That( rows.size() >= 2 && rows[0].key == "targetname" && rows[1].key == "_light" &&
		                 rows[0].inSchema && rows[1].inSchema,
		    "shared keys first" );
		const KeyRow *cone = FindRow( *inspector, "_cone" );
		const KeyRow *st = FindRow( *inspector, "style" );
		checks.That( cone && !cone->inSchema && !st, "keys one class lacks are extras or absent" );
		checks.Equal( inspector->Flags().size(), std::size_t( 2 ), "shared flags" );
	}

	// Outputs with target validity.
	checks.That( session->SelectObjects( { buttonSolid }, SelectMode::Replace ).HasValue(),
	    "select button solid" );
	checks.That( inspector->Entities().size() == 1 && inspector->Entities()[0] == button,
	    "a brush solid stands for its entity" );
	{
		const auto &outs = inspector->Outputs();
		checks.That( outs.size() == 4 && outs[0].targetValid && !outs[1].targetValid &&
		                 outs[2].targetValid && outs[3].targetValid,
		    "named target valid, missing invalid, procedural valid" );
		checks.That( outs.size() == 4 && outs[0].outputKnown && !outs[3].outputKnown,
		    "an output the class does not declare is flagged" );
	}

	// --- Draft commit as one undo step ----------------------------------------------------
	checks.That( session->SelectObjects( { lamp1, lamp2 }, SelectMode::Replace ).HasValue(),
	    "reselect lamps" );
	std::size_t history = session->History().Size();
	checks.That( inspector->SetDraft( "_light", "0 255 0 100" ).HasValue(), "draft _light" );
	checks.That( inspector->SetDraft( "style", "10" ).HasValue(), "draft style" );
	checks.That( inspector->HasDraft() && session->History().Size() == history,
	    "a draft changes nothing yet" );
	{
		const KeyRow *st = FindRow( *inspector, "style" );
		checks.That( st && st->drafted && st->draftValue == "10" && st->draftError.empty(),
		    "row shows the draft" );
	}
	checks.That( inspector->Commit().HasValue(), "commit" );
	checks.Equal( session->History().Size(), history + 1, "one undo step" );
	checks.Equal(
	    session->History().UndoEntry()->label, std::string( "Edit properties" ), "label" );
	checks.That( !inspector->HasDraft(), "draft cleared" );
	checks.That( *session->Document().FindEntity( lamp2 )->Key( "_light" ) == "0 255 0 100" &&
	                 *session->Document().FindEntity( lamp1 )->Key( "style" ) == "10",
	    "both keys on both entities" );
	{
		const KeyRow *st = FindRow( *inspector, "style" );
		checks.That(
		    st && st->value.IsSingle() && st->valueLabel == "Fluorescent", "choice label shown" );
	}
	checks.That( session->Undo().HasValue() &&
	                 !session->Document().FindEntity( lamp1 )->Key( "style" ) &&
	                 *session->Document().FindEntity( lamp2 )->Key( "_light" ) == "255 0 0 200",
	    "one undo reverts every drafted key" );

	// --- Validation (negative) ---------------------------------------------------------------
	const std::uint64_t docRev = session->Revision();
	history = session->History().Size();
	checks.That(
	    inspector->SetDraft( "_light", "300 0 0" ).HasValue(), "an invalid draft is stored" );
	checks.That(
	    !FindRow( *inspector, "_light" )->draftError.empty(), "the row reports its error" );
	{
		auto r = inspector->Commit();
		checks.That( !r && r.Error().code == EditErrorCode::Rejected &&
		                 r.Error().message.find( "_light" ) != std::string::npos,
		    "commit refused naming the key" );
	}
	checks.That( session->Revision() == docRev && session->History().Size() == history &&
	                 inspector->HasDraft(),
	    "nothing changes on refusal; the draft stays" );
	checks.That( !inspector->LastError().empty(), "the refusal is reported" );
	checks.That(
	    inspector->SetDraft( "style", "7" ).HasValue() && inspector->DraftErrors().size() == 2,
	    "a value outside the choices is invalid" );
	inspector->SetAllowFreeChoices( true );
	checks.That( inspector->DraftErrors().size() == 1, "free text allowed for choices" );
	inspector->SetAllowFreeChoices( false );
	inspector->Cancel();
	checks.That( !inspector->HasDraft() && session->Revision() == docRev, "cancel discards" );

	checks.That( !ValidateKeyValue( &light, "255 255 255", false ), "color255 rgb" );
	checks.That( !ValidateKeyValue( &light, "-1 -1 -1 1", false ), "color255 HDR -1" );
	checks.That( ValidateKeyValue( &light, "255 255", false ).has_value(),
	    "color255 two numbers (negative)" );
	checks.That(
	    ValidateKeyValue( &light, "1.5 0 0", false ).has_value(), "color255 fraction (negative)" );
	ports::KeyDefinition integer = C::Key( "n", "integer" );
	checks.That(
	    !ValidateKeyValue( &integer, "-12", false ) && ValidateKeyValue( &integer, "1.5", false ) &&
	        ValidateKeyValue( &integer, "", false ) && ValidateKeyValue( &integer, "abc", false ),
	    "integer rules" );
	ports::KeyDefinition real = C::Key( "f", "float" );
	checks.That( !ValidateKeyValue( &real, "1e3", false ) &&
	                 ValidateKeyValue( &real, "nan", false ) &&
	                 ValidateKeyValue( &real, "1 2", false ),
	    "float rules" );
	ports::KeyDefinition angles = C::Key( "angles", "angle" );
	checks.That(
	    !ValidateKeyValue( &angles, "0 90 0", false ) && ValidateKeyValue( &angles, "0 90", false ),
	    "angle rules" );
	checks.That( ValidateKeyValue( nullptr, "a\"b", false ) &&
	                 ValidateKeyValue( nullptr, "a\nb", false ) &&
	                 !ValidateKeyValue( nullptr, "", false ),
	    "text rules without a schema" );

	checks.That( inspector->SetDraft( "classname", "x" ).Error().code == EditErrorCode::Rejected &&
	                 inspector->SetDraft( "id", "3" ).Error().code == EditErrorCode::Rejected &&
	                 !inspector->SetDraft( "a\"b", "1" ) && !inspector->SetDraft( "hint", "y" ),
	    "reserved, quoted and read-only keys refused" );
	checks.That( !inspector->HasDraft(), "refused drafts store nothing" );

	// --- Selection-change draft policy ---------------------------------------------------------
	// Invalid draft: the change is vetoed and reported.
	checks.That( inspector->SetDraft( "_light", "bad" ).HasValue(), "invalid draft" );
	{
		auto r = session->SelectObjects( { spot }, SelectMode::Replace );
		checks.That( !r && r.Error().code == EditErrorCode::Vetoed, "an invalid draft vetoes" );
	}
	checks.That( session->CurrentSelection().objects.size() == 2 && inspector->HasDraft(),
	    "selection and draft kept" );
	checks.That(
	    inspector->LastError().find( "_light" ) != std::string::npos, "the veto names the key" );
	checks.That( session->ClearSelection().Error().code == EditErrorCode::Vetoed,
	    "every entry point shares the guard" );

	// Valid draft: committed inside the guard, before the selection changes.
	checks.That( inspector->SetDraft( "_light", "1 2 3 4" ).HasValue(), "valid draft" );
	history = session->History().Position();
	checks.That( session->SelectObjects( { spot }, SelectMode::Replace ).HasValue(),
	    "a valid draft allows" );
	checks.That( session->CurrentSelection().objects == std::vector<ObjectId>{ spot } &&
	                 !inspector->HasDraft(),
	    "selection moved; draft committed" );
	checks.Equal( session->History().Position(), history + 1, "the commit is one step" );
	checks.That( session->History().UndoEntry()->label == "Edit properties" &&
	                 *session->Document().FindEntity( lamp1 )->Key( "_light" ) == "1 2 3 4" &&
	                 *session->Document().FindEntity( lamp2 )->Key( "_light" ) == "1 2 3 4" &&
	                 !session->Document().FindEntity( spot )->Key( "_light" ),
	    "applied to the drafted entities, not the new selection" );
	checks.That( session->History().UndoEntry()->selectionBefore.objects.size() == 2 &&
	                 session->History().UndoEntry()->selectionAfter.objects.size() == 2,
	    "recorded before the selection changed" );
	checks.That( session->Undo().HasValue() &&
	                 session->CurrentSelection().objects == std::vector<ObjectId>{ lamp1, lamp2 },
	    "undo restores the drafted selection" );
	checks.That(
	    session->SelectObjects( { lamp1 }, SelectMode::Replace ).HasValue(), "select lamp1" );

	// Targetname drafts rename with reference updates.
	checks.That( inspector->SetDraft( "targetname", "hall_lamp" ).HasValue() &&
	                 inspector->Commit().HasValue(),
	    "rename through the draft" );
	checks.That( session->Document().FindEntity( lamp1 )->Name() == "hall_lamp" &&
	                 session->Document().FindEntity( button )->connections[0].target == "hall_lamp",
	    "the button's output follows the rename" );
	checks.That( session->Undo().HasValue() &&
	                 session->Document().FindEntity( button )->connections[0].target == "lamp1",
	    "one undo step reverts both" );

	// --- Immediate operations --------------------------------------------------------------------
	history = session->History().Position();
	checks.That( inspector->SetFlag( 1, false ).HasValue(), "clear flag" );
	checks.That( session->History().UndoEntry()->label == "Clear flag Initially dark" &&
	                 inspector->Flags()[0].state == FlagState::Off,
	    "flag toggle is one labeled step" );
	checks.That( inspector->SetFlag( 1, false ).Error().code == EditErrorCode::Nothing,
	    "unchanged flag refused (negative)" );
	checks.That( inspector->SetClass( "light_spot" ).HasValue() &&
	                 inspector->Class().Value() == "light_spot",
	    "class change" );
	checks.That( session->History().UndoEntry()->label == "Change class to light_spot" &&
	                 FindRow( *inspector, "_cone" ) && FindRow( *inspector, "_cone" )->inSchema,
	    "class change relabels the schema" );
	checks.That(
	    !inspector->SetClass( "func_button" ) && inspector->Class().Value() == "light_spot",
	    "a point entity cannot take a solid class (negative)" );
	checks.That(
	    inspector->RemoveKey( "customkey" ).HasValue() && !FindRow( *inspector, "customkey" ),
	    "remove key" );
	checks.Equal( session->History().Position(), history + 3, "three immediate steps" );

	// Outputs: add, replace, remove.
	scene::Connection c = *scene::ParseConnection( "OnUser1", "lamp2,TurnOn,,0,-1" );
	checks.That(
	    inspector->AddOutput( c ).HasValue() && inspector->Outputs().size() == 1, "add output" );
	c.delay = 2;
	checks.That( inspector->ReplaceOutput( lamp1, 0, c ).HasValue() &&
	                 inspector->Outputs()[0].connection.delay == 2,
	    "replace output" );
	checks.That( !inspector->ReplaceOutput( button, 0, c ), "not an inspected entity (negative)" );
	checks.That( inspector->AddOutput( c ).HasValue() && inspector->Outputs().size() == 2,
	    "an identical second output" );
	checks.That( inspector->RemoveOutput( lamp1, 0 ).HasValue() && inspector->Outputs().size() == 1,
	    "remove output removes exactly one" );
	checks.That( inspector->RemoveOutput( lamp1, 0 ).HasValue() && inspector->Outputs().empty(),
	    "remove the other" );
	checks.That( !inspector->RemoveOutput( lamp1, 5 ), "no such output (negative)" );

	// --- Drafts and entities changing without the guard ------------------------------------------
	checks.That(
	    inspector->SetDraft( "_light", "9 9 9" ).HasValue(), "draft before an edit that selects" );
	auto sel = session->Execute(
	    "Touch",
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::SetKey( edit, { spot }, "_cone", "33" );
	    },
	    app::Selection{ { spot }, {}, spot } );
	checks.That( sel.HasValue() && !inspector->HasDraft() &&
	                 inspector->LastError().find( "discarded" ) != std::string::npos,
	    "a draft whose entities changed without the guard is discarded and reported" );
	checks.That(
	    *session->Document().FindEntity( lamp1 )->Key( "_light" ) != "9 9 9", "and never applied" );

	// Replacement: the guard commits a valid draft to the old document first.
	checks.That( inspector->SetDraft( "_cone", "11" ).HasValue(), "draft before replace" );
	checks.That( session->Replace( doc ).HasValue(), "replace passes a valid draft" );
	checks.That( inspector->IsWorld() && !inspector->HasDraft() && session->History().Size() == 0,
	    "the model follows the new document" );

	// --- World properties ------------------------------------------------------------------------
	checks.That( session->ClearSelection().HasValue() && inspector->IsWorld(), "world again" );
	checks.That(
	    inspector->SetDraft( "skyname", "sky_night" ).HasValue() && inspector->Commit().HasValue(),
	    "edit a world key" );
	checks.That(
	    session->History().UndoEntry()->label == "Edit world properties" &&
	        *session->Document().Settings().WorldKey( "skyname" ) == "sky_night" &&
	        FindRow( *inspector, "skyname" )->value == app::PropertyValue::Single( "sky_night" ),
	    "world key committed and shown" );
	checks.That( !inspector->SetClass( "light" ) && !inspector->SetFlag( 1, true ) &&
	                 !inspector->RemoveKey( "skyname" ) && !inspector->AddOutput( c ),
	    "class, flag, key and output operations refused for the world (negative)" );
	checks.That( inspector->SetDraft( "detailvbsp", "detail.vbsp" ).HasValue() &&
	                 session->SelectObjects( { lamp1 }, SelectMode::Replace ).HasValue(),
	    "a world draft passes the guard" );
	checks.That( *session->Document().Settings().WorldKey( "detailvbsp" ) == "detail.vbsp" &&
	                 !session->Document().FindEntity( lamp1 )->Key( "detailvbsp" ),
	    "and lands on the world, not the new selection" );

	// --- Lifetime -------------------------------------------------------------------------------
	session.reset();
	inspector.reset();
	checks.That( true, "an inspector may be destroyed after its session" );

	return checks.Report();
}
