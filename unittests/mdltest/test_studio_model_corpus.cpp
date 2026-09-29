//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Corpus conformance for content.studio-model.v1
//			(unittests/mdltest/contracts/content.studio-model.v1.md): every
//			model of the game VPKs STUDIO_MODEL_CORPUS_VPKS names (a comma-
//			separated list of _dir.vpk paths; Portal's portal_pak_dir.vpk and
//			Portal 2's pak01_dir.vpk) that has its .vvd parses, with relations
//			an independent look at the data must satisfy: triangles face the
//			way their vertex normals do (the winding swap), every model with
//			triangles has a non-empty box, and most materials resolve in the
//			same VPK. Each game's metal_box.mdl is checked in detail. The VPK
//			reader is the editor's (hammer.formats); the content is not in
//			the repository, so the row is optional.
//
//=============================================================================//

#include "hammer/adapters/platform/disk_byte_store.h"
#include "hammer/formats/vpk_archive.h"
#include "mdl/studio_model.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace
{
int g_checks = 0;
int g_failures = 0;

void Check( bool condition, const std::string &what )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
	}
	std::printf( "%s: %s\n", condition ? "ok" : "FAIL", what.c_str() );
}

class VpkModelFiles : public mdl::IModelFiles
{
public:
	explicit VpkModelFiles( const hammer::formats::VpkArchive &vpk ) : m_vpk( vpk ) {}
	bool Exists( const std::string &path ) const override { return m_vpk.HasAsset( path ); }
	bool Read( const std::string &path, std::string &out ) const override
	{
		return m_vpk.ReadAsset( path, out );
	}

private:
	const hammer::formats::VpkArchive &m_vpk;
};

struct Tally
{
	int models = 0;
	int parsed = 0;
	int withTriangles = 0;
	int emptyBoxes = 0;
	std::size_t triangles = 0;
	std::size_t facingNormals = 0;
	std::size_t textures = 0;
	std::size_t resolved = 0;
};

// A triangle faces its normals when its geometric normal points along the
// sum of its vertex normals.
bool FacesNormals( const mdl::Mesh &mesh, std::size_t i )
{
	const mdl::Vertex &a = mesh.vertices[mesh.indices[i]];
	const mdl::Vertex &b = mesh.vertices[mesh.indices[i + 1]];
	const mdl::Vertex &c = mesh.vertices[mesh.indices[i + 2]];
	const double e1[3] = {
	    b.position.x - a.position.x, b.position.y - a.position.y, b.position.z - a.position.z };
	const double e2[3] = {
	    c.position.x - a.position.x, c.position.y - a.position.y, c.position.z - a.position.z };
	const double n[3] = { e1[1] * e2[2] - e1[2] * e2[1], e1[2] * e2[0] - e1[0] * e2[2],
	    e1[0] * e2[1] - e1[1] * e2[0] };
	const double s[3] = { double( a.normal.x ) + b.normal.x + c.normal.x,
	    double( a.normal.y ) + b.normal.y + c.normal.y,
	    double( a.normal.z ) + b.normal.z + c.normal.z };
	return n[0] * s[0] + n[1] * s[1] + n[2] * s[2] > 0.0;
}

