//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VMT import (RFC 0016 K4). The reading rules follow the legacy
//			material system: KeyValues::LoadFromBuffer for tags,
//			AccumulateRecursiveVmtPatches/ApplyPatchKeyValues for patches,
//			FindBuiltinFallbackBlock for fallback blocks, and
//			CMaterial::ParseMaterialVars/ShouldSkipVar for variables
//			(materialsystem/cmaterial.cpp).
//
//=============================================================================//

#include "kvtext/keyvalues.h"
#include "render/material/vmt_import.h"
#include "render/pbr_material_schema.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>

namespace render::material
{

namespace
{

using kvtext::KeyValue;
using kvtext::KeyValueNode;

// The legacy loader follows at most this many patch includes.
constexpr int kMaxPatchDepth = 10;

char Lower( char c )
{
	return c >= 'A' && c <= 'Z' ? static_cast<char>( c - 'A' + 'a' ) : c;
}

std::string Lowered( std::string_view text )
{
	std::string out( text );
	for ( char &c : out )
		c = Lower( c );
	return out;
}

bool SameName( std::string_view a, std::string_view b )
{
	if ( a.size() != b.size() )
		return false;
	for ( std::size_t i = 0; i < a.size(); ++i )
	{
		if ( Lower( a[i] ) != Lower( b[i] ) )
			return false;
	}
	return true;
}

std::string_view Trimmed( std::string_view text )
{
	while ( !text.empty() && ( text.front() == ' ' || text.front() == '\t' ) )
		text.remove_prefix( 1 );
	while ( !text.empty() && ( text.back() == ' ' || text.back() == '\t' ) )
		text.remove_suffix( 1 );
	return text;
}

foundation::Unexpected<ImportError> Fail( ImportStatus status, std::string detail )
{
	return foundation::MakeUnexpected( ImportError{ status, std::move( detail ) } );
}

// A file path as the resolver sees it: lower case, forward slashes, relative.
std::string NormalizedPath( std::string_view path )
{
	std::string out = Lowered( Trimmed( path ) );
	for ( char &c : out )
	{
		if ( c == '\\' )
			c = '/';
	}
	while ( !out.empty() && out.front() == '/' )
		out.erase( out.begin() );
	return out;
}

KeyValue *FindPair( KeyValueNode &node, std::string_view key )
{
	for ( KeyValue &pair : node.pairs )
	{
		if ( SameName( pair.key, key ) )
			return &pair;
	}
	return nullptr;
}

const KeyValueNode *FindBlock( const KeyValueNode &node, std::string_view name )
{
	for ( const KeyValueNode &child : node.children )
	{
		if ( SameName( child.name, name ) )
			return &child;
	}
	return nullptr;
}

KeyValueNode &FindOrAddBlock( KeyValueNode &node, std::string_view name )
{
	for ( KeyValueNode &child : node.children )
	{
		if ( SameName( child.name, name ) )
			return child;
	}
	KeyValueNode child;
	child.name = std::string( name );
	node.children.push_back( std::move( child ) );
	return node.children.back();
}

// KeyValues::SetString: the first key of that name takes the value, or a new
// key is appended.
void SetValue( KeyValueNode &node, const std::string &key, const std::string &value )
{
	if ( KeyValue *pair = FindPair( node, key ) )
	{
		pair->value = value;
		return;
	}
	node.pairs.push_back( { key, value, {} } );
}

// ---------------------------------------------------------------------------
// Conditions

class Conditions
{
public:
	Conditions( const VmtProfile &profile, std::vector<std::string> &diagnostics )
	    : m_Profile( profile ), m_Diagnostics( diagnostics )
	{
	}

	// A [tag]: KeyValues evaluates it and rejects a malformed one.
	bool TagHolds( const std::string &tag ) const
	{
		const std::optional<bool> holds = kvtext::EvaluateCondition( tag,
		    [this]( std::string_view symbol )
		    {
			    return Symbol( symbol );
		    } );
		if ( !holds )
			m_Diagnostics.push_back( "malformed conditional tag [" + tag + "]" );
		return holds.value_or( false );
	}

