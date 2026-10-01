//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance suite for content.studio-model.v1
//			(unittests/mdltest/contracts/content.studio-model.v1.md): synthetic
//			models written by synthetic_model.h, independently of the
//			reader, parse to the meshes, vertices, winding, bounds, skins and
//			materials they describe, for the version 44 and 49 layouts, with
//			and without VVD fixups; malformed files fail with their named
//			status, file and offset; every truncation fails and random byte
//			damage never breaks the output's invariants.
//
//=============================================================================//

#include "synthetic_model.h"

#include "mdl/studio_model.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <random>
#include <string>

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
		std::printf( "FAIL: %s\n", what.c_str() );
	}
}

using mdltest::SyntheticFiles;
using mdltest::SyntheticModel;

foundation::Expected<mdl::Model, mdl::ModelError> Parse(
    const SyntheticFiles &files, std::int32_t body = 0 )
{
	return mdl::ParseModel( mdl::ModelBytes{ files.mdl, files.vvd, files.vtx, files.ani }, body );
}

bool Fails( const SyntheticFiles &files, mdl::ModelStatus status, mdl::ModelFile file )
{
	auto result = Parse( files );
	if ( result.HasValue() )
	{
		return false;
	}
	if ( result.Error().status != status || result.Error().file != file )
	{
		std::printf( "  got %s\n", mdl::Describe( result.Error() ).c_str() );
		return false;
	}
	return true;
}

