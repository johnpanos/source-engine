//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless integration test for the composition the thin GTK sibling
//			shell drives (RFC 0002). The GTK shell holds no editor logic: every
//			button and list selection delegates to hammer::app::EditorDocument and
//			the shared entity editor (hammer::app::EntitySelection +
//			AggregateProperty / SetPropertyOnSelection), persisted through a
//			hammer::ports::IFileStore. This test builds that SAME composition with
//			the real DiskFileStore and exercises the New -> select entities ->
//			Set Key (atomic multi-entity edit) -> Save -> reopen -> Undo/Redo
//			workflow the shell exposes, with no GTK dependency. It proves the
//			sibling's behavior is correct independently of the toolkit; the shell
//			binary itself is separately proven to compile+link against these
//			libraries (build_gtk_shell.sh).
//
//=============================================================================//

#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/app/editor_document.h"
#include "hammer/app/entity_selection.h"
#include "hammer/app/property_value.h"
#include "hammer/formats/keyvalues.h"

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <random>
#include <string>

using hammer::app::EditorDocument;
using hammer::app::EntitySelection;
using hammer::app::PropertyValue;
using hammer::app::SaveStatus;

namespace
{
int g_failures = 0;
void Check( bool ok, const char *label )
{
	if ( !ok )
	{
		std::printf( "FAIL: %s\n", label );
		++g_failures;
	}
}

// The value of a key on entity 'index', or nullptr, for assertions.
const std::string *EntityValue(
    const EditorDocument &doc, std::size_t index, const std::string &key )
{
	if ( index >= doc.EntityCount() )
	{
		return nullptr;
	}
	return doc.Content().children[index].Find( key );
}

// The two-entity seed the shell starts on (world + one entity).
const char *const kSeed = "world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
                          "entity\n{\n\t\"classname\" \"info_player_start\"\n}\n";
} // namespace

int main()
{
	namespace fs = std::filesystem;
	std::error_code ec;
	fs::path dir = fs::temp_directory_path( ec ) /
	               ( "hammer_shell_wf_" + std::to_string( std::random_device{}() ) );
	fs::create_directories( dir, ec );
	if ( ec )
	{
		std::printf( "FAIL: temp dir\n" );
		return 1;
	}
	const std::string path = ( dir / "map.vmf" ).string();

	hammer::adapters::platform::DiskFileStore store;

	// "New" (OnNew): seed a fresh document and build the entity list.
	EditorDocument doc;
	std::string error;
	Check( doc.LoadFromText( kSeed, error ), "New: seed document loads" );
	Check( !doc.IsModified(), "New: fresh document is unmodified" );
	Check( doc.EntityCount() == 2, "New: two entities enumerated for the list" );

	// Select BOTH entities in the list, then "Set Key" (OnSetKey ->
	// SetPropertyOnSelection): one atomic edit across the whole selection.
	EntitySelection selection;
	selection.Add( 0 );
	selection.Add( 1 );
	Check( doc.SetPropertyOnSelection( selection, "_note", "shared" ),
	    "Set Key commits across selection" );
	Check( doc.IsModified(), "Set Key marks modified" );
	Check( doc.CanUndo(), "Set Key enables undo" );

	// The aggregate display the shell shows: both agree -> Single("shared").
	const PropertyValue aggregate = doc.AggregateProperty( selection, "_note" );
	Check(
	    aggregate.IsSingle() && aggregate.Value() == "shared", "aggregate shows the shared value" );

	// "Save" (OnSave) through the real disk store.
	Check( doc.Save( store, path ) == SaveStatus::kOk, "Save returns kOk" );
	Check( store.Exists( path ), "Save persisted a file on disk" );
	Check( !doc.IsModified(), "Save clears modified" );

	// "Open" (OnOpen) into a FRESH document reads the saved content back; the
	// atomic multi-entity edit persisted on BOTH entities.
	EditorDocument reopened;
	Check( reopened.Load( store, path, error ), "Open loads the saved file" );
	Check( reopened.AggregateProperty( selection, "_note" ).Value() == "shared",
	    "reopened content matches what was saved (both entities)" );

	// "Undo" (OnUndo): one unit reverts BOTH entities (aggregate back to Unset).
	Check( doc.Undo(), "Undo succeeds" );
	Check( EntityValue( doc, 0, "_note" ) == nullptr && EntityValue( doc, 1, "_note" ) == nullptr,
	    "Undo removed the key from both entities in one step" );
	Check( !doc.CanUndo(), "the multi-entity edit was a single undo unit" );

	// "Redo" (OnRedo): restores the edit on both.
	Check( doc.Redo(), "Redo succeeds" );
	Check( doc.AggregateProperty( selection, "_note" ).Value() == "shared", "Redo restores both" );

	fs::remove_all( dir, ec );

	if ( g_failures != 0 )
	{
		std::printf( "adapters.shell_workflow: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "adapters.shell_workflow: GTK-sibling composition (EditorDocument + entity editor "
	             "+ DiskFileStore) round-trips New/Select/SetKey/Save/Open/Undo/Redo\n" );
	return 0;
}
