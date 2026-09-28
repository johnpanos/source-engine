//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/formats/vmf_map_codec.h: the
//			strict VMF codec. Every block and key maps to a typed field of the
//			scene model; content the model cannot hold is a CodecError naming
//			its block path and line. Key orders and value formats follow the
//			legacy writer (hammer/mapdoc.cpp, mapworld.cpp, mapentity.cpp,
//			mapsolid.cpp, mapface.cpp, mapdisp.cpp, mapclass.cpp,
//			editgameclass.cpp, mapgroup.cpp), so the output loads in legacy
//			Hammer and vbsp.
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
using hammer::scene::Displacement;
using hammer::scene::DocumentSettings;
using hammer::scene::EditorInfo;
using hammer::scene::Entity;
using hammer::scene::FaceTexture;
using hammer::scene::Group;
using hammer::scene::MapDocument;
using hammer::scene::ObjectId;
using hammer::scene::Rgb;
using hammer::scene::Side;
using hammer::scene::Solid;
using hammer::scene::TextureAxis;
using hammer::scene::VersionInfo;
using hammer::scene::ViewSettings;
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

std::optional<std::vector<double>> ParseNumbers( std::string_view text )
{
	std::vector<double> out;
	for ( const std::string_view word : Words( text ) )
	{
		const std::optional<double> v = ParseNumber( word );
		if ( !v )
		{
			return std::nullopt;
		}
		out.push_back( *v );
	}
	return out;
}

std::optional<std::vector<std::int64_t>> ParseIntegers( std::string_view text )
{
	std::vector<std::int64_t> out;
	for ( const std::string_view word : Words( text ) )
	{
		const std::optional<std::int64_t> v = ParseInteger( word );
		if ( !v )
		{
			return std::nullopt;
		}
		out.push_back( *v );
	}
	return out;
}

// "<open>n1 ... nk<close>" with exactly 'count' numbers.
std::optional<std::vector<double>> ParseEnclosed(
    std::string_view text, char open, char close, std::size_t count )
{
	text = Trim( text );
	if ( text.size() < 2 || text.front() != open || text.back() != close )
	{
		return std::nullopt;
	}
	std::optional<std::vector<double>> values = ParseNumbers( text.substr( 1, text.size() - 2 ) );
	if ( !values || values->size() != count )
	{
		return std::nullopt;
	}
	return values;
}