mdl::Float3 Cross( mdl::Float3 a, mdl::Float3 b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

mdl::Float3 Sub( mdl::Float3 a, mdl::Float3 b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

float Dot( mdl::Float3 a, mdl::Float3 b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Every triangle is counter-clockwise seen from the side its normals face.
bool CounterClockwise( const mdl::Mesh &mesh )
{
	for ( std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3 )
	{
		const mdl::Vertex &a = mesh.vertices[mesh.indices[i]];
		const mdl::Vertex &b = mesh.vertices[mesh.indices[i + 1]];
		const mdl::Vertex &c = mesh.vertices[mesh.indices[i + 2]];
		const mdl::Float3 n = Cross( Sub( b.position, a.position ), Sub( c.position, a.position ) );
		if ( Dot( n, a.normal ) <= 0.0f )
		{
			return false;
		}
	}
	return true;
}

bool IndicesInRange( const mdl::Model &model )
{
	for ( const mdl::Mesh &mesh : model.meshes )
	{
		if ( mesh.indices.size() % 3 != 0 )
		{
			return false;
		}
		for ( std::uint32_t index : mesh.indices )
		{
			if ( index >= mesh.vertices.size() )
			{
				return false;
			}
		}
	}
	return true;
}

class FakeFiles : public mdl::IModelFiles
{
public:
	std::map<std::string, std::string> files;
	bool Exists( const std::string &path ) const override { return files.count( path ) != 0; }
	bool Read( const std::string &path, std::string &out ) const override
	{
		auto it = files.find( path );
		if ( it == files.end() )
		{
			return false;
		}
		out = it->second;
		return true;
	}
};

void Put32( std::string &bytes, std::size_t at, std::int32_t value )
{
	mdltest::detail::Put32( bytes, at, value );
}

std::size_t Get32( const std::string &bytes, std::size_t at )
{
	std::uint32_t u = 0;
	for ( int i = 0; i < 4; ++i )
	{
		u |= std::uint32_t( static_cast<std::uint8_t>( bytes[at + i] ) ) << ( 8 * i );
	}
	return static_cast<std::size_t>( static_cast<std::int32_t>( u ) );
}

// --- Positive cases ----------------------------------------------------------------------

void BoxCases()
{
	for ( std::int32_t version : { 44, 45, 48, 49 } )
	{
		const std::string tag = "v" + std::to_string( version ) + ": ";
		SyntheticModel box =
		    mdltest::BoxModel( { 16, 8, 4 }, "Models\\Props/", "Metal_Box", version );
		auto parsed = Parse( mdltest::WriteModel( box ) );
		Check( parsed.HasValue(), tag + "box parses" );
		if ( !parsed.HasValue() )
		{
			std::printf( "  %s\n", mdl::Describe( parsed.Error() ).c_str() );
			continue;
		}
		const mdl::Model &m = parsed.Value();
		Check( m.version == version && m.checksum == box.checksum, tag + "version and checksum" );
		Check( m.name == "synthetic/box.mdl", tag + "name" );
		Check( m.meshes.size() == 1, tag + "one mesh" );
		if ( m.meshes.size() != 1 )
		{
			continue;
		}
		Check( m.meshes[0].vertices.size() == 24 && m.meshes[0].indices.size() == 36,
		    tag + "24 vertices, 12 triangles" );
		Check( m.TriangleCount() == 12, tag + "TriangleCount" );
		Check( CounterClockwise( m.meshes[0] ),
		    tag + "triangles wind counter-clockwise from outside" );
		Check( m.mins == mdl::Float3{ -16, -8, -4 } && m.maxs == mdl::Float3{ 16, 8, 4 },
		    tag + "bounds are the box" );
		Check( m.meshes[0].vertices == box.bodyParts[0][0][0].vertices,
		    tag + "vertices equal the written ones, in order" );
		Check( m.textures == std::vector<std::string>{ "metal_box" }, tag + "texture lower-cased" );
		Check( m.cdMaterials == std::vector<std::string>{ "models/props/" },
		    tag + "$cdmaterials normalized" );
		Check( mdl::TextureIndex( m, m.meshes[0], 0 ) == 0, tag + "skin 0 texture" );
	}

	// The VVD's separate tangent block follows the raw vertex records.
	SyntheticFiles tangentFiles =
	    mdltest::WriteModel( mdltest::BoxModel( { 4, 4, 4 }, "a/", "t" ) );
	const std::size_t tangentStart = tangentFiles.vvd.size();
	const std::size_t tangentCount = Get32( tangentFiles.vvd, 16 );
	Put32( tangentFiles.vvd, 60, static_cast<std::int32_t>( tangentStart ) );
	tangentFiles.vvd.resize( tangentStart + tangentCount * 16 );
	for ( std::size_t i = 0; i < tangentCount; ++i )
	{
		mdltest::detail::PutF( tangentFiles.vvd, tangentStart + i * 16, 1.0f );
		mdltest::detail::PutF( tangentFiles.vvd, tangentStart + i * 16 + 4, 0.0f );
		mdltest::detail::PutF( tangentFiles.vvd, tangentStart + i * 16 + 8, 0.0f );
		mdltest::detail::PutF(
		    tangentFiles.vvd, tangentStart + i * 16 + 12, i == 0 ? -1.0f : 1.0f );
	}
	auto tangentModel = Parse( tangentFiles );
	Check( tangentModel.HasValue() &&
	           tangentModel.Value().meshes[0].vertices[0].tangent == mdl::Float3{ 1, 0, 0 } &&
	           tangentModel.Value().meshes[0].vertices[0].tangentSign == -1.0f &&
	           tangentModel.Value().meshes[0].vertices[1].tangentSign == 1.0f,
	    "VVD tangent frame and handedness reach the model vertices" );
	tangentFiles.vvd.pop_back();
	Check( Fails( tangentFiles, mdl::ModelStatus::Truncated, mdl::ModelFile::Vvd ),
	    "a truncated VVD tangent block fails" );

	// Fixups restore the logical vertex order.
	SyntheticModel two = mdltest::BoxModel( { 8, 8, 8 }, "a/", "t" );
	two.bodyParts[0][0].push_back( mdltest::BoxMesh( { 40, 0, 0 }, { 4, 4, 4 } ) );
	auto plain = Parse( mdltest::WriteModel( two ) );
	two.fixups = true;
	auto fixed = Parse( mdltest::WriteModel( two ) );
	Check( plain.HasValue() && fixed.HasValue(), "fixup model parses" );
	if ( plain.HasValue() && fixed.HasValue() )
	{
		Check( plain.Value().meshes == fixed.Value().meshes, "fixups give the unfixed meshes" );
		Check( fixed.Value().maxs.x == 44.0f && fixed.Value().mins.x == -8.0f,
		    "bounds cover both meshes" );
	}

	// A triangle strip becomes a list, wound counter-clockwise.
	SyntheticModel strip = mdltest::BoxModel( { 1, 1, 1 }, "a/", "t" );
	mdltest::SyntheticMesh quad;
	for ( float y : { 0.0f, 1.0f } )
	{
		for ( float x : { 0.0f, 1.0f } )
		{
			mdl::Vertex v;
			v.position = { x, y, 0 };
			v.normal = { 0, 0, 1 };
			quad.vertices.push_back( v );
		}
	}
	// Positions 0 (0,0) 1 (1,0) 2 (0,1) 3 (1,1); the strip 0 2 1 3 is
	// clockwise from +z for its first triangle, as a stored strip is.
	quad.stored = { 0, 2, 1, 3 };
	quad.tristrip = true;
	strip.bodyParts = { { { quad } } };
	auto stripped = Parse( mdltest::WriteModel( strip ) );
	Check( stripped.HasValue() && stripped.Value().TriangleCount() == 2 &&
	           CounterClockwise( stripped.Value().meshes[0] ),
	    "tristrip -> two counter-clockwise triangles" );
}

void LodCases()
{
	SyntheticModel model = mdltest::BoxModel( { 1, 1, 1 }, "fixture/", "base" );
	model.lodSwitches = { 0, 100, -1 };
	model.lodReplacements = { {}, { { 0, "low_detail" } }, {} };
	model.bodyParts[0][0][0].lodIndices = { { 0, 2, 1 }, {} };
	for ( int version : { 44, 49 } )
	{
		model.version = version;
		for ( bool fixups : { false, true } )
		{
			model.fixups = fixups;
			const SyntheticFiles files = mdltest::WriteModel( model );
			const mdl::ModelBytes bytes{ files.mdl, files.vvd, files.vtx, {} };
			const auto all = mdl::ParseModelGeometryVariants( bytes );
			Check( all && all.Value().lodTextures.size() == 3 && all.Value().meshes.size() == 2,
			    "all geometry levels include an explicitly blank shadow LOD" );
			if ( !all )
				continue;
			Check( all.Value().lodTextures[0][0] == "base" &&
			           all.Value().lodTextures[1][0] == "low_detail" &&
			           all.Value().lodTextures[2][0] == "base",
			    "LOD replacements preserve slot identity" );
			Check( all.Value().meshes[0].lod == 0 && all.Value().meshes[0].indices.size() == 36 &&
			           all.Value().meshes[1].lod == 1 &&
			           all.Value().meshes[1].indices == std::vector<std::uint32_t>{ 0, 1, 2 } &&
			           all.Value().meshes[1].vertices == model.bodyParts[0][0][0].vertices,
			    "lower LOD topology indexes the fixed-up mesh vertices" );
			for ( std::uint32_t lod = 0; lod < 3; ++lod )
			{
				const auto selected = mdl::ParseModel( bytes, 0, lod );
				std::vector<mdl::Mesh> expected;
				for ( const mdl::Mesh &mesh : all.Value().meshes )
					if ( mesh.lod == lod )
						expected.push_back( mesh );
				Check( selected && selected.Value().meshes == expected,
				    "selected LOD agrees with the complete geometry import" );
			}
			const auto invalid = mdl::ParseModel( bytes, 0, 3 );
			Check( !invalid && invalid.Error().status == mdl::ModelStatus::BadIndex &&
			           invalid.Error().file == mdl::ModelFile::Vtx,
			    "out-of-range LOD is refused" );
			SyntheticFiles damaged = files;
			const std::size_t part = Get32( damaged.vtx, 32 );
			const std::size_t sub = part + Get32( damaged.vtx, part + 4 );
			const std::size_t lowLod = sub + Get32( damaged.vtx, sub + 4 ) + 12;
			mdltest::detail::Put32( damaged.vtx, lowLod, -1 );
			const auto corrupted =
			    mdl::ParseModelGeometryVariants( { damaged.mdl, damaged.vvd, damaged.vtx, {} } );
			Check( Parse( damaged ) && !corrupted && corrupted.Error().file == mdl::ModelFile::Vtx,
			    "malformed inactive geometry level fails the full import without partial output" );
		}
	}
}

void FlexCases()
{
	SyntheticModel model = mdltest::BoxModel( { 1, 1, 1 }, "fixture/", "base" );
	model.flexDescriptors = { "left", "right" };
	model.lodSwitches = { 0, 100 };
	model.bodyParts[0][0][0].lodIndices = { { 0, 2, 1 } };
	mdl::Flex stereo;
	stereo.pair = 1;
	stereo.targets = { 0, 1, 2, 3 };
	mdl::FlexDelta delta;
	delta.vertex = 1;
	delta.position = { 0.625f, -0.5f, 0.25f };
	delta.normal = { 0.125f, 0.0f, -0.25f };
	delta.wrinkle = -0.5f;
	delta.speed = 64;
	delta.side = 192;
	stereo.deltas = { delta };
	mdl::Flex mono = stereo;
	mono.pair = 0;
	mono.deltas[0].vertex = 2;
	mono.deltas[0].wrinkle = 0.0f;
	model.bodyParts[0][0][0].flexes = { stereo, mono };
	for ( int version : { 44, 49 } )
	{
		model.version = version;
		for ( std::uint32_t flags : { 0u, 0x4000u, 0x200000u, 0x204000u } )
		{
			model.flags = flags;
			model.flexScale = 1.0f / 2048.0f;
			model.fixups = true;
			const SyntheticFiles files = mdltest::WriteModel( model );
			const auto parsed = mdl::ParseModelGeometryVariants(
			    { files.mdl, files.vvd, files.vtx, {} } );
			Check( parsed && parsed.Value().flexDescriptors == model.flexDescriptors &&
			           parsed.Value().meshes.size() == 2,
			    "flex descriptors and both LODs import with fixed-up vertices" );
			if ( !parsed )
				continue;
			Check( parsed.Value().meshes[0].flexes == model.bodyParts[0][0][0].flexes &&
			           parsed.Value().meshes[1].flexes == model.bodyParts[0][0][0].flexes,
			    "mono/stereo deltas preserve indices, speed, side, normals and signed wrinkle" );
		}
	}
	Check( stereo.Weight( -1 ) == 0 && stereo.Weight( 0 ) == 0 &&
	           stereo.Weight( 0.25f ) == 0.25f && stereo.Weight( 1 ) == 1 &&
	           stereo.Weight( 1.5f ) == 1 && stereo.Weight( 2 ) == 1 &&
	           stereo.Weight( 2.75f ) == 0.25f && stereo.Weight( 3 ) == 0 &&
	           stereo.Weight( 4 ) == 0,
	    "Studio flex ramp has zero endpoints, rising/falling ramps and a plateau" );
	mdl::Flex step = stereo;
	step.targets = { 0, 0, 1, 1 };
	Check( step.Weight( 0 ) == 0 && step.Weight( 0.5f ) == 1 && step.Weight( 1 ) == 0,
	    "coincident flex targets preserve endpoint behavior without division by zero" );
	model.flags = 0;
	model.bodyParts[0][0][0].flexes[0].deltas[0].position.x = 0.0003f;
	SyntheticFiles files = mdltest::WriteModel( model );
	const auto quantized = Parse( files );
	Check( quantized && quantized.Value().meshes[0].flexes[0].deltas[0].position.x ==
	                       1.0f / 4096.0f,
	    "on-disk half deltas use the engine's signed-fixed-point truncation" );
	const std::size_t part = Get32( files.mdl, 236 );
	const std::size_t sub = part + Get32( files.mdl, part + 12 );
	const std::size_t mesh = sub + Get32( files.mdl, sub + 76 );
	const std::size_t flex = mesh + Get32( files.mdl, mesh + 20 );
	const std::size_t vertex = flex + Get32( files.mdl, flex + 24 );
	const auto bad32 = [&]( std::size_t at, std::int32_t value, mdl::ModelStatus status,
	                       const char *name )
	{
		SyntheticFiles bad = files;
		mdltest::detail::Put32( bad.mdl, at, value );
		const auto parsed = mdl::ParseModelGeometryVariants( { bad.mdl, bad.vvd, bad.vtx, {} } );
		Check( !parsed && parsed.Error().file == mdl::ModelFile::Mdl &&
		           parsed.Error().status == status, name );
	};
	bad32( mesh + 16, -1, mdl::ModelStatus::BadCount, "negative flex count is refused" );
	bad32( mesh + 20, 0x7fffffff, mdl::ModelStatus::BadOffset, "out-of-file flex table is refused" );
	bad32( flex, 2, mdl::ModelStatus::BadIndex, "unknown flex descriptor is refused" );
	bad32( flex + 28, 2, mdl::ModelStatus::BadIndex, "unknown stereo partner is refused" );
	bad32( flex + 4, 0x7fc00000, mdl::ModelStatus::BadCount, "nonfinite flex ramp is refused" );
	bad32( flex + 8, 0x40800000, mdl::ModelStatus::BadCount, "unordered flex ramp is refused" );
	bad32( flex + 32, 2, mdl::ModelStatus::BadCount, "unknown flex delta encoding is refused" );
	bad32( vertex, 0xffff, mdl::ModelStatus::BadIndex, "flex delta outside its mesh is refused" );
	bad32( flex + 20, 0x7fffffff, mdl::ModelStatus::Truncated, "truncated flex deltas are refused" );
	mdltest::detail::Put32( files.mdl, 152, 0x200000 );
	bad32( 392, 0, mdl::ModelStatus::BadCount, "zero flex fixed-point scale is refused" );
	bad32( 392, 0x7f800000, mdl::ModelStatus::BadCount, "nonfinite flex fixed-point scale is refused" );
}

void BodyAndSkinCases()
{
	SyntheticModel model;
	model.textures = { "red", "blue", "green" };
	model.cdMaterials = { "m/" };
	model.skins = { { 0, 1 }, { 2, 1 } };
	model.bodyParts = {
	    { { mdltest::BoxMesh( { 0, 0, 0 }, { 1, 1, 1 }, 0 ) } },
	    { { mdltest::BoxMesh( { 10, 0, 0 }, { 1, 1, 1 }, 1 ) },
	        { mdltest::BoxMesh( { 20, 0, 0 }, { 2, 2, 2 }, 1 ) }, {} },
	};
	const SyntheticFiles files = mdltest::WriteModel( model );
	auto body0 = Parse( files, 0 );
	auto body1 = Parse( files, 1 );
	auto body2 = Parse( files, 2 );
	Check( body0.HasValue() && body1.HasValue() && body2.HasValue(), "body groups parse" );
	if ( !body0.HasValue() || !body1.HasValue() || !body2.HasValue() )
	{
		return;
	}
	Check( body0.Value().meshes.size() == 2 && body0.Value().maxs.x == 11.0f,
	    "body 0: part 0 and part 1's first model" );
	Check( body1.Value().meshes.size() == 2 && body1.Value().maxs.x == 22.0f,
	    "body 1 (base 1): part 1's second model" );
	Check( body2.Value().meshes.size() == 1 && body2.Value().maxs.x == 1.0f,
	    "body 2: part 1's blank model draws nothing" );
	auto variants =
	    mdl::ParseModelGeometryVariants( { files.mdl, files.vvd, files.vtx, files.ani } );
	Check( variants && variants.Value().meshes.size() == 3 &&
	           variants.Value().bodyParts.size() == 2 && variants.Value().maxs.x == 22.0f,
	    "all body variants are read once, including a blank submodel's selection" );
	if ( variants )
	{
		const mdl::Model &all = variants.Value();
		for ( int body : { -1, 0, 1, 2, 3, 4, 5, 1000000 } )
		{
			std::vector<mdl::Mesh> selected;
			for ( const mdl::Mesh &mesh : all.meshes )
			{
				if ( all.bodyParts[mesh.bodyPart].SelectedModel( body ) == mesh.bodyModel )
					selected.push_back( mesh );
			}
			const auto single = Parse( files, body );
			Check( single && single.Value().meshes == selected,
			    "body variants select the same geometry as the single-body reader: " +
			        std::to_string( body ) );
		}
		Check( all.meshes[2].bodyPart == 1 && all.meshes[2].bodyModel == 1 &&
		           all.bodyParts[1].SelectedModel( 2 ) == 2,
		    "body variant identity preserves the second and blank models" );
	}
	SyntheticFiles damaged = files;
	const std::size_t secondPart = Get32( damaged.mdl, 236 ) + 16;
	const std::size_t secondModel = secondPart + Get32( damaged.mdl, secondPart + 12 ) + 148;
	const std::size_t secondMesh = secondModel + Get32( damaged.mdl, secondModel + 76 );
	mdltest::detail::Put32( damaged.mdl, secondMesh + 8, -1 );
	const auto damagedVariants =
	    mdl::ParseModelGeometryVariants( { damaged.mdl, damaged.vvd, damaged.vtx, damaged.ani } );
	Check( Parse( damaged, 0 ) && !damagedVariants &&
	           damagedVariants.Error().status == mdl::ModelStatus::BadCount &&
	           damagedVariants.Error().file == mdl::ModelFile::Mdl,
	    "all-body read refuses damage in an inactive alternative, without a partial result" );
	SyntheticModel combinations = model;
	combinations.bodyParts[0].push_back( { mdltest::BoxMesh( { -10, 0, 0 }, { 1, 1, 1 }, 0 ) } );
	const SyntheticFiles combinedFiles = mdltest::WriteModel( combinations );
	const auto combined = mdl::ParseModelGeometryVariants(
	    { combinedFiles.mdl, combinedFiles.vvd, combinedFiles.vtx, combinedFiles.ani } );
	Check(
	    combined && combined.Value().meshes.size() == 4 && combined.Value().bodyParts[1].base == 2,
	    "multiple body groups store submodels without expanding their combinations" );
	if ( combined )
	{
		for ( int body = 0; body < 6; ++body )
		{
			std::vector<mdl::Mesh> selected;
			for ( const mdl::Mesh &mesh : combined.Value().meshes )
			{
				if ( combined.Value().bodyParts[mesh.bodyPart].SelectedModel( body ) ==
				     mesh.bodyModel )
					selected.push_back( mesh );
			}
			const auto single = Parse( combinedFiles, body );
			Check( single && selected == single.Value().meshes,
			    "multiple body-group selection preserves Source base arithmetic: " +
			        std::to_string( body ) );
		}
	}
	const mdl::Model &m = body0.Value();
	Check( m.meshes[1].bodyPart == 1, "mesh records its body part" );
	Check(
	    mdl::TextureIndex( m, m.meshes[0], 0 ) == 0 && mdl::TextureIndex( m, m.meshes[0], 1 ) == 2,
	    "skin family selects the texture" );
	Check( mdl::TextureIndex( m, m.meshes[1], 1 ) == 1, "skin 1 shares a texture" );
	Check(
	    mdl::TextureIndex( m, m.meshes[0], 7 ) == 0 && mdl::TextureIndex( m, m.meshes[0], -1 ) == 0,
	    "a skin out of range uses family 0" );
}

void MaterialAndLoadCases()
{
	SyntheticModel model = mdltest::BoxModel( { 1, 1, 1 }, "models/first", "crate" );
	model.textures = { "crate", "lid", "Sub\\Dir\\Plate" };
	model.cdMaterials = { "models/first", "\\models\\second\\" };
	model.skins = { { 0 } };
	const SyntheticFiles files = mdltest::WriteModel( model );
	FakeFiles fake;
	fake.files["materials/models/second/crate.vmt"] = "x";
	fake.files["materials/models/first/crate.vmt"] = "x";
	fake.files["materials/models/second/lid.vmt"] = "x";
	fake.files["materials/models/second/sub/dir/plate.vmt"] = "x";
	fake.files["models/props/crate.mdl"] = files.mdl;
	fake.files["models/props/crate.vvd"] = files.vvd;
	std::string oldVtx = files.vtx;
	Put32( oldVtx, 0, 6 ); // a version-6 VTX: read only when .dx90.vtx is absent
	fake.files["models/props/crate.vtx"] = oldVtx;
	fake.files["models/props/crate.dx90.vtx"] = files.vtx;

	auto loaded = mdl::LoadModel( fake, "Models\\Props\\Crate.MDL" );
	Check( loaded.HasValue(), "LoadModel canonicalizes the path and prefers .dx90.vtx" );
	if ( loaded.HasValue() )
	{
		const std::vector<mdl::ResolvedMaterial> materials =
		    mdl::ResolveMaterials( loaded.Value(), fake );
		Check( materials.size() == 3, "one resolved material per texture" );
		if ( materials.size() == 3 )
		{
			Check( materials[0] == mdl::ResolvedMaterial{ "models/first/crate", true },
			    "first $cdmaterials directory wins" );
			Check( materials[1] == mdl::ResolvedMaterial{ "models/second/lid", true },
			    "later directory when the first lacks it" );
			Check( materials[2] == mdl::ResolvedMaterial{ "models/second/sub/dir/plate", true },
			    "texture paths with directories" );
		}
		fake.files.erase( "materials/models/second/lid.vmt" );
		Check( mdl::ResolveMaterials( loaded.Value(), fake )[1] ==
		           mdl::ResolvedMaterial{ "models/first/lid", false },
		    "an unresolved texture names the first directory's candidate, not found" );
	}
	fake.files.erase( "models/props/crate.dx90.vtx" );
	auto fallback = mdl::LoadModel( fake, "models/props/crate.mdl" );
	Check( !fallback.HasValue() &&
	           fallback.Error().status == mdl::ModelStatus::UnsupportedVersion &&
	           fallback.Error().file == mdl::ModelFile::Vtx,
	    "the next VTX (.vtx) is read when .dx90.vtx is absent" );
	fake.files.erase( "models/props/crate.vtx" );
	auto noVtx = mdl::LoadModel( fake, "models/props/crate.mdl" );
	Check( !noVtx.HasValue() && noVtx.Error() == mdl::ModelError{ mdl::ModelStatus::MissingFile,
	                                                 mdl::ModelFile::Vtx, 0 },
	    "no VTX: MissingFile in vtx" );
	fake.files.erase( "models/props/crate.vvd" );
	auto noVvd = mdl::LoadModel( fake, "models/props/crate.mdl" );
	Check( !noVvd.HasValue() && noVvd.Error().file == mdl::ModelFile::Vvd,
	    "no VVD: MissingFile in vvd" );
	auto noMdl = mdl::LoadModel( fake, "models/props/absent.mdl" );
	Check( !noMdl.HasValue() && noMdl.Error().file == mdl::ModelFile::Mdl,
	    "no MDL: MissingFile in mdl" );
	auto notMdl = mdl::LoadModel( fake, "models/props/crate.vvd" );
	Check( !notMdl.HasValue() && notMdl.Error().status == mdl::ModelStatus::MissingFile,
	    "a path without .mdl is MissingFile" );
	Check(
	    mdl::CanonicalModelPath( "\\Models/A\\B.mdl" ) == "models/a/b.mdl", "CanonicalModelPath" );
	Check( mdl::Describe( { mdl::ModelStatus::Truncated, mdl::ModelFile::Vvd, 64 } ) ==
	           "Truncated in vvd at byte 64",
	    "Describe names status, file and offset" );
}

// --- Malformed input ---------------------------------------------------------------------

void MalformedCases()
{
	using mdl::ModelFile;
	using mdl::ModelStatus;
	const SyntheticModel box = mdltest::BoxModel( { 1, 1, 1 }, "a/", "t" );
	const SyntheticFiles good = mdltest::WriteModel( box );
	Check( Parse( good ).HasValue(), "the unmodified model parses" );

	auto mutate = [&]( auto edit )
	{
		SyntheticFiles files = good;
		edit( files );
		return files;
	};
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  f.mdl[0] = 'X';
	                  } ),
	           ModelStatus::BadMagic, ModelFile::Mdl ),
	    "MDL magic" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  f.vvd[0] = 'X';
	                  } ),
	           ModelStatus::BadMagic, ModelFile::Vvd ),
	    "VVD magic" );
	for ( std::int32_t version : { 43, 50, 2531 } )
	{
		Check( Fails( mutate(
		                  [&]( SyntheticFiles &f )
		                  {
			                  Put32( f.mdl, 4, version );
		                  } ),
		           ModelStatus::UnsupportedVersion, ModelFile::Mdl ),
		    "MDL version " + std::to_string( version ) );
	}
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.vvd, 4, 3 );
	                  } ),
	           ModelStatus::UnsupportedVersion, ModelFile::Vvd ),
	    "VVD version" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.vtx, 0, 6 );
	                  } ),
	           ModelStatus::UnsupportedVersion, ModelFile::Vtx ),
	    "VTX version" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.vvd, 8, 1 );
	                  } ),
	           ModelStatus::ChecksumMismatch, ModelFile::Vvd ),
	    "VVD checksum" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.vtx, 16, 1 );
	                  } ),
	           ModelStatus::ChecksumMismatch, ModelFile::Vtx ),
	    "VTX checksum" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.mdl, 204, -1 );
	                  } ),
	           ModelStatus::BadCount, ModelFile::Mdl ),
	    "negative texture count" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.mdl, 208, 0x7ffffff0 );
	                  } ),
	           ModelStatus::Truncated, ModelFile::Mdl ),
	    "texture table past the end" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.mdl, 232, 2 );
	                  } ),
	           ModelStatus::BadCount, ModelFile::Vtx ),
	    "body part counts disagree" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.vtx, 28, 2 );
	                  } ),
	           ModelStatus::BadCount, ModelFile::Vtx ),
	    "VTX body part count disagrees" );
	// Skin table entry naming a texture that does not exist.
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  const std::size_t skins = Get32( f.mdl, 228 );
		                  mdltest::detail::Put16( f.mdl, skins, 5 );
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Mdl ),
	    "skin entry out of range" );
	// VVD: vertex data offset before the header, LOD 0 count past the data.
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.vvd, 56, 8 );
	                  } ),
	           ModelStatus::BadOffset, ModelFile::Vvd ),
	    "VVD data inside the header" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.vvd, 16, 25 );
	                  } ),
	           ModelStatus::Truncated, ModelFile::Vvd ),
	    "VVD LOD 0 count past the data" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.vvd, 16, 20 );
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Mdl ),
	    "a mesh past the LOD 0 vertices" );
	Check( Fails( mutate(
	                  []( SyntheticFiles &f )
	                  {
		                  Put32( f.vtx, 32, -1000 );
	                  } ),
	           ModelStatus::BadOffset, ModelFile::Vtx ),
	    "VTX body part offset before the file" );

	// Index and vertex references in the VTX. Find the strip group: the
	// one mesh's group offset, relative to the VTX mesh record.
	const auto groupOf = []( const std::string &vtx )
	{
		const std::size_t part = Get32( vtx, 32 );
		const std::size_t sub = part + Get32( vtx, part + 4 );
		const std::size_t lod = sub + Get32( vtx, sub + 4 );
		const std::size_t mesh = lod + Get32( vtx, lod + 4 );
		return mesh + Get32( vtx, mesh + 4 );
	};
	const std::size_t group = groupOf( good.vtx );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  const std::size_t verts = group + Get32( f.vtx, group + 4 );
		                  mdltest::detail::Put16( f.vtx, verts + 4, 24 );
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Vtx ),
	    "a VTX vertex naming a mesh vertex that does not exist" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  const std::size_t indices = group + Get32( f.vtx, group + 12 );
		                  mdltest::detail::Put16( f.vtx, indices + 6, 24 );
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Vtx ),
	    "a strip index past the group's vertices" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  Put32( f.vtx, group + 8, 35 );
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Vtx ),
	    "a strip past the group's indices" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  const std::size_t strip = group + Get32( f.vtx, group + 20 );
		                  Put32( f.vtx, strip, 35 );
		                  Put32( f.vtx, group + 8, 36 );
	                  } ),
	           ModelStatus::BadCount, ModelFile::Vtx ),
	    "a triangle list strip that is not a multiple of three" );
	// Version 44 read with the version 49 strides (and the reverse) must
	// not pass: the MDL version selects the VTX layout.
	SyntheticModel old = box;
	old.version = 44;
	SyntheticFiles mixed = mdltest::WriteModel( old );
	Put32( mixed.mdl, 4, 49 );
	Check( !Parse( mixed ).HasValue(), "a version 44 VTX under a version 49 header fails" );
}

