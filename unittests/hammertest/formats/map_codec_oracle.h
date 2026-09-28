//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The hammer::ports::IMapCodec conformance oracle (RFC 0002,
//			ports.map_codec.v1), shared by the VMF codec's positive suite and
//			its sensitivity suite. Every clause takes 'const IMapCodec &' and
//			returns Findings (empty = pass), so any codec can be run through
//			the same predicates and a seeded bad codec is judged by exactly the
//			oracle that passes the real one.
//
//			Clauses:
//			  RoundTrip          decode -> Validate -> encode -> decode is the
//			                     same content with the same runtime ids, the
//			                     second encode is byte-identical, ids carry the
//			                     requested serial, and decoding is repeatable.
//			  DocumentRoundTrip  an edited document (ids in any order) survives
//			                     encode -> decode up to runtime ids.
//			  CanonicalText      text already in the writer's canonical form is
//			                     reproduced byte for byte and compares equal
//			                     under kvtext::CompareKeyValues.
//			  SemanticText       parse(text) and parse(encode(decode(text)))
//			                     compare equal under kvtext::CompareKeyValues.
//			  CanonicalFixture   every modeled field of the VMF feature fixture
//			                     decodes to its exact expected value.
//			  MissingIdsFixture  objects and sides without ids get fresh unique
//			                     ids, with one warning (the only lossless oddity).
//			  RejectsUnmodeled   each kind of content the model cannot hold is a
//			                     CodecError naming its block path and line.
//			  CountText/CountDocument, CompareFileSemantics
//			                     file-level checks for maps from other writers:
//			                     structural counts, and block/key/value equality
//			                     with numbers compared by value.
//
//			The fixtures are VMF text in the legacy writer's layout (tabs, pairs
//			before child blocks, legacy key order); they are built by small
//			writers here so solids stay correct by construction.
//
//=============================================================================//

#ifndef HAMMERTEST_FORMATS_MAP_CODEC_ORACLE_H
#define HAMMERTEST_FORMATS_MAP_CODEC_ORACLE_H