std::optional<Vec3d> ParseEnclosedVec3( std::string_view text, char open, char close )
{
	const std::optional<std::vector<double>> v = ParseEnclosed( text, open, close, 3 );
	if ( !v )
	{
		return std::nullopt;
	}
	return Vec3d( ( *v )[0], ( *v )[1], ( *v )[2] );
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
		const std::size_t close = text.find( ')', i );
		if ( text[i] != '(' || found == 3 || close == std::string_view::npos )
		{
			return std::nullopt;
		}
		const std::optional<Vec3d> point =
		    ParseEnclosedVec3( text.substr( i, close - i + 1 ), '(', ')' );
		if ( !point )
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
	if ( close == std::string_view::npos )
	{
		return std::nullopt;
	}
	const std::optional<std::vector<double>> inner =
	    ParseEnclosed( text.substr( 0, close + 1 ), '[', ']', 4 );
	const std::optional<std::vector<double>> after = ParseNumbers( text.substr( close + 1 ) );
	if ( !inner || !after || after->size() != 1 )
	{
		return std::nullopt;
	}
	TextureAxis axis;
	axis.axis = Vec3d( ( *inner )[0], ( *inner )[1], ( *inner )[2] );
	axis.shift = ( *inner )[3];
	axis.scale = ( *after )[0];
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

// "[x y]" with integer components (the logical-view position).
std::optional<std::array<int, 2>> ParseLogicalPos( std::string_view text )
{
	text = Trim( text );
	if ( text.size() < 2 || text.front() != '[' || text.back() != ']' )
	{
		return std::nullopt;
	}
	const std::vector<std::string_view> words = Words( text.substr( 1, text.size() - 2 ) );
	if ( words.size() != 2 )
	{
		return std::nullopt;
	}
	const std::optional<int> x = ParseInt( words[0] );
	const std::optional<int> y = ParseInt( words[1] );
	if ( !x || !y )
	{
		return std::nullopt;
	}
	return std::array<int, 2>{ *x, *y };
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

// --- Source lines ------------------------------------------------------------------

// The line of every parsed block and of each of its pairs. The kvtext tree
// carries no lines, so the text is re-tokenized with the parser's rules and
// walked with the parser's grammar; blocks then match the tree in preorder.
class SourceLines
{
public:
	SourceLines( const std::string &text, const KeyValueNode &root )
	{
		Tokenize( text );
		std::vector<Block> blocks;
		std::size_t at = 0;
		Block top;
		Walk( at, top, blocks );
		m_rootPairs = top.pairLines;
		std::size_t next = 0;
		Assign( root, blocks, next );
	}

	int Line( const KeyValueNode &node ) const
	{
		const auto it = m_blocks.find( &node );
		return it == m_blocks.end() ? 0 : it->second.line;
	}

	int PairLine( const KeyValueNode &node, std::size_t pair, bool isRoot = false ) const
	{
		const std::vector<int> *lines = &m_rootPairs;
		if ( !isRoot )
		{
			const auto it = m_blocks.find( &node );
			if ( it == m_blocks.end() )
			{
				return 0;
			}
			lines = &it->second.pairLines;
		}
		return pair < lines->size() ? ( *lines )[pair] : Line( node );
	}

private:
	struct Token
	{
		char kind; // 's' string, '{', '}'
		int line;
	};
	struct Block
	{
		int line = 0;
		std::vector<int> pairLines;
	};

	void Tokenize( const std::string &text )
	{
		int line = 1;
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
			else if ( c == '{' || c == '}' )
			{
				m_tokens.push_back( { c, line } );
				++i;
			}
			else if ( c == '"' )
			{
				m_tokens.push_back( { 's', line } );
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
				m_tokens.push_back( { 's', line } );
				while ( i < text.size() && !IsBlank( text[i] ) && text[i] != '{' &&
				        text[i] != '}' && text[i] != '"' )
				{
					++i;
				}
			}
		}
	}

	// Mirrors kvtext's parser on text it accepted: a string followed by '{'
	// opens a block, a string followed by a string is a pair.
	void Walk( std::size_t &at, Block &into, std::vector<Block> &blocks )
	{
		while ( at < m_tokens.size() && m_tokens[at].kind != '}' )
		{
			const Token first = m_tokens[at++];
			if ( at < m_tokens.size() && m_tokens[at].kind == '{' )
			{
				++at;
				const std::size_t index = blocks.size();
				blocks.push_back( { first.line, {} } );
				Block inner{ first.line, {} };
				Walk( at, inner, blocks );
				blocks[index].pairLines = std::move( inner.pairLines );
				++at; // '}'
			}
			else
			{
				into.pairLines.push_back( first.line );
				++at;
			}
		}
	}

	void Assign( const KeyValueNode &node, const std::vector<Block> &blocks, std::size_t &next )
	{
		for ( const KeyValueNode &child : node.children )
		{
			if ( next < blocks.size() )
			{
				m_blocks[&child] = blocks[next];
			}
			++next;
			Assign( child, blocks, next );
		}
	}

	std::vector<Token> m_tokens;
	std::vector<int> m_rootPairs;
	std::unordered_map<const KeyValueNode *, Block> m_blocks;
};

// --- Decoding ------------------------------------------------------------------------

struct GroupRef
{
	std::uint32_t id = 0;
	std::string path;
	int line = 0;
};

struct Where
{
	std::string path;
	int line = 0;
};

struct PendingSolid
{
	Solid solid;
	bool hasId = false;
	std::vector<bool> sideHasId;
	std::vector<Where> sideWhere;
	std::optional<GroupRef> group;
	Where where;
};

struct PendingEntity
{
	Entity entity;
	bool hasId = false;
	std::optional<GroupRef> group;
	std::vector<PendingSolid> solids;
	Where where;
};

struct PendingGroup
{
	Group group;
	bool hasId = false;
	std::optional<GroupRef> parent;
	Where where;
};

std::string ChildPath( const std::string &parent, const std::string &name, int index )
{
	const std::string self = index < 0 ? name : name + "[" + std::to_string( index ) + "]";
	return parent.empty() ? self : parent + "/" + self;
}

// Numbers each child by its position among same-named siblings.
std::vector<int> SiblingIndices( const KeyValueNode &node )
{
	std::map<std::string, int> counts;
	std::vector<int> indices;
	for ( const KeyValueNode &child : node.children )
	{
		indices.push_back( counts[child.name]++ );
	}
	return indices;
}

class Decoder
{
public:
	explicit Decoder( const SourceLines &lines ) : m_lines( lines ) {}

	std::optional<CodecError> error;
	DocumentSettings settings;
	bool sawWorld = false;
	std::vector<PendingSolid> worldSolids;
	std::vector<PendingGroup> groups;
	std::vector<PendingEntity> entities;

	bool Fail( const std::string &path, const std::string &message, int line )
	{
		if ( !error )
		{
			error = CodecError{ ( path.empty() ? std::string() : path + ": " ) + message, line };
		}
		return false;
	}

	bool DecodeTop( const KeyValueNode &root )
	{
		if ( !root.pairs.empty() )
		{
			return Fail( "", "key '" + root.pairs.front().key + "' is outside any block",
			    m_lines.PairLine( root, 0, true ) );
		}
		const std::vector<int> index = SiblingIndices( root );
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < root.children.size(); ++i )
		{
			const KeyValueNode &block = root.children[i];
			const std::string &name = block.name;
			const bool singleton = name == "versioninfo" || name == "visgroups" ||
			                       name == "viewsettings" || name == "world" || name == "cameras" ||
			                       name == "cordon" || name == "cordons";
			const std::string path = ChildPath( "", name, singleton ? -1 : index[i] );
			if ( singleton && !seen.insert( name == "cordons" ? "cordon" : name ).second )
			{
				return Fail(
				    path, "a second '" + name + "' block (the map holds one)", Line( block ) );
			}
			bool ok = true;
			if ( name == "versioninfo" )
			{
				ok = DecodeVersion( block, path );
			}
			else if ( name == "viewsettings" )
			{
				ok = DecodeView( block, path );
			}
			else if ( name == "visgroups" )
			{
				ok = DecodeVisgroups( block, path );
			}
			else if ( name == "world" )
			{
				ok = DecodeWorld( block, path );
			}
			else if ( name == "entity" )
			{
				ok = DecodeEntity( block, path, false );
			}
			else if ( name == "hidden" )
			{
				ok = DecodeTopHidden( block, path );
			}
			else if ( name == "cameras" )
			{
				ok = DecodeCameras( block, path );
			}
			else if ( name == "cordon" )
			{
				ok = DecodeSingleCordon( block, path );
			}
			else if ( name == "cordons" )
			{
				ok = DecodeCordonList( block, path );
			}
			else
			{
				ok = Fail( path, "unknown top-level block '" + name + "'", Line( block ) );
			}
			if ( !ok )
			{
				return false;
			}
		}
		return true;
	}

private:
	int Line( const KeyValueNode &node ) const { return m_lines.Line( node ); }
	int PairLine( const KeyValueNode &node, std::size_t pair ) const
	{
		return m_lines.PairLine( node, pair );
	}

	bool UnknownKey( const KeyValueNode &node, const std::string &path, std::size_t pair )
	{
		return Fail( path, "unknown key '" + node.pairs[pair].key + "'", PairLine( node, pair ) );
	}

	bool Malformed(
	    const KeyValueNode &node, const std::string &path, std::size_t pair, const char *expected )
	{
		const KeyValue &kv = node.pairs[pair];
		return Fail( path, "key '" + kv.key + "' value \"" + kv.value + "\" is not " + expected,
		    PairLine( node, pair ) );
	}

	// False (with an error) when 'key' was already seen in this block.
	bool Once( std::set<std::string> &seen, const KeyValueNode &node, const std::string &path,
	    std::size_t pair )
	{
		if ( !seen.insert( node.pairs[pair].key ).second )
		{
			return Fail(
			    path, "duplicate key '" + node.pairs[pair].key + "'", PairLine( node, pair ) );
		}
		return true;
	}

	bool NoChildren( const KeyValueNode &node, const std::string &path )
	{
		if ( !node.children.empty() )
		{
			return Fail( path, "unknown block '" + node.children.front().name + "'",
			    Line( node.children.front() ) );
		}
		return true;
	}

	template <typename T, typename Parse>
	bool Read( const KeyValueNode &node, const std::string &path, std::size_t pair, T &into,
	    Parse parse, const char *expected )
	{
		auto value = parse( node.pairs[pair].value );
		if ( !value )
		{
			return Malformed( node, path, pair, expected );
		}
		into = *value;
		return true;
	}

	bool DecodeVersion( const KeyValueNode &node, const std::string &path )
	{
		VersionInfo &v = settings.version;
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const std::string &key = node.pairs[i].key;
			if ( !Once( seen, node, path, i ) )
			{
				return false;
			}
			bool ok = true;
			if ( key == "editorversion" )
			{
				ok = Read( node, path, i, v.editorVersion, ParseInt, "an integer" );
			}
			else if ( key == "editorbuild" )
			{
				ok = Read( node, path, i, v.editorBuild, ParseInt, "an integer" );
			}
			else if ( key == "mapversion" )
			{
				ok = Read( node, path, i, v.mapVersion, ParseInt, "an integer" );
			}
			else if ( key == "formatversion" )
			{
				ok = Read( node, path, i, v.formatVersion, ParseInt, "an integer" );
			}
			else if ( key == "prefab" )
			{
				ok = Read( node, path, i, v.prefab, ParseFlag, "0 or 1" );
			}
			else
			{
				ok = UnknownKey( node, path, i );
			}
			if ( !ok )
			{
				return false;
			}
		}
		return NoChildren( node, path );
	}

	bool DecodeView( const KeyValueNode &node, const std::string &path )
	{
		ViewSettings &v = settings.view;
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const std::string &key = node.pairs[i].key;
			if ( !Once( seen, node, path, i ) )
			{
				return false;
			}
			bool ok = true;
			if ( key == "bSnapToGrid" )
			{
				ok = Read( node, path, i, v.snapToGrid, ParseFlag, "0 or 1" );
			}
			else if ( key == "bShowGrid" )
			{
				ok = Read( node, path, i, v.showGrid, ParseFlag, "0 or 1" );
			}
			else if ( key == "bShowLogicalGrid" )
			{
				ok = Read( node, path, i, v.showLogicalGrid, ParseFlag, "0 or 1" );
			}
			else if ( key == "nGridSpacing" )
			{
				ok = Read( node, path, i, v.gridSpacing, ParseInt, "an integer" );
			}
			else if ( key == "bShow3DGrid" )
			{
				ok = Read( node, path, i, v.show3DGrid, ParseFlag, "0 or 1" );
			}
			else
			{
				ok = UnknownKey( node, path, i );
			}
			if ( !ok )
			{
				return false;
			}
		}
		return NoChildren( node, path );
	}

	bool DecodeVisgroup( const KeyValueNode &node, const std::string &path, Visgroup &out )
	{
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const std::string &key = node.pairs[i].key;
			if ( !Once( seen, node, path, i ) )
			{
				return false;
			}
			bool ok = true;
			if ( key == "name" )
			{
				out.name = node.pairs[i].value;
			}
			else if ( key == "visgroupid" )
			{
				ok = Read( node, path, i, out.id, ParseInt, "an integer" );
			}
			else if ( key == "color" )
			{
				ok = Read( node, path, i, out.color, ParseRgb, "three integers" );
			}
			else
			{
				ok = UnknownKey( node, path, i );
			}
			if ( !ok )
			{
				return false;
			}
		}
		if ( !seen.count( "name" ) || !seen.count( "visgroupid" ) )
		{
			return Fail( path, "a visgroup needs a name and a visgroupid", Line( node ) );
		}
		return DecodeVisgroupChildren( node, path, out.children );
	}

	bool DecodeVisgroupChildren(
	    const KeyValueNode &node, const std::string &path, std::vector<Visgroup> &into )
	{
		const std::vector<int> index = SiblingIndices( node );
		for ( std::size_t i = 0; i < node.children.size(); ++i )
		{
			const KeyValueNode &child = node.children[i];
			const std::string childPath = ChildPath( path, child.name, index[i] );
			if ( child.name != "visgroup" )
			{
				return Fail( childPath, "unknown block '" + child.name + "'", Line( child ) );
			}
			Visgroup v;
			if ( !DecodeVisgroup( child, childPath, v ) )
			{
				return false;
			}
			into.push_back( std::move( v ) );
		}
		return true;
	}

	bool DecodeVisgroups( const KeyValueNode &node, const std::string &path )
	{
		if ( !node.pairs.empty() )
		{
			return UnknownKey( node, path, 0 );
		}
		return DecodeVisgroupChildren( node, path, settings.visgroups );
	}

	bool DecodeCameras( const KeyValueNode &node, const std::string &path )
	{
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			if ( !Once( seen, node, path, i ) )
			{
				return false;
			}
			if ( node.pairs[i].key != "activecamera" )
			{
				return UnknownKey( node, path, i );
			}
			if ( !Read( node, path, i, settings.activeCamera, ParseInt, "an integer" ) )
			{
				return false;
			}
		}
		const std::vector<int> index = SiblingIndices( node );
		for ( std::size_t c = 0; c < node.children.size(); ++c )
		{
			const KeyValueNode &child = node.children[c];
			const std::string childPath = ChildPath( path, child.name, index[c] );
			if ( child.name != "camera" )
			{
				return Fail( childPath, "unknown block '" + child.name + "'", Line( child ) );
			}
			CameraBookmark camera;
			std::set<std::string> keys;
			for ( std::size_t i = 0; i < child.pairs.size(); ++i )
			{
				const std::string &key = child.pairs[i].key;
				if ( !Once( keys, child, childPath, i ) )
				{
					return false;
				}
				bool ok = true;
				auto vec = []( std::string_view t )
				{
					return ParseEnclosedVec3( t, '[', ']' );
				};
				if ( key == "position" )
				{
					ok = Read( child, childPath, i, camera.position, vec, "\"[x y z]\"" );
				}
				else if ( key == "look" )
				{
					ok = Read( child, childPath, i, camera.look, vec, "\"[x y z]\"" );
				}
				else
				{
					ok = UnknownKey( child, childPath, i );
				}
				if ( !ok )
				{
					return false;
				}
			}
			if ( keys.size() != 2 )
			{
				return Fail( childPath, "a camera needs a position and a look", Line( child ) );
			}
			if ( !NoChildren( child, childPath ) )
			{
				return false;
			}
			settings.cameras.push_back( camera );
		}
		return true;
	}

	// mins/maxs (and, where allowed, active and name) of a cordon box block.
	bool DecodeBox( const KeyValueNode &node, const std::string &path, CordonBox &box,
	    std::set<std::string> &seen, bool *active, std::string *name )
	{
		auto point = []( std::string_view t )
		{
			return ParseEnclosedVec3( t, '(', ')' );
		};
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const std::string &key = node.pairs[i].key;
			if ( !Once( seen, node, path, i ) )
			{
				return false;
			}
			bool ok = true;
			if ( key == "mins" )
			{
				ok = Read( node, path, i, box.mins, point, "\"(x y z)\"" );
			}
			else if ( key == "maxs" )
			{
				ok = Read( node, path, i, box.maxs, point, "\"(x y z)\"" );
			}
			else if ( key == "active" && active )
			{
				ok = Read( node, path, i, *active, ParseFlag, "0 or 1" );
			}
			else if ( key == "name" && name )
			{
				*name = node.pairs[i].value;
			}
			else
			{
				ok = UnknownKey( node, path, i );
			}
			if ( !ok )
			{
				return false;
			}
		}
		return true;
	}

	bool DecodeSingleCordon( const KeyValueNode &node, const std::string &path )
	{
		Cordon cordon;
		CordonBox box;
		std::set<std::string> seen;
		bool active = false;
		if ( !DecodeBox( node, path, box, seen, &active, nullptr ) || !NoChildren( node, path ) )
		{
			return false;
		}
		if ( !seen.count( "mins" ) || !seen.count( "maxs" ) )
		{
			return Fail( path, "a cordon needs mins and maxs", Line( node ) );
		}
		cordon.active = active;
		cordon.boxes.push_back( box );
		settings.cordons = { cordon };
		settings.cordonsActive = active;
		settings.cordonForm = DocumentSettings::CordonForm::Single;
		return true;
	}

	bool DecodeCordonList( const KeyValueNode &node, const std::string &path )
	{
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			if ( !Once( seen, node, path, i ) )
			{
				return false;
			}
			if ( node.pairs[i].key != "active" )
			{
				return UnknownKey( node, path, i );
			}
			if ( !Read( node, path, i, settings.cordonsActive, ParseFlag, "0 or 1" ) )
			{
				return false;
			}
		}
		const std::vector<int> index = SiblingIndices( node );
		for ( std::size_t c = 0; c < node.children.size(); ++c )
		{
			const KeyValueNode &child = node.children[c];
			const std::string childPath = ChildPath( path, child.name, index[c] );
			if ( child.name != "cordon" )
			{
				return Fail( childPath, "unknown block '" + child.name + "'", Line( child ) );
			}
			Cordon cordon;
			std::set<std::string> keys;
			CordonBox unused;
			if ( !DecodeBox( child, childPath, unused, keys, &cordon.active, &cordon.name ) )
			{
				return false;
			}
			if ( keys.count( "mins" ) || keys.count( "maxs" ) )
			{
				return Fail(
				    childPath, "a listed cordon holds its extents in 'box' blocks", Line( child ) );
			}
			const std::vector<int> boxIndex = SiblingIndices( child );
			for ( std::size_t b = 0; b < child.children.size(); ++b )
			{
				const KeyValueNode &boxNode = child.children[b];
				const std::string boxPath = ChildPath( childPath, boxNode.name, boxIndex[b] );
				if ( boxNode.name != "box" )
				{
					return Fail( boxPath, "unknown block '" + boxNode.name + "'", Line( boxNode ) );
				}
				CordonBox box;
				std::set<std::string> boxKeys;
				if ( !DecodeBox( boxNode, boxPath, box, boxKeys, nullptr, nullptr ) ||
				     !NoChildren( boxNode, boxPath ) )
				{
					return false;
				}
				if ( boxKeys.size() != 2 )
				{
					return Fail( boxPath, "a box needs mins and maxs", Line( boxNode ) );
				}
				cordon.boxes.push_back( box );
			}
			settings.cordons.push_back( std::move( cordon ) );
		}
		settings.cordonForm = DocumentSettings::CordonForm::List;
		return true;
	}

	bool DecodeEditor( const KeyValueNode &node, const std::string &path, EditorInfo &info,
	    std::optional<GroupRef> &group )
	{
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const std::string &key = node.pairs[i].key;
			if ( key != "visgroupid" && !Once( seen, node, path, i ) )
			{
				return false;
			}
			bool ok = true;
			if ( key == "color" )
			{
				ok = Read( node, path, i, info.color, ParseRgb, "three integers" );
			}
			else if ( key == "visgroupid" )
			{
				int id = 0;
				ok = Read( node, path, i, id, ParseInt, "an integer" );
				info.visgroupIds.push_back( id );
			}
			else if ( key == "groupid" )
			{
				std::uint32_t id = 0;
				ok = Read( node, path, i, id, ParseId, "an id" );
				group = GroupRef{ id, path, PairLine( node, i ) };
			}
			else if ( key == "visgroupshown" )
			{
				ok = Read( node, path, i, info.visgroupShown, ParseFlag, "0 or 1" );
			}
			else if ( key == "visgroupautoshown" )
			{
				ok = Read( node, path, i, info.visgroupAutoShown, ParseFlag, "0 or 1" );
			}
			else if ( key == "logicalpos" )
			{
				ok = Read( node, path, i, info.logicalPos, ParseLogicalPos, "\"[x y]\" integers" );
			}
			else if ( key == "comments" )
			{
				info.comments = node.pairs[i].value;
			}
			else
			{
				ok = UnknownKey( node, path, i );
			}
			if ( !ok )
			{
				return false;
			}
		}
		return NoChildren( node, path );
	}

	bool ReadId( const KeyValueNode &node, const std::string &path, std::size_t pair,
	    std::uint32_t &into, bool &hasId )
	{
		hasId = Read( node, path, pair, into, ParseId, "an id" );
		return hasId;
	}

	// A block of 'rows' pairs "row0".."rowN-1", each holding 'perRow' numbers.
	template <typename T, typename Parse>
	bool DecodeRows( const KeyValueNode &node, const std::string &path, int rows, int perRow,
	    Parse parse, std::vector<T> &out )
	{
		if ( !NoChildren( node, path ) )
		{
			return false;
		}
		if ( node.pairs.size() != static_cast<std::size_t>( rows ) )
		{
			return Fail( path,
			    "has " + std::to_string( node.pairs.size() ) + " rows; the power needs " +
			        std::to_string( rows ),
			    Line( node ) );
		}
		for ( int r = 0; r < rows; ++r )
		{
			const KeyValue &kv = node.pairs[static_cast<std::size_t>( r )];
			const int line = PairLine( node, static_cast<std::size_t>( r ) );
			if ( kv.key != "row" + std::to_string( r ) )
			{
				return Fail( path,
				    "expected key 'row" + std::to_string( r ) + "', found '" + kv.key + "'", line );
			}
			auto values = parse( kv.value );
			if ( !values || values->size() != static_cast<std::size_t>( perRow ) )
			{
				return Fail( path,
				    kv.key + " needs " + std::to_string( perRow ) + " numbers" +
				        ( values ? ", found " + std::to_string( values->size() ) : std::string() ),
				    line );
			}
			out.insert( out.end(), values->begin(), values->end() );
		}
		return true;
	}

	bool DecodeVectorRows( const KeyValueNode &node, const std::string &path, int verts,
	    std::optional<std::vector<Vec3d>> &into )
	{
		std::vector<double> flat;
		if ( !DecodeRows( node, path, verts, verts * 3, ParseNumbers, flat ) )
		{
			return false;
		}
		std::vector<Vec3d> out;
		for ( std::size_t i = 0; i + 2 < flat.size(); i += 3 )
		{
			out.push_back( Vec3d( flat[i], flat[i + 1], flat[i + 2] ) );
		}
		into = std::move( out );
		return true;
	}

	bool DecodeDisplacement( const KeyValueNode &node, const std::string &path, Displacement &d )
	{
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const std::string &key = node.pairs[i].key;
			if ( !Once( seen, node, path, i ) )
			{
				return false;
			}
			bool ok = true;
			if ( key == "power" )
			{
				ok = Read( node, path, i, d.power, ParseInt, "an integer" );
				if ( ok && ( d.power < 1 || d.power > 4 ) )
				{
					ok = Malformed( node, path, i, "a displacement power from 1 to 4" );
				}
			}
			else if ( key == "startposition" )
			{
				ok = Read(
				    node, path, i, d.startPosition,
				    []( std::string_view t )
				    {
					    return ParseEnclosedVec3( t, '[', ']' );
				    },
				    "\"[x y z]\"" );
			}
			else if ( key == "flags" )
			{
				ok = Read( node, path, i, d.flags, ParseInt, "an integer" );
			}
			else if ( key == "elevation" )
			{
				ok = Read( node, path, i, d.elevation, ParseNumber, "a number" );
			}
			else if ( key == "subdiv" )
			{
				ok = Read( node, path, i, d.subdivided, ParseFlag, "0 or 1" );
			}
			else if ( key == "mintess" )
			{
				ok = Read( node, path, i, d.minTess, ParseInt, "an integer" );
			}
			else if ( key == "smooth" )
			{
				ok = Read( node, path, i, d.smoothingAngle, ParseNumber, "a number" );
			}
			else if ( key == "alpha" )
			{
				std::vector<double> v;
				ok = Read(
				    node, path, i, v,
				    []( std::string_view t )
				    {
					    return ParseEnclosed( t, '[', ']', 4 );
				    },
				    "\"[a b c d]\"" );
				if ( ok )
				{
					d.cornerAlphas = std::array<double, 4>{ v[0], v[1], v[2], v[3] };
				}
			}
			else
			{
				ok = UnknownKey( node, path, i );
			}
			if ( !ok )
			{
				return false;
			}
		}
		if ( !seen.count( "power" ) || !seen.count( "startposition" ) )
		{
			return Fail( path, "a displacement needs a power and a startposition", Line( node ) );
		}
		const int verts = d.VertsPerRow();
		const int quads = d.QuadsPerRow();
		std::set<std::string> blocks;
		for ( const KeyValueNode &child : node.children )
		{
			const std::string childPath = ChildPath( path, child.name, -1 );
			if ( !blocks.insert( child.name ).second )
			{
				return Fail( childPath, "duplicate block '" + child.name + "'", Line( child ) );
			}
			bool ok = true;
			if ( child.name == "normals" )
			{
				ok = DecodeVectorRows( child, childPath, verts, d.normals );
			}
			else if ( child.name == "offsets" )
			{
				ok = DecodeVectorRows( child, childPath, verts, d.offsets );
			}
			else if ( child.name == "offset_normals" )
			{
				ok = DecodeVectorRows( child, childPath, verts, d.offsetNormals );
			}
			else if ( child.name == "distances" || child.name == "alphas" )
			{
				std::vector<double> values;
				ok = DecodeRows( child, childPath, verts, verts, ParseNumbers, values );
				( child.name == "distances" ? d.distances : d.alphas ) = std::move( values );
			}
			else if ( child.name == "triangle_tags" )
			{
				std::vector<std::int64_t> tags;
				ok = DecodeRows( child, childPath, quads, quads * 2, ParseIntegers, tags );
				std::vector<int> out;
				for ( const std::int64_t tag : tags )
				{
					if ( tag < std::numeric_limits<int>::min() ||
					     tag > std::numeric_limits<int>::max() )
					{
						return Fail( childPath, "a triangle tag is out of range", Line( child ) );
					}
					out.push_back( static_cast<int>( tag ) );
				}
				d.triangleTags = std::move( out );
			}
			else if ( child.name == "allowed_verts" )
			{
				ok = DecodeAllowedVerts( child, childPath, d );
			}
			else
			{
				ok = Fail( childPath, "unknown block '" + child.name + "'", Line( child ) );
			}
			if ( !ok )
			{
				return false;
			}
		}
		return true;
	}

	// One pair whose key is the number of words it holds ("10" in legacy files).
	bool DecodeAllowedVerts( const KeyValueNode &node, const std::string &path, Displacement &d )
	{
		if ( !NoChildren( node, path ) )
		{
			return false;
		}
		if ( node.pairs.size() != 1 )
		{
			return Fail( path, "needs exactly one key (the word count)", Line( node ) );
		}
		const std::optional<std::vector<std::int64_t>> words = ParseIntegers( node.pairs[0].value );
		if ( !words )
		{
			return Malformed( node, path, 0, "integers" );
		}
		if ( node.pairs[0].key != std::to_string( words->size() ) )
		{
			return Fail( path,
			    "key '" + node.pairs[0].key + "' is not the word count " +
			        std::to_string( words->size() ),
			    PairLine( node, 0 ) );
		}
		d.allowedVerts = *words;
		return true;
	}

	bool DecodeSide( const KeyValueNode &node, const std::string &path, Side &side, bool &hasId )
	{
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const std::string &key = node.pairs[i].key;
			if ( !Once( seen, node, path, i ) )
			{
				return false;
			}
			FaceTexture &t = side.texture;
			bool ok = true;
			if ( key == "id" )
			{
				ok = ReadId( node, path, i, side.vmfId, hasId );
			}
			else if ( key == "plane" )
			{
				ok = Read( node, path, i, side.points, ParsePlane, "\"(x y z) (x y z) (x y z)\"" );
			}
			else if ( key == "material" )
			{
				t.material = node.pairs[i].value;
			}
			else if ( key == "uaxis" )
			{
				ok = Read( node, path, i, t.u, ParseAxis, "\"[x y z shift] scale\"" );
			}
			else if ( key == "vaxis" )
			{
				ok = Read( node, path, i, t.v, ParseAxis, "\"[x y z shift] scale\"" );
			}
			else if ( key == "rotation" )
			{
				ok = Read( node, path, i, t.rotation, ParseNumber, "a number" );
			}
			else if ( key == "lightmapscale" )
			{
				ok = Read( node, path, i, t.lightmapScale, ParseNumber, "a number" );
			}
			else if ( key == "smoothing_groups" )
			{
				ok = Read( node, path, i, t.smoothingGroups, ParseMask, "a 32-bit integer" );
			}
			else
			{
				ok = UnknownKey( node, path, i );
			}
			if ( !ok )
			{
				return false;
			}
		}
		if ( !seen.count( "plane" ) )
		{
			return Fail( path, "a side needs a plane", Line( node ) );
		}
		for ( const KeyValueNode &child : node.children )
		{
			const std::string childPath = ChildPath( path, child.name, -1 );
			if ( child.name != "dispinfo" )
			{
				return Fail( childPath, "unknown block '" + child.name + "'", Line( child ) );
			}
			if ( side.displacement )
			{
				return Fail( childPath, "duplicate block 'dispinfo'", Line( child ) );
			}
			Displacement d;
			if ( !DecodeDisplacement( child, childPath, d ) )
			{
				return false;
			}
			side.displacement = std::move( d );
		}
		return true;
	}

	std::optional<PendingSolid> DecodeSolid(
	    const KeyValueNode &node, const std::string &path, bool hidden )
	{
		PendingSolid pending;
		pending.where = { path, Line( node ) };
		pending.solid.hidden = hidden;
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			if ( !Once( seen, node, path, i ) )
			{
				return std::nullopt;
			}
			if ( node.pairs[i].key != "id" )
			{
				UnknownKey( node, path, i );
				return std::nullopt;
			}
			if ( !ReadId( node, path, i, pending.solid.vmfId, pending.hasId ) )
			{
				return std::nullopt;
			}
		}
		const std::vector<int> index = SiblingIndices( node );
		bool sawEditor = false;
		for ( std::size_t c = 0; c < node.children.size(); ++c )
		{
			const KeyValueNode &child = node.children[c];
			if ( child.name == "side" )
			{
				const std::string sidePath = ChildPath( path, "side", index[c] );
				Side side;
				bool sideHasId = false;
				if ( !DecodeSide( child, sidePath, side, sideHasId ) )
				{
					return std::nullopt;
				}
				pending.solid.sides.push_back( std::move( side ) );
				pending.sideHasId.push_back( sideHasId );
				pending.sideWhere.push_back( { sidePath, Line( child ) } );
			}
			else if ( child.name == "editor" && !sawEditor )
			{
				sawEditor = true;
				if ( !DecodeEditor( child, ChildPath( path, "editor", -1 ), pending.solid.editor,
				         pending.group ) )
				{
					return std::nullopt;
				}
			}
			else
			{
				Fail( ChildPath( path, child.name, index[c] ), "unknown block '" + child.name + "'",
				    Line( child ) );
				return std::nullopt;
			}
		}
		if ( pending.solid.sides.size() < 4 )
		{
			Fail( path,
			    "a solid needs at least four sides; it has " +
			        std::to_string( pending.solid.sides.size() ),
			    Line( node ) );
			return std::nullopt;
		}
		return pending;
	}

	bool DecodeGroup( const KeyValueNode &node, const std::string &path, bool hidden )
	{
		PendingGroup pending;
		pending.where = { path, Line( node ) };
		pending.group.hidden = hidden;
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			if ( !Once( seen, node, path, i ) )
			{
				return false;
			}
			if ( node.pairs[i].key != "id" )
			{
				return UnknownKey( node, path, i );
			}
			if ( !ReadId( node, path, i, pending.group.vmfId, pending.hasId ) )
			{
				return false;
			}
		}
		bool sawEditor = false;
		for ( const KeyValueNode &child : node.children )
		{
			if ( child.name != "editor" || sawEditor )
			{
				return Fail( ChildPath( path, child.name, -1 ),
				    "unknown block '" + child.name + "'", Line( child ) );
			}
			sawEditor = true;
			if ( !DecodeEditor( child, ChildPath( path, "editor", -1 ), pending.group.editor,
			         pending.parent ) )
			{
				return false;
			}
		}
		groups.push_back( std::move( pending ) );
		return true;
	}

	bool DecodeWorld( const KeyValueNode &node, const std::string &path )
	{
		sawWorld = true;
		settings.worldKeys.clear();
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const KeyValue &kv = node.pairs[i];
			if ( kv.key == "id" || kv.key == "classname" )
			{
				if ( !Once( seen, node, path, i ) )
				{
					return false;
				}
				if ( kv.key == "id" )
				{
					if ( !Read( node, path, i, settings.worldVmfId, ParseId, "an id" ) )
					{
						return false;
					}
				}
				else if ( kv.value != "worldspawn" )
				{
					return Malformed( node, path, i, "worldspawn" );
				}
				continue;
			}
			settings.worldKeys.push_back( kv );
		}
		const std::vector<int> index = SiblingIndices( node );
		for ( std::size_t c = 0; c < node.children.size(); ++c )
		{
			const KeyValueNode &child = node.children[c];
			const std::string childPath = ChildPath( path, child.name, index[c] );
			if ( child.name == "solid" )
			{
				if ( !WorldSolid( child, childPath, false ) )
				{
					return false;
				}
			}
			else if ( child.name == "group" )
			{
				if ( !DecodeGroup( child, childPath, false ) )
				{
					return false;
				}
			}
			else if ( child.name == "hidden" )
			{
				if ( !child.pairs.empty() )
				{
					return UnknownKey( child, childPath, 0 );
				}
				const std::vector<int> inner = SiblingIndices( child );
				for ( std::size_t h = 0; h < child.children.size(); ++h )
				{
					const KeyValueNode &object = child.children[h];
					const std::string objectPath = ChildPath( childPath, object.name, inner[h] );
					bool ok = true;
					if ( object.name == "solid" )
					{
						ok = WorldSolid( object, objectPath, true );
					}
					else if ( object.name == "group" )
					{
						ok = DecodeGroup( object, objectPath, true );
					}
					else
					{
						ok = Fail(
						    objectPath, "unknown block '" + object.name + "'", Line( object ) );
					}
					if ( !ok )
					{
						return false;
					}
				}
			}
			else
			{
				return Fail( childPath, "unknown block '" + child.name + "'", Line( child ) );
			}
		}
		return true;
	}

	bool WorldSolid( const KeyValueNode &node, const std::string &path, bool hidden )
	{
		std::optional<PendingSolid> solid = DecodeSolid( node, path, hidden );
		if ( !solid )
		{
			return false;
		}
		worldSolids.push_back( std::move( *solid ) );
		return true;
	}

	bool DecodeConnections( const KeyValueNode &node, const std::string &path, Entity &entity )
	{
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const KeyValue &kv = node.pairs[i];
			std::optional<Connection> c = hammer::scene::ParseConnection( kv.key, kv.value );
			if ( !c )
			{
				return Fail( path,
				    "connection '" + kv.key + "' \"" + kv.value +
				        "\" is not target,input,parameter,delay,times",
				    PairLine( node, i ) );
			}
			entity.connections.push_back( std::move( *c ) );
		}
		return NoChildren( node, path );
	}

	bool DecodeEntity( const KeyValueNode &node, const std::string &path, bool hidden )
	{
		PendingEntity pending;
		pending.where = { path, Line( node ) };
		Entity &entity = pending.entity;
		entity.hidden = hidden;
		std::set<std::string> seen;
		for ( std::size_t i = 0; i < node.pairs.size(); ++i )
		{
			const KeyValue &kv = node.pairs[i];
			if ( kv.key == "id" || kv.key == "classname" )
			{
				if ( !Once( seen, node, path, i ) )
				{
					return false;
				}
				if ( kv.key == "id" )
				{
					if ( !ReadId( node, path, i, entity.vmfId, pending.hasId ) )
					{
						return false;
					}
				}
				else
				{
					entity.classname = kv.value;
				}
				continue;
			}
			entity.keys.push_back( kv );
		}
		if ( !seen.count( "classname" ) )
		{
			return Fail( path, "an entity needs a classname", Line( node ) );
		}
		const std::vector<int> index = SiblingIndices( node );
		bool sawEditor = false;
		for ( std::size_t c = 0; c < node.children.size(); ++c )
		{
			const KeyValueNode &child = node.children[c];
			const std::string childPath = ChildPath( path, child.name, index[c] );
			bool ok = true;
			if ( child.name == "connections" )
			{
				ok = DecodeConnections( child, childPath, entity );
			}
			else if ( child.name == "solid" )
			{
				ok = EntitySolid( pending, child, childPath, false );
			}
			else if ( child.name == "hidden" )
			{
				if ( !child.pairs.empty() )
				{
					return UnknownKey( child, childPath, 0 );
				}
				const std::vector<int> inner = SiblingIndices( child );
				for ( std::size_t h = 0; h < child.children.size() && ok; ++h )
				{
					const KeyValueNode &object = child.children[h];
					const std::string objectPath = ChildPath( childPath, object.name, inner[h] );
					ok = object.name == "solid"
					         ? EntitySolid( pending, object, objectPath, true )
					         : Fail( objectPath, "unknown block '" + object.name + "'",
					               Line( object ) );
				}
			}
			else if ( child.name == "editor" && !sawEditor )
			{
				sawEditor = true;
				ok = DecodeEditor(
				    child, ChildPath( path, "editor", -1 ), entity.editor, pending.group );
			}
			else
			{
				ok = Fail( childPath, "unknown block '" + child.name + "'", Line( child ) );
			}
			if ( !ok )
			{
				return false;
			}
		}
		entities.push_back( std::move( pending ) );
		return true;
	}

	bool EntitySolid(
	    PendingEntity &entity, const KeyValueNode &node, const std::string &path, bool hidden )
	{
		std::optional<PendingSolid> solid = DecodeSolid( node, path, hidden );
		if ( !solid )
		{
			return false;
		}
		entity.solids.push_back( std::move( *solid ) );
		return true;
	}

	bool DecodeTopHidden( const KeyValueNode &node, const std::string &path )
	{
		if ( !node.pairs.empty() )
		{
			return UnknownKey( node, path, 0 );
		}
		const std::vector<int> index = SiblingIndices( node );
		for ( std::size_t c = 0; c < node.children.size(); ++c )
		{
			const KeyValueNode &child = node.children[c];
			const std::string childPath = ChildPath( path, child.name, index[c] );
			if ( child.name != "entity" )
			{
				return Fail( childPath, "unknown block '" + child.name + "'", Line( child ) );
			}
			if ( !DecodeEntity( child, childPath, true ) )
			{
				return false;
			}
		}
		return true;
	}

	const SourceLines &m_lines;
};

