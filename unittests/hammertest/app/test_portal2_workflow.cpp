//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Cross-layer workflow on real Portal 2 maps (RFC 0002, R08 domain
//			logic). Each vendored fixture (unittests/hammertest/fixtures/portal2)
//			is decoded by the VMF codec into an EditSession, then edited through
//			the session and the operation families exactly as the UI would:
//			select everything, move it, undo, run the map check, copy and paste
//			a brush entity, sculpt nothing, save through the codec and reopen.
//			Checks: the document validates after every step, undo restores the
//			decoded content exactly, a save/reopen of the edited map is the same
//			content, and each step finishes within a budget sized for
//			interactive use on maps of this size (a quadratic query path would
//			blow it).
//
//=============================================================================//

#include "hammer/app/clipboard.h"
#include "hammer/app/document_io.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/map_check.h"
#include "hammer/app/ops/structure_ops.h"
#include "hammer/app/ops/transform_ops.h"
#include "hammer/formats/vmf_map_codec.h"
#include "hammer/scene/map_queries.h"
#include "testing/checks.h"

#include "app/fake_file_store.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

using namespace hammer;
using mapgeometry::Vec3d;

namespace
{

std::string ReadFile( const std::filesystem::path &path )
{
	std::ifstream input( path, std::ios::binary );
	return std::string( std::istreambuf_iterator<char>( input ), std::istreambuf_iterator<char>() );
}

// Wall-clock seconds of 'work'.
template <typename F> double Seconds( F &&work )
{
	const auto start = std::chrono::steady_clock::now();
	work();
	return std::chrono::duration<double>( std::chrono::steady_clock::now() - start ).count();
}

// Interactive budget per whole-map step. Generous for a loaded CI host; a
// quadratic pass over thousands of objects takes far longer.
constexpr double kStepBudgetSeconds = 2.0;

} // namespace

int main()
{
	testing::Checks checks;
	const std::filesystem::path dir = "unittests/hammertest/fixtures/portal2";
	checks.That( std::filesystem::is_directory( dir ), "the Portal 2 fixtures are present" );

	for ( const char *name : { "sp_a2_trust_fling.vmf", "zoo_mechanics.vmf" } )
	{
		const std::string label = name;
		const std::string text = ReadFile( dir / name );
		checks.That( !text.empty(), label + ": read" );

		formats::VmfMapCodec codec;
		hammertest::InMemoryFileStore store;
		store.files["maps/in.vmf"] = text;
		app::EditSession session;

		double openTime = 0.0;
		bool opened = false;
		openTime = Seconds( [&] { opened = app::OpenDocument( session, codec, store, "maps/in.vmf" ).HasValue(); } );
		checks.That( opened, label + ": opens through the codec" );
		if ( !opened )
		{
			continue;
		}
		checks.That( openTime < kStepBudgetSeconds, label + ": opens within budget" );
		checks.That( session.Document().Validate().empty(), label + ": validates" );
		const scene::MapDocument decoded = session.Document();
		checks.That( decoded.Solids().size() > 100 && decoded.Entities().size() > 50, label + ": a real map" );

		// Select all and move it: one undo step, then undo restores it exactly.
		double selectTime = Seconds( [&]
		    { (void)session.SetSelection( app::SelectAll( session.Document(), app::SelectionGranularity::Groups ) ); } );
		checks.That( !session.CurrentSelection().Empty() && selectTime < kStepBudgetSeconds,
		    label + ": select all within budget" );
		bool moved = false;
		const double moveTime = Seconds( [&]
		    {
			    const std::vector<scene::ObjectId> ids = session.CurrentSelection().objects;
			    moved = session
			                .Execute( "Move",
			                    [&]( scene::DocumentEdit &e ) { return app::ops::Translate( e, ids, Vec3d( 64, 0, 0 ) ); } )
			                .HasValue();
		    } );
		checks.That( moved, label + ": move everything" );
		checks.That( moveTime < kStepBudgetSeconds, label + ": move within budget" );
		checks.That( session.Document().Validate().empty(), label + ": validates after the move" );
		checks.That( !scene::SameContent( session.Document(), decoded ), label + ": the move changed content" );
		const double undoTime = Seconds( [&] { (void)session.Undo(); } );
		checks.That( scene::SameContent( session.Document(), decoded ), label + ": undo restores the decoded map exactly" );
		checks.That( undoTime < kStepBudgetSeconds, label + ": undo within budget" );

		// Map check over the whole map.
		std::vector<app::MapProblem> problems;
		const double checkTime =
		    Seconds( [&] { problems = app::CheckMap( session.Document(), nullptr, nullptr ); } );
		checks.That( checkTime < kStepBudgetSeconds, label + ": map check within budget" );

		// Copy a brush entity and paste it offset: new ids, valid document.
		scene::ObjectId brushEntity;
		for ( const auto &[id, e] : session.Document().Entities() )
		{
			if ( !scene::EntitySolids( session.Document(), id ).empty() )
			{
				brushEntity = id;
				break;
			}
		}
		checks.That( brushEntity.IsValid(), label + ": has a brush entity" );
		const app::MapFragment fragment = app::Copy( session.Document(), { brushEntity } );
		app::PasteOptions paste;
		paste.offset = Vec3d( 0, 0, 512 );
		auto pasted = session.Execute( "Paste", [&]( scene::DocumentEdit &e ) { return app::Paste( e, fragment, paste ); } );
		checks.That( pasted && !pasted.Value().created.empty(), label + ": paste a copied brush entity" );
		checks.That( session.Document().Validate().empty(), label + ": validates after the paste" );

		// Delete a group's worth of objects and check the cleanup rules held.
		if ( !session.Document().Groups().empty() )
		{
			const scene::ObjectId group = session.Document().Groups().begin()->first;
			auto deleted =
			    session.Execute( "Delete", [&]( scene::DocumentEdit &e ) { return app::ops::DeleteObjects( e, { group } ); } );
			checks.That( deleted.HasValue() && !session.Document().FindGroup( group ), label + ": delete a group" );
			checks.That( session.Document().Validate().empty(), label + ": validates after the delete" );
		}

		// Save the edited map and reopen it: same content.
		const scene::MapDocument edited = session.Document();
		bool saved = false;
		const double saveTime = Seconds(
		    [&] { saved = app::SaveDocumentAs( session, codec, store, "maps/out.vmf", false ).HasValue(); } );
		checks.That( saved && !session.IsModified(), label + ": saves" );
		checks.That( saveTime < kStepBudgetSeconds, label + ": save within budget" );
		app::EditSession reopened;
		checks.That( app::OpenDocument( reopened, codec, store, "maps/out.vmf" ).HasValue(), label + ": reopens" );
		// Runtime ids differ between documents; compare the re-encoded text.
		const auto a = codec.Encode( edited );
		const auto b = codec.Encode( reopened.Document() );
		checks.That( a && b && a.Value() == b.Value(), label + ": the reopened map encodes identically" );
		std::printf( "%s: open %.3fs select %.3fs move %.3fs undo %.3fs check %.3fs save %.3fs (%zu problems)\n",
		    name, openTime, selectTime, moveTime, undoTime, checkTime, saveTime, problems.size() );
	}

	return checks.Report();
}
