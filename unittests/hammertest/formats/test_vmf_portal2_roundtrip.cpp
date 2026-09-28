//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Real Portal 2 maps through the strict VMF codec (RFC 0002, R08
//			domain logic; formats.vmf_map_codec.v1). The maps vendored under
//			unittests/hammertest/fixtures/portal2/ (CRLF, written by the retail
//			Portal 2 Hammer) must load into the typed scene model with no
//			warnings, save, and reload without losing any state:
//			  * Decode succeeds with no warnings and Validate() is empty;
//			  * Decode(Encode(doc)) is SameContent with doc (same serial, same
//			    runtime ids);
//			  * Encode(Decode(Encode(x))) == Encode(Decode(x)) byte for byte;
//			  * the original file and the re-encoded text are semantically
//			    equal: block structure and order, key/value multisets, numbers
//			    compared by value (legacy writes "1.52588e-005");
//			  * structural counts (solids, sides, displacements, entities,
//			    connections, groups, visgroups, hidden objects, cameras,
//			    cordons) of the original text equal the decoded document's.
//			Negative: the file comparator catches a seeded single-key change
//			and a dropped connection, and accepts a respelled number.
//
//=============================================================================//

#include "hammer/formats/vmf_map_codec.h"
#include "map_codec_oracle.h"
#include "testing/checks.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

using hammer::formats::VmfMapCodec;
using hammer::scene::MapDocument;

namespace
{

std::string ReadFile( const std::filesystem::path &path )
{
	std::ifstream input( path, std::ios::binary );
	return std::string( std::istreambuf_iterator<char>{ input }, std::istreambuf_iterator<char>{} );
}

// The repository root: the working directory when it holds the fixtures, else
// derived from this source file's path.
std::filesystem::path RepoRoot()
{
	if ( std::filesystem::is_directory( "unittests/hammertest/fixtures/portal2" ) )
	{
		return std::filesystem::current_path();
	}
	return std::filesystem::path( __FILE__ )
	    .parent_path()
	    .parent_path()
	    .parent_path()
	    .parent_path();
}

// The first block named 'name' below 'node' (depth first), or nullptr.
kvtext::KeyValueNode *FindFirst( kvtext::KeyValueNode &node, const std::string &name )
{
	for ( kvtext::KeyValueNode &child : node.children )
	{
		if ( child.name == name )
		{
			return &child;
		}
		if ( kvtext::KeyValueNode *inner = FindFirst( child, name ) )
		{
			return inner;
		}
	}
	return nullptr;
}

} // namespace