// --- Encoding --------------------------------------------------------------------------

void Add( KeyValueNode &node, std::string key, std::string value )
{
	node.pairs.push_back( { std::move( key ), std::move( value ) } );
}

KeyValueNode Wrap( std::string name, KeyValueNode inner )
{
	KeyValueNode wrapper;
	wrapper.name = std::move( name );
	wrapper.children.push_back( std::move( inner ) );
	return wrapper;
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

	// Legacy CMapClass::SaveVMF order; every object gets its editor block.
	KeyValueNode EncodeEditor( const EditorInfo &info, ObjectId group, const char *what )
	{
		KeyValueNode editor;
		editor.name = "editor";
		if ( info.color )
		{
			Add( editor, "color", FormatRgb( *info.color ) );
		}
		if ( const std::optional<std::uint32_t> groupId = GroupVmfId( group, what ) )
		{
			Add( editor, "groupid", std::to_string( *groupId ) );
		}
		for ( const int id : info.visgroupIds )
		{
			Add( editor, "visgroupid", std::to_string( id ) );
		}
		Add( editor, "visgroupshown", info.visgroupShown ? "1" : "0" );
		Add( editor, "visgroupautoshown", info.visgroupAutoShown ? "1" : "0" );
		if ( info.comments )
		{
			Add( editor, "comments", *info.comments );
		}
		if ( info.logicalPos )
		{
			Add( editor, "logicalpos",
			    "[" + std::to_string( ( *info.logicalPos )[0] ) + " " +
			        std::to_string( ( *info.logicalPos )[1] ) + "]" );
		}
		return editor;
	}

	template <typename T, typename Format>
	void AddRows( KeyValueNode &parent, const char *name,
	    const std::optional<std::vector<T>> &values, int rows, int perRow, Format format )
	{
		if ( !values )
		{
			return;
		}
		if ( values->size() !=
		     static_cast<std::size_t>( rows ) * static_cast<std::size_t>( perRow ) )
		{
			Fail( std::string( "displacement " ) + name + " has " +
			      std::to_string( values->size() ) + " entries; its power needs " +
			      std::to_string( rows * perRow ) );
			return;
		}
		KeyValueNode block;
		block.name = name;
		for ( int r = 0; r < rows; ++r )
		{
			std::string row;
			for ( int c = 0; c < perRow; ++c )
			{
				row += ( c ? " " : "" ) +
				       format( ( *values )[static_cast<std::size_t>( r * perRow + c )] );
			}
			Add( block, "row" + std::to_string( r ), row );
		}
		parent.children.push_back( std::move( block ) );
	}

	// Legacy CMapDisp::SaveVMF layout (with the pre-release keys where present).
	KeyValueNode EncodeDisplacement( const Displacement &d )
	{
		KeyValueNode node;
		node.name = "dispinfo";
		if ( d.power < 1 || d.power > 4 )
		{
			Fail( "displacement power " + std::to_string( d.power ) + " is not 1 to 4" );
			return node;
		}
		Add( node, "power", std::to_string( d.power ) );
		Add( node, "startposition", "[" + Vec( d.startPosition ) + "]" );
		Add( node, "flags", std::to_string( d.flags ) );
		if ( d.minTess )
		{
			Add( node, "mintess", std::to_string( *d.minTess ) );
		}
		if ( d.smoothingAngle )
		{
			Add( node, "smooth", Number( *d.smoothingAngle ) );
		}
		if ( d.cornerAlphas )
		{
			const std::array<double, 4> &a = *d.cornerAlphas;
			Add( node, "alpha",
			    "[" + Number( a[0] ) + " " + Number( a[1] ) + " " + Number( a[2] ) + " " +
			        Number( a[3] ) + "]" );
		}
		Add( node, "elevation", Number( d.elevation ) );
		Add( node, "subdiv", d.subdivided ? "1" : "0" );
		const int verts = d.VertsPerRow();
		const int quads = d.QuadsPerRow();
		auto vec = [this]( const Vec3d &v )
		{
			return Vec( v );
		};
		auto num = [this]( double v )
		{
			return Number( v );
		};
		auto integer = []( int v )
		{
			return std::to_string( v );
		};
		AddRows( node, "normals", d.normals, verts, verts, vec );
		AddRows( node, "distances", d.distances, verts, verts, num );
		AddRows( node, "offsets", d.offsets, verts, verts, vec );
		AddRows( node, "offset_normals", d.offsetNormals, verts, verts, vec );
		AddRows( node, "alphas", d.alphas, verts, verts, num );
		AddRows( node, "triangle_tags", d.triangleTags, quads, quads * 2, integer );
		if ( d.allowedVerts )
		{
			KeyValueNode block;
			block.name = "allowed_verts";
			std::string words;
			for ( std::size_t i = 0; i < d.allowedVerts->size(); ++i )
			{
				words += ( i ? " " : "" ) + std::to_string( ( *d.allowedVerts )[i] );
			}
			Add( block, std::to_string( d.allowedVerts->size() ), words );
			node.children.push_back( std::move( block ) );
		}
		return node;
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
		const hammer::scene::FaceTexture &t = side.texture;
		if ( !IsFinite( t.u.axis ) || !IsFinite( t.v.axis ) || !std::isfinite( t.u.shift ) ||
		     !std::isfinite( t.v.shift ) || !std::isfinite( t.u.scale ) ||
		     !std::isfinite( t.v.scale ) )
		{
			Fail( "a texture axis is not finite" );
		}
		Add( node, "id", std::to_string( side.vmfId ) );
		Add( node, "plane", FormatPlane( side.points ) );
		Add( node, "material", t.material );
		Add( node, "uaxis", FormatAxis( t.u ) );
		Add( node, "vaxis", FormatAxis( t.v ) );
		Add( node, "rotation", Number( t.rotation ) );
		Add( node, "lightmapscale", Number( t.lightmapScale ) );
		Add( node, "smoothing_groups",
		    std::to_string( static_cast<std::int32_t>( t.smoothingGroups ) ) );
		if ( side.displacement )
		{
			node.children.push_back( EncodeDisplacement( *side.displacement ) );
		}
		return node;
	}

	KeyValueNode EncodeSolid( const Solid &solid )
	{
		KeyValueNode node;
		node.name = "solid";
		Add( node, "id", std::to_string( solid.vmfId ) );
		for ( const Side &side : solid.sides )
		{
			node.children.push_back( EncodeSide( side ) );
		}
		node.children.push_back( EncodeEditor( solid.editor, solid.group, "a solid" ) );
		return solid.hidden ? Wrap( "hidden", std::move( node ) ) : node;
	}

	KeyValueNode EncodeGroup( const Group &group )
	{
		KeyValueNode node;
		node.name = "group";
		Add( node, "id", std::to_string( group.vmfId ) );
		node.children.push_back( EncodeEditor( group.editor, group.group, "a group" ) );
		return group.hidden ? Wrap( "hidden", std::move( node ) ) : node;
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
			value = c.target + c.separator + c.input + c.separator + c.parameter + c.separator +
			        ExactNumber( c.delay ) + c.separator + std::to_string( c.timesToFire );
			back = hammer::scene::ParseConnection( c.output, value );
			if ( !back || !( *back == c ) )
			{
				Fail( "connection '" + c.output +
				      "' cannot be written: a field holds the separator or the separator is not "
				      "',' or 0x1B" );
			}
		}
		return value;
	}

	KeyValueNode EncodeEntity( const Entity &entity, const std::vector<const Solid *> &solids )
	{
		KeyValueNode node;
		node.name = "entity";
		Add( node, "id", std::to_string( entity.vmfId ) );
		Add( node, "classname", entity.classname );
		for ( const KeyValue &kv : entity.keys )
		{
			if ( kv.key == "id" || kv.key == "classname" )
			{
				Fail( "entity " + std::to_string( entity.vmfId ) + " holds a '" + kv.key +
				      "' key among its keys" );
			}
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
		node.children.push_back( EncodeEditor( entity.editor, entity.group, "an entity" ) );
		return entity.hidden ? Wrap( "hidden", std::move( node ) ) : node;
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

		// Legacy CMapDoc::SaveVersionInfoVMF and SaveViewSettingsVMF.
		KeyValueNode version;
		version.name = "versioninfo";
		Add( version, "editorversion", std::to_string( settings.version.editorVersion ) );
		Add( version, "editorbuild", std::to_string( settings.version.editorBuild ) );
		Add( version, "mapversion", std::to_string( settings.version.mapVersion ) );
		Add( version, "formatversion", std::to_string( settings.version.formatVersion ) );
		Add( version, "prefab", settings.version.prefab ? "1" : "0" );
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
		Add( view, "bSnapToGrid", settings.view.snapToGrid ? "1" : "0" );
		Add( view, "bShowGrid", settings.view.showGrid ? "1" : "0" );
		Add( view, "bShowLogicalGrid", settings.view.showLogicalGrid ? "1" : "0" );
		Add( view, "nGridSpacing", std::to_string( settings.view.gridSpacing ) );
		Add( view, "bShow3DGrid", settings.view.show3DGrid ? "1" : "0" );
		root.children.push_back( std::move( view ) );

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
			const KeyValue &kv = settings.worldKeys[key];
			if ( kv.key == "id" || kv.key == "classname" )
			{
				Fail( "the world holds a '" + kv.key + "' key among its keys" );
			}
			world.pairs.push_back( kv );
		}
		for ( const Solid *solid : worldSolids )
		{
			world.children.push_back( EncodeSolid( *solid ) );
		}
		for ( const auto &[id, group] : m_doc.Groups() )
		{
			world.children.push_back( EncodeGroup( group ) );
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
		return root;
	}

private:
	const MapDocument &m_doc;
};