	// "cond?$key": CMaterial::ShouldSkipVar.
	bool SkipVariable( std::string_view condition ) const
	{
		bool toggle = false;
		if ( !condition.empty() && condition.front() == '!' )
		{
			condition.remove_prefix( 1 );
			toggle = true;
		}
		bool skip = true;
		if ( SameName( condition, "lowfill" ) )
			skip = !m_Profile.reduceParticles;
		else if ( SameName( condition, "hdr" ) )
			skip = false; // the legacy test is disabled: hdr? variables always apply
		else if ( SameName( condition, "srgb" ) || SameName( condition, "srgb_pc" ) )
			skip = !m_Profile.srgbBlending;
		else if ( SameName( condition, "ldr" ) )
			skip = m_Profile.hdr;
		else if ( SameName( condition, "sonyps3" ) || SameName( condition, "360" ) ||
		          SameName( condition, "gameconsole" ) )
			skip = true;
		else if ( const std::optional<bool> holds = GpuLevel( condition ) )
			skip = !*holds;
		else
			m_Diagnostics.push_back( "unrecognized conditional test " + std::string( condition ) );
		return skip != toggle;
	}

	// "GPU>=n" and "GPU<n" (one digit), or nullopt for other text.
	std::optional<bool> GpuLevel( std::string_view condition ) const
	{
		if ( condition.size() == 6 && SameName( condition.substr( 0, 5 ), "GPU>=" ) &&
		     condition[5] >= '0' && condition[5] <= '9' )
			return m_Profile.gpuLevel >= condition[5] - '0';
		if ( condition.size() == 5 && SameName( condition.substr( 0, 4 ), "GPU<" ) &&
		     condition[4] >= '0' && condition[4] <= '9' )
			return m_Profile.gpuLevel < condition[4] - '0';
		return std::nullopt;
	}

	const VmtProfile &Profile() const { return m_Profile; }

private:
	bool Symbol( std::string_view symbol ) const
	{
		for ( const std::string &held : m_Profile.symbols )
		{
			if ( SameName( held, symbol ) )
				return true;
		}
		return false;
	}

