//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance suite for the material-info port and its adapter over
//			the material catalog (ports.material_info.v1). An in-memory asset
//			source holds VMTs and VTFs built by the independent fake VTF writer.
//			Checks: existence and names follow the catalog listing with the
//			port's name rules (any case, either slash); the size is the base
//			texture's VTF header size, through a patch include; missing
//			materials, missing base textures, undecodable VTFs and KTX2-only
//			textures report no size (never a default); the fake of the port
//			agrees on the same clauses.
//
//=============================================================================//

#include "fakes/fake_material_info.h"
#include "formats/fake_asset_source.h"
#include "formats/fake_vtf.h"

#include "hammer/formats/material_catalog.h"
#include "hammer/formats/material_info_adapter.h"
#include "testing/checks.h"

#include <algorithm>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using hammer::formats::MaterialCatalog;
using hammer::formats::MaterialInfoAdapter;
using hammer::ports::IMaterialInfo;
using hammer::ports::MaterialSize;

namespace
{

bool SizeIs( const IMaterialInfo &info, std::string_view name, int width, int height )
{
	const std::optional<MaterialSize> size = info.Size( name );
	return size && size->width == width && size->height == height;
}

// The port clauses both providers share, on the same content: "dev/grid"
// 256x128 and "Tools/Nodraw" existing without a size.
void PortClauses( testing::Checks &checks, const IMaterialInfo &info, const char *who )
{
	const std::string w = std::string( who ) + ": ";
	checks.That(
	    info.Exists( "dev/grid" ) && info.Exists( "DEV\\GRID" ) && info.Exists( "Dev/Grid" ),
	    w + "Exists is case-insensitive, '/' == '\\'" );
	checks.That( SizeIs( info, "dev/grid", 256, 128 ) && SizeIs( info, "DEV\\Grid", 256, 128 ),
	    w + "Size is the mapping size under any spelling" );
	checks.That( info.Exists( "tools/nodraw" ) && !info.Size( "tools/nodraw" ),
	    w + "an existing material may have no size" );
	checks.That( !info.Exists( "dev/missing" ) && !info.Size( "dev/missing" ),
	    w + "a missing material does not exist and has no size" );
	checks.That( !info.Exists( "" ) && !info.Size( "" ), w + "the empty name is missing" );
	const std::vector<std::string> names = info.Names();
	checks.That( std::is_sorted( names.begin(), names.end() ), w + "Names sorted" );
	checks.That( std::find( names.begin(), names.end(), "dev/grid" ) != names.end() &&
	                 std::find( names.begin(), names.end(), "tools/nodraw" ) != names.end(),
	    w + "Names lists every material, normalized" );
	bool allExist = true;
	for ( const std::string &name : names )
	{
		allExist = allExist && info.Exists( name );
	}
	checks.That( allExist, w + "every listed name exists" );
}

std::string Vtf( int width, int height )
{
	return hammertest::BuildVtf( width, height, hammertest::VTF_FMT_BGRA8888, 1, {}, false );
}

} // namespace

int main()
{
	testing::Checks checks;

	hammertest::InMemoryAssetSource src;
	src.assets["materials/dev/grid.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"dev/grid_tex\" }";
	src.assets["materials/dev/grid_tex.vtf"] = Vtf( 256, 128 );
	src.assets["materials/tools/nodraw.vmt"] = "\"LightmappedGeneric\" { \"%compilenodraw\" 1 }";
	// A patch material takes its base texture (and so its size) from its include.
	src.assets["materials/custom/wall.vmt"] =
	    "\"patch\" { \"include\" \"materials/dev/grid.vmt\" }";
	// Base texture named but absent; corrupt VTF; KTX2-only base texture.
	src.assets["materials/dev/nobase.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"dev/gone\" }";
	src.assets["materials/dev/corrupt.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"dev/corrupt\" }";
	src.assets["materials/dev/corrupt.vtf"] = "not a vtf";
	src.assets["materials/dev/packed.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"dev/packed\" }";
	src.assets["materials/dev/packed.ktx2"] = "KTX2 bytes";
	// A texture without a material is not a material.
	src.assets["materials/dev/orphan.vtf"] = Vtf( 64, 64 );

	MaterialCatalog catalog( src );
	const MaterialInfoAdapter adapter( catalog, src );
	PortClauses( checks, adapter, "adapter" );

	checks.That( SizeIs( adapter, "custom/wall", 256, 128 ),
	    "adapter: patch material sized from its include's base texture" );
	checks.That( SizeIs( adapter, "materials/Dev/Grid.vmt", 256, 128 ),
	    "adapter: 'materials/' prefix and '.vmt' suffix accepted" );
	checks.That( adapter.Exists( "dev/nobase" ) && !adapter.Size( "dev/nobase" ),
	    "adapter: absent base texture -> no size" );
	checks.That( adapter.Exists( "dev/corrupt" ) && !adapter.Size( "dev/corrupt" ),
	    "adapter: unreadable VTF header -> no size" );
	checks.That( adapter.Exists( "dev/packed" ) && !adapter.Size( "dev/packed" ),
	    "adapter: KTX2-only base texture -> no size (declared limit)" );
	checks.That( !adapter.Exists( "dev/orphan" ) && !adapter.Exists( "dev/grid_tex" ),
	    "adapter: textures without a VMT are not materials" );
	checks.Equal( adapter.Names(),
	    std::vector<std::string>{
	        "custom/wall", "dev/corrupt", "dev/grid", "dev/nobase", "dev/packed", "tools/nodraw" },
	    "adapter: Names is the catalog listing" );

	// Cached: a size stays what it was when first asked (the catalog is a
	// snapshot too), and a miss is not turned into a default later.
	src.assets.erase( "materials/dev/grid_tex.vtf" );
	checks.That( SizeIs( adapter, "dev/grid", 256, 128 ), "adapter: size cached per material" );
	checks.That( !adapter.Size( "dev/missing" ), "adapter: miss stays a miss" );

	// The fake claims the same port and passes the same clauses.
	hammertest::FakeMaterialInfo fake;
	fake.Add( "Dev/Grid", 256, 128 ).AddUnsized( "tools\\nodraw" );
	PortClauses( checks, fake, "fake" );

	// Sensitivity: a provider that answers a default size for everything must
	// fail the shared clauses.
	struct DefaultSize final : IMaterialInfo
	{
		bool Exists( std::string_view ) const override { return true; }
		std::optional<MaterialSize> Size( std::string_view ) const override
		{
			return MaterialSize{ 256, 128 };
		}
		std::vector<std::string> Names() const override { return { "dev/grid", "tools/nodraw" }; }
	};
	std::FILE *sink = std::tmpfile();
	testing::Checks inner( sink != nullptr ? sink : stdout );
	PortClauses( inner, DefaultSize(), "default-size" );
	if ( sink != nullptr )
	{
		std::fclose( sink );
	}
	checks.That( inner.Failures() > 0, "a default-size provider is flagged" );

	return checks.Report();
}