// Text the kvtext writer cannot represent: a quote in a key or value.
std::optional<std::string> Unwritable( const KeyValueNode &node )
{
	for ( const KeyValue &kv : node.pairs )
	{
		if ( kv.key.find( '"' ) != std::string::npos || kv.value.find( '"' ) != std::string::npos )
		{
			return "key '" + kv.key + "' in '" + node.name + "' holds a '\"'";
		}
	}
	for ( const KeyValueNode &child : node.children )
	{
		if ( std::optional<std::string> why = Unwritable( child ) )
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

	const SourceLines lines( owned, parsed.root );
	Decoder decoder( lines );
	if ( !decoder.DecodeTop( parsed.root ) )
	{
		return foundation::MakeUnexpected( *decoder.error );
	}

	MapDocument doc( serial );
	DocumentSettings &settings = doc.MutableSettings();
	if ( !decoder.sawWorld )
	{
		decoder.settings.worldKeys = settings.worldKeys; // the new-document default
	}
	settings = std::move( decoder.settings );

	// Persistent ids: every object and side id must be unique (legacy Hammer
	// keeps one id space for objects; overlays name side ids). Missing ids get
	// fresh ones above every id in the file.
	std::set<std::uint32_t> objectIds;
	std::set<std::uint32_t> sideIds;
	std::optional<CodecError> duplicate;
	auto noteObject = [&]( std::uint32_t id, bool hasId, const Where &where, const char *what )
	{
		if ( !hasId || duplicate )
		{
			return;
		}
		if ( !objectIds.insert( id ).second )
		{
			duplicate = CodecError{ where.path + ": " + what + " id " + std::to_string( id ) +
			                            " is already used by another object",
			    where.line };
		}
		doc.NoteVmfId( id );
	};
	auto noteSolid = [&]( const PendingSolid &s )
	{
		noteObject( s.solid.vmfId, s.hasId, s.where, "solid" );
		for ( std::size_t i = 0; i < s.solid.sides.size() && !duplicate; ++i )
		{
			if ( !s.sideHasId[i] )
			{
				continue;
			}
			const std::uint32_t id = s.solid.sides[i].vmfId;
			if ( !sideIds.insert( id ).second )
			{
				duplicate = CodecError{ s.sideWhere[i].path + ": side id " + std::to_string( id ) +
				                            " is already used by another side",
				    s.sideWhere[i].line };
			}
			doc.NoteVmfId( id );
		}
	};
	doc.NoteVmfId( settings.worldVmfId );
	for ( const PendingSolid &s : decoder.worldSolids )
	{
		noteSolid( s );
	}
	for ( const PendingGroup &g : decoder.groups )
	{
		noteObject( g.group.vmfId, g.hasId, g.where, "group" );
	}
	for ( const PendingEntity &e : decoder.entities )
	{
		noteObject( e.entity.vmfId, e.hasId, e.where, "entity" );
		for ( const PendingSolid &s : e.solids )
		{
			noteSolid( s );
		}
	}
	if ( duplicate )
	{
		return foundation::MakeUnexpected( *duplicate );
	}

	std::size_t missing = 0;
	auto fresh = [&]( std::uint32_t &id, bool hasId )
	{
		if ( !hasId )
		{
			id = doc.AllocateVmfId();
			++missing;
		}
	};
	auto freshSolid = [&]( PendingSolid &s )
	{
		fresh( s.solid.vmfId, s.hasId );
		for ( std::size_t i = 0; i < s.solid.sides.size(); ++i )
		{
			fresh( s.solid.sides[i].vmfId, s.sideHasId[i] );
		}
	};
	for ( PendingSolid &s : decoder.worldSolids )
	{
		freshSolid( s );
	}
	for ( PendingGroup &g : decoder.groups )
	{
		fresh( g.group.vmfId, g.hasId );
	}
	for ( PendingEntity &e : decoder.entities )
	{
		fresh( e.entity.vmfId, e.hasId );
		for ( PendingSolid &s : e.solids )
		{
			freshSolid( s );
		}
	}
	std::vector<CodecDiagnostic> warnings;
	if ( missing > 0 )
	{
		warnings.push_back(
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

	// Group references must resolve, and groups must not contain themselves.
	std::optional<CodecError> unresolved;
	auto resolve = [&]( const std::optional<GroupRef> &ref, ObjectId &into )
	{
		if ( !ref || unresolved )
		{
			return;
		}
		const auto it = groupByVmfId.find( ref->id );
		if ( it == groupByVmfId.end() )
		{
			unresolved = CodecError{
			    ref->path + ": groupid " + std::to_string( ref->id ) + " names no group",
			    ref->line };
			return;
		}
		into = it->second;
	};
	for ( PendingSolid &s : decoder.worldSolids )
	{
		resolve( s.group, s.solid.group );
	}
	for ( PendingGroup &g : decoder.groups )
	{
		resolve( g.parent, g.group.group );
	}
	for ( PendingEntity &e : decoder.entities )
	{
		resolve( e.group, e.entity.group );
		for ( PendingSolid &s : e.solids )
		{
			resolve( s.group, s.solid.group );
		}
	}
	if ( unresolved )
	{
		return foundation::MakeUnexpected( *unresolved );
	}
	std::map<ObjectId, const PendingGroup *> groupsById;
	for ( const PendingGroup &g : decoder.groups )
	{
		groupsById[g.group.id] = &g;
	}
	for ( const PendingGroup &g : decoder.groups )
	{
		ObjectId at = g.group.group;
		for ( std::size_t steps = 0; at.IsValid() && steps <= groupsById.size(); ++steps )
		{
			if ( at == g.group.id )
			{
				return foundation::MakeUnexpected(
				    CodecError{ g.where.path + ": group " + std::to_string( g.group.vmfId ) +
				                    " contains itself",
				        g.where.line } );
			}
			at = groupsById[at]->group.group;
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
	return DecodedMap{ std::move( doc ), std::move( warnings ) };
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
	if ( std::optional<std::string> why = Unwritable( root ) )
	{
		return foundation::MakeUnexpected( CodecError{ *why, 0 } );
	}
	return kvtext::WriteKeyValues( root );
}

} // namespace hammer::formats