void RunVpk( const std::string &path )
{
	hammer::adapters::platform::DiskByteStore store;
	std::string error;
	std::unique_ptr<hammer::formats::VpkArchive> vpk =
	    hammer::formats::VpkArchive::Open( store, path, error );
	Check( vpk != nullptr, "opens " + path + ( error.empty() ? "" : " (" + error + ")" ) );
	if ( !vpk )
	{
		return;
	}
	VpkModelFiles files( *vpk );
	std::vector<std::string> models;
	vpk->ListAssets( "models/", ".mdl", models );
	Tally tally;
	int firstFailures = 0;
	for ( const std::string &model : models )
	{
		const std::string stem = model.substr( 0, model.size() - 4 );
		if ( !vpk->HasAsset( stem + ".vvd" ) )
		{
			continue; // an animation-only or $includemodel file
		}
		++tally.models;
		auto result = mdl::LoadModel( files, model );
		if ( !result.HasValue() )
		{
			if ( ++firstFailures <= 10 )
			{
				std::printf( "  %s: %s\n", model.c_str(), mdl::Describe( result.Error() ).c_str() );
			}
			continue;
		}
		++tally.parsed;
		const mdl::Model &m = result.Value();
		if ( m.TriangleCount() > 0 )
		{
			++tally.withTriangles;
			if ( !( m.mins.x < m.maxs.x || m.mins.y < m.maxs.y || m.mins.z < m.maxs.z ) )
			{
				++tally.emptyBoxes;
			}
		}
		for ( const mdl::Mesh &mesh : m.meshes )
		{
			for ( std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3 )
			{
				++tally.triangles;
				tally.facingNormals += FacesNormals( mesh, i ) ? 1 : 0;
			}
		}
		for ( const mdl::ResolvedMaterial &material : mdl::ResolveMaterials( m, files ) )
		{
			++tally.textures;
			tally.resolved += material.found ? 1 : 0;
		}
	}
	std::printf( "  %s: %d models, %d parsed, %zu triangles (%zu facing their normals), "
	             "%zu of %zu materials resolved in this VPK\n",
	    path.c_str(), tally.models, tally.parsed, tally.triangles, tally.facingNormals,
	    tally.resolved, tally.textures );
	Check( tally.models >= 100,
	    "the VPK holds a model corpus (" + std::to_string( tally.models ) + ")" );
	Check( tally.parsed == tally.models, "every model with a .vvd parses (" +
	                                         std::to_string( tally.parsed ) + " of " +
	                                         std::to_string( tally.models ) + ")" );
	Check( tally.emptyBoxes == 0, "every model with triangles has a box" );
	Check( tally.triangles > 0 && double( tally.facingNormals ) >= 0.97 * double( tally.triangles ),
	    "at least 97% of triangles wind counter-clockwise about their normals" );

	// The cube, checked in detail against figures an independent reader
	// (a Python walk of the same records) gave: Portal's version 44 box
	// and Portal 2's version 49 box.
	if ( vpk->HasAsset( "models/props/metal_box.mdl" ) &&
	     vpk->HasAsset( "materials/models/props/metal_box.vmt" ) )
	{
		auto box = mdl::LoadModel( files, "models/props/metal_box.mdl" );
		Check( box.HasValue(), "metal_box.mdl parses" );
		if ( box.HasValue() )
		{
			const mdl::Model &m = box.Value();
			const bool portal2 = m.version == 49;
			const std::size_t triangles = portal2 ? 5352 : 4664;
			const std::size_t textures = portal2 ? 12 : 2;
			Check( ( m.version == 44 || portal2 ) && m.meshes.size() == 1 &&
			           m.TriangleCount() == triangles,
			    "metal_box: version " + std::to_string( m.version ) + ", one mesh, " +
			        std::to_string( triangles ) + " triangles" );
			Check( m.textures.size() == textures && m.textures[0] == "metal_box" &&
			           m.cdMaterials == std::vector<std::string>{ "models/props/" },
			    "metal_box: " + std::to_string( textures ) +
			        " textures, $cdmaterials models/props/" );
			const auto materials = mdl::ResolveMaterials( m, files );
			Check( !materials.empty() &&
			           materials[0] == mdl::ResolvedMaterial{ "models/props/metal_box", true },
			    "metal_box: skin 0 material resolves" );
			Check( m.mins.x > -40 && m.maxs.x < 40 && m.maxs.z - m.mins.z > 20,
			    "metal_box: a cube about 36 units across" );
		}
	}
}

} // namespace

int main()
{
	const char *list = std::getenv( "STUDIO_MODEL_CORPUS_VPKS" );
	Check( list && *list, "STUDIO_MODEL_CORPUS_VPKS names the corpus" );
	if ( list )
	{
		std::stringstream stream( list );
		std::string path;
		while ( std::getline( stream, path, ',' ) )
		{
			if ( !path.empty() )
			{
				RunVpk( path );
			}
		}
	}
	return testing::ReportConformance( g_checks, g_failures );
}
