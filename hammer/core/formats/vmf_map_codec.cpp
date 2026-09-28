//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/formats/vmf_map_codec.h: the
//			full-fidelity VMF codec. Key orders and value formats follow the
//			legacy writer (hammer/mapdoc.cpp, mapworld.cpp, mapentity.cpp,
//			mapsolid.cpp, mapface.cpp, mapclass.cpp, editgameclass.cpp,
//			mapgroup.cpp), so the output loads in legacy Hammer and vbsp.
//
//=============================================================================//

#include "hammer/formats/vmf_map_codec.h"

#include "hammer/scene/map_document.h"
#include "hammer/scene/map_objects.h"
#include "kvtext/keyvalues.h"
#include "mapgeometry/brush.h"

#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

namespace hammer::formats
{

using hammer::ports::CodecDiagnostic;
using hammer::ports::CodecError;
using hammer::ports::DecodedMap;
using hammer::scene::CameraBookmark;
using hammer::scene::Connection;
using hammer::scene::Cordon;
using hammer::scene::CordonBox;
using hammer::scene::DocumentSettings;
using hammer::scene::EditorInfo;
using hammer::scene::Entity;
using hammer::scene::Group;
using hammer::scene::MapDocument;
using hammer::scene::ObjectId;
using hammer::scene::Rgb;
using hammer::scene::Side;
using hammer::scene::Solid;
using hammer::scene::TextureAxis;
using hammer::scene::Visgroup;
using kvtext::KeyValue;
using kvtext::KeyValueNode;
using mapgeometry::Vec3d;

namespace
{

// --- Numbers ---------------------------------------------------------------------

bool IsBlank( char c )
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

std::string_view Trim( std::string_view text )
{
	while ( !text.empty() && IsBlank( text.front() ) )
	{
		text.remove_prefix( 1 );
	}
	while ( !text.empty() && IsBlank( text.back() ) )
	{
		text.remove_suffix( 1 );
	}
	return text;
}

// A whole, finite decimal number (locale independent). A leading '+' is accepted.
std::optional<double> ParseNumber( std::string_view text )
{
	text = Trim( text );
	if ( !text.empty() && text.front() == '+' )
	{
		text.remove_prefix( 1 );
	}
	if ( text.empty() )
	{
		return std::nullopt;
	}
	double value = 0.0;
	const auto [end, ec] = std::from_chars( text.data(), text.data() + text.size(), value );
	if ( ec != std::errc() || end != text.data() + text.size() || !std::isfinite( value ) )
	{
		return std::nullopt;
	}
	return value;
}

std::optional<std::int64_t> ParseInteger( std::string_view text )
{
	text = Trim( text );
	if ( !text.empty() && text.front() == '+' )
	{
		text.remove_prefix( 1 );
	}
	if ( text.empty() )
	{
		return std::nullopt;
	}
	std::int64_t value = 0;
	const auto [end, ec] = std::from_chars( text.data(), text.data() + text.size(), value );
	if ( ec != std::errc() || end != text.data() + text.size() )
	{
		return std::nullopt;
	}
	return value;
}

std::optional<std::uint32_t> ParseId( std::string_view text )
{
	const std::optional<std::int64_t> value = ParseInteger( text );
	if ( !value || *value < 0 || *value > std::numeric_limits<std::uint32_t>::max() )
	{
		return std::nullopt;
	}
	return static_cast<std::uint32_t>( *value );
}

std::optional<int> ParseInt( std::string_view text )
{
	const std::optional<std::int64_t> value = ParseInteger( text );
	if ( !value || *value < std::numeric_limits<int>::min() ||
	     *value > std::numeric_limits<int>::max() )
	{
		return std::nullopt;
	}
	return static_cast<int>( *value );
}

// A 32-bit mask written signed (legacy WriteKeyValueInt) or unsigned.
std::optional<std::uint32_t> ParseMask( std::string_view text )
{
	const std::optional<std::int64_t> value = ParseInteger( text );
	if ( !value || *value < std::numeric_limits<std::int32_t>::min() ||
	     *value > std::numeric_limits<std::uint32_t>::max() )
	{
		return std::nullopt;
	}
	return static_cast<std::uint32_t>( *value );
}

std::optional<bool> ParseFlag( std::string_view text )
{
	text = Trim( text );
	if ( text == "1" )
	{
		return true;
	}
	if ( text == "0" )
	{
		return false;
	}
	return std::nullopt;
}

// scene::FormatNumber when it reads back exactly, else the shortest exact text.
std::string ExactNumber( double value )
{
	std::string text = hammer::scene::FormatNumber( value );
	const std::optional<double> back = ParseNumber( text );
	if ( back && *back == value )
	{
		return text;
	}
	char buffer[64];
	const auto result = std::to_chars( buffer, buffer + sizeof( buffer ), value );
	return std::string( buffer, result.ptr );
}

std::string ExactVec3( const Vec3d &v )
{
	return ExactNumber( v.x ) + " " + ExactNumber( v.y ) + " " + ExactNumber( v.z );
}

// Splits on blanks.
std::vector<std::string_view> Words( std::string_view text )
{
	std::vector<std::string_view> words;
	std::size_t i = 0;
	while ( i < text.size() )
	{
		while ( i < text.size() && IsBlank( text[i] ) )
		{
			++i;
		}
		const std::size_t start = i;
		while ( i < text.size() && !IsBlank( text[i] ) )
		{
			++i;
		}
		if ( i > start )
		{
			words.push_back( text.substr( start, i - start ) );
		}
	}
	return words;
}

std::optional<Vec3d> ParseVec3( std::string_view text )
{
	const std::vector<std::string_view> words = Words( text );
	if ( words.size() != 3 )
	{
		return std::nullopt;
	}
	const std::optional<double> x = ParseNumber( words[0] );
	const std::optional<double> y = ParseNumber( words[1] );
	const std::optional<double> z = ParseNumber( words[2] );
	if ( !x || !y || !z )
	{
		return std::nullopt;
	}
	return Vec3d( *x, *y, *z );
}

// "<open>x y z<close>" (blanks around allowed).
std::optional<Vec3d> ParseEnclosedVec3( std::string_view text, char open, char close )
{
	text = Trim( text );
	if ( text.size() < 2 || text.front() != open || text.back() != close )
	{
		return std::nullopt;
	}
	return ParseVec3( text.substr( 1, text.size() - 2 ) );
}

// "(x y z) (x y z) (x y z)", nothing else.
std::optional<std::array<Vec3d, 3>> ParsePlane( std::string_view text )
{
	std::array<Vec3d, 3> points;
	std::size_t found = 0;
	std::size_t i = 0;
	while ( true )
	{
		while ( i < text.size() && IsBlank( text[i] ) )
		{
			++i;
		}
		if ( i == text.size() )
		{
			break;
		}
		if ( text[i] != '(' || found == 3 )
		{
			return std::nullopt;
		}
		const std::size_t close = text.find( ')', i );
		if ( close == std::string_view::npos )
		{
			return std::nullopt;
		}
		const std::optional<Vec3d> point = ParseVec3( text.substr( i + 1, close - i - 1 ) );
		if ( !point || text.substr( i + 1, close - i - 1 ).find( '(' ) != std::string_view::npos )
		{
			return std::nullopt;
		}
		points[found++] = *point;
		i = close + 1;
	}
	if ( found != 3 )
	{
		return std::nullopt;
	}
	return points;
}

// "[x y z shift] scale".
std::optional<TextureAxis> ParseAxis( std::string_view text )
{
	text = Trim( text );
	const std::size_t close = text.find( ']' );
	if ( text.empty() || text.front() != '[' || close == std::string_view::npos )
	{
		return std::nullopt;
	}
	const std::vector<std::string_view> inner = Words( text.substr( 1, close - 1 ) );
	const std::vector<std::string_view> after = Words( text.substr( close + 1 ) );
	if ( inner.size() != 4 || after.size() != 1 )
	{
		return std::nullopt;
	}
	double values[5];
	const std::string_view parts[5] = { inner[0], inner[1], inner[2], inner[3], after[0] };
	for ( int i = 0; i < 5; ++i )
	{
		const std::optional<double> v = ParseNumber( parts[i] );
		if ( !v )
		{
			return std::nullopt;
		}
		values[i] = *v;
	}
	TextureAxis axis;
	axis.axis = Vec3d( values[0], values[1], values[2] );
	axis.shift = values[3];
	axis.scale = values[4];
	return axis;
}

std::optional<Rgb> ParseRgb( std::string_view text )
{
	const std::vector<std::string_view> words = Words( text );
	if ( words.size() != 3 )
	{
		return std::nullopt;
	}
	const std::optional<int> r = ParseInt( words[0] );
	const std::optional<int> g = ParseInt( words[1] );
	const std::optional<int> b = ParseInt( words[2] );
	if ( !r || !g || !b )
	{
		return std::nullopt;
	}
	return Rgb{ *r, *g, *b };
}

std::string FormatRgb( const Rgb &c )
{
	return std::to_string( c.r ) + " " + std::to_string( c.g ) + " " + std::to_string( c.b );
}

std::string FormatPlane( const std::array<Vec3d, 3> &p )
{
	return "(" + ExactVec3( p[0] ) + ") (" + ExactVec3( p[1] ) + ") (" + ExactVec3( p[2] ) + ")";
}

std::string FormatAxis( const TextureAxis &a )
{
	return "[" + ExactVec3( a.axis ) + " " + ExactNumber( a.shift ) + "] " + ExactNumber( a.scale );
}

// --- Source lines of blocks -------------------------------------------------------

// Maps each parsed block to the line of its name. The kvtext tree carries no
// lines, so this re-scans the text with the tokenizer's rules: blocks appear in
// the text in the same (pre)order as in the tree.
class BlockLines
{
public:
	BlockLines( const std::string &text, const KeyValueNode &root )
	{
		std::vector<int> lines;
		int line = 1;
		int lastStringLine = 1;
		std::size_t i = 0;
		while ( i < text.size() )
		{
			const char c = text[i];
			if ( c == '\n' )
			{
				++line;
				++i;
			}
			else if ( c == ' ' || c == '\t' || c == '\r' )
			{
				++i;
			}
			else if ( c == '/' && i + 1 < text.size() && text[i + 1] == '/' )
			{
				while ( i < text.size() && text[i] != '\n' )
				{
					++i;
				}
			}
			else if ( c == '{' )
			{
				lines.push_back( lastStringLine );
				++i;
			}
			else if ( c == '}' )
			{
				++i;
			}
			else if ( c == '"' )
			{
				lastStringLine = line;
				++i;
				while ( i < text.size() && text[i] != '"' )
				{
					line += text[i] == '\n' ? 1 : 0;
					++i;
				}
				++i;
			}
			else
			{
				lastStringLine = line;
				while ( i < text.size() && !IsBlank( text[i] ) && text[i] != '{' &&
				        text[i] != '}' && text[i] != '"' )
				{
					++i;
				}
			}
		}
		std::size_t next = 0;
		Assign( root, lines, next );
	}