// Every strict prefix of each file fails; random damage never produces an
// out-of-range index.
void RobustnessCases()
{
	SyntheticModel model = mdltest::BoxModel( { 3, 2, 1 }, "a/", "t" );
	model.bodyParts[0][0].push_back( mdltest::BoxMesh( { 9, 0, 0 }, { 1, 1, 1 } ) );
	model.fixups = true;
	const SyntheticFiles good = mdltest::WriteModel( model );
	int prefixes = 0;
	int prefixFailures = 0;
	for ( int which = 0; which < 3; ++which )
	{
		const std::string &source = which == 0 ? good.mdl : which == 1 ? good.vvd : good.vtx;
		for ( std::size_t length = 0; length < source.size(); ++length )
		{
			SyntheticFiles cut = good;
			std::string &target = which == 0 ? cut.mdl : which == 1 ? cut.vvd : cut.vtx;
			target.resize( length );
			++prefixes;
			if ( !Parse( cut ).HasValue() )
			{
				++prefixFailures;
			}
		}
	}
	Check( prefixes > 1000 && prefixFailures == prefixes,
	    "every strict prefix of every file fails (" + std::to_string( prefixFailures ) + " of " +
	        std::to_string( prefixes ) + ")" );

	std::mt19937 random( 20260928u );
	int damaged = 0;
	int parsedAnyway = 0;
	bool invariantHeld = true;
	for ( int trial = 0; trial < 3000; ++trial )
	{
		SyntheticFiles broken = good;
		const int flips = 1 + static_cast<int>( random() % 4 );
		for ( int f = 0; f < flips; ++f )
		{
			std::string &target = trial % 3 == 0   ? broken.mdl
			                      : trial % 3 == 1 ? broken.vvd
			                                       : broken.vtx;
			target[random() % target.size()] = static_cast<char>( random() & 0xff );
		}
		++damaged;
		auto result = Parse( broken );
		if ( result.HasValue() )
		{
			++parsedAnyway;
			invariantHeld = invariantHeld && IndicesInRange( result.Value() );
		}
	}
	Check( invariantHeld, "damaged files that still parse keep every index in range (" +
	                          std::to_string( parsedAnyway ) + " of " + std::to_string( damaged ) +
	                          " parsed)" );
}

} // namespace

int main()
{
	BoxCases();
	BodyAndSkinCases();
	LodCases();
	FlexCases();
	MaterialAndLoadCases();
	MalformedCases();
	RobustnessCases();
	if ( const char *directory = std::getenv( "STUDIO_MODEL_LOD_FIXTURE_DIR" ) )
	{
		const SyntheticFiles fixture = mdltest::WriteModel( mdltest::LodPixelModel() );
		std::filesystem::create_directories( directory );
		for ( const auto &[name, data] :
		    { std::pair{ "selection.mdl", fixture.mdl }, std::pair{ "selection.vvd", fixture.vvd },
		        std::pair{ "selection.vtx", fixture.vtx } } )
		{
			std::ofstream out( std::filesystem::path( directory ) / name, std::ios::binary );
			out.write( data.data(), static_cast<std::streamsize>( data.size() ) );
			Check( out.good(), "LOD pixel fixture writes complete bytes" );
		}
	}
	std::printf( "content.studio-model: %d checks, %d failures\n", g_checks, g_failures );
	return testing::ReportConformance( g_checks, g_failures );
}
