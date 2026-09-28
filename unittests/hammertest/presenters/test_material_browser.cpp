//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters material browser (RFC 0002, R08 domain logic)
//			over the fake material port and a real EditSession: keyword filter
//			(every word, case-insensitive), "used in map" with face counts
//			(case and slash insensitive) including missing materials, counts
//			following edits and undo, the bounded recent list and the active
//			material in app::EditorSettings. Negative checks: unknown materials
//			refused without changing state, a filter with no match, destruction
//			after the session.
//
//=============================================================================//

#include "hammer/presenters/material_browser.h"

#include "hammer/app/edit_session.h"
#include "hammer/app/ops/texture_ops.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include "fakes/fake_material_info.h"

#include <memory>

using namespace hammer;
using namespace hammer::presenters;
using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

std::vector<std::string> Names( const MaterialBrowser &browser )
{
	std::vector<std::string> names;
	for ( const MaterialRow &row : browser.Rows() )
	{
		names.push_back( row.name );
	}
	return names;
}

} // namespace

int main()
{
	testing::Checks checks;

	hammertest::FakeMaterialInfo materials;
	materials.Add( "brick/brickwall01", 256, 256 )
	    .Add( "brick/brickfloor02", 256, 256 )
	    .Add( "concrete/concretewall01", 512, 512 )
	    .Add( "dev/dev_measuregeneric01b", 128, 128 )
	    .AddUnsized( "tools/toolsnodraw" );

	scene::FaceTexture tex;
	tex.material = "DEV\\DEV_MEASUREGENERIC01B";
	scene::MapDocument doc;
	ObjectId box;
	{
		scene::DocumentEdit edit( doc );
		scene::Solid s = scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) }, tex );
		s.sides[0].texture.material = "missing/gone";
		s.sides[1].texture.material = "Tools/ToolsNodraw";
		box = edit.Add( s );
		scene::CommitEdit( doc, edit );
	}
	const std::uint32_t side2 = doc.FindSolid( box )->sides[2].vmfId;

	auto session = std::make_unique<app::EditSession>( doc );
	app::EditorSettings settings;
	auto browser = std::make_unique<MaterialBrowser>( *session, materials, settings, 2 );

	checks.Equal( browser->Rows().size(), std::size_t( 5 ), "every known material" );
	checks.Equal( browser->FaceCount( "dev/dev_measuregeneric01b" ), std::size_t( 4 ),
	    "counts ignore case and slash direction" );

	// Keywords.
	const std::uint64_t rev = browser->Revision();
	browser->SetFilter( "BRICK wall" );
	checks.That( browser->Revision() > rev, "revision moves" );
	checks.That( Names( *browser ) == std::vector<std::string>{ "brick/brickwall01" },
	    "every keyword must match" );
	browser->SetFilter( "  wall  " );
	checks.That( Names( *browser ) ==
	                 std::vector<std::string>{ "brick/brickwall01", "concrete/concretewall01" },
	    "one keyword, whitespace ignored" );
	browser->SetFilter( "brick zzz" );
	checks.That( browser->Rows().empty(), "no match (negative)" );
	browser->SetFilter( "" );

	// Used in map.
	browser->SetUsedOnly( true );
	{
		const auto &rows = browser->Rows();
		checks.That( rows.size() == 3 && rows[0].name == "dev/dev_measuregeneric01b" &&
		                 rows[0].faceCount == 4 && rows[0].known,
		    "used material with its count, port spelling" );
		checks.That( rows.size() == 3 && rows[1].name == "missing/gone" && !rows[1].known &&
		                 rows[1].faceCount == 1,
		    "a missing material is listed as unknown" );
		checks.That( rows.size() == 3 && rows[2].name == "tools/toolsnodraw" && rows[2].known,
		    "sorted by name" );
	}
	auto applied = session->Execute( "Apply",
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::ApplyMaterial(
		        edit, { scene::FaceRef{ box, side2 } }, "brick/brickwall01" );
	    } );
	checks.That( applied.HasValue() && browser->Rows().size() == 4 &&
	                 browser->FaceCount( "brick/brickwall01" ) == 1 &&
	                 browser->FaceCount( "dev/dev_measuregeneric01b" ) == 3,
	    "counts follow edits" );
	checks.That( session->Undo().HasValue() && browser->Rows().size() == 3, "and undo" );
	browser->SetFilter( "gone" );
	checks.That( Names( *browser ) == std::vector<std::string>{ "missing/gone" },
	    "filter applies to used rows" );
	browser->SetFilter( "" );
	browser->SetUsedOnly( false );

	// Active and recent.
	checks.Equal(
	    browser->Active(), std::string( "DEV/DEV_MEASUREGENERIC01B" ), "reads the settings owner" );
	checks.That( browser->SetActive( "BRICK\\BRICKWALL01" ).HasValue() &&
	                 settings.faceTexture.material == "brick/brickwall01",
	    "writes the port's spelling to the settings owner" );
	checks.That( browser->SetActive( "tools/toolsnodraw" ).HasValue() &&
	                 browser->SetActive( "brick/brickwall01" ).HasValue() &&
	                 browser->SetActive( "concrete/concretewall01" ).HasValue(),
	    "more uses" );
	checks.That( browser->Recent() ==
	                 std::vector<std::string>{ "concrete/concretewall01", "brick/brickwall01" },
	    "most recent first, no duplicates, bounded to 2" );
	const std::vector<std::string> recent = browser->Recent();
	checks.That( !browser->SetActive( "missing/gone" ) &&
	                 browser->Active() == "concrete/concretewall01" && browser->Recent() == recent,
	    "unknown material refused, nothing changes (negative)" );

	// Replacement recounts.
	checks.That( session->Replace( scene::MapDocument( 1 ) ).HasValue() &&
	                 browser->FaceCount( "dev/dev_measuregeneric01b" ) == 0,
	    "replacement recounts" );

	session.reset();
	browser.reset();
	checks.That( true, "destroyed after the session" );
	return checks.Report();
}