	int Line( const KeyValueNode &node ) const
	{
		const auto it = m_lines.find( &node );
		return it == m_lines.end() ? 0 : it->second;
	}

private:
	void Assign( const KeyValueNode &node, const std::vector<int> &lines, std::size_t &next )
	{
		for ( const KeyValueNode &child : node.children )
		{
			m_lines[&child] = next < lines.size() ? lines[next] : 0;
			++next;
			Assign( child, lines, next );
		}
	}

	std::unordered_map<const KeyValueNode *, int> m_lines;
};

// --- Decoding ------------------------------------------------------------------------

struct GroupRef
{
	std::uint32_t id = 0;
	std::string raw;
	int line = 0;
};

struct PendingSolid
{
	Solid solid;
	bool hasId = false;
	std::vector<bool> sideHasId;
	std::optional<GroupRef> group;
	int line = 0;
};

struct PendingEntity
{
	Entity entity;
	bool hasId = false;
	std::optional<GroupRef> group;
	std::vector<PendingSolid> solids;
	int line = 0;
};

struct PendingGroup
{
	Group group;
	bool hasId = false;
	std::optional<GroupRef> parent;
	int line = 0;
};

KeyValueNode Wrap( std::string name, KeyValueNode inner )
{
	KeyValueNode wrapper;
	wrapper.name = std::move( name );
	wrapper.children.push_back( std::move( inner ) );
	return wrapper;
}

class Decoder
{
public:
	explicit Decoder( const BlockLines &lines ) : m_lines( lines ) {}

	std::vector<CodecDiagnostic> warnings;
	DocumentSettings settings;
	bool sawWorld = false;
	std::vector<PendingSolid> worldSolids;
	std::vector<PendingGroup> groups;
	std::vector<PendingEntity> entities;
	std::set<std::uint32_t> verbatimGroupIds;

	void Warn( std::string message, int line )
	{
		warnings.push_back( { std::move( message ), line } );
	}
	int Line( const KeyValueNode &node ) const { return m_lines.Line( node ); }

	void DecodeTop( const KeyValueNode &root )
	{
		std::set<std::string> seen;
		for ( const KeyValueNode &block : root.children )
		{
			const std::string &name = block.name;
			const bool singleton = name == "versioninfo" || name == "visgroups" ||
			                       name == "viewsettings" || name == "world" || name == "cameras" ||
			                       name == "cordon" || name == "cordons";
			if ( singleton )
			{
				// 'cordon' and 'cordons' hold the same data: only one is read.
				const std::string family = name == "cordons" ? "cordon" : name;
				if ( !seen.insert( family ).second )
				{
					Warn( "a second '" + name + "' block is kept verbatim", Line( block ) );
					settings.unknownBlocks.push_back( block );
					continue;
				}
			}
			if ( name == "versioninfo" )
			{
				DecodePairBlock( block, settings.versionInfo );
			}
			else if ( name == "viewsettings" )
			{
				DecodePairBlock( block, settings.viewSettings );
			}
			else if ( name == "visgroups" )
			{
				DecodeVisgroups( block );
			}
			else if ( name == "world" )
			{
				DecodeWorld( block );
			}
			else if ( name == "entity" )
			{
				DecodeEntity( block, false );
			}
			else if ( name == "hidden" )
			{
				DecodeTopHidden( block );
			}
			else if ( name == "cameras" )
			{
				DecodeCameras( block );
			}
			else if ( name == "cordon" )
			{
				DecodeSingleCordon( block );
			}
			else if ( name == "cordons" )
			{
				DecodeCordonList( block );
			}
			else
			{
				settings.unknownBlocks.push_back( block );
			}
		}
	}

private:
	// A block of plain pairs; stray child blocks keep a verbatim copy (only the
	// children) among the unknown blocks.
	void DecodePairBlock( const KeyValueNode &block, std::vector<KeyValue> &into )
	{
		into = block.pairs;
		if ( !block.children.empty() )
		{
			Warn( "child blocks of '" + block.name + "' are kept in a separate verbatim block",
			    Line( block ) );
			KeyValueNode rest;
			rest.name = block.name;
			rest.children = block.children;
			settings.unknownBlocks.push_back( std::move( rest ) );
		}
	}