	const VmtProfile &m_Profile;
	std::vector<std::string> &m_Diagnostics;
};

// Drops the pairs and blocks whose tags do not hold, as the loader does.
void ApplyTags( KeyValueNode &node, const Conditions &conditions )
{
	std::vector<KeyValue> pairs;
	for ( KeyValue &pair : node.pairs )
	{
		if ( pair.condition.empty() || conditions.TagHolds( pair.condition ) )
		{
			pair.condition.clear();
			pairs.push_back( std::move( pair ) );
		}
	}
	node.pairs = std::move( pairs );
	std::vector<KeyValueNode> children;
	for ( KeyValueNode &child : node.children )
	{
		if ( child.condition.empty() || conditions.TagHolds( child.condition ) )
		{
			child.condition.clear();
			ApplyTags( child, conditions );
			children.push_back( std::move( child ) );
		}
	}
	node.children = std::move( children );
}

// The file's first top-level block that its tags keep.
foundation::Expected<KeyValueNode, ImportError> LoadBlock( std::string_view text,
    std::string_view what, const Conditions &conditions, std::vector<std::string> &diagnostics )
{
	kvtext::ParseOptions options;
	options.closeBlocksAtEnd = true; // KeyValues keeps what it read before the end
	kvtext::ParseResult parsed = kvtext::ParseKeyValues( std::string( text ), options );
	if ( !parsed.ok )
	{
		return Fail( ImportStatus::kMalformed, std::string( what ) + " line " +
		                                           std::to_string( parsed.errorLine ) + ": " +
		                                           parsed.error );
	}
	for ( const KeyValue &pair : parsed.root.pairs )
	{
		// #include appends top-level keys after the material, which the
		// material system never reads; #base would merge keys into it.
		if ( SameName( pair.key, "#base" ) )
			return Fail( ImportStatus::kMalformed, std::string( what ) + ": #base is not read" );
		if ( !SameName( pair.key, "#include" ) )
		{
			return Fail( ImportStatus::kMalformed,
			    std::string( what ) + ": top-level key '" + pair.key + "' is not a block" );
		}
		diagnostics.push_back( std::string( what ) + ": #include " + pair.value + " ignored" );
	}
	ApplyTags( parsed.root, conditions );
	if ( parsed.root.children.empty() )
		return Fail( ImportStatus::kMalformed, std::string( what ) + ": no material block" );
	return std::move( parsed.root.children.front() );
}

// ---------------------------------------------------------------------------
// Patches

// MergeKeyValues: every value of src overwrites dest's, recursively.
void MergeInto( const KeyValueNode &src, KeyValueNode &dest )
{
	for ( const KeyValue &pair : src.pairs )
		SetValue( dest, pair.key, pair.value );
	for ( const KeyValueNode &child : src.children )
		MergeInto( child, FindOrAddBlock( dest, child.name ) );
}

// InsertKeyValues: insert sets every key; replace (onlyExisting) sets only keys
// the material already has.
void InsertInto( KeyValueNode &dest, const KeyValueNode &src, bool onlyExisting )
{
	for ( const KeyValue &pair : src.pairs )
	{
		if ( !onlyExisting || FindPair( dest, pair.key ) || FindBlock( dest, pair.key ) )
			SetValue( dest, pair.key, pair.value );
	}
	for ( const KeyValueNode &child : src.children )
	{
		if ( !onlyExisting || FindPair( dest, child.name ) || FindBlock( dest, child.name ) )
			InsertInto( FindOrAddBlock( dest, child.name ), child, onlyExisting );
	}
}

foundation::Expected<KeyValueNode, ImportError> ExpandPatches( KeyValueNode block,
    const VmtImportContext &context, const Conditions &conditions, MaterialDesc &desc )
{
	if ( !SameName( block.name, "patch" ) )
		return block;
	KeyValueNode insert;
	KeyValueNode replace;
	int depth = 0;
	while ( depth < kMaxPatchDepth && SameName( block.name, "patch" ) )
	{
		if ( const KeyValueNode *section = FindBlock( block, "insert" ) )
			MergeInto( *section, insert );
		if ( const KeyValueNode *section = FindBlock( block, "replace" ) )
			MergeInto( *section, replace );
		const KeyValue *include = FindPair( block, "include" );
		if ( !include || Trimmed( include->value ).empty() )
			return Fail( ImportStatus::kPatchWithoutInclude, "a patch without an include" );
		const std::string path = NormalizedPath( include->value );
		std::optional<std::string> text =
		    context.resolve ? context.resolve( path ) : std::optional<std::string>();
		if ( !text )
			return Fail( ImportStatus::kMissingInclude, path );
		desc.includes.push_back( path );
		auto loaded = LoadBlock( *text, path, conditions, desc.diagnostics );
		if ( !loaded )
			return foundation::MakeUnexpected( loaded.Error() );
		block = std::move( loaded.Value() );
		++depth;
	}
	if ( SameName( block.name, "patch" ) )
	{
		return Fail( ImportStatus::kIncludeDepth,
		    "patches nest deeper than " + std::to_string( kMaxPatchDepth ) );
	}
	InsertInto( block, insert, false );
	InsertInto( block, replace, true );
	return block;
}

// ---------------------------------------------------------------------------
// Fallback blocks (FindBuiltinFallbackBlock, in its order)

const KeyValueNode *FallbackBlock( const KeyValueNode &block, std::string_view shader,
    const Conditions &conditions, std::string &chosen )
{
	const auto find = [&]( std::string_view suffix ) -> const KeyValueNode *
	{
		if ( const KeyValueNode *found = FindBlock( block, suffix ) )
		{
			chosen = found->name;
			return found;
		}
		const std::string named = std::string( shader ) + "_" + std::string( suffix );
		if ( const KeyValueNode *found = FindBlock( block, named ) )
		{
			chosen = found->name;
			return found;
		}
		return nullptr;
	};
	const VmtProfile &profile = conditions.Profile();
	for ( const std::string_view level : { "GPU<1", "GPU<2", "GPU>=1", "GPU>=2" } )
	{
		if ( conditions.GpuLevel( level ).value_or( false ) )
		{
			if ( const KeyValueNode *found = find( level ) )
				return found;
		}
	}
	const bool ps20b = profile.pixelShader20b;
	const struct
	{
		bool holds;
		std::string_view suffix;
	} kLevels[] = {
	    { profile.dxLevel < 90, "<DX90" },
	    { profile.dxLevel < 95, "<DX95" },
	    { profile.dxLevel < 90 || !ps20b, "<DX90_20b" },
	    { profile.dxLevel >= 90 && ps20b, ">=DX90_20b" },
	    { profile.dxLevel <= 90, "<=DX90" },
	    { profile.dxLevel >= 90, ">=DX90" },
	    { profile.dxLevel > 90, ">DX90" },
	    { true, "hdr_dx9" },
	    { true, "hdr" },
	    { !profile.hdr, "ldr" },
	    { profile.srgbBlending, "srgb" },
	    { profile.dxLevel >= 90, "dx9" },
	};
	for ( const auto &level : kLevels )
	{
		if ( !level.holds )
			continue;
		if ( const KeyValueNode *found = find( level.suffix ) )
			return found;
	}
	return nullptr;
}

// ---------------------------------------------------------------------------
// Variables (CMaterial::ParseMaterialVars)

struct Variable
{
	std::string key; // after any "cond?"
	std::string value;
};

std::vector<Variable> CollectVariables( const KeyValueNode &block, const KeyValueNode *fallback,
    const Conditions &conditions, MaterialDesc &desc )
{
	std::vector<Variable> variables;
	std::vector<bool> conditional;
	const auto visit = [&]( const KeyValue &pair )
	{
		std::string_view key = pair.key;
		const std::size_t question = key.find( '?' );
		const bool isConditional = question != std::string_view::npos && question != 0;
		if ( isConditional )
		{
			if ( conditions.SkipVariable( key.substr( 0, question ) ) )
				return;
			key = key.substr( question + 1 );
		}
		if ( !pair.key.empty() && pair.key.front() == '%' )
		{
			desc.editorKeys.push_back( { pair.key, pair.value } );
			return;
		}
		for ( std::size_t i = 0; i < variables.size(); ++i )
		{
			if ( !SameName( variables[i].key, Trimmed( key ) ) )
				continue;
			// A later unconditional definition is ignored; a conditional one wins.
			if ( isConditional )
			{
				variables[i].value = pair.value;
				conditional[i] = true;
			}
			return;
		}
		variables.push_back( { std::string( Trimmed( key ) ), pair.value } );
		conditional.push_back( isConditional );
	};
	if ( fallback )
	{
		for ( const KeyValue &pair : fallback->pairs )
			visit( pair );
	}
	for ( const KeyValue &pair : block.pairs )
		visit( pair );
	return variables;
}

// ---------------------------------------------------------------------------
// Values (CreateMaterialVarFromKeyValue and ParseVectorFromKeyValueString)

bool IsSpecialTexture( std::string_view name )
{
	return name.starts_with( "_rt_" ) || name == "env_cubemap" || name.starts_with( "[" );
}

std::string TextureReference( std::string_view value )
{
	std::string name = NormalizedPath( value );
	if ( name.size() > 4 && name.ends_with( ".vtf" ) )
		name.resize( name.size() - 4 );
	if ( name.empty() || IsSpecialTexture( name ) )
		return name;
	if ( name.starts_with( "materials/" ) )
		return name;
	return "materials/" + name;
}

// The numbers of "[a b c]", "{A B C}" (0-255) or a scalar; returns how many.
int ReadNumbers( std::string_view text, float ( &out )[4] )
{
	text = Trimmed( text );
	float scale = 1.0f;
	char close = 0;
	if ( !text.empty() && ( text.front() == '[' || text.front() == '{' ) )
	{
		close = text.front() == '[' ? ']' : '}';
		scale = text.front() == '{' ? 1.0f / 255.0f : 1.0f;
		text.remove_prefix( 1 );
	}
	const std::string buffer( text );
	const char *cursor = buffer.c_str();
	int count = 0;
	while ( count < 4 )
	{
		while ( *cursor == ' ' || *cursor == '\t' )
			++cursor;
		if ( !*cursor || *cursor == close )
			break;
		char *end = nullptr;
		const float value = std::strtof( cursor, &end );
		if ( end == cursor )
			break;
		out[count++] = value * scale;
		cursor = end;
		if ( !close )
			break; // a scalar is one number
	}
	return count;
}

// A texture transform as CreateMatrixMaterialVarFromKeyValue (cmaterial.cpp)
// parses one: 16 row-major numbers in brackets, or center, scale, rotate
// (degrees) and translate, composed as T(center + translate) Rz S T(-center)
// with VMatrix's float arithmetic. Rows 0 and 1 go to `rows`.
bool ReadTransform( const std::string &text, float ( &rows )[8] )
{
	using Matrix = std::array<std::array<float, 4>, 4>;
	Matrix m = {};
	const int full =
	    std::sscanf( text.c_str(), " [ %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f ]",
	        &m[0][0], &m[0][1], &m[0][2], &m[0][3], &m[1][0], &m[1][1], &m[1][2], &m[1][3],
	        &m[2][0], &m[2][1], &m[2][2], &m[2][3], &m[3][0], &m[3][1], &m[3][2], &m[3][3] );
	if ( full != 16 )
	{
		float center[2], scale[2], angle, translate[2];
		if ( std::sscanf( text.c_str(), " center %f %f scale %f %f rotate %f translate %f %f",
		         &center[0], &center[1], &scale[0], &scale[1], &angle, &translate[0],
		         &translate[1] ) != 7 )
			return false;
		auto identity = []
		{
			Matrix out = {};
			for ( int i = 0; i < 4; ++i )
				out[i][i] = 1.0f;
			return out;
		};
		auto multiply = []( const Matrix &a, const Matrix &b )
		{
			Matrix out = {};
			for ( int i = 0; i < 4; ++i )
			{
				for ( int j = 0; j < 4; ++j )
					out[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] +
					            a[i][3] * b[3][j];
			}
			return out;
		};
		Matrix step = identity();
		step[0][3] = -center[0];
		step[1][3] = -center[1];
		m = step;
		step = identity();
		step[0][0] = scale[0];
		step[1][1] = scale[1];
		m = multiply( step, m );
		const double radians = angle * ( 3.14159265358979323846 / 180.0f );
		const float sine = static_cast<float>( std::sin( radians ) );
		const float cosine = static_cast<float>( std::cos( radians ) );
		step = identity();
		step[0][0] = cosine;
		step[0][1] = -sine;
		step[1][0] = sine;
		step[1][1] = cosine;
		m = multiply( step, m );
		step = identity();
		step[0][3] = center[0] + translate[0];
		step[1][3] = center[1] + translate[1];
		m = multiply( step, m );
	}
	for ( int i = 0; i < 4; ++i )
	{
		rows[i] = m[0][i];
		rows[4 + i] = m[1][i];
	}
	return true;
}

bool ReadValue( const VmtKeyRow &row, const std::string &value, MaterialValue &out,
    std::vector<std::string> &diagnostics )
{
	out.parameter = std::string( row.parameter );
	out.kind = row.kind;
	switch ( row.kind )
	{
	case ValueKind::kTexture:
		out.text = TextureReference( value );
		return true;
	case ValueKind::kMaterial:
		out.text = NormalizedPath( value );
		return true;
	case ValueKind::kTransform:
		if ( ReadTransform( value, out.numbers ) )
			return true;
		diagnostics.push_back(
		    out.key + " \"" + value + "\" is not a transform; the identity applies" );
		return false;
	case ValueKind::kEnum:
	{
		std::string_view names = row.fallback;
		for ( int index = 0; !names.empty(); ++index )
		{
			const std::size_t bar = names.find( '|' );
			if ( SameName( names.substr( 0, bar ), Trimmed( value ) ) )
			{
				out.numbers[0] = static_cast<float>( index );
				return true;
			}
			names = bar == std::string_view::npos ? std::string_view() : names.substr( bar + 1 );
		}
		float numbers[4] = {};
		if ( ReadNumbers( value, numbers ) > 0 )
		{
			out.numbers[0] = static_cast<float>( static_cast<int>( numbers[0] ) );
			return true;
		}
		diagnostics.push_back( out.key + " \"" + value + "\" is none of " +
		                       std::string( row.fallback ) + "; the first applies" );
		return true;
	}
	default:
		break;
	}
	float numbers[4] = {};
	const int count = ReadNumbers( value, numbers );
	if ( count == 0 )
	{
		diagnostics.push_back(
		    out.key + " \"" + value + "\" is not a number; its default applies" );
		return false;
	}
	const bool isVector =
	    Trimmed( value ).starts_with( "[" ) || Trimmed( value ).starts_with( "{" );
	for ( int i = 0; i < 4; ++i )
		out.numbers[i] = isVector ? ( i < count ? numbers[i] : 0.0f ) : numbers[0];
	if ( row.kind == ValueKind::kInt || row.kind == ValueKind::kBool )
		out.numbers[0] = static_cast<float>( static_cast<int>( out.numbers[0] ) );
	if ( row.kind == ValueKind::kBool )
		out.numbers[0] = out.numbers[0] != 0.0f ? 1.0f : 0.0f;
	return true;
}

// ---------------------------------------------------------------------------
// Shaders

std::string_view LegacyName( const VmtMappingTable &mapping, std::string_view name )
{
	for ( const LegacyShaderName &shader : mapping.legacyShaders )
	{
		if ( shader.name == name )
			return shader.name;
	}
	return {};
}

// The shader after DEFINE_FALLBACK_SHADER aliases, or empty when unknown.
std::string CanonicalShader( const VmtMappingTable &mapping, std::string_view name )
{
	std::string current = Lowered( name );
	for ( int hop = 0; hop < 8; ++hop )
	{
		const LegacyShaderName *found = nullptr;
		for ( const LegacyShaderName &shader : mapping.legacyShaders )
		{
			if ( shader.name == current )
				found = &shader;
		}
		if ( !found )
			return hop == 0 ? std::string() : current;
		if ( found->target.empty() || LegacyName( mapping, found->target ).empty() )
			return current;
		current = std::string( found->target );
	}
	return current;
}

foundation::Expected<void, ImportError> CheckPbr(
    const MaterialDesc &desc, const VmtImportContext &context )
{
	for ( const pbr::MaterialParameterSpec &spec : pbr::kMaterialParameters )
	{
		if ( !spec.required )
			continue;
		bool present = false;
		for ( const VmtPair &variable : desc.variables )
			present = present ||
			          ( SameName( variable.key, spec.name ) && !Trimmed( variable.value ).empty() );
		if ( !present )
			return Fail( ImportStatus::kMissingRequired, spec.name );
	}
	for ( const MaterialValue &value : desc.values )
	{
		if ( value.kind != ValueKind::kMaterial )
			continue;
		if ( !pbr::IsValidFallbackReference( value.text.c_str() ) )
			return Fail( ImportStatus::kInvalidReference, value.key + " \"" + value.text + "\"" );
		const std::string file = "materials/" + value.text + ".vmt";
		if ( file == NormalizedPath( context.path ) )
			return Fail(
			    ImportStatus::kInvalidReference, value.key + " names the material itself" );
		if ( !context.resolve || !context.resolve( file ) )
			return Fail( ImportStatus::kInvalidReference, value.key + " " + file + " is missing" );
	}
	return {};
}

// The shader's row (nullptr for the legacy family), with desc's shader,
// legacyShader, family and reason set; fails on an unknown shader.
foundation::Expected<const VmtShaderRow *, ImportError> ResolveShader(
    const VmtMappingTable &mapping, std::string_view shader, MaterialDesc &desc )
{
	desc.shader = std::string( shader );
	desc.legacyShader = CanonicalShader( mapping, shader );
	// A row may also name a material kind that is not a shader ("subrect").
	const std::string name = desc.legacyShader.empty() ? Lowered( shader ) : desc.legacyShader;
	const VmtShaderRow *row = nullptr;
	for ( const VmtShaderRow &candidate : mapping.shaders )
	{
		if ( candidate.shader == name )
			row = &candidate;
	}
	if ( !row && desc.legacyShader.empty() )
		return Fail( ImportStatus::kUnknownShader, std::string( shader ) );
	desc.legacyShader = name;
	desc.family = row ? std::string( row->family ) : std::string( kLegacyFamily );
	desc.reason = row ? std::string( row->reason ) : std::string( mapping.legacyReason );
	if ( row && row->family == kLegacyFamily )
		row = nullptr; // the legacy family maps no keys
	return row;
}

// desc.variables into the family's values, metadata and unmapped keys: a
// family's own rows first, then metadata, then the rest is unmapped.
void MapValues( const VmtMappingTable &mapping, const VmtShaderRow *row, MaterialDesc &desc )
{
	for ( const VmtPair &variable : desc.variables )
	{
		const std::string key = Lowered( variable.key );
		const VmtKeyRow *keyRow = nullptr;
		for ( const VmtKeyRow &candidate : mapping.keys )
		{
			if ( row && candidate.family == row->family && candidate.key == key )
				keyRow = &candidate;
		}
		if ( !keyRow )
		{
			bool metadata = false;
			for ( const VmtMetadataRow &item : mapping.metadata )
				metadata = metadata || item.key == key;
			if ( metadata )
				desc.metadata.push_back( variable );
			else if ( row )
				desc.unmapped.push_back( key );
			continue;
		}
		MaterialValue value;
		value.key = variable.key;
		if ( ReadValue( *keyRow, variable.value, value, desc.diagnostics ) )
			desc.values.push_back( std::move( value ) );
	}
}

} // namespace

std::string_view ImportStatusName( ImportStatus status )
{
	switch ( status )
	{
	case ImportStatus::kMalformed:
		return "malformed";
	case ImportStatus::kMissingInclude:
		return "missing-include";
	case ImportStatus::kPatchWithoutInclude:
		return "patch-without-include";
	case ImportStatus::kIncludeDepth:
		return "include-depth";
	case ImportStatus::kUnknownShader:
		return "unknown-shader";
	case ImportStatus::kMissingRequired:
		return "missing-required";
	case ImportStatus::kInvalidReference:
		return "invalid-reference";
	}
	return "unknown";
}

foundation::Expected<MaterialDesc, ImportError> ImportVmt(
    std::string_view text, const VmtImportContext &context )
{
	const VmtMappingTable &mapping = context.mapping ? *context.mapping : BuiltinVmtMapping();
	MaterialDesc desc;
	const Conditions conditions( context.profile, desc.diagnostics );
	auto loaded = LoadBlock( text, "material", conditions, desc.diagnostics );
	if ( !loaded )
		return foundation::MakeUnexpected( loaded.Error() );
	auto expanded = ExpandPatches( std::move( loaded.Value() ), context, conditions, desc );
	if ( !expanded )
		return foundation::MakeUnexpected( expanded.Error() );
	const KeyValueNode &block = expanded.Value();

	auto row = ResolveShader( mapping, block.name, desc );
	if ( !row )
		return foundation::MakeUnexpected( row.Error() );

	const KeyValueNode *fallback =
	    FallbackBlock( block, block.name, conditions, desc.fallbackBlock );
	for ( Variable &variable : CollectVariables( block, fallback, conditions, desc ) )
		desc.variables.push_back( { std::move( variable.key ), std::move( variable.value ) } );
	MapValues( mapping, row.Value(), desc );

	const KeyValueNode *proxies = fallback ? FindBlock( *fallback, "proxies" ) : nullptr;
	if ( !proxies )
		proxies = FindBlock( block, "proxies" );
	if ( proxies )
	{
		for ( const KeyValueNode &proxy : proxies->children )
		{
			ProxyDesc item;
			item.name = proxy.name;
			for ( const KeyValue &pair : proxy.pairs )
				item.parameters.push_back( { pair.key, pair.value } );
			desc.proxies.push_back( std::move( item ) );
		}
	}

	if ( desc.family == "pbr" )
	{
		auto checked = CheckPbr( desc, context );
		if ( !checked )
			return foundation::MakeUnexpected( checked.Error() );
	}
	return desc;
}

foundation::Expected<MaterialDesc, ImportError> MapVariables(
    std::string_view shader, std::vector<VmtPair> variables, const VmtImportContext &context )
{
	const VmtMappingTable &mapping = context.mapping ? *context.mapping : BuiltinVmtMapping();
	MaterialDesc desc;
	auto row = ResolveShader( mapping, shader, desc );
	if ( !row )
		return foundation::MakeUnexpected( row.Error() );
	desc.variables = std::move( variables );
	MapValues( mapping, row.Value(), desc );
	return desc;
}

std::string VmtTextureReference( std::string_view value )
{
	return TextureReference( value );
}

int VmtNumbers( std::string_view value, float ( &out )[4] )
{
	return ReadNumbers( value, out );
}

foundation::Expected<void, MaterialError> ApplyValues(
    const MaterialDesc &material, ParameterBlock &block )
{
	for ( const MaterialValue &value : material.values )
	{
		foundation::Expected<void, MaterialError> written;
		switch ( value.kind )
		{
		case ValueKind::kTexture:
		case ValueKind::kMaterial:
			continue;
		case ValueKind::kFloat:
			written = block.SetFloat( value.parameter, value.numbers[0] );
			break;
		case ValueKind::kFloat2:
		{
			const float numbers[2] = { value.numbers[0], value.numbers[1] };
			written = block.SetFloat2( value.parameter, numbers );
			break;
		}
		case ValueKind::kFloat3:
		{
			const float numbers[3] = { value.numbers[0], value.numbers[1], value.numbers[2] };
			written = block.SetFloat3( value.parameter, numbers );
			break;
		}
		case ValueKind::kFloat4:
		{
			const float numbers[4] = {
			    value.numbers[0], value.numbers[1], value.numbers[2], value.numbers[3] };
			written = block.SetFloat4( value.parameter, numbers );
			break;
		}
		case ValueKind::kTransform:
			written = block.SetTransform( value.parameter, value.numbers );
			break;
		case ValueKind::kInt:
		case ValueKind::kBool:
		case ValueKind::kEnum:
			written =
			    block.SetInt( value.parameter, static_cast<std::int32_t>( value.numbers[0] ) );
			break;
		}
		if ( !written )
			return written;
	}
	return {};
}

} // namespace render::material
