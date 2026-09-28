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
//			  OddityFixture      recoverable oddities decode, warn, and keep
//			                     their content (verbatim where unmodeled).
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
	std::function<void( VmfWriter & )> tail;                     // after the sides
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

// Every well-formed VMF feature, in the writer's canonical layout.
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
	w.Open( "viewsettings" );
	w.Pair( "bSnapToGrid", "1" );
	w.Pair( "nGridSpacing", "16" );
	w.Close();

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
			if ( side == 1 )
			{
				e.Pair( "texturelock", "1" ); // not modeled: kept as an extra pair
			}
			if ( side == 0 )
			{
				e.Open( "dispinfo" );
				e.Pair( "power", "2" );
				e.Pair( "startposition", "[-64.5 -0.333333 96.125]" );
				e.Pair( "elevation", "0" );
				e.Pair( "subdiv", "0" );
				e.Open( "normals" );
				for ( int row = 0; row < 5; ++row )
				{
					e.Pair( "row" + std::to_string( row ), "0 0 1 0 0 1 0 0 1 0 0 1 0 0 1" );
				}
				e.Close();
				e.Open( "distances" );
				for ( int row = 0; row < 5; ++row )
				{
					e.Pair( "row" + std::to_string( row ), "0 1.5 3 1.5 0" );
				}
				e.Close();
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
	WriteEditor(
	    w, { { "color", "10 20 30" }, { "visgroupshown", "1" }, { "visgroupautoshown", "1" } } );
	w.Close();
	w.Open( "hidden" );
	w.Open( "group" );
	w.Pair( "id", "42" );
	w.Close();
	w.Close();
	w.Open( "mystery" );
	w.Pair( "keep", "me" );
	w.Open( "nested" );
	w.Pair( "a", "b" );
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
	           { "logicalpos", "[0 500]" }, { "comments", "fires the door" } } );
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
		b.sideExtra = []( VmfWriter &e, int side )
		{
			if ( side == 0 )
			{
				e.Open( "vertices_plus" );
				e.Pair( "v", "64 64 64" );
				e.Close();
			}
		};
		WriteBox( w, b );
	}
	w.Open( "hidden" );
	{
		BoxSpec b;
		b.id = 78;
		b.firstSide = 79;
		b.c = { "128", "0", "0", "192", "64", "64" };
		WriteBox( w, b );
	}
	w.Close();
	WriteEditor(
	    w, { { "color", "220 30 220" }, { "visgroupshown", "1" }, { "visgroupautoshown", "1" } } );
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
	w.Open( "blob" );
	w.Pair( "x", "y" );
	w.Close();
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

	w.Open( "quickhide" );
	w.Pair( "count", "0" );
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
	w.Open( "viewsettings" );
	w.Pair( "bSnapToGrid", "1" );
	w.Close();
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