	static bool DecodeVisgroup( const KeyValueNode &node, Visgroup &out )
	{
		std::optional<std::string> name;
		std::optional<int> id;
		for ( const KeyValue &kv : node.pairs )
		{
			if ( kv.key == "name" && !name )
			{
				name = kv.value;
			}
			else if ( kv.key == "visgroupid" && !id )
			{
				id = ParseInt( kv.value );
				if ( !id )
				{
					return false;
				}
			}
			else if ( kv.key == "color" && !out.color )
			{
				out.color = ParseRgb( kv.value );
				if ( !out.color )
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		if ( !name || !id )
		{
			return false;
		}
		out.name = *name;
		out.id = *id;
		for ( const KeyValueNode &child : node.children )
		{
			Visgroup inner;
			if ( child.name != "visgroup" || !DecodeVisgroup( child, inner ) )
			{
				return false;
			}
			out.children.push_back( std::move( inner ) );
		}
		return true;
	}

	void DecodeVisgroups( const KeyValueNode &block )
	{
		std::vector<Visgroup> tree;
		bool ok = block.pairs.empty();
		for ( const KeyValueNode &child : block.children )
		{
			Visgroup v;
			if ( !ok || child.name != "visgroup" || !DecodeVisgroup( child, v ) )
			{
				ok = false;
				break;
			}
			tree.push_back( std::move( v ) );
		}
		if ( !ok )
		{
			Warn(
			    "the 'visgroups' block is not in the expected form; kept verbatim", Line( block ) );
			settings.unknownBlocks.push_back( block );
			return;
		}
		settings.visgroups = std::move( tree );
	}

	void DecodeCameras( const KeyValueNode &block )
	{
		std::vector<CameraBookmark> cameras;
		std::optional<int> active;
		bool ok = true;
		for ( const KeyValue &kv : block.pairs )
		{
			if ( kv.key == "activecamera" && !active )
			{
				active = ParseInt( kv.value );
				ok = ok && active.has_value();
			}
			else
			{
				ok = false;
			}
		}
		for ( const KeyValueNode &child : block.children )
		{
			std::optional<Vec3d> position;
			std::optional<Vec3d> look;
			ok = ok && child.name == "camera" && child.children.empty() && child.pairs.size() == 2;
			for ( const KeyValue &kv : child.pairs )
			{
				if ( kv.key == "position" && !position )
				{
					position = ParseEnclosedVec3( kv.value, '[', ']' );
				}
				else if ( kv.key == "look" && !look )
				{
					look = ParseEnclosedVec3( kv.value, '[', ']' );
				}
			}
			ok = ok && position && look;
			if ( !ok )
			{
				break;
			}
			cameras.push_back( { *position, *look } );
		}
		if ( !ok )
		{
			Warn( "the 'cameras' block is not in the expected form; kept verbatim", Line( block ) );
			settings.unknownBlocks.push_back( block );
			return;
		}
		settings.cameras = std::move( cameras );
		settings.activeCamera = active.value_or( -1 );
	}

	static std::optional<CordonBox> DecodeBoxPairs( const std::vector<KeyValue> &pairs,
	    std::optional<bool> *active, std::optional<std::string> *name )
	{
		std::optional<Vec3d> mins;
		std::optional<Vec3d> maxs;
		for ( const KeyValue &kv : pairs )
		{
			if ( kv.key == "mins" && !mins )
			{
				mins = ParseEnclosedVec3( kv.value, '(', ')' );
				if ( !mins )
				{
					return std::nullopt;
				}
			}
			else if ( kv.key == "maxs" && !maxs )
			{
				maxs = ParseEnclosedVec3( kv.value, '(', ')' );
				if ( !maxs )
				{
					return std::nullopt;
				}
			}
			else if ( active && kv.key == "active" && !*active )
			{
				*active = ParseFlag( kv.value );
				if ( !*active )
				{
					return std::nullopt;
				}
			}
			else if ( name && kv.key == "name" && !*name )
			{
				*name = kv.value;
			}
			else
			{
				return std::nullopt;
			}
		}
		if ( !mins || !maxs )
		{
			return std::nullopt;
		}
		return CordonBox{ *mins, *maxs };
	}

	void DecodeSingleCordon( const KeyValueNode &block )
	{
		std::optional<bool> active;
		const std::optional<CordonBox> box =
		    block.children.empty() ? DecodeBoxPairs( block.pairs, &active, nullptr ) : std::nullopt;
		if ( !box )
		{
			Warn( "the 'cordon' block is not in the expected form; kept verbatim", Line( block ) );
			settings.unknownBlocks.push_back( block );
			return;
		}
		Cordon cordon;
		cordon.active = active.value_or( false );
		cordon.boxes.push_back( *box );
		settings.cordons = { cordon };
		settings.cordonsActive = cordon.active;
		settings.cordonForm = DocumentSettings::CordonForm::Single;
	}

	void DecodeCordonList( const KeyValueNode &block )
	{
		std::optional<bool> active;
		std::vector<Cordon> cordons;
		bool ok = true;
		for ( const KeyValue &kv : block.pairs )
		{
			if ( kv.key == "active" && !active )
			{
				active = ParseFlag( kv.value );
				ok = ok && active.has_value();
			}
			else
			{
				ok = false;
			}
		}
		for ( const KeyValueNode &child : block.children )
		{
			if ( !ok || child.name != "cordon" )
			{
				ok = false;
				break;
			}
			Cordon cordon;
			std::optional<std::string> name;
			std::optional<bool> cordonActive;
			for ( const KeyValue &kv : child.pairs )
			{
				if ( kv.key == "name" && !name )
				{
					name = kv.value;
				}
				else if ( kv.key == "active" && !cordonActive )
				{
					cordonActive = ParseFlag( kv.value );
					ok = ok && cordonActive.has_value();
				}
				else
				{
					ok = false;
				}
			}
			for ( const KeyValueNode &boxNode : child.children )
			{
				const std::optional<CordonBox> box =
				    boxNode.name == "box" && boxNode.children.empty()
				        ? DecodeBoxPairs( boxNode.pairs, nullptr, nullptr )
				        : std::nullopt;
				if ( !box )
				{
					ok = false;
					break;
				}
				cordon.boxes.push_back( *box );
			}
			if ( !ok )
			{
				break;
			}
			cordon.name = name.value_or( std::string() );
			cordon.active = cordonActive.value_or( true );
			cordons.push_back( std::move( cordon ) );
		}
		if ( !ok )
		{
			Warn( "the 'cordons' block is not in the expected form; kept verbatim", Line( block ) );
			settings.unknownBlocks.push_back( block );
			return;
		}
		settings.cordons = std::move( cordons );
		settings.cordonsActive = active.value_or( false );
		settings.cordonForm = DocumentSettings::CordonForm::List;
	}

	// The 'editor' block. Unknown child blocks are returned in 'children'.
	EditorInfo DecodeEditor( const KeyValueNode &node, std::optional<GroupRef> &group,
	    std::vector<KeyValueNode> &children )
	{
		EditorInfo info;
		std::set<std::string> seen;
		for ( const KeyValue &kv : node.pairs )
		{
			const bool first = seen.insert( kv.key ).second;
			if ( kv.key == "color" && first )
			{
				info.color = ParseRgb( kv.value );
				if ( info.color )
				{
					continue;
				}
			}
			else if ( kv.key == "visgroupid" )
			{
				if ( const std::optional<int> id = ParseInt( kv.value ) )
				{
					info.visgroupIds.push_back( *id );
					continue;
				}
			}
			else if ( kv.key == "groupid" && first )
			{
				if ( const std::optional<std::uint32_t> id = ParseId( kv.value ) )
				{
					group = GroupRef{ *id, kv.value, Line( node ) };
					continue;
				}
			}
			else if ( kv.key == "visgroupshown" && first )
			{
				if ( const std::optional<bool> flag = ParseFlag( kv.value ) )
				{
					info.visgroupShown = *flag;
					continue;
				}
			}
			else if ( kv.key == "visgroupautoshown" && first )
			{
				if ( const std::optional<bool> flag = ParseFlag( kv.value ) )
				{
					info.visgroupAutoShown = *flag;
					continue;
				}
			}
			info.extra.push_back( kv );
		}
		children = node.children;
		return info;
	}

	// Reads a persistent id; a malformed one is dropped (a fresh id replaces it).
	bool ReadId( const KeyValue &kv, std::uint32_t &into, const char *what, int line )
	{
		if ( const std::optional<std::uint32_t> id = ParseId( kv.value ) )
		{
			into = *id;
			return true;
		}
		Warn(
		    std::string( what ) + " id '" + kv.value + "' is not a number; a fresh id replaces it",
		    line );
		return false;
	}

	// A side; returns the reason when the side cannot be modeled.
	std::optional<std::string> DecodeSide( const KeyValueNode &node, Side &side, bool &hasId )
	{
		std::set<std::string> seen;
		bool hasPlane = false;
		for ( const KeyValue &kv : node.pairs )
		{
			const bool first = seen.insert( kv.key ).second;
			if ( !first )
			{
				side.extraPairs.push_back( kv );
			}
			else if ( kv.key == "id" )
			{
				hasId = ReadId( kv, side.vmfId, "side", Line( node ) );
			}
			else if ( kv.key == "plane" )
			{
				const std::optional<std::array<Vec3d, 3>> points = ParsePlane( kv.value );
				if ( !points )
				{
					return "side plane '" + kv.value + "' is malformed";
				}
				side.points = *points;
				hasPlane = true;
			}
			else if ( kv.key == "material" )
			{
				side.texture.material = kv.value;
			}
			else if ( kv.key == "uaxis" || kv.key == "vaxis" )
			{
				const std::optional<TextureAxis> axis = ParseAxis( kv.value );
				if ( !axis )
				{
					return "side " + kv.key + " '" + kv.value + "' is malformed";
				}
				( kv.key == "uaxis" ? side.texture.u : side.texture.v ) = *axis;
			}
			else if ( kv.key == "rotation" || kv.key == "lightmapscale" )
			{
				const std::optional<double> value = ParseNumber( kv.value );
				if ( !value )
				{
					return "side " + kv.key + " '" + kv.value + "' is not a number";
				}
				( kv.key == "rotation" ? side.texture.rotation : side.texture.lightmapScale ) =
				    *value;
			}
			else if ( kv.key == "smoothing_groups" )
			{
				const std::optional<std::uint32_t> mask = ParseMask( kv.value );
				if ( !mask )
				{
					return "side smoothing_groups '" + kv.value + "' is not a number";
				}
				side.texture.smoothingGroups = *mask;
			}
			else
			{
				side.extraPairs.push_back( kv );
			}
		}
		if ( !hasPlane )
		{
			return std::string( "a side has no plane" );
		}
		for ( const KeyValueNode &child : node.children )
		{
			if ( child.name == "dispinfo" && !side.dispinfo )
			{
				side.dispinfo = child;
			}
			else
			{
				side.extraChildren.push_back( child );
			}
		}
		return std::nullopt;
	}

	// A solid; nothing (after a warning) when it must be kept verbatim.
	std::optional<PendingSolid> DecodeSolid( const KeyValueNode &node, bool hidden )
	{
		PendingSolid pending;
		pending.line = Line( node );
		pending.solid.hidden = hidden;
		bool sawId = false;
		bool sawEditor = false;
		for ( const KeyValue &kv : node.pairs )
		{
			if ( kv.key == "id" && !sawId )
			{
				sawId = true;
				pending.hasId = ReadId( kv, pending.solid.vmfId, "solid", pending.line );
			}
			else
			{
				pending.solid.extraPairs.push_back( kv );
			}
		}
		const std::string label =
		    "solid" +
		    ( pending.hasId ? " " + std::to_string( pending.solid.vmfId ) : std::string() );
		for ( const KeyValueNode &child : node.children )
		{
			if ( child.name == "side" )
			{
				Side side;
				bool sideHasId = false;
				if ( const std::optional<std::string> why = DecodeSide( child, side, sideHasId ) )
				{
					Warn( label + ": " + *why + "; the solid is kept verbatim", Line( child ) );
					return std::nullopt;
				}
				pending.solid.sides.push_back( std::move( side ) );
				pending.sideHasId.push_back( sideHasId );
			}
			else if ( child.name == "editor" && !sawEditor )
			{
				sawEditor = true;
				std::vector<KeyValueNode> rest;
				pending.solid.editor = DecodeEditor( child, pending.group, rest );
				PreserveEditorChildren(
				    std::move( rest ), pending.solid.extraChildren, Line( child ) );
			}
			else
			{
				pending.solid.extraChildren.push_back( child );
			}
		}
		if ( pending.solid.sides.size() < 4 )
		{
			Warn( label + " has " + std::to_string( pending.solid.sides.size() ) +
			          " sides (a solid needs at least four); it is kept verbatim",
			    pending.line );
			return std::nullopt;
		}
		return pending;
	}

	void PreserveEditorChildren(
	    std::vector<KeyValueNode> children, std::vector<KeyValueNode> &into, int line )
	{
		if ( children.empty() )
		{
			return;
		}
		Warn( "child blocks of an 'editor' block are kept in a separate verbatim 'editor' block",
		    line );
		KeyValueNode rest;
		rest.name = "editor";
		rest.children = std::move( children );
		into.push_back( std::move( rest ) );
	}

	void DecodeWorld( const KeyValueNode &block )
	{
		sawWorld = true;
		settings.worldKeys.clear();
		bool sawId = false;
		bool sawClass = false;
		for ( const KeyValue &kv : block.pairs )
		{
			if ( kv.key == "id" && !sawId )
			{
				sawId = true;
				if ( const std::optional<std::uint32_t> id = ParseId( kv.value ) )
				{
					settings.worldVmfId = *id;
				}
				else
				{
					Warn( "world id '" + kv.value + "' is not a number; 1 is used", Line( block ) );
				}
			}
			else if ( kv.key == "classname" && !sawClass && kv.value == "worldspawn" )
			{
				sawClass = true; // implied by the block
			}
			else
			{
				if ( kv.key == "classname" && !sawClass )
				{
					Warn( "world classname '" + kv.value + "' is not worldspawn; kept as a key",
					    Line( block ) );
				}
				settings.worldKeys.push_back( kv );
			}
		}
		for ( const KeyValueNode &child : block.children )
		{
			if ( child.name == "solid" )
			{
				WorldSolid( child, false, child );
			}
			else if ( child.name == "group" )
			{
				WorldGroup( child, false, child );
			}
			else if ( child.name == "hidden" && child.pairs.empty() )
			{
				for ( const KeyValueNode &inner : child.children )
				{
					if ( inner.name == "solid" )
					{
						WorldSolid( inner, true, Wrap( "hidden", inner ) );
					}
					else if ( inner.name == "group" )
					{
						WorldGroup( inner, true, Wrap( "hidden", inner ) );
					}
					else
					{
						Warn( "a hidden '" + inner.name + "' in the world is kept verbatim",
						    Line( inner ) );
						settings.worldExtraChildren.push_back( Wrap( "hidden", inner ) );
					}
				}
			}
			else
			{
				if ( child.name == "hidden" )
				{
					Warn( "a 'hidden' block with keys is kept verbatim", Line( child ) );
				}
				settings.worldExtraChildren.push_back( child );
			}
		}
	}

	void WorldSolid( const KeyValueNode &node, bool hidden, const KeyValueNode &verbatim )
	{
		if ( std::optional<PendingSolid> solid = DecodeSolid( node, hidden ) )
		{
			worldSolids.push_back( std::move( *solid ) );
		}
		else
		{
			settings.worldExtraChildren.push_back( verbatim );
		}
	}

	void WorldGroup( const KeyValueNode &node, bool hidden, const KeyValueNode &verbatim )
	{
		PendingGroup pending;
		pending.line = Line( node );
		pending.group.hidden = hidden;
		bool sawId = false;
		for ( const KeyValue &kv : node.pairs )
		{
			if ( kv.key == "id" && !sawId )
			{
				sawId = true;
				pending.hasId = ReadId( kv, pending.group.vmfId, "group", pending.line );
			}
			else
			{
				pending.group.extraPairs.push_back( kv );
			}
		}
		// A Group models one 'editor' block and nothing else below it.
		std::vector<KeyValueNode> rest;
		if ( node.children.size() == 1 && node.children.front().name == "editor" )
		{
			pending.group.editor = DecodeEditor( node.children.front(), pending.parent, rest );
		}
		if ( node.children.size() > 1 || !rest.empty() ||
		     ( node.children.size() == 1 && node.children.front().name != "editor" ) )
		{
			Warn( "a group with child blocks other than one 'editor' is kept verbatim",
			    pending.line );
			if ( pending.hasId )
			{
				verbatimGroupIds.insert( pending.group.vmfId );
			}
			settings.worldExtraChildren.push_back( verbatim );
			return;
		}
		groups.push_back( std::move( pending ) );
	}

	void DecodeEntity( const KeyValueNode &block, bool hidden )
	{
		PendingEntity pending;
		pending.line = Line( block );
		Entity &entity = pending.entity;
		entity.hidden = hidden;
		bool sawId = false;
		bool sawClass = false;
		bool sawEditor = false;
		for ( const KeyValue &kv : block.pairs )
		{
			if ( kv.key == "id" && !sawId )
			{
				sawId = true;
				pending.hasId = ReadId( kv, entity.vmfId, "entity", pending.line );
			}
			else if ( kv.key == "classname" && !sawClass )
			{
				sawClass = true;
				entity.classname = kv.value;
			}
			else
			{
				entity.keys.push_back( kv );
			}
		}
		KeyValueNode preservedConnections;
		preservedConnections.name = "connections";
		for ( const KeyValueNode &child : block.children )
		{
			if ( child.name == "connections" )
			{
				for ( const KeyValue &kv : child.pairs )
				{
					if ( std::optional<Connection> c =
					         hammer::scene::ParseConnection( kv.key, kv.value ) )
					{
						entity.connections.push_back( std::move( *c ) );
					}
					else
					{
						Warn( "entity connection '" + kv.key + "' \"" + kv.value +
						          "\" is not target,input,parameter,delay,times; kept verbatim",
						    Line( child ) );
						preservedConnections.pairs.push_back( kv );
					}
				}
				if ( !child.children.empty() )
				{
					Warn(
					    "child blocks of a 'connections' block are kept verbatim", Line( child ) );
					for ( const KeyValueNode &inner : child.children )
					{
						preservedConnections.children.push_back( inner );
					}
				}
			}
			else if ( child.name == "solid" )
			{
				EntitySolid( pending, child, false, child );
			}
			else if ( child.name == "hidden" && child.pairs.empty() )
			{
				for ( const KeyValueNode &inner : child.children )
				{
					if ( inner.name == "solid" )
					{
						EntitySolid( pending, inner, true, Wrap( "hidden", inner ) );
					}
					else
					{
						Warn( "a hidden '" + inner.name + "' in an entity is kept verbatim",
						    Line( inner ) );
						entity.extraChildren.push_back( Wrap( "hidden", inner ) );
					}
				}
			}
			else if ( child.name == "editor" && !sawEditor )
			{
				sawEditor = true;
				std::vector<KeyValueNode> rest;
				entity.editor = DecodeEditor( child, pending.group, rest );
				PreserveEditorChildren( std::move( rest ), entity.extraChildren, Line( child ) );
			}
			else
			{
				if ( child.name == "hidden" )
				{
					Warn( "a 'hidden' block with keys is kept verbatim", Line( child ) );
				}
				entity.extraChildren.push_back( child );
			}
		}
		if ( !preservedConnections.pairs.empty() || !preservedConnections.children.empty() )
		{
			entity.extraChildren.push_back( std::move( preservedConnections ) );
		}
		entities.push_back( std::move( pending ) );
	}

	void EntitySolid(
	    PendingEntity &entity, const KeyValueNode &node, bool hidden, const KeyValueNode &verbatim )
	{
		if ( std::optional<PendingSolid> solid = DecodeSolid( node, hidden ) )
		{
			entity.solids.push_back( std::move( *solid ) );
		}
		else
		{
			entity.entity.extraChildren.push_back( verbatim );
		}
	}

	void DecodeTopHidden( const KeyValueNode &block )
	{
		if ( !block.pairs.empty() )
		{
			Warn( "a 'hidden' block with keys is kept verbatim", Line( block ) );
			settings.unknownBlocks.push_back( block );
			return;
		}
		for ( const KeyValueNode &inner : block.children )
		{
			if ( inner.name == "entity" )
			{
				DecodeEntity( inner, true );
			}
			else
			{
				Warn( "a hidden '" + inner.name + "' at the top level is kept verbatim",
				    Line( inner ) );
				settings.unknownBlocks.push_back( Wrap( "hidden", inner ) );
			}
		}
	}

	const BlockLines &m_lines;
};

// Every "id" in content kept verbatim, so fresh ids never collide with it.
void NoteVerbatimIds( const KeyValueNode &node, MapDocument &doc )
{
	for ( const KeyValue &kv : node.pairs )
	{
		if ( kv.key == "id" )
		{
			if ( const std::optional<std::uint32_t> id = ParseId( kv.value ) )
			{
				doc.NoteVmfId( *id );
			}
		}
	}
	for ( const KeyValueNode &child : node.children )
	{
		NoteVerbatimIds( child, doc );
	}
}

void NoteVerbatimIds( const std::vector<KeyValueNode> &nodes, MapDocument &doc )
{
	for ( const KeyValueNode &node : nodes )
	{
		NoteVerbatimIds( node, doc );
	}
}

// --- Encoding --------------------------------------------------------------------------

void Add( KeyValueNode &node, std::string key, std::string value )
{
	node.pairs.push_back( { std::move( key ), std::move( value ) } );
}

bool HasChild( const std::vector<KeyValueNode> &children, std::string_view name )
{
	for ( const KeyValueNode &child : children )
	{
		if ( child.name == name )
		{
			return true;
		}
	}
	return false;
}

bool IsFinite( const Vec3d &v )
{
	return std::isfinite( v.x ) && std::isfinite( v.y ) && std::isfinite( v.z );
}

class Encoder
{
public:
	explicit Encoder( const MapDocument &doc ) : m_doc( doc ) {}

	std::optional<CodecError> error;

	void Fail( std::string message )
	{
		if ( !error )
		{
			error = CodecError{ std::move( message ), 0 };
		}
	}

	std::string Number( double value )
	{
		if ( !std::isfinite( value ) )
		{
			Fail( "a number is not finite" );
		}
		return ExactNumber( value );
	}

	std::string Vec( const Vec3d &v )
	{
		if ( !IsFinite( v ) )
		{
			Fail( "a vector is not finite" );
		}
		return ExactVec3( v );
	}

	// The persistent id of a group reference; empty when there is none.
	std::optional<std::uint32_t> GroupVmfId( ObjectId group, const char *what )
	{
		if ( !group.IsValid() )
		{
			return std::nullopt;
		}
		const Group *g = m_doc.FindGroup( group );
		if ( !g )
		{
			Fail( std::string( what ) + " refers to a group that is not in the document" );
			return std::nullopt;
		}
		return g->vmfId;
	}

	void EmitEditor( KeyValueNode &parent, const EditorInfo &info, ObjectId group, const char *what,
	    const std::vector<KeyValueNode> &extraChildren )
	{
		const std::optional<std::uint32_t> groupId = GroupVmfId( group, what );
		if ( info == EditorInfo{} && !groupId && !HasChild( extraChildren, "editor" ) )
		{
			return;
		}
		KeyValueNode editor;
		editor.name = "editor";
		if ( info.color )
		{
			Add( editor, "color", FormatRgb( *info.color ) );
		}
		if ( groupId )
		{
			Add( editor, "groupid", std::to_string( *groupId ) );
		}
		for ( const int id : info.visgroupIds )
		{
			Add( editor, "visgroupid", std::to_string( id ) );
		}
		Add( editor, "visgroupshown", info.visgroupShown ? "1" : "0" );
		Add( editor, "visgroupautoshown", info.visgroupAutoShown ? "1" : "0" );
		for ( const KeyValue &kv : info.extra )
		{
			editor.pairs.push_back( kv );
		}
		parent.children.push_back( std::move( editor ) );
	}

	KeyValueNode EncodeSide( const Side &side )
	{
		KeyValueNode node;
		node.name = "side";
		for ( const Vec3d &p : side.points )
		{
			if ( !IsFinite( p ) )
			{
				Fail( "a side point is not finite" );
			}
		}
		Add( node, "id", std::to_string( side.vmfId ) );
		Add( node, "plane", FormatPlane( side.points ) );
		Add( node, "material", side.texture.material );
		if ( !IsFinite( side.texture.u.axis ) || !IsFinite( side.texture.v.axis ) ||
		     !std::isfinite( side.texture.u.shift ) || !std::isfinite( side.texture.v.shift ) ||
		     !std::isfinite( side.texture.u.scale ) || !std::isfinite( side.texture.v.scale ) )
		{
			Fail( "a texture axis is not finite" );
		}
		Add( node, "uaxis", FormatAxis( side.texture.u ) );
		Add( node, "vaxis", FormatAxis( side.texture.v ) );
		Add( node, "rotation", Number( side.texture.rotation ) );
		Add( node, "lightmapscale", Number( side.texture.lightmapScale ) );
		Add( node, "smoothing_groups",
		    std::to_string( static_cast<std::int32_t>( side.texture.smoothingGroups ) ) );
		for ( const KeyValue &kv : side.extraPairs )
		{
			node.pairs.push_back( kv );
		}
		if ( side.dispinfo )
		{
			node.children.push_back( *side.dispinfo );
		}
		for ( const KeyValueNode &child : side.extraChildren )
		{
			node.children.push_back( child );
		}
		return node;
	}

	KeyValueNode EncodeSolid( const Solid &solid )
	{
		KeyValueNode node;
		node.name = "solid";
		Add( node, "id", std::to_string( solid.vmfId ) );
		for ( const KeyValue &kv : solid.extraPairs )
		{
			node.pairs.push_back( kv );
		}
		for ( const Side &side : solid.sides )
		{
			node.children.push_back( EncodeSide( side ) );
		}
		EmitEditor( node, solid.editor, solid.group, "a solid", solid.extraChildren );
		for ( const KeyValueNode &child : solid.extraChildren )
		{
			node.children.push_back( child );
		}
		return solid.hidden ? Wrap( "hidden", std::move( node ) ) : node;
	}

	KeyValueNode EncodeGroup( const Group &group )
	{
		KeyValueNode node;
		node.name = "group";
		Add( node, "id", std::to_string( group.vmfId ) );
		for ( const KeyValue &kv : group.extraPairs )
		{
			node.pairs.push_back( kv );
		}
		EmitEditor( node, group.editor, group.group, "a group", {} );
		return group.hidden ? Wrap( "hidden", std::move( node ) ) : node;
	}

	KeyValueNode EncodeEntity( const Entity &entity, const std::vector<const Solid *> &solids )
	{
		KeyValueNode node;
		node.name = "entity";
		Add( node, "id", std::to_string( entity.vmfId ) );
		Add( node, "classname", entity.classname );
		for ( const KeyValue &kv : entity.keys )
		{
			node.pairs.push_back( kv );
		}
		if ( !entity.connections.empty() )
		{
			KeyValueNode connections;
			connections.name = "connections";
			for ( const Connection &c : entity.connections )
			{
				Add( connections, c.output, ConnectionValue( c ) );
			}
			node.children.push_back( std::move( connections ) );
		}
		for ( const Solid *solid : solids )
		{
			node.children.push_back( EncodeSolid( *solid ) );
		}
		EmitEditor( node, entity.editor, entity.group, "an entity", entity.extraChildren );
		for ( const KeyValueNode &child : entity.extraChildren )
		{
			node.children.push_back( child );
		}
		return entity.hidden ? Wrap( "hidden", std::move( node ) ) : node;
	}

	std::string ConnectionValue( const Connection &c )
	{
		if ( !std::isfinite( c.delay ) )
		{
			Fail( "connection '" + c.output + "' has a non-finite delay" );
			return std::string();
		}
		std::string value = hammer::scene::FormatConnectionValue( c );
		std::optional<Connection> back = hammer::scene::ParseConnection( c.output, value );
		if ( !back || !( *back == c ) )
		{
			// The owner's format rounds the delay; write it exactly.
			Connection exact = c;
			value = c.target + c.separator + c.input + c.separator + c.parameter + c.separator +
			        ExactNumber( c.delay ) + c.separator + std::to_string( c.timesToFire );
			back = hammer::scene::ParseConnection( c.output, value );
			if ( !back || !( *back == exact ) )
			{
				Fail( "connection '" + c.output +
				      "' cannot be written: a field holds the separator or the separator is not "
				      "',' or 0x1B" );
			}
		}
		return value;
	}

	KeyValueNode EncodeVisgroup( const Visgroup &v )
	{
		KeyValueNode node;
		node.name = "visgroup";
		Add( node, "name", v.name );
		Add( node, "visgroupid", std::to_string( v.id ) );
		if ( v.color )
		{
			Add( node, "color", FormatRgb( *v.color ) );
		}
		for ( const Visgroup &child : v.children )
		{
			node.children.push_back( EncodeVisgroup( child ) );
		}
		return node;
	}

	KeyValueNode EncodeBox( const CordonBox &box, const char *name )
	{
		KeyValueNode node;
		node.name = name;
		Add( node, "mins", "(" + Vec( box.mins ) + ")" );
		Add( node, "maxs", "(" + Vec( box.maxs ) + ")" );
		return node;
	}

	KeyValueNode Encode()
	{
		const DocumentSettings &settings = m_doc.Settings();
		KeyValueNode root;

		KeyValueNode version;
		version.name = "versioninfo";
		version.pairs = settings.versionInfo;
		root.children.push_back( std::move( version ) );

		KeyValueNode visgroups;
		visgroups.name = "visgroups";
		for ( const Visgroup &v : settings.visgroups )
		{
			visgroups.children.push_back( EncodeVisgroup( v ) );
		}
		root.children.push_back( std::move( visgroups ) );

		KeyValueNode view;
		view.name = "viewsettings";
		view.pairs = settings.viewSettings;
		root.children.push_back( std::move( view ) );

		// Objects by owner, in id order.
		std::vector<const Solid *> worldSolids;
		std::map<ObjectId, std::vector<const Solid *>> entitySolids;
		for ( const auto &[id, solid] : m_doc.Solids() )
		{
			if ( !solid.owner.IsValid() )
			{
				worldSolids.push_back( &solid );
			}
			else if ( m_doc.FindEntity( solid.owner ) )
			{
				entitySolids[solid.owner].push_back( &solid );
			}
			else
			{
				Fail( "solid " + std::to_string( solid.vmfId ) +
				      " is owned by an entity that is not in the document" );
			}
		}

		KeyValueNode world;
		world.name = "world";
		Add( world, "id", std::to_string( settings.worldVmfId ) );
		std::size_t key = 0;
		if ( !settings.worldKeys.empty() && settings.worldKeys.front().key == "mapversion" )
		{
			world.pairs.push_back( settings.worldKeys.front() );
			key = 1;
		}
		Add( world, "classname", "worldspawn" );
		for ( ; key < settings.worldKeys.size(); ++key )
		{
			world.pairs.push_back( settings.worldKeys[key] );
		}
		for ( const Solid *solid : worldSolids )
		{
			world.children.push_back( EncodeSolid( *solid ) );
		}
		for ( const auto &[id, group] : m_doc.Groups() )
		{
			world.children.push_back( EncodeGroup( group ) );
		}
		for ( const KeyValueNode &child : settings.worldExtraChildren )
		{
			world.children.push_back( child );
		}
		root.children.push_back( std::move( world ) );

		for ( const auto &[id, entity] : m_doc.Entities() )
		{
			const auto solids = entitySolids.find( id );
			root.children.push_back( EncodeEntity( entity,
			    solids == entitySolids.end() ? std::vector<const Solid *>() : solids->second ) );
		}

		KeyValueNode cameras;
		cameras.name = "cameras";
		Add( cameras, "activecamera", std::to_string( settings.activeCamera ) );
		for ( const CameraBookmark &camera : settings.cameras )
		{
			KeyValueNode node;
			node.name = "camera";
			Add( node, "position", "[" + Vec( camera.position ) + "]" );
			Add( node, "look", "[" + Vec( camera.look ) + "]" );
			cameras.children.push_back( std::move( node ) );
		}
		root.children.push_back( std::move( cameras ) );

		using Form = DocumentSettings::CordonForm;
		const bool singleExpressible = settings.cordons.size() == 1 &&
		                               settings.cordons.front().boxes.size() == 1 &&
		                               settings.cordons.front().name.empty() &&
		                               settings.cordons.front().active == settings.cordonsActive;
		if ( settings.cordonForm == Form::Single && singleExpressible )
		{
			KeyValueNode cordon = EncodeBox( settings.cordons.front().boxes.front(), "cordon" );
			Add( cordon, "active", settings.cordonsActive ? "1" : "0" );
			root.children.push_back( std::move( cordon ) );
		}
		else if ( settings.cordonForm != Form::None || !settings.cordons.empty() )
		{
			KeyValueNode list;
			list.name = "cordons";
			Add( list, "active", settings.cordonsActive ? "1" : "0" );
			for ( const Cordon &c : settings.cordons )
			{
				KeyValueNode node;
				node.name = "cordon";
				Add( node, "name", c.name );
				Add( node, "active", c.active ? "1" : "0" );
				for ( const CordonBox &box : c.boxes )
				{
					node.children.push_back( EncodeBox( box, "box" ) );
				}
				list.children.push_back( std::move( node ) );
			}
			root.children.push_back( std::move( list ) );
		}

		for ( const KeyValueNode &block : settings.unknownBlocks )
		{
			root.children.push_back( block );
		}
		return root;
	}

private:
	const MapDocument &m_doc;
};

// Text the kvtext writer cannot represent: a quote anywhere, and block names
// that would not read back as one bare word.
std::optional<std::string> Unwritable( const KeyValueNode &node, bool isRoot )
{
	if ( !isRoot )
	{
		if ( node.name.empty() || node.name.rfind( "//", 0 ) == 0 )
		{
			return "block name '" + node.name + "' cannot be written";
		}
		for ( const char c : node.name )
		{
			if ( IsBlank( c ) || c == '{' || c == '}' || c == '"' )
			{
				return "block name '" + node.name + "' cannot be written";
			}
		}
	}
	for ( const KeyValue &kv : node.pairs )
	{
		if ( kv.key.find( '"' ) != std::string::npos || kv.value.find( '"' ) != std::string::npos )
		{
			return "key '" + kv.key + "' in '" + node.name + "' holds a '\"'";
		}
	}
	for ( const KeyValueNode &child : node.children )
	{
		if ( std::optional<std::string> why = Unwritable( child, false ) )
		{
			return why;
		}
	}
	return std::nullopt;
}

} // namespace

foundation::Expected<DecodedMap, CodecError> VmfMapCodec::Decode(
    std::string_view text, std::uint32_t serial ) const
{
	const std::string owned( text );
	const kvtext::ParseResult parsed = kvtext::ParseKeyValues( owned );
	if ( !parsed.ok )
	{
		return foundation::MakeUnexpected(
		    CodecError{ "not VMF text: " + parsed.error, static_cast<int>( parsed.errorLine ) } );
	}
	if ( !parsed.root.pairs.empty() )
	{
		return foundation::MakeUnexpected(
		    CodecError{ "not VMF text: key/value pairs outside any block", 0 } );
	}

	const BlockLines lines( owned, parsed.root );
	Decoder decoder( lines );
	decoder.DecodeTop( parsed.root );

	MapDocument doc( serial );
	DocumentSettings &settings = doc.MutableSettings();
	if ( !decoder.sawWorld )
	{
		decoder.settings.worldKeys = settings.worldKeys; // the new-document default
	}
	settings = std::move( decoder.settings );

	// Persistent ids: note every id first, then replace missing and duplicate ones
	// with fresh ids above all of them, in the runtime-id order.
	doc.NoteVmfId( settings.worldVmfId );
	NoteVerbatimIds( settings.worldExtraChildren, doc );
	NoteVerbatimIds( settings.unknownBlocks, doc );
	auto noteSolid = [&]( const PendingSolid &s )
	{
		if ( s.hasId )
		{
			doc.NoteVmfId( s.solid.vmfId );
		}
		for ( std::size_t i = 0; i < s.solid.sides.size(); ++i )
		{
			if ( s.sideHasId[i] )
			{
				doc.NoteVmfId( s.solid.sides[i].vmfId );
			}
		}
		NoteVerbatimIds( s.solid.extraChildren, doc );
	};
	for ( const PendingSolid &s : decoder.worldSolids )
	{
		noteSolid( s );
	}
	for ( const PendingGroup &g : decoder.groups )
	{
		doc.NoteVmfId( g.group.vmfId );
	}
	for ( const PendingEntity &e : decoder.entities )
	{
		if ( e.hasId )
		{
			doc.NoteVmfId( e.entity.vmfId );
		}
		NoteVerbatimIds( e.entity.extraChildren, doc );
		for ( const PendingSolid &s : e.solids )
		{
			noteSolid( s );
		}
	}

	std::size_t missing = 0;
	std::set<std::uint32_t> sideIds;
	std::set<std::uint32_t> objectIds;
	auto objectId = [&]( std::uint32_t &vmfId, bool hasId, const char *what, int line )
	{
		if ( !hasId )
		{
			vmfId = doc.AllocateVmfId();
			++missing;
		}
		else if ( !objectIds.insert( vmfId ).second )
		{
			decoder.warnings.push_back( { std::string( what ) + " id " + std::to_string( vmfId ) +
			                                  " is used by more than one object; kept",
			    line } );
		}
	};
	auto fixSolid = [&]( PendingSolid &s )
	{
		objectId( s.solid.vmfId, s.hasId, "solid", s.line );
		for ( std::size_t i = 0; i < s.solid.sides.size(); ++i )
		{
			Side &side = s.solid.sides[i];
			if ( !s.sideHasId[i] )
			{
				side.vmfId = doc.AllocateVmfId();
				++missing;
			}
			else if ( !sideIds.insert( side.vmfId ).second )
			{
				const std::uint32_t fresh = doc.AllocateVmfId();
				decoder.warnings.push_back(
				    { "side id " + std::to_string( side.vmfId ) +
				            " is used more than once; reassigned " + std::to_string( fresh ),
				        s.line } );
				side.vmfId = fresh;
			}
			sideIds.insert( side.vmfId );
		}
	};
	for ( PendingSolid &s : decoder.worldSolids )
	{
		fixSolid( s );
	}
	for ( PendingGroup &g : decoder.groups )
	{
		objectId( g.group.vmfId, g.hasId, "group", g.line );
	}
	for ( PendingEntity &e : decoder.entities )
	{
		objectId( e.entity.vmfId, e.hasId, "entity", e.line );
		for ( PendingSolid &s : e.solids )
		{
			fixSolid( s );
		}
	}
	if ( missing > 0 )
	{
		decoder.warnings.push_back(
		    { std::to_string( missing ) + " objects or sides had no id; fresh ids were assigned",
		        0 } );
	}

	// Runtime ids in the documented order.
	for ( PendingSolid &s : decoder.worldSolids )
	{
		s.solid.id = doc.AllocateId();
	}
	std::map<std::uint32_t, ObjectId> groupByVmfId;
	for ( PendingGroup &g : decoder.groups )
	{
		g.group.id = doc.AllocateId();
		groupByVmfId.emplace( g.group.vmfId, g.group.id );
	}
	for ( PendingEntity &e : decoder.entities )
	{
		e.entity.id = doc.AllocateId();
		for ( PendingSolid &s : e.solids )
		{
			s.solid.id = doc.AllocateId();
			s.solid.owner = e.entity.id;
		}
	}

	// Group references.
	auto resolve = [&]( const std::optional<GroupRef> &ref, ObjectId &into, EditorInfo &editor )
	{
		if ( !ref )
		{
			return;
		}
		const auto it = groupByVmfId.find( ref->id );
		if ( it != groupByVmfId.end() )
		{
			into = it->second;
		}
		else if ( decoder.verbatimGroupIds.count( ref->id ) )
		{
			editor.extra.push_back( { "groupid", ref->raw } ); // names a verbatim group
		}
		else
		{
			decoder.warnings.push_back(
			    { "groupid " + ref->raw + " names no group; the membership is dropped",
			        ref->line } );
		}
	};
	for ( PendingSolid &s : decoder.worldSolids )
	{
		resolve( s.group, s.solid.group, s.solid.editor );
	}
	for ( PendingGroup &g : decoder.groups )
	{
		resolve( g.parent, g.group.group, g.group.editor );
	}
	for ( PendingEntity &e : decoder.entities )
	{
		resolve( e.group, e.entity.group, e.entity.editor );
		for ( PendingSolid &s : e.solids )
		{
			resolve( s.group, s.solid.group, s.solid.editor );
		}
	}
	// Break group cycles at the group that closes them.
	std::map<ObjectId, PendingGroup *> groupsById;
	for ( PendingGroup &g : decoder.groups )
	{
		groupsById[g.group.id] = &g;
	}
	for ( PendingGroup &g : decoder.groups )
	{
		ObjectId at = g.group.group;
		std::size_t steps = 0;
		while ( at.IsValid() && steps <= groupsById.size() )
		{
			if ( at == g.group.id )
			{
				decoder.warnings.push_back( { "group " + std::to_string( g.group.vmfId ) +
				                                  " contains itself; its parent group is dropped",
				    g.line } );
				g.group.group = ObjectId();
				break;
			}
			at = groupsById[at]->group.group;
			++steps;
		}
	}

	for ( PendingSolid &s : decoder.worldSolids )
	{
		doc.Put( std::move( s.solid ) );
	}
	for ( PendingGroup &g : decoder.groups )
	{
		doc.Put( std::move( g.group ) );
	}
	for ( PendingEntity &e : decoder.entities )
	{
		doc.Put( std::move( e.entity ) );
		for ( PendingSolid &s : e.solids )
		{
			doc.Put( std::move( s.solid ) );
		}
	}

	const std::vector<std::string> problems = doc.Validate();
	if ( !problems.empty() )
	{
		return foundation::MakeUnexpected( CodecError{
		    "internal: the decoded document is inconsistent: " + problems.front(), 0 } );
	}
	return DecodedMap{ std::move( doc ), std::move( decoder.warnings ) };
}

foundation::Expected<std::string, CodecError> VmfMapCodec::Encode(
    const MapDocument &document ) const
{
	Encoder encoder( document );
	const KeyValueNode root = encoder.Encode();
	if ( encoder.error )
	{
		return foundation::MakeUnexpected( *encoder.error );
	}
	if ( std::optional<std::string> why = Unwritable( root, true ) )
	{
		return foundation::MakeUnexpected( CodecError{ *why, 0 } );
	}
	return kvtext::WriteKeyValues( root );
}

} // namespace hammer::formats