#include "hammer/ports/map_codec.h"
#include "hammer/scene/map_document.h"
#include "hammer/scene/map_queries.h"
#include "kvtext/keyvalues.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace map_codec_oracle
{

using hammer::ports::DecodedMap;
using hammer::ports::IMapCodec;
using hammer::scene::DocumentSettings;
using hammer::scene::Entity;
using hammer::scene::Group;
using hammer::scene::MapDocument;
using hammer::scene::ObjectId;
using hammer::scene::Solid;
using mapgeometry::Vec3d;

struct Findings
{
	std::vector<std::string> problems;

	bool Ok() const { return problems.empty(); }
	bool Expect( bool condition, const std::string &what )
	{
		if ( !condition )
		{
			problems.push_back( what );
		}
		return condition;
	}
	std::string Summary() const
	{
		std::string out;
		for ( std::size_t i = 0; i < problems.size() && i < 4; ++i )
		{
			out += ( i ? "; " : "" ) + problems[i];
		}
		return out;
	}
};

inline std::optional<DecodedMap> DecodeOrReport( const IMapCodec &codec, std::string_view text,
    std::uint32_t serial, Findings &f, const std::string &what )
{
	auto result = codec.Decode( text, serial );
	if ( !result )
	{
		f.problems.push_back( what + ": decode failed: " + result.Error().message + " (line " +
		                      std::to_string( result.Error().line ) + ")" );
		return std::nullopt;
	}
	return std::move( result ).Value();
}

inline std::optional<std::string> EncodeOrReport(
    const IMapCodec &codec, const MapDocument &doc, Findings &f, const std::string &what )
{
	auto result = codec.Encode( doc );
	if ( !result )
	{
		f.problems.push_back( what + ": encode failed: " + result.Error().message );
		return std::nullopt;
	}
	return std::move( result ).Value();
}

// --- Content comparison up to runtime ids ----------------------------------------------

// The cordon form a codec must produce for 'settings' (map_document.h: the
// single form only when it can express the content: one cordon with one box,
// and, since that form stores one flag and no name, an unnamed cordon whose
// flag equals cordonsActive; otherwise the list form).
inline DocumentSettings::CordonForm ExpectedCordonForm( const DocumentSettings &s )
{
	using Form = DocumentSettings::CordonForm;
	const bool single = s.cordons.size() == 1 && s.cordons.front().boxes.size() == 1 &&
	                    s.cordons.front().name.empty() &&
	                    s.cordons.front().active == s.cordonsActive;
	if ( s.cordonForm == Form::Single && single )
	{
		return Form::Single;
	}
	return s.cordonForm != Form::None || !s.cordons.empty() ? Form::List : Form::None;
}

// True when 'b' holds 'a''s content with runtime ids renamed: objects of each
// kind pair up by persistent (VMF) id, in id order among equal persistent ids,
// and every reference (id, owner, group) maps through the pairing.
inline bool SameContentUpToIds( const MapDocument &a, const MapDocument &b, std::string &why )
{
	std::map<ObjectId, ObjectId> aToB;
	auto pairUp = [&]( const auto &mapA, const auto &mapB, const char *kind ) -> bool
	{
		if ( mapA.size() != mapB.size() )
		{
			why = std::string( kind ) + " count " + std::to_string( mapA.size() ) + " vs " +
			      std::to_string( mapB.size() );
			return false;
		}
		std::multimap<std::uint32_t, ObjectId> free;
		for ( const auto &[id, object] : mapB )
		{
			free.emplace( object.vmfId, id );
		}
		for ( const auto &[id, object] : mapA )
		{
			const auto it = free.find( object.vmfId );
			if ( it == free.end() )
			{
				why = std::string( kind ) + " with persistent id " +
				      std::to_string( object.vmfId ) + " is missing";
				return false;
			}
			aToB[id] = it->second;
			free.erase( it );
		}
		return true;
	};
	if ( !pairUp( a.Solids(), b.Solids(), "solid" ) ||
	     !pairUp( a.Entities(), b.Entities(), "entity" ) ||
	     !pairUp( a.Groups(), b.Groups(), "group" ) )
	{
		return false;
	}
	auto map = [&]( ObjectId id ) -> ObjectId
	{
		if ( !id.IsValid() )
		{
			return id;
		}
		const auto it = aToB.find( id );
		return it == aToB.end() ? ObjectId() : it->second;
	};
	for ( const auto &[id, solid] : a.Solids() )
	{
		Solid expected = solid;
		expected.id = map( solid.id );
		expected.owner = map( solid.owner );
		expected.group = map( solid.group );
		const Solid *actual = b.FindSolid( expected.id );
		if ( !actual || !( *actual == expected ) )
		{
			why = "solid " + std::to_string( solid.vmfId ) + " differs";
			return false;
		}
	}
	for ( const auto &[id, entity] : a.Entities() )
	{
		Entity expected = entity;
		expected.id = map( entity.id );
		expected.group = map( entity.group );
		const Entity *actual = b.FindEntity( expected.id );
		if ( !actual || !( *actual == expected ) )
		{
			why = "entity " + std::to_string( entity.vmfId ) + " differs";
			return false;
		}
	}
	for ( const auto &[id, group] : a.Groups() )
	{
		Group expected = group;
		expected.id = map( group.id );
		expected.group = map( group.group );
		const Group *actual = b.FindGroup( expected.id );
		if ( !actual || !( *actual == expected ) )
		{
			why = "group " + std::to_string( group.vmfId ) + " differs";
			return false;
		}
	}
	DocumentSettings expected = a.Settings();
	expected.cordonForm = ExpectedCordonForm( expected );
	if ( !( expected == b.Settings() ) )
	{
		why = "document settings differ";
		return false;
	}
	return true;
}

// --- Clauses -------------------------------------------------------------------------------

inline Findings RoundTrip( const IMapCodec &codec, std::string_view text, std::uint32_t serial = 7 )
{
	Findings f;
	const std::optional<DecodedMap> first = DecodeOrReport( codec, text, serial, f, "decode" );
	if ( !first )
	{
		return f;
	}
	const std::vector<std::string> problems = first->document.Validate();
	f.Expect( problems.empty(),
	    "decoded document is inconsistent: " + ( problems.empty() ? "" : problems.front() ) );
	f.Expect( first->document.Serial() == serial, "decoded document carries the serial" );
	bool serialOk = true;
	for ( const ObjectId id : first->document.SolidIds() )
	{
		serialOk = serialOk && hammer::scene::DocumentSerialOf( id ) == serial;
	}
	for ( const ObjectId id : first->document.EntityIds() )
	{
		serialOk = serialOk && hammer::scene::DocumentSerialOf( id ) == serial;
	}
	for ( const ObjectId id : first->document.GroupIds() )
	{
		serialOk = serialOk && hammer::scene::DocumentSerialOf( id ) == serial;
	}
	f.Expect( serialOk, "every runtime id carries the requested serial" );

	const std::optional<DecodedMap> again =
	    DecodeOrReport( codec, text, serial, f, "decode again" );
	f.Expect( again && hammer::scene::SameContent( again->document, first->document ) &&
	              again->document.Solids() == first->document.Solids(),
	    "decoding the same text twice gives the same document" );

	const std::optional<std::string> encoded =
	    EncodeOrReport( codec, first->document, f, "encode" );
	if ( !encoded )
	{
		return f;
	}
	const std::optional<DecodedMap> second =
	    DecodeOrReport( codec, *encoded, serial, f, "decode(encode)" );
	if ( !second )
	{
		return f;
	}
	f.Expect( hammer::scene::SameContent( second->document, first->document ),
	    "decode(encode(decode(text))) is the same content with the same runtime ids" );
	f.Expect( second->document.Validate().empty(), "the re-decoded document is consistent" );
	const std::optional<std::string> reencoded =
	    EncodeOrReport( codec, second->document, f, "re-encode" );
	f.Expect( reencoded && *reencoded == *encoded, "encoding is idempotent (byte-identical)" );
	return f;
}

inline Findings DocumentRoundTrip( const IMapCodec &codec, const MapDocument &doc )
{
	Findings f;
	const std::optional<std::string> encoded = EncodeOrReport( codec, doc, f, "encode" );
	if ( !encoded )
	{
		return f;
	}
	const std::optional<DecodedMap> decoded =
	    DecodeOrReport( codec, *encoded, doc.Serial(), f, "decode(encode(doc))" );
	if ( !decoded )
	{
		return f;
	}
	f.Expect( decoded->document.Validate().empty(), "decode(encode(doc)) is consistent" );
	std::string why;
	f.Expect( SameContentUpToIds( doc, decoded->document, why ),
	    "decode(encode(doc)) holds doc's content up to runtime ids: " + why );
	return f;
}

// How SemanticText compares values: as written, or with every numeric token
// compared by value (for maps from other writers, e.g. MSVC's "1e-005").
enum class NumberText
{
	Exact,
	ByValue,
};

// Rewrites each numeric token of every value (tokens split at blanks and at
// "()[]") in its shortest exact spelling.
inline void NormalizeNumbers( kvtext::KeyValueNode &node )
{
	auto normalize = []( const std::string &value )
	{
		std::string out;
		std::size_t i = 0;
		while ( i < value.size() )
		{
			const char c = value[i];
			if ( c == ' ' || c == '\t' || c == '(' || c == ')' || c == '[' || c == ']' )
			{
				out += c;
				++i;
				continue;
			}
			std::size_t end = i;
			while ( end < value.size() && value[end] != ' ' && value[end] != '\t' &&
			        value[end] != '(' && value[end] != ')' && value[end] != '[' &&
			        value[end] != ']' )
			{
				++end;
			}
			const std::string token = value.substr( i, end - i );
			char *stop = nullptr;
			const double number = std::strtod( token.c_str(), &stop );
			if ( stop && *stop == '\0' && std::isfinite( number ) )
			{
				char buffer[64];
				std::snprintf( buffer, sizeof( buffer ), "%.17g", number == 0.0 ? 0.0 : number );
				out += buffer;
			}
			else
			{
				out += token;
			}
			i = end;
		}
		return out;
	};
	for ( kvtext::KeyValue &kv : node.pairs )
	{
		kv.value = normalize( kv.value );
	}
	for ( kvtext::KeyValueNode &child : node.children )
	{
		NormalizeNumbers( child );
	}
}

inline Findings SemanticText(
    const IMapCodec &codec, std::string_view text, NumberText numbers = NumberText::Exact )
{
	Findings f;
	const std::optional<DecodedMap> decoded = DecodeOrReport( codec, text, 1, f, "decode" );
	if ( !decoded )
	{
		return f;
	}
	const std::optional<std::string> encoded =
	    EncodeOrReport( codec, decoded->document, f, "encode" );
	if ( !encoded )
	{
		return f;
	}
	kvtext::ParseResult original = kvtext::ParseKeyValues( std::string( text ) );
	kvtext::ParseResult written = kvtext::ParseKeyValues( *encoded );
	f.Expect( original.ok && written.ok, "both texts parse as keyvalues" );
	if ( numbers == NumberText::ByValue )
	{
		NormalizeNumbers( original.root );
		NormalizeNumbers( written.root );
	}
	const kvtext::CompareResult compare = kvtext::CompareKeyValues( original.root, written.root );
	f.Expect( compare.equal,
	    "encode(decode(text)) is semantically the text: " + compare.firstDivergence );
	return f;
}

inline Findings CanonicalText( const IMapCodec &codec, std::string_view text )
{
	Findings f = SemanticText( codec, text );
	const std::optional<DecodedMap> decoded = DecodeOrReport( codec, text, 1, f, "decode" );
	if ( !decoded )
	{
		return f;
	}
	const std::optional<std::string> encoded =
	    EncodeOrReport( codec, decoded->document, f, "encode" );
	if ( encoded && *encoded != text )
	{
		std::size_t at = 0;
		while ( at < encoded->size() && at < text.size() && ( *encoded )[at] == text[at] )
		{
			++at;
		}
		const std::size_t line = static_cast<std::size_t>(
		    std::count( text.begin(), text.begin() + static_cast<std::ptrdiff_t>( at ), '\n' ) +
		    1 );
		f.problems.push_back( "canonical text is not reproduced byte for byte (first difference "
		                      "at line " +
		                      std::to_string( line ) + ")" );
	}
	return f;
}

// --- Fixture writers -------------------------------------------------------------------------

class VmfWriter
{
public:
	explicit VmfWriter( int depth = 0 ) : m_depth( depth ) {}

	void Open( const std::string &name )
	{
		m_out += Tabs() + name + "\n" + Tabs() + "{\n";
		++m_depth;
	}
	void Close()
	{
		--m_depth;
		m_out += Tabs() + "}\n";
	}
	void Pair( const std::string &key, const std::string &value )
	{
		m_out += Tabs() + "\"" + key + "\" \"" + value + "\"\n";
	}
	const std::string &Text() const { return m_out; }

private:
	std::string Tabs() const { return std::string( static_cast<std::size_t>( m_depth ), '\t' ); }

	std::string m_out;
	int m_depth = 0;
};

// A box solid in the legacy layout: sides top, bottom, +x, -x, +y, -y.
struct BoxSpec
{
	int id = 0;                                                         // 0 = no "id" key
	int firstSide = 0;                                                  // 0 = sides without ids
	std::array<std::string, 6> c = { "0", "0", "0", "64", "64", "64" }; // x0 y0 z0 x1 y1 z1
	std::string material = "DEV/DEV_MEASUREGENERIC01B";
	int sideCount = 6;
	std::map<int, std::map<std::string, std::string>> overrides; // side -> key -> value
	std::function<void( VmfWriter &, int )> sideExtra;           // after a side's standard keys
	std::function<void( VmfWriter & )> tail;                     // after the sides (the editor)
};

inline void WriteBox( VmfWriter &w, const BoxSpec &b )
{
	const auto &[x0, y0, z0, x1, y1, z1] = b.c;
	auto pt = []( const std::string &x, const std::string &y, const std::string &z )
	{
		return "(" + x + " " + y + " " + z + ")";
	};
	const std::array<std::string, 6> planes = {
	    pt( x0, y1, z1 ) + " " + pt( x1, y1, z1 ) + " " + pt( x1, y0, z1 ),
	    pt( x0, y0, z0 ) + " " + pt( x1, y0, z0 ) + " " + pt( x1, y1, z0 ),
	    pt( x1, y0, z0 ) + " " + pt( x1, y0, z1 ) + " " + pt( x1, y1, z1 ),
	    pt( x0, y1, z0 ) + " " + pt( x0, y1, z1 ) + " " + pt( x0, y0, z1 ),
	    pt( x1, y1, z0 ) + " " + pt( x1, y1, z1 ) + " " + pt( x0, y1, z1 ),
	    pt( x0, y0, z0 ) + " " + pt( x0, y0, z1 ) + " " + pt( x1, y0, z1 ),
	};
	w.Open( "solid" );
	if ( b.id )
	{
		w.Pair( "id", std::to_string( b.id ) );
	}
	for ( int i = 0; i < b.sideCount; ++i )
	{
		std::map<std::string, std::string> keys = {
		    { "plane", planes[static_cast<std::size_t>( i )] },
		    { "material", b.material },
		    { "uaxis", "[1 0 0 0] 0.25" },
		    { "vaxis", "[0 -1 0 0] 0.25" },
		    { "rotation", "0" },
		    { "lightmapscale", "16" },
		    { "smoothing_groups", "0" },
		};
		if ( b.firstSide )
		{
			keys["id"] = std::to_string( b.firstSide + i );
		}
		const auto over = b.overrides.find( i );
		if ( over != b.overrides.end() )
		{
			for ( const auto &[key, value] : over->second )
			{
				keys[key] = value;
			}
		}
		w.Open( "side" );
		for ( const char *key : { "id", "plane", "material", "uaxis", "vaxis", "rotation",
		          "lightmapscale", "smoothing_groups" } )
		{
			if ( keys.count( key ) )
			{
				w.Pair( key, keys[key] );
			}
		}
		if ( b.sideExtra )
		{
			b.sideExtra( w, i );
		}
		w.Close();
	}
	if ( b.tail )
	{
		b.tail( w );
	}
	w.Close();
}

inline void WriteEditor(
    VmfWriter &w, const std::vector<std::pair<std::string, std::string>> &pairs )
{
	w.Open( "editor" );
	for ( const auto &[key, value] : pairs )
	{
		w.Pair( key, value );
	}
	w.Close();
}

// The editor block legacy writes for an object with a color and nothing else.
inline std::function<void( VmfWriter & )> PlainEditor( const std::string &color )
{
	return [color]( VmfWriter &w )
	{
		WriteEditor(
		    w, { { "color", color }, { "visgroupshown", "1" }, { "visgroupautoshown", "1" } } );
	};
}

inline void WriteHeader( VmfWriter &w, const std::string &mapVersion )
{
	w.Open( "versioninfo" );
	w.Pair( "editorversion", "400" );
	w.Pair( "editorbuild", "8864" );
	w.Pair( "mapversion", mapVersion );
	w.Pair( "formatversion", "100" );
	w.Pair( "prefab", "0" );
	w.Close();
}

inline void WriteViewSettings( VmfWriter &w, const std::string &grid )
{
	w.Open( "viewsettings" );
	w.Pair( "bSnapToGrid", "1" );
	w.Pair( "bShowGrid", "1" );
	w.Pair( "bShowLogicalGrid", "0" );
	w.Pair( "nGridSpacing", grid );
	w.Pair( "bShow3DGrid", "0" );
	w.Close();
}

// "v v v ... " with 'count' copies of 'v'.
inline std::string Repeat( const std::string &v, int count )
{
	std::string out;
	for ( int i = 0; i < count; ++i )
	{
		out += ( i ? " " : "" ) + v;
	}
	return out;
}

// Rows "row0".."rowN-1" of a displacement block.
inline void WriteRows( VmfWriter &w, const std::string &name, int rows, const std::string &row )
{
	w.Open( name );
	for ( int r = 0; r < rows; ++r )
	{
		w.Pair( "row" + std::to_string( r ), row );
	}
	w.Close();
}

// A power-2 displacement with every block legacy Hammer writes.
inline void WriteFullDisplacement( VmfWriter &w, const std::string &start )
{
	w.Open( "dispinfo" );
	w.Pair( "power", "2" );
	w.Pair( "startposition", start );
	w.Pair( "flags", "0" );
	w.Pair( "elevation", "0" );
	w.Pair( "subdiv", "0" );
	WriteRows( w, "normals", 5, Repeat( "0 0 1", 5 ) );
	WriteRows( w, "distances", 5, "0 1.5 3 1.5 0" );
	WriteRows( w, "offsets", 5, Repeat( "0 0 0", 5 ) );
	WriteRows( w, "offset_normals", 5, Repeat( "0 0 1", 5 ) );
	WriteRows( w, "alphas", 5, "0 64 128 64 255" );
	WriteRows( w, "triangle_tags", 4, "9 9 1 1 9 9 1 1" );
	w.Open( "allowed_verts" );
	w.Pair( "10", Repeat( "-1", 10 ) );
	w.Close();
	w.Close();
}

// Every VMF feature the model holds, in the writer's canonical layout.
inline std::string CanonicalVmf()
{
	VmfWriter w;
	WriteHeader( w, "3" );
	w.Open( "visgroups" );
	w.Open( "visgroup" );
	w.Pair( "name", "Walls" );
	w.Pair( "visgroupid", "2" );
	w.Pair( "color", "255 0 0" );
	w.Open( "visgroup" );
	w.Pair( "name", "North" );
	w.Pair( "visgroupid", "5" );
	w.Pair( "color", "0 128 255" );
	w.Close();
	w.Close();
	w.Open( "visgroup" );
	w.Pair( "name", "Lights" );
	w.Pair( "visgroupid", "3" );
	w.Close();
	w.Close();
	WriteViewSettings( w, "16" );

	w.Open( "world" );
	w.Pair( "id", "1" );
	w.Pair( "mapversion", "3" );
	w.Pair( "classname", "worldspawn" );
	w.Pair( "skyname", "sky_day01_01" );
	w.Pair( "maxpropscreenwidth", "-1" );
	{
		BoxSpec b;
		b.id = 10;
		b.firstSide = 11;
		b.overrides[0] = { { "uaxis", "[1 0 0 16] 0.25" },
		    { "vaxis", "[0 -0.7071067812 -0.7071067812 -8.5] 0.5" }, { "rotation", "15" },
		    { "lightmapscale", "32" }, { "smoothing_groups", "-2147483648" },
		    { "material", "TOOLS/TOOLSNODRAW" } };
		b.tail = []( VmfWriter &e )
		{
			WriteEditor( e, { { "color", "0 180 90" }, { "groupid", "40" }, { "visgroupid", "5" },
			                    { "visgroupshown", "1" }, { "visgroupautoshown", "1" } } );
		};
		WriteBox( w, b );
	}
	w.Open( "hidden" );
	{
		BoxSpec b;
		b.id = 20;
		b.firstSide = 21;
		b.c = { "-64.5", "-0.333333", "0", "-16", "32", "96.125" };
		b.sideExtra = []( VmfWriter &e, int side )
		{
			if ( side == 0 )
			{
				WriteFullDisplacement( e, "[-64.5 -0.333333 96.125]" );
			}
			if ( side == 1 )
			{
				// A power-3 displacement with the pre-release keys and only the
				// blocks old files carry.
				e.Open( "dispinfo" );
				e.Pair( "power", "3" );
				e.Pair( "startposition", "[0 0 0]" );
				e.Pair( "flags", "2" );
				e.Pair( "mintess", "-1" );
				e.Pair( "smooth", "0.5" );
				e.Pair( "alpha", "[0 255 0 255]" );
				e.Pair( "elevation", "8" );
				e.Pair( "subdiv", "1" );
				WriteRows( e, "normals", 9, Repeat( "0 0 1", 9 ) );
				WriteRows( e, "distances", 9, Repeat( "0.25", 9 ) );
				e.Close();
			}
		};
		b.tail = []( VmfWriter &e )
		{
			WriteEditor( e, { { "color", "220 30 220" }, { "visgroupshown", "0" },
			                    { "visgroupautoshown", "1" } } );
		};
		WriteBox( w, b );
	}
	w.Close();
	w.Open( "group" );
	w.Pair( "id", "40" );
	WriteEditor( w, { { "color", "242 195 0" }, { "groupid", "41" }, { "visgroupshown", "1" },
	                    { "visgroupautoshown", "1" } } );
	w.Close();
	w.Open( "group" );
	w.Pair( "id", "41" );
	PlainEditor( "10 20 30" )( w );
	w.Close();
	w.Open( "hidden" );
	w.Open( "group" );
	w.Pair( "id", "42" );
	WriteEditor( w, { { "visgroupshown", "1" }, { "visgroupautoshown", "1" } } );
	w.Close();
	w.Close();
	w.Close(); // world

	w.Open( "entity" );
	w.Pair( "id", "60" );
	w.Pair( "classname", "logic_relay" );
	w.Pair( "targetname", "relay" );
	w.Pair( "spawnflags", "0" );
	w.Pair( "spawnflags", "1" );
	w.Open( "connections" );
	w.Pair( "OnTrigger", "door,Open,,0,-1" );
	w.Pair( "OnTrigger", "lamp\x1bSetPattern\x1b"
	                     "a,b,c\x1b"
	                     "0.5\x1b"
	                     "1" );
	w.Close();
	WriteEditor(
	    w, { { "color", "220 30 220" }, { "groupid", "40" }, { "visgroupid", "3" },
	           { "visgroupid", "2" }, { "visgroupshown", "1" }, { "visgroupautoshown", "1" },
	           { "comments", "fires the door" }, { "logicalpos", "[0 500]" } } );
	w.Close();

	w.Open( "entity" );
	w.Pair( "id", "70" );
	w.Pair( "classname", "func_door" );
	w.Pair( "speed", "100" );
	w.Pair( "origin", "96 32 32" );
	{
		BoxSpec b;
		b.id = 71;
		b.firstSide = 72;
		b.c = { "64", "0", "0", "128", "64", "64" };
		b.tail = PlainEditor( "0 180 90" );
		WriteBox( w, b );
	}
	w.Open( "hidden" );
	{
		BoxSpec b;
		b.id = 78;
		b.firstSide = 79;
		b.c = { "128", "0", "0", "192", "64", "64" };
		b.tail = PlainEditor( "0 180 90" );
		WriteBox( w, b );
	}
	w.Close();
	PlainEditor( "220 30 220" )( w );
	w.Close();

	w.Open( "hidden" );
	w.Open( "entity" );
	w.Pair( "id", "90" );
	w.Pair( "classname", "info_player_start" );
	w.Pair( "angles", "0 90 0" );
	w.Pair( "origin", "32 32 72" );
	WriteEditor( w, { { "color", "0 255 0" }, { "visgroupshown", "1" },
	                    { "visgroupautoshown", "1" }, { "logicalpos", "[0 0]" } } );
	w.Close();
	w.Close();

	w.Open( "entity" );
	w.Pair( "id", "95" );
	w.Pair( "classname", "info_target" );
	w.Pair( "origin", "0 0 0" );
	WriteEditor( w, { { "visgroupshown", "1" }, { "visgroupautoshown", "0" } } );
	w.Close();

	w.Open( "cameras" );
	w.Pair( "activecamera", "1" );
	w.Open( "camera" );
	w.Pair( "position", "[0 -128 64]" );
	w.Pair( "look", "[0 0 32]" );
	w.Close();
	w.Open( "camera" );
	w.Pair( "position", "[256.5 12.25 -3]" );
	w.Pair( "look", "[0 0 0]" );
	w.Close();
	w.Close();

	w.Open( "cordons" );
	w.Pair( "active", "1" );
	w.Open( "cordon" );
	w.Pair( "name", "main" );
	w.Pair( "active", "1" );
	w.Open( "box" );
	w.Pair( "mins", "(-512 -512 -128)" );
	w.Pair( "maxs", "(512 512 256)" );
	w.Close();
	w.Open( "box" );
	w.Pair( "mins", "(600 0 0)" );
	w.Pair( "maxs", "(700 100 100)" );
	w.Close();
	w.Close();
	w.Open( "cordon" );
	w.Pair( "name", "off" );
	w.Pair( "active", "0" );
	w.Open( "box" );
	w.Pair( "mins", "(0 0 0)" );
	w.Pair( "maxs", "(1 1 1)" );
	w.Close();
	w.Close();
	w.Close();
	return w.Text();
}

// The older single-cordon form, in canonical layout.
inline std::string SingleCordonVmf()
{
	VmfWriter w;
	WriteHeader( w, "1" );
	w.Open( "visgroups" );
	w.Close();
	WriteViewSettings( w, "64" );
	w.Open( "world" );
	w.Pair( "id", "1" );
	w.Pair( "mapversion", "1" );
	w.Pair( "classname", "worldspawn" );
	w.Close();
	w.Open( "cameras" );
	w.Pair( "activecamera", "-1" );
	w.Close();
	w.Open( "cordon" );
	w.Pair( "mins", "(-1024 -1024 -1024)" );
	w.Pair( "maxs", "(1024 1024 1024.5)" );
	w.Pair( "active", "0" );
	w.Close();
	return w.Text();
}

// Objects and sides without ids (early files): the one recoverable oddity.
inline std::string MissingIdsVmf()
{
	VmfWriter w;
	w.Open( "world" );
	w.Pair( "id", "1" );
	w.Pair( "classname", "worldspawn" );
	{
		BoxSpec b;
		b.id = 10;
		b.firstSide = 11;
		WriteBox( w, b );
	}
	{
		BoxSpec b; // no ids at all
		b.c = { "200", "0", "0", "264", "64", "64" };
		WriteBox( w, b );
	}
	w.Close();
	w.Open( "entity" );
	w.Pair( "classname", "info_target" );
	w.Pair( "origin", "1 2 3" );
	w.Close();
	return w.Text();
}

// A small complete map the rejection cases each break in one place.
inline std::string SmallVmf()
{
	VmfWriter w;
	WriteHeader( w, "1" );
	w.Open( "visgroups" );
	w.Open( "visgroup" );
	w.Pair( "name", "A" );
	w.Pair( "visgroupid", "1" );
	w.Pair( "color", "1 2 3" );
	w.Close();
	w.Close();
	WriteViewSettings( w, "64" );
	w.Open( "world" );
	w.Pair( "id", "1" );
	w.Pair( "mapversion", "1" );
	w.Pair( "classname", "worldspawn" );
	w.Pair( "skyname", "sky" );
	{
		BoxSpec b;
		b.id = 2;
		b.firstSide = 3;
		b.tail = PlainEditor( "1 1 1" );
		WriteBox( w, b );
	}
	{
		BoxSpec b;
		b.id = 9;
		b.firstSide = 10;
		b.c = { "100", "0", "0", "164", "64", "64" };
		b.sideExtra = []( VmfWriter &e, int side )
		{
			if ( side == 0 )
			{
				WriteFullDisplacement( e, "[100 0 64]" );
			}
		};
		b.tail = PlainEditor( "2 2 2" );
		WriteBox( w, b );
	}
	w.Open( "group" );
	w.Pair( "id", "20" );
	PlainEditor( "3 3 3" )( w );
	w.Close();
	w.Close();
	w.Open( "entity" );
	w.Pair( "id", "30" );
	w.Pair( "classname", "logic_relay" );
	w.Pair( "targetname", "r" );
	w.Open( "connections" );
	w.Pair( "OnTrigger", "a,b,,0,-1" );
	w.Close();
	WriteEditor( w, { { "color", "4 4 4" }, { "visgroupshown", "1" }, { "visgroupautoshown", "1" },
	                    { "logicalpos", "[0 0]" } } );
	w.Close();
	w.Open( "cameras" );
	w.Pair( "activecamera", "-1" );
	w.Open( "camera" );
	w.Pair( "position", "[0 0 0]" );
	w.Pair( "look", "[1 0 0]" );
	w.Close();
	w.Close();
	w.Open( "cordons" );
	w.Pair( "active", "0" );
	w.Open( "cordon" );
	w.Pair( "name", "c" );
	w.Pair( "active", "1" );
	w.Open( "box" );
	w.Pair( "mins", "(0 0 0)" );
	w.Pair( "maxs", "(1 1 1)" );
	w.Close();
	w.Close();
	w.Close();
	return w.Text();
}

// --- Text surgery for the rejection cases ---------------------------------------------------

// Offset of the 'occurrence'-th (1-based) 'needle', or npos.
inline std::size_t FindNth( const std::string &text, const std::string &needle, int occurrence )
{
	std::size_t at = std::string::npos;
	std::size_t from = 0;
	for ( int i = 0; i < occurrence; ++i )
	{
		at = text.find( needle, from );
		if ( at == std::string::npos )
		{
			return at;
		}
		from = at + 1;
	}
	return at;
}

// 1-based line of the 'occurrence'-th 'needle'; -1 when absent.
inline int LineOf( const std::string &text, const std::string &needle, int occurrence = 1 )
{
	const std::size_t at = FindNth( text, needle, occurrence );
	if ( at == std::string::npos )
	{
		return -1;
	}
	return static_cast<int>( std::count(
	           text.begin(), text.begin() + static_cast<std::ptrdiff_t>( at ), '\n' ) ) +
	       1;
}

// Inserts 'lines' after the line holding the 'occurrence'-th 'marker'.
inline std::string InsertAfter( const std::string &text, const std::string &marker,
    const std::string &lines, int occurrence = 1 )
{
	const std::size_t at = FindNth( text, marker, occurrence );
	const std::size_t eol = at == std::string::npos ? std::string::npos : text.find( '\n', at );
	if ( eol == std::string::npos )
	{
		return text + lines;
	}
	return text.substr( 0, eol + 1 ) + lines + text.substr( eol + 1 );
}

inline std::string ReplaceNth(
    const std::string &text, const std::string &from, const std::string &to, int occurrence = 1 )
{
	const std::size_t at = FindNth( text, from, occurrence );
	if ( at == std::string::npos )
	{
		return text;
	}
	return text.substr( 0, at ) + to + text.substr( at + from.size() );
}

// Replaces the whole line holding the 'occurrence'-th 'marker'.
inline std::string ReplaceLine( const std::string &text, const std::string &marker,
    const std::string &line, int occurrence = 1 )
{
	const std::size_t at = FindNth( text, marker, occurrence );
	if ( at == std::string::npos )
	{
		return text;
	}
	const std::size_t bol = text.rfind( '\n', at ) + 1;
	const std::size_t eol = text.find( '\n', at );
	return text.substr( 0, bol ) + line + text.substr( eol );
}

// One kind of content the model cannot hold, in an otherwise valid map. Decode
// must fail with a message containing 'expect' (block path and reason) at
// 'line'.
struct RejectionCase
{
	std::string name;
	std::string text;
	std::string expect;
	int line = 0;
};

inline std::vector<RejectionCase> RejectionCases()
{
	const std::string base = SmallVmf();
	std::vector<RejectionCase> cases;
	auto add =
	    [&]( const std::string &name, const std::string &text, const std::string &expect, int line )
	{
		cases.push_back( { name, text, expect, line } );
	};
	std::string t;

	t = base + "quickhide\n{\n\t\"count\" \"1\"\n}\n";
	add( "unknown top-level block", t, "quickhide[0]: unknown top-level block 'quickhide'",
	    LineOf( t, "quickhide" ) );
	t = "\"loose\" \"1\"\n" + base;
	add( "key outside any block", t, "key 'loose' is outside any block", 1 );
	t = base + "world\n{\n}\n";
	add( "second world block", t, "world: a second 'world' block", LineOf( t, "world\n{", 2 ) );
	t = InsertAfter( base, "\"prefab\" \"0\"", "\t\"autosave\" \"1\"\n" );
	add( "unknown versioninfo key", t, "versioninfo: unknown key 'autosave'",
	    LineOf( t, "autosave" ) );
	t = InsertAfter( base, "\"bShow3DGrid\" \"0\"", "\t\"nLogicalGridSpacing\" \"8\"\n" );
	add( "unknown viewsettings key", t, "viewsettings: unknown key 'nLogicalGridSpacing'",
	    LineOf( t, "nLogicalGridSpacing" ) );
	t = InsertAfter( base, "\"visgroupid\" \"1\"", "\t\t\"hidden\" \"1\"\n" );
	add( "unknown visgroup key", t, "visgroups/visgroup[0]: unknown key 'hidden'",
	    LineOf( t, "\"hidden\"" ) );
	t = InsertAfter( base, "\"skyname\" \"sky\"", "\tmystery\n\t{\n\t}\n" );
	add( "unknown world child", t, "world/mystery[0]: unknown block 'mystery'",
	    LineOf( t, "mystery" ) );
	t = InsertAfter( base, "\"id\" \"2\"", "\t\t\"note\" \"x\"\n" );
	add( "unknown solid key", t, "world/solid[0]: unknown key 'note'", LineOf( t, "\"note\"" ) );
	t = InsertAfter( base, "\"id\" \"4\"", "\t\t\t\"texturelock\" \"1\"\n" );
	add( "unknown side key", t, "world/solid[0]/side[1]: unknown key 'texturelock'",
	    LineOf( t, "texturelock" ) );
	t = InsertAfter( base, "\"id\" \"3\"", "\t\t\tvertices_plus\n\t\t\t{\n\t\t\t}\n" );
	add( "unknown side child", t,
	    "world/solid[0]/side[0]/vertices_plus: unknown block 'vertices_plus'",
	    LineOf( t, "vertices_plus" ) );
	t = InsertAfter(
	    base, "\"material\" \"DEV/DEV_MEASUREGENERIC01B\"", "\t\t\t\"material\" \"X\"\n" );
	add( "duplicate side key", t, "world/solid[0]/side[0]: duplicate key 'material'",
	    LineOf( t, "\"material\" \"X\"" ) );
	t = ReplaceLine(
	    base, "(0 0 0) (64 0 0) (64 64 0)", "\t\t\t\"plane\" \"(0 0 0) (1 1) (2 2 2)\"" );
	add( "malformed plane", t, "world/solid[0]/side[1]: key 'plane' value", LineOf( t, "(1 1)" ) );
	t = ReplaceNth( base, "\"id\" \"2\"", "\"id\" \"two\"" );
	add( "malformed id", t, "world/solid[0]: key 'id' value \"two\" is not an id",
	    LineOf( t, "\"two\"" ) );
	t = InsertAfter( base, "\"color\" \"1 1 1\"", "\t\t\t\"cordonsolid\" \"1\"\n" );
	add( "unknown editor key", t, "world/solid[0]/editor: unknown key 'cordonsolid'",
	    LineOf( t, "cordonsolid" ) );
	t = InsertAfter( base, "\"color\" \"1 1 1\"", "\t\t\t\"groupid\" \"999\"\n" );
	add( "groupid naming no group", t, "world/solid[0]/editor: groupid 999 names no group",
	    LineOf( t, "\"999\"" ) );
	t = ReplaceNth( base, "\"logicalpos\" \"[0 0]\"", "\"logicalpos\" \"[1.5 0]\"" );
	add( "fractional logicalpos", t, "entity[0]/editor: key 'logicalpos' value",
	    LineOf( t, "[1.5 0]" ) );
	t = InsertAfter( base, "\"power\" \"2\"", "\t\t\t\t\"uaxis\" \"[1 0 0]\"\n" );
	add( "unknown dispinfo key", t, "world/solid[1]/side[0]/dispinfo: unknown key 'uaxis'",
	    LineOf( t, "\"uaxis\" \"[1 0 0]\"" ) );
	t = ReplaceLine( base, "\"row4\" \"0 0 1", "", 1 );
	t = ReplaceNth( t, "\n\n", "\n" );
	add( "missing displacement row", t,
	    "world/solid[1]/side[0]/dispinfo/normals: has 4 rows; the power needs 5",
	    LineOf( t, "normals" ) );
	t = ReplaceNth( base, "\"row0\" \"9 9 1 1 9 9 1 1\"", "\"row0\" \"9 9\"" );
	add( "short triangle-tag row", t, "dispinfo/triangle_tags: row0 needs 8 numbers, found 2",
	    LineOf( t, "\"9 9\"" ) );
	t = ReplaceNth( base, "\"10\" \"-1", "\"9\" \"-1" );
	add( "allowed_verts key mismatch", t,
	    "dispinfo/allowed_verts: key '9' is not the word count 10", LineOf( t, "\"9\" \"-1" ) );
	t = InsertAfter( base, "\"id\" \"30\"", "\tblob\n\t{\n\t}\n" );
	add(
	    "unknown entity child", t, "entity[0]/blob[0]: unknown block 'blob'", LineOf( t, "blob" ) );
	t = ReplaceNth( base, "\"a,b,,0,-1\"", "\"garbage\"" );
	add( "unparsable connection", t, "entity[0]/connections[0]: connection 'OnTrigger' \"garbage\"",
	    LineOf( t, "garbage" ) );
	t = ReplaceNth( base, "\t\"classname\" \"logic_relay\"\n", "" );
	add( "entity without classname", t, "entity[0]: an entity needs a classname",
	    LineOf( t, "entity" ) );
	t = ReplaceNth( base, "\"classname\" \"worldspawn\"", "\"classname\" \"func_detail\"" );
	add( "world classname", t, "world: key 'classname' value \"func_detail\" is not worldspawn",
	    LineOf( t, "func_detail" ) );
	t = InsertAfter( base, "\"id\" \"20\"", "\t\t\"name\" \"g\"\n" );
	add( "unknown group key", t, "world/group[0]: unknown key 'name'",
	    LineOf( t, "\"name\" \"g\"" ) );
	t = InsertAfter( base, "\"color\" \"3 3 3\"", "\t\t\t\"groupid\" \"20\"\n" );
	add( "group containing itself", t, "world/group[0]: group 20 contains itself",
	    LineOf( t, "\tgroup" ) );
	t = ReplaceNth( base, "\"position\" \"[0 0 0]\"", "\"position\" \"[0 0]\"" );
	add( "malformed camera vector", t, "cameras/camera[0]: key 'position' value",
	    LineOf( t, "\"position\" \"[0 0]\"" ) );
	t = InsertAfter( base, "\"maxs\" \"(1 1 1)\"", "\t\t\t\"inset\" \"1\"\n" );
	add( "unknown cordon box key", t, "cordons/cordon[0]/box[0]: unknown key 'inset'",
	    LineOf( t, "inset" ) );
	t = ReplaceNth( base, "\"id\" \"15\"", "\"id\" \"3\"" );
	add( "duplicate side id", t, "world/solid[1]/side[5]: side id 3 is already used",
	    LineOf( t, "\"id\" \"3\"", 2 ) - 2 );
	t = ReplaceNth( base, "\"id\" \"30\"", "\"id\" \"9\"" );
	add( "duplicate object id", t, "entity[0]: entity id 9 is already used",
	    LineOf( t, "\"id\" \"9\"", 2 ) - 2 );
	t = base + "hidden\n{\n\tsolid\n\t{\n\t}\n}\n";
	add( "hidden non-entity at the top level", t, "hidden[0]/solid[0]: unknown block 'solid'",
	    LineOf( t, "hidden\n{\n\tsolid" ) + 2 );
	{
		// A solid with three sides.
		VmfWriter w( 1 );
		BoxSpec b;
		b.id = 90;
		b.firstSide = 91;
		b.sideCount = 3;
		WriteBox( w, b );
		t = InsertAfter( base, "\"skyname\" \"sky\"", w.Text() );
		add( "three-sided solid", t, "world/solid[0]: a solid needs at least four sides; it has 3",
		    LineOf( t, "\"id\" \"90\"" ) - 2 );
	}
	return cases;
}

// --- Fixture predicates ------------------------------------------------------------------

// Element-wise equality with a braced list (no vector temporary).
template <typename T>
bool Same( const std::vector<T> &actual, std::type_identity_t<std::initializer_list<T>> expected )
{
	return std::equal( actual.begin(), actual.end(), expected.begin(), expected.end() );
}

template <typename T> const T *ByVmfId( const std::map<ObjectId, T> &objects, std::uint32_t vmfId )
{
	for ( const auto &entry : objects )
	{
		if ( entry.second.vmfId == vmfId )
		{
			return &entry.second;
		}
	}
	return nullptr;
}

inline Findings CanonicalFixture( const IMapCodec &codec )
{
	Findings f;
	const std::string text = CanonicalVmf();
	const std::optional<DecodedMap> decoded = DecodeOrReport( codec, text, 5, f, "canonical" );
	if ( !decoded )
	{
		return f;
	}
	const MapDocument &doc = decoded->document;
	const DocumentSettings &s = doc.Settings();
	f.Expect( doc.Validate().empty(), "canonical fixture is consistent" );
	f.Expect( decoded->warnings.empty(), "canonical fixture decodes without warnings" );

	// Document settings.
	const hammer::scene::VersionInfo version{ 400, 8864, 3, 100, false };
	f.Expect( s.version == version, "versioninfo fields" );
	const hammer::scene::ViewSettings view{ true, true, false, 16, false };
	f.Expect( s.view == view, "viewsettings fields" );
	f.Expect( s.visgroups.size() == 2 && s.visgroups[0].name == "Walls" && s.visgroups[0].id == 2 &&
	              s.visgroups[0].color == hammer::scene::Rgb{ 255, 0, 0 } &&
	              s.visgroups[0].children.size() == 1 && s.visgroups[0].children[0].id == 5 &&
	              s.visgroups[0].children[0].name == "North" &&
	              s.visgroups[0].children[0].color == hammer::scene::Rgb{ 0, 128, 255 } &&
	              s.visgroups[1].name == "Lights" && s.visgroups[1].id == 3 &&
	              !s.visgroups[1].color,
	    "nested visgroup tree with colors" );
	f.Expect( s.worldVmfId == 1, "world id" );
	f.Expect( Same( s.worldKeys, { { "mapversion", "3" }, { "skyname", "sky_day01_01" },
	                                 { "maxpropscreenwidth", "-1" } } ),
	    "worldspawn keys in order, without id and classname" );
	f.Expect( s.activeCamera == 1 && s.cameras.size() == 2 &&
	              s.cameras[0].position == Vec3d( 0, -128, 64 ) &&
	              s.cameras[1].position == Vec3d( 256.5, 12.25, -3 ) &&
	              s.cameras[1].look == Vec3d( 0, 0, 0 ),
	    "cameras and the active camera" );
	f.Expect( s.cordonForm == DocumentSettings::CordonForm::List && s.cordonsActive &&
	              s.cordons.size() == 2 && s.cordons[0].name == "main" && s.cordons[0].active &&
	              s.cordons[0].boxes.size() == 2 &&
	              s.cordons[0].boxes[0].mins == Vec3d( -512, -512, -128 ) &&
	              s.cordons[0].boxes[1].maxs == Vec3d( 700, 100, 100 ) &&
	              s.cordons[1].name == "off" && !s.cordons[1].active,
	    "cordon list with names, flags and boxes" );

	// Objects.
	f.Expect( doc.Solids().size() == 4 && doc.Entities().size() == 4 && doc.Groups().size() == 3,
	    "object counts (4 solids, 4 entities, 3 groups)" );
	const Solid *s10 = ByVmfId( doc.Solids(), 10 );
	const Solid *s20 = ByVmfId( doc.Solids(), 20 );
	const Solid *s71 = ByVmfId( doc.Solids(), 71 );
	const Solid *s78 = ByVmfId( doc.Solids(), 78 );
	const Entity *e60 = ByVmfId( doc.Entities(), 60 );
	const Entity *e70 = ByVmfId( doc.Entities(), 70 );
	const Entity *e90 = ByVmfId( doc.Entities(), 90 );
	const Entity *e95 = ByVmfId( doc.Entities(), 95 );
	const Group *g40 = ByVmfId( doc.Groups(), 40 );
	const Group *g41 = ByVmfId( doc.Groups(), 41 );
	const Group *g42 = ByVmfId( doc.Groups(), 42 );
	if ( !f.Expect( s10 && s20 && s71 && s78 && e60 && e70 && e90 && e95 && g40 && g41 && g42,
	         "every object keeps its persistent id" ) )
	{
		return f;
	}
	f.Expect( s10->sides.size() == 6 && s10->sides[0].vmfId == 11 && s10->sides[5].vmfId == 16,
	    "side ids in authored order" );
	if ( s10->sides.size() == 6 )
	{
		const hammer::scene::Side &top = s10->sides[0];
		f.Expect( top.points[0] == Vec3d( 0, 64, 64 ) && top.points[1] == Vec3d( 64, 64, 64 ) &&
		              top.points[2] == Vec3d( 64, 0, 64 ),
		    "authored plane points exactly" );
		f.Expect( top.texture.material == "TOOLS/TOOLSNODRAW", "side material" );
		f.Expect( top.texture.u.axis == Vec3d( 1, 0, 0 ) && top.texture.u.shift == 16 &&
		              top.texture.u.scale == 0.25,
		    "u axis, shift and scale" );
		f.Expect( top.texture.v.axis == Vec3d( 0, -0.7071067812, -0.7071067812 ) &&
		              top.texture.v.shift == -8.5 && top.texture.v.scale == 0.5,
		    "v axis kept as authored (not regenerated)" );
		f.Expect( top.texture.rotation == 15 && top.texture.lightmapScale == 32,
		    "rotation and lightmap scale" );
		f.Expect( top.texture.smoothingGroups == 0x80000000u, "smoothing groups (signed text)" );
		f.Expect( !top.displacement, "a flat side has no displacement" );
	}
	f.Expect( !s10->owner.IsValid() && !s10->hidden && s10->group == g40->id,
	    "world solid: no owner, visible, in group 40" );
	f.Expect( s10->editor.color == hammer::scene::Rgb{ 0, 180, 90 } &&
	              Same( s10->editor.visgroupIds, { 5 } ) && s10->editor.visgroupShown &&
	              !s10->editor.logicalPos && !s10->editor.comments,
	    "solid editor color and visgroup" );
	f.Expect( s20->hidden && !s20->owner.IsValid(), "hidden world solid" );
	f.Expect( !s20->editor.visgroupShown && s20->editor.visgroupAutoShown, "visgroup shown flags" );
	if ( f.Expect(
	         s20->sides.size() == 6 && s20->sides[0].displacement && s20->sides[1].displacement,
	         "two displaced sides" ) )
	{
		f.Expect( s20->sides[0].points[0] == Vec3d( -64.5, 32, 96.125 ) &&
		              s20->sides[0].points[2] == Vec3d( -16, -0.333333, 96.125 ),
		    "fractional plane points exactly" );
		const hammer::scene::Displacement &d = *s20->sides[0].displacement;
		f.Expect( d.power == 2 && d.startPosition == Vec3d( -64.5, -0.333333, 96.125 ) &&
		              d.flags == 0 && d.elevation == 0 && !d.subdivided && !d.minTess &&
		              !d.smoothingAngle && !d.cornerAlphas,
		    "displacement scalars" );
		f.Expect( d.normals && d.normals->size() == 25 && ( *d.normals )[24] == Vec3d( 0, 0, 1 ),
		    "25 displacement normals" );
		f.Expect( d.distances && d.distances->size() == 25 && ( *d.distances )[1] == 1.5 &&
		              ( *d.distances )[7] == 3,
		    "displacement distances row-major" );
		f.Expect( d.offsets && d.offsets->size() == 25 && d.offsetNormals &&
		              d.offsetNormals->size() == 25,
		    "displacement offsets and offset normals" );
		f.Expect(
		    d.alphas && d.alphas->size() == 25 && ( *d.alphas )[4] == 255, "displacement alphas" );
		f.Expect( d.triangleTags && d.triangleTags->size() == 32 && ( *d.triangleTags )[0] == 9 &&
		              ( *d.triangleTags )[2] == 1,
		    "two triangle tags per quad" );
		f.Expect( d.allowedVerts && d.allowedVerts->size() == 10 && ( *d.allowedVerts )[9] == -1,
		    "allowed verts words" );
		const hammer::scene::Displacement &old = *s20->sides[1].displacement;
		const std::array<double, 4> corners = { 0, 255, 0, 255 };
		f.Expect( old.power == 3 && old.flags == 2 && old.minTess == -1 &&
		              old.smoothingAngle == 0.5 && old.cornerAlphas == corners &&
		              old.elevation == 8 && old.subdivided,
		    "pre-release displacement keys" );
		f.Expect( old.normals && old.normals->size() == 81 && old.distances &&
		              old.distances->size() == 81 && ( *old.distances )[80] == 0.25,
		    "power 3 rows hold 9 x 9 values" );
		f.Expect( !old.offsets && !old.offsetNormals && !old.alphas && !old.triangleTags &&
		              !old.allowedVerts,
		    "blocks the file omits stay absent" );
	}
	f.Expect( s71->owner == e70->id && !s71->hidden && s78->owner == e70->id && s78->hidden,
	    "brush entity owns its solids (one hidden)" );
	f.Expect( g40->group == g41->id && !g41->group.IsValid() &&
	              g40->editor.color == hammer::scene::Rgb{ 242, 195, 0 },
	    "nested groups" );
	f.Expect( g42->hidden && !g40->hidden && !g42->editor.color, "hidden group without a color" );
	const std::vector<kvtext::KeyValue> relayKeys = {
	    { "targetname", "relay" }, { "spawnflags", "0" }, { "spawnflags", "1" } };
	f.Expect( e60->classname == "logic_relay" && e60->keys == relayKeys,
	    "entity keys in order with duplicates" );
	f.Expect( e60->connections.size() == 2, "both connections" );
	if ( e60->connections.size() == 2 )
	{
		const hammer::scene::Connection &a = e60->connections[0];
		const hammer::scene::Connection &b = e60->connections[1];
		f.Expect( a.output == "OnTrigger" && a.target == "door" && a.input == "Open" &&
		              a.parameter.empty() && a.delay == 0 && a.timesToFire == -1 &&
		              a.separator == ',',
		    "comma connection" );
		f.Expect( b.target == "lamp" && b.input == "SetPattern" && b.parameter == "a,b,c" &&
		              b.delay == 0.5 && b.timesToFire == 1 && b.separator == '\x1b',
		    "0x1B connection with commas in its parameter" );
	}
	f.Expect( e60->group == g40->id && Same( e60->editor.visgroupIds, { 3, 2 } ),
	    "entity group and visgroups" );
	const std::array<int, 2> logical = { 0, 500 };
	f.Expect( e60->editor.logicalPos == logical &&
	              e60->editor.comments == std::string( "fires the door" ),
	    "logical position and comments" );
	f.Expect(
	    hammer::scene::EntitySolids( doc, e70->id ).size() == 2, "brush entity has two solids" );
	f.Expect( e90->hidden && !e60->hidden && !e70->hidden, "hidden point entity" );
	f.Expect( !e95->editor.color && e95->editor.visgroupShown && !e95->editor.visgroupAutoShown,
	    "an editor block without a color" );
	return f;
}

inline Findings SingleCordonFixture( const IMapCodec &codec )
{
	Findings f = CanonicalText( codec, SingleCordonVmf() );
	const std::optional<DecodedMap> decoded =
	    DecodeOrReport( codec, SingleCordonVmf(), 2, f, "single cordon" );
	if ( !decoded )
	{
		return f;
	}
	const DocumentSettings &s = decoded->document.Settings();
	f.Expect( s.cordonForm == DocumentSettings::CordonForm::Single && s.cordons.size() == 1 &&
	              s.cordons[0].boxes.size() == 1 &&
	              s.cordons[0].boxes[0].maxs == Vec3d( 1024, 1024, 1024.5 ) && !s.cordonsActive,
	    "single cordon form" );
	return f;
}

// Missing ids are the only lossless oddity: fresh unique ids and one warning.
inline Findings MissingIdsFixture( const IMapCodec &codec )
{
	Findings f;
	const std::string text = MissingIdsVmf();
	const std::optional<DecodedMap> decoded = DecodeOrReport( codec, text, 3, f, "missing ids" );
	if ( !decoded )
	{
		return f;
	}
	const MapDocument &doc = decoded->document;
	f.Expect( doc.Validate().empty(), "missing-id fixture decodes to a consistent document" );
	f.Expect( decoded->warnings.size() == 1, "one warning names the fresh ids" );
	// The file holds ids 1 and 10..16; everything else is fresh.
	std::vector<std::uint32_t> all;
	for ( const auto &[id, solid] : doc.Solids() )
	{
		all.push_back( solid.vmfId );
		for ( const hammer::scene::Side &side : solid.sides )
		{
			all.push_back( side.vmfId );
		}
	}
	for ( const auto &[id, entity] : doc.Entities() )
	{
		all.push_back( entity.vmfId );
	}
	const std::set<std::uint32_t> ids( all.begin(), all.end() );
	bool fresh = ids.size() == all.size() && all.size() == 15;
	for ( const std::uint32_t id : ids )
	{
		fresh = fresh && ( ( id >= 10 && id <= 16 ) || id > 16 );
	}
	f.Expect( fresh && *ids.rbegin() > 16, "fresh ids are unique and above every file id" );
	const Solid *kept = ByVmfId( doc.Solids(), 10 );
	f.Expect( kept && kept->sides[0].vmfId == 11, "ids the file has are kept" );
	Findings stable = RoundTrip( codec, text, 3 );
	for ( const std::string &p : stable.problems )
	{
		f.problems.push_back( "missing-id round trip: " + p );
	}
	return f;
}

// Every kind of content the model cannot hold is rejected with its block path
// and source line; nothing is dropped silently.
inline Findings RejectsUnmodeled( const IMapCodec &codec )
{
	Findings f;
	f.Expect(
	    codec.Decode( SmallVmf(), 1 ).HasValue(), "the base map of the rejection cases decodes" );
	for ( const RejectionCase &c : RejectionCases() )
	{
		const auto result = codec.Decode( c.text, 1 );
		if ( result )
		{
			f.problems.push_back( c.name + ": accepted content the model cannot hold" );
			continue;
		}
		const hammer::ports::CodecError &e = result.Error();
		f.Expect( e.message.find( c.expect ) != std::string::npos,
		    c.name + ": message \"" + e.message + "\" lacks \"" + c.expect + "\"" );
		f.Expect( e.line == c.line, c.name + ": line " + std::to_string( e.line ) + ", expected " +
		                                std::to_string( c.line ) );
	}
	return f;
}

// --- File-level semantics (for maps from other writers) ------------------------------------

// Structural counts of a map, from its text or from a decoded document.
struct ContentCounts
{
	std::size_t solids = 0;
	std::size_t sides = 0;
	std::size_t displacements = 0;
	std::size_t entities = 0;
	std::size_t connections = 0;
	std::size_t groups = 0;
	std::size_t visgroups = 0;
	std::size_t hidden = 0;
	std::size_t cameras = 0;
	std::size_t cordons = 0;

	friend bool operator==( const ContentCounts &, const ContentCounts & ) = default;

	std::string Describe() const
	{
		return "solids " + std::to_string( solids ) + ", sides " + std::to_string( sides ) +
		       ", displacements " + std::to_string( displacements ) + ", entities " +
		       std::to_string( entities ) + ", connections " + std::to_string( connections ) +
		       ", groups " + std::to_string( groups ) + ", visgroups " +
		       std::to_string( visgroups ) + ", hidden " + std::to_string( hidden ) + ", cameras " +
		       std::to_string( cameras ) + ", cordons " + std::to_string( cordons );
	}
};

inline void CountBlocks( const kvtext::KeyValueNode &node, ContentCounts &c )
{
	for ( const kvtext::KeyValueNode &child : node.children )
	{
		const std::string &n = child.name;
		c.solids += n == "solid";
		c.sides += n == "side";
		c.displacements += n == "dispinfo";
		c.entities += n == "entity";
		c.connections += n == "connections" ? child.pairs.size() : 0;
		c.groups += n == "group";
		c.visgroups += n == "visgroup";
		c.hidden += n == "hidden" ? child.children.size() : 0;
		c.cameras += n == "camera";
		if ( n == "cordons" )
		{
			c.cordons += child.children.size();
			continue;
		}
		c.cordons += n == "cordon";
		CountBlocks( child, c );
	}
}

inline ContentCounts CountText( const kvtext::KeyValueNode &root )
{
	ContentCounts c;
	CountBlocks( root, c );
	return c;
}

inline std::size_t CountVisgroups( const std::vector<hammer::scene::Visgroup> &tree )
{
	std::size_t n = 0;
	for ( const hammer::scene::Visgroup &v : tree )
	{
		n += 1 + CountVisgroups( v.children );
	}
	return n;
}

inline ContentCounts CountDocument( const MapDocument &doc )
{
	ContentCounts c;
	c.solids = doc.Solids().size();
	c.entities = doc.Entities().size();
	c.groups = doc.Groups().size();
	for ( const auto &[id, solid] : doc.Solids() )
	{
		c.sides += solid.sides.size();
		c.hidden += solid.hidden;
		for ( const hammer::scene::Side &side : solid.sides )
		{
			c.displacements += side.displacement.has_value();
		}
	}
	for ( const auto &[id, entity] : doc.Entities() )
	{
		c.connections += entity.connections.size();
		c.hidden += entity.hidden;
	}
	for ( const auto &[id, group] : doc.Groups() )
	{
		c.hidden += group.hidden;
	}
	c.visgroups = CountVisgroups( doc.Settings().visgroups );
	c.cameras = doc.Settings().cameras.size();
	c.cordons = doc.Settings().cordons.size();
	return c;
}

// Compares two parsed maps: block names and order, key/value multisets, with
// every numeric token compared by value (legacy MSVC writes "1.52588e-005").
inline kvtext::CompareResult CompareFileSemantics(
    kvtext::KeyValueNode original, kvtext::KeyValueNode written )
{
	NormalizeNumbers( original );
	NormalizeNumbers( written );
	return kvtext::CompareKeyValues( original, written );
}

} // namespace map_codec_oracle

#endif // HAMMERTEST_FORMATS_MAP_CODEC_ORACLE_H