int main()
{
	testing::Checks checks;
	const VmfMapCodec codec;
	const std::filesystem::path dir =
	    RepoRoot() / "unittests" / "hammertest" / "fixtures" / "portal2";

	for ( const char *name : { "sp_a2_trust_fling.vmf", "zoo_mechanics.vmf" } )
	{
		const std::string label = name;
		const std::string original = ReadFile( dir / name );
		if ( !checks.That( !original.empty(), label + ": the vendored map is readable" ) )
		{
			continue;
		}

		constexpr std::uint32_t kSerial = 21;
		const auto first = codec.Decode( original, kSerial );
		if ( !checks.That( first.HasValue(),
		         label + ": decodes" +
		             ( first ? ""
		                     : " (" + first.Error().message + ", line " +
		                           std::to_string( first.Error().line ) + ")" ) ) )
		{
			continue;
		}
		const MapDocument &doc = first.Value().document;
		checks.That( first.Value().warnings.empty(),
		    label + ": decodes with no warnings" +
		        ( first.Value().warnings.empty()
		                ? ""
		                : " (" + first.Value().warnings.front().message + ")" ) );
		checks.That( doc.Validate().empty(), label + ": the document is consistent" );

		const auto encoded = codec.Encode( doc );
		if ( !checks.That( encoded.HasValue(), label + ": encodes" ) )
		{
			continue;
		}
		const auto second = codec.Decode( encoded.Value(), kSerial );
		checks.That( second && second.Value().warnings.empty(),
		    label + ": the saved text reloads with no warnings" );
		checks.That( second && hammer::scene::SameContent( second.Value().document, doc ),
		    label + ": reload is the same content with the same runtime ids" );
		const auto reencoded = second ? codec.Encode( second.Value().document ) : encoded;
		checks.That( reencoded && reencoded.Value() == encoded.Value(),
		    label + ": saving is a fixed point" );

		// The original file against the saved text.
		const kvtext::ParseResult source = kvtext::ParseKeyValues( original );
		kvtext::ParseResult saved = kvtext::ParseKeyValues( encoded.Value() );
		checks.That( source.ok && saved.ok, label + ": both texts parse" );
		const kvtext::CompareResult same =
		    map_codec_oracle::CompareFileSemantics( source.root, saved.root );
		checks.That( same.equal,
		    label + ": the saved text is semantically the original: " + same.firstDivergence );

		const map_codec_oracle::ContentCounts inText = map_codec_oracle::CountText( source.root );
		const map_codec_oracle::ContentCounts inDoc = map_codec_oracle::CountDocument( doc );
		checks.That( inText == inDoc, label + ": counts match (text: " + inText.Describe() +
		                                  "; document: " + inDoc.Describe() + ")" );
		checks.That( map_codec_oracle::CountText( saved.root ) == inText,
		    label + ": the saved text has the same counts" );
		checks.That( inDoc.solids > 0 && inDoc.sides >= 4 * inDoc.solids && inDoc.entities > 0 &&
		                 inDoc.connections > 0 && inDoc.hidden > 0 && inDoc.cameras > 0 &&
		                 inDoc.visgroups > 0,
		    label + ": the map exercises solids, entities, connections, hidden objects, cameras "
		            "and visgroups" );

		// Negative: the comparator catches a changed key and a dropped connection,
		// and accepts a number spelled differently.
		{
			kvtext::ParseResult changed = kvtext::ParseKeyValues( encoded.Value() );
			kvtext::KeyValueNode *entity = FindFirst( changed.root, "entity" );
			if ( checks.That(
			         entity && !entity->pairs.empty(), label + ": the saved text has an entity" ) )
			{
				entity->pairs.back().value += "_changed";
				checks.That(
				    !map_codec_oracle::CompareFileSemantics( source.root, changed.root ).equal,
				    label + ": a single changed key is detected (negative)" );
			}
			kvtext::ParseResult dropped = kvtext::ParseKeyValues( encoded.Value() );
			kvtext::KeyValueNode *connections = FindFirst( dropped.root, "connections" );
			if ( checks.That( connections && !connections->pairs.empty(),
			         label + ": the saved text has connections" ) )
			{
				connections->pairs.erase( connections->pairs.begin() );
				checks.That(
				    !map_codec_oracle::CompareFileSemantics( source.root, dropped.root ).equal,
				    label + ": a dropped connection is detected (negative)" );
				checks.That( !( map_codec_oracle::CountText( dropped.root ) == inText ),
				    label + ": a dropped connection changes the counts (negative)" );
			}
			kvtext::ParseResult respelled = kvtext::ParseKeyValues( encoded.Value() );
			kvtext::KeyValueNode *side = FindFirst( respelled.root, "side" );
			if ( checks.That(
			         side && side->Find( "rotation" ), label + ": the saved text has a side" ) )
			{
				for ( kvtext::KeyValue &kv : side->pairs )
				{
					if ( kv.key == "rotation" )
					{
						kv.value = kv.value == "0" ? "0.000" : kv.value + "e0";
					}
				}
				checks.That(
				    map_codec_oracle::CompareFileSemantics( source.root, respelled.root ).equal,
				    label + ": a respelled number compares equal by value" );
			}
		}
	}

	return checks.Report();
}