// Recoverable oddities. Each must decode, warn, and keep its content.
inline std::string OddityVmf()
{
	VmfWriter w;
	WriteHeader( w, "1" );
	w.Open( "world" );
	w.Pair( "id", "1" );
	w.Pair( "classname", "worldspawn" );
	{
		BoxSpec b; // a good solid
		b.id = 10;
		b.firstSide = 11;
		WriteBox( w, b );
	}
	{
		BoxSpec b; // three sides
		b.id = 30;
		b.firstSide = 31;
		b.sideCount = 3;
		WriteBox( w, b );
	}
	{
		BoxSpec b; // a malformed plane
		b.id = 34;
		b.firstSide = 140;
		b.overrides[2] = { { "plane", "(0 0 0) (1 1) (2 2 2)" } };
		WriteBox( w, b );
	}
	w.Open( "hidden" );
	{
		BoxSpec b; // three sides, hidden
		b.id = 36;
		b.firstSide = 150;
		b.sideCount = 3;
		WriteBox( w, b );
	}
	w.Close();
	{
		BoxSpec b; // reuses side id 11
		b.id = 50;
		b.firstSide = 51;
		b.overrides[5] = { { "id", "11" } };
		WriteBox( w, b );
	}
	{
		BoxSpec b; // no ids at all
		b.c = { "200", "0", "0", "264", "64", "64" };
		WriteBox( w, b );
	}
	{
		BoxSpec b; // a missing group
		b.id = 57;
		b.firstSide = 160;
		b.tail = []( VmfWriter &e )
		{
			WriteEditor( e, { { "groupid", "999" } } );
		};
		WriteBox( w, b );
	}
	w.Close();
	w.Open( "entity" );
	w.Pair( "id", "60" );
	w.Pair( "classname", "logic_relay" );
	w.Open( "connections" );
	w.Pair( "OnTrigger", "a,b,c,0,-1" );
	w.Pair( "OnSpawn", "garbage" );
	w.Pair( "OnUser1", "x,y,z,notanumber,1" );
	w.Close();
	w.Close();
	w.Open( "entity" );
	w.Pair( "id", "bad" );
	w.Pair( "classname", "info_target" );
	w.Close();
	w.Open( "cordons" );
	w.Pair( "active", "0" );
	w.Close();
	w.Open( "cordon" );
	w.Pair( "mins", "(0 0 0)" );
	w.Pair( "maxs", "(1 1 1)" );
	w.Pair( "active", "1" );
	w.Close();
	return w.Text();
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

// Finds a block named 'name' with "id" 'id' anywhere below 'node'.
inline const kvtext::KeyValueNode *FindBlock(
    const kvtext::KeyValueNode &node, const std::string &name, const std::string &id )
{
	for ( const kvtext::KeyValueNode &child : node.children )
	{
		const std::string *value = child.Find( "id" );
		if ( child.name == name && value && *value == id )
		{
			return &child;
		}
		if ( const kvtext::KeyValueNode *inner = FindBlock( child, name, id ) )
		{
			return inner;
		}
	}
	return nullptr;
}

inline bool HasPair(
    const kvtext::KeyValueNode &node, const std::string &key, const std::string &value )
{
	for ( const kvtext::KeyValue &kv : node.pairs )
	{
		if ( kv.key == key && kv.value == value )
		{
			return true;
		}
	}
	for ( const kvtext::KeyValueNode &child : node.children )
	{
		if ( HasPair( child, key, value ) )
		{
			return true;
		}
	}
	return false;
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
	f.Expect(
	    s.versionInfo.size() == 5 && s.versionInfo[2] == kvtext::KeyValue{ "mapversion", "3" },
	    "versioninfo pairs in order" );
	f.Expect( Same( s.viewSettings, { { "bSnapToGrid", "1" }, { "nGridSpacing", "16" } } ),
	    "viewsettings pairs in order" );
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
	f.Expect( s.worldExtraChildren.size() == 1 && s.worldExtraChildren[0].name == "mystery" &&
	              s.worldExtraChildren[0].children.size() == 1 &&
	              s.worldExtraChildren[0].children[0].name == "nested",
	    "unknown world child kept verbatim" );
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
	const std::vector<kvtext::KeyValue> quickhide = { { "count", "0" } };
	f.Expect( s.unknownBlocks.size() == 1 && s.unknownBlocks[0].name == "quickhide" &&
	              s.unknownBlocks[0].pairs == quickhide,
	    "unknown top-level block kept verbatim" );

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
	}
	f.Expect( !s10->owner.IsValid() && !s10->hidden && s10->group == g40->id,
	    "world solid: no owner, visible, in group 40" );
	f.Expect( s10->editor.color == hammer::scene::Rgb{ 0, 180, 90 } &&
	              Same( s10->editor.visgroupIds, { 5 } ) && s10->editor.visgroupShown,
	    "solid editor color and visgroup" );
	f.Expect( s20->hidden && !s20->owner.IsValid(), "hidden world solid" );
	f.Expect( !s20->editor.visgroupShown && s20->editor.visgroupAutoShown, "visgroup shown flags" );
	if ( s20->sides.size() == 6 )
	{
		f.Expect( s20->sides[0].dispinfo && s20->sides[0].dispinfo->Find( "power" ) &&
		              *s20->sides[0].dispinfo->Find( "power" ) == "2" &&
		              s20->sides[0].dispinfo->children.size() == 2,
		    "displacement kept verbatim on its side" );
		f.Expect( s20->sides[0].points[0] == Vec3d( -64.5, 32, 96.125 ) &&
		              s20->sides[0].points[2] == Vec3d( -16, -0.333333, 96.125 ),
		    "fractional plane points exactly" );
		f.Expect(
		    Same( s20->sides[1].extraPairs, { { "texturelock", "1" } } ), "unknown side key kept" );
	}
	else
	{
		f.Expect( false, "hidden solid has six sides" );
	}
	f.Expect( s71->owner == e70->id && !s71->hidden && s78->owner == e70->id && s78->hidden,
	    "brush entity owns its solids (one hidden)" );
	f.Expect( !s71->sides.empty() && s71->sides[0].extraChildren.size() == 1 &&
	              s71->sides[0].extraChildren[0].name == "vertices_plus",
	    "unknown side child kept" );
	f.Expect( g40->group == g41->id && !g41->group.IsValid() &&
	              g40->editor.color == hammer::scene::Rgb{ 242, 195, 0 },
	    "nested groups" );
	f.Expect( g42->hidden && !g40->hidden, "hidden group" );
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
	f.Expect( Same( e60->editor.extra,
	              { { "logicalpos", "[0 500]" }, { "comments", "fires the door" } } ),
	    "editor extras in order" );
	f.Expect(
	    hammer::scene::EntitySolids( doc, e70->id ).size() == 2, "brush entity has two solids" );
	f.Expect( e90->hidden && !e60->hidden && !e70->hidden, "hidden point entity" );
	f.Expect( e95->extraChildren.size() == 1 && e95->extraChildren[0].name == "blob",
	    "unknown entity child kept" );
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

inline Findings OddityFixture( const IMapCodec &codec )
{
	Findings f;
	const std::string text = OddityVmf();
	const std::optional<DecodedMap> decoded = DecodeOrReport( codec, text, 3, f, "oddities" );
	if ( !decoded )
	{
		return f;
	}
	const MapDocument &doc = decoded->document;
	f.Expect( doc.Validate().empty(), "oddity fixture decodes to a consistent document" );
	// Solids 10, 50, 57 and the id-less one are modeled; 30, 34 and 36 are not.
	f.Expect( doc.Solids().size() == 4, "four modeled solids" );
	f.Expect( decoded->warnings.size() >= 9, "a warning per oddity (at least nine)" );
	const Solid *s10 = ByVmfId( doc.Solids(), 10 );
	const Solid *s50 = ByVmfId( doc.Solids(), 50 );
	const Solid *s57 = ByVmfId( doc.Solids(), 57 );
	f.Expect( s10 && s10->sides.size() == 6 && s10->sides[0].vmfId == 11,
	    "the first owner of a side id keeps it" );
	if ( s50 && s50->sides.size() == 6 )
	{
		f.Expect(
		    s50->sides[0].vmfId == 51 && s50->sides[4].vmfId == 55 && s50->sides[5].vmfId > 160,
		    "a duplicate side id is replaced by a fresh one above every file id" );
	}
	else
	{
		f.Expect( false, "solid 50 has six sides" );
	}
	f.Expect( s57 && !s57->group.IsValid(), "a missing group leaves the solid ungrouped" );
	std::set<std::uint32_t> ids;
	bool fresh = true;
	for ( const auto &[id, solid] : doc.Solids() )
	{
		fresh = fresh && solid.vmfId != 0;
		for ( const hammer::scene::Side &side : solid.sides )
		{
			fresh = fresh && side.vmfId != 0 && ids.insert( side.vmfId ).second;
		}
	}
	f.Expect( fresh, "id-less solids and sides get fresh unique ids" );
	const Entity *e60 = ByVmfId( doc.Entities(), 60 );
	f.Expect( e60 && e60->connections.size() == 1, "the parsable connection is modeled" );
	f.Expect( doc.Entities().size() == 2, "both entities decode" );
	bool freshEntity = false;
	for ( const auto &[id, entity] : doc.Entities() )
	{
		freshEntity = freshEntity || ( entity.classname == "info_target" && entity.vmfId > 160 );
	}
	f.Expect( freshEntity, "a malformed entity id is replaced by a fresh one" );
	f.Expect( doc.Settings().cordonForm == DocumentSettings::CordonForm::List,
	    "the first cordon block wins" );

	// Unmodeled content survives an encode.
	const std::optional<std::string> encoded = EncodeOrReport( codec, doc, f, "oddities encode" );
	if ( !encoded )
	{
		return f;
	}
	const kvtext::ParseResult out = kvtext::ParseKeyValues( *encoded );
	f.Expect( out.ok, "encoded oddities parse" );
	const kvtext::KeyValueNode *s30 = FindBlock( out.root, "solid", "30" );
	f.Expect( s30 && s30->children.size() == 3, "the three-sided solid is kept verbatim" );
	const kvtext::KeyValueNode *s34 = FindBlock( out.root, "solid", "34" );
	f.Expect( s34 && HasPair( *s34, "plane", "(0 0 0) (1 1) (2 2 2)" ),
	    "the solid with a malformed plane is kept verbatim" );
	bool hiddenKept = false;
	for ( const kvtext::KeyValueNode &top : out.root.children )
	{
		const kvtext::KeyValueNode *hidden = nullptr;
		for ( const kvtext::KeyValueNode &child : top.children )
		{
			if ( child.name == "hidden" && FindBlock( child, "solid", "36" ) )
			{
				hidden = &child;
			}
		}
		hiddenKept = hiddenKept || hidden;
	}
	f.Expect( hiddenKept, "a hidden unmodeled solid stays inside its hidden wrapper" );
	f.Expect( HasPair( out.root, "OnSpawn", "garbage" ) &&
	              HasPair( out.root, "OnUser1", "x,y,z,notanumber,1" ),
	    "unparsable connections are kept verbatim" );
	f.Expect( HasPair( out.root, "maxs", "(1 1 1)" ), "the second cordon block is kept verbatim" );

	Findings stable = RoundTrip( codec, text, 3 );
	for ( const std::string &p : stable.problems )
	{
		f.problems.push_back( "oddity round trip: " + p );
	}
	return f;
}

} // namespace map_codec_oracle

#endif // HAMMERTEST_FORMATS_MAP_CODEC_ORACLE_H
