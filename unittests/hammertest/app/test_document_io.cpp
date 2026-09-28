//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app document I/O (RFC 0002 "Persistence contracts"): open
//			decodes into a detached document and replaces the session only on
//			success; save encodes, writes atomically and only then marks the
//			session saved; the map version bump is bookkeeping (no history, not
//			modified, never rolled back by undo). Negative checks: read, decode,
//			encode and write failures each leave the session and the previous
//			file untouched; a selection guard veto keeps the open document.
//
//=============================================================================//

#include "hammer/app/document_io.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include "app/fake_file_store.h"
#include "fakes/fake_map_codec.h"

using namespace hammer;
using namespace hammer::app;
using mapgeometry::Vec3d;

int main()
{
	testing::Checks checks;

	hammertest::FakeMapCodec codec;
	hammertest::InMemoryFileStore store;
	EditSession session;
	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::ObjectId box;
	(void)session.Execute( "Create",
	    [&]( scene::DocumentEdit &e )
	    {
		    box = e.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) }, tex ) );
		    return EditResult{};
	    } );
	checks.That( session.IsModified(), "edited" );

	// Save: atomic write, saved state, version bump as bookkeeping.
	checks.That( SaveDocumentAs( session, codec, store, "maps/a.vmf" ).HasValue(), "save" );
	checks.That( !session.IsModified() && store.files.count( "maps/a.vmf" ), "saved and written" );
	checks.Equal( session.Document().Settings().version.mapVersion, 2,
	    "versioninfo follows the bumped world mapversion" );
	checks.That( session.Document().Settings().WorldKey( "mapversion" ) &&
	                 *session.Document().Settings().WorldKey( "mapversion" ) == "2",
	    "world mapversion bumped from its value" );
	checks.Equal( session.History().Size(), std::size_t( 1 ), "the bump records no history" );
	(void)session.Undo();
	checks.Equal( session.Document().Settings().version.mapVersion, 2,
	    "undo does not roll the version back" );
	(void)session.Redo();
	checks.That( !session.IsModified(), "redo back to the saved state is clean" );
	checks.That( SaveDocumentAs( session, codec, store, "maps/b.vmf", false ).HasValue() &&
	                 session.Document().Settings().version.mapVersion == 2,
	    "save without a bump" );

	// Encode and write failures keep the session modified and the old file.
	(void)session.Execute( "Move",
	    [&]( scene::DocumentEdit &e )
	    {
		    e.MutableSolid( box )->sides[0].texture.material = "TOOLS/TOOLSNODRAW";
		    return EditResult{};
	    } );
	const std::string before = store.files["maps/a.vmf"];
	codec.failEncode = true;
	auto encodeFailed = SaveDocumentAs( session, codec, store, "maps/a.vmf" );
	checks.That( !encodeFailed && encodeFailed.Error().status == DocumentIoStatus::EncodeFailed,
	    "encode failure (negative)" );
	codec.failEncode = false;
	store.failAllWrites = true;
	auto writeFailed = SaveDocumentAs( session, codec, store, "maps/a.vmf" );
	checks.That( !writeFailed && writeFailed.Error().status == DocumentIoStatus::WriteFailed,
	    "write failure (negative)" );
	store.failAllWrites = false;
	checks.That( session.IsModified() && store.files["maps/a.vmf"] == before,
	    "failures keep the session modified and the file" );
	checks.Equal( session.Document().Settings().version.mapVersion, 2,
	    "failed saves do not bump the version" );

	// Open.
	const std::uint32_t oldSerial = session.Document().Serial();
	auto opened = OpenDocument( session, codec, store, "maps/a.vmf" );
	checks.That( opened.HasValue() && opened.Value().empty(), "open without warnings" );
	checks.That(
	    !session.IsModified() && session.History().Size() == 0, "opened documents are clean" );
	checks.That( session.Document().Serial() != oldSerial, "a new serial retires the old ids" );
	checks.That( !session.Document().FindSolid( box ), "old ids do not resolve" );
	checks.Equal( session.Document().Solids().size(), std::size_t( 1 ), "the saved solid is back" );
	checks.That(
	    session.Document().Solids().begin()->second.sides[0].texture.material == tex.material,
	    "the saved content, not the unsaved edit" );

	// Negative opens.
	const scene::MapDocument current = session.Document();
	auto missing = OpenDocument( session, codec, store, "maps/none.vmf" );
	checks.That( !missing && missing.Error().status == DocumentIoStatus::ReadFailed,
	    "missing file (negative)" );
	store.files["maps/bad.vmf"] = "garbage";
	auto bad = OpenDocument( session, codec, store, "maps/bad.vmf" );
	checks.That(
	    !bad && bad.Error().status == DocumentIoStatus::DecodeFailed && bad.Error().line == 1,
	    "decode failure with its line (negative)" );
	{
		SessionSubscription veto = session.AddSelectionGuard(
		    []
		    {
			    return false;
		    } );
		auto vetoed = OpenDocument( session, codec, store, "maps/a.vmf" );
		checks.That( !vetoed && vetoed.Error().status == DocumentIoStatus::Vetoed,
		    "a guard keeps the document (negative)" );
	}
	checks.That(
	    scene::SameContent( session.Document(), current ), "failed opens leave the document" );

	return checks.Report();
}
