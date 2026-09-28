//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the FGD-backed entity catalog. See
//			public/hammer/formats/fgd_entity_catalog.h. FGD syntax and base-class
//			resolution are owned by fgd.h; this file loads files, merges them,
//			and translates resolved classes into the port's EntityClassInfo.
//
//=============================================================================//

#include "hammer/formats/fgd_entity_catalog.h"
#include "hammer/formats/fgd.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <set>
#include <utility>

namespace hammer::formats
{

namespace
{

using hammer::ports::EntityClassInfo;
using hammer::ports::EntityClassKind;
using hammer::ports::IoDefinition;
using hammer::ports::KeyChoice;
using hammer::ports::KeyDefinition;
using hammer::ports::KeyType;
using mapgeometry::Vec3d;

char LowerChar( char c )
{
	return static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
}

bool EqualsNoCase( std::string_view a, std::string_view b )
{
	return a.size() == b.size() && std::equal( a.begin(), a.end(), b.begin(),
	                                   []( char x, char y )
	                                   {
		                                   return LowerChar( x ) == LowerChar( y );
	                                   } );
}

bool LessNoCase( std::string_view a, std::string_view b )
{
	return std::lexicographical_compare( a.begin(), a.end(), b.begin(), b.end(),
	    []( char x, char y )
	    {
		    return static_cast<unsigned char>( LowerChar( x ) ) <
		           static_cast<unsigned char>( LowerChar( y ) );
	    } );
}

// The identity of a file name for the process-once rule.
std::string FileKey( const std::string &name )
{
	std::string key = name;
	for ( char &c : key )
	{
		c = c == '\\' ? '/' : LowerChar( c );
	}
	return key;
}

// Parses exactly three space-separated numbers.
std::optional<Vec3d> ParseVec3( const std::string &text )
{
	double v[3] = {};
	const char *p = text.data();
	const char *end = text.data() + text.size();
	for ( double &component : v )
	{
		while ( p < end && *p == ' ' )
		{
			++p;
		}
		if ( p < end && *p == '+' )
		{
			++p; // from_chars does not accept a leading '+'
		}
		const std::from_chars_result r = std::from_chars( p, end, component );
		if ( r.ec != std::errc() )
		{
			return std::nullopt;
		}
		p = r.ptr;
	}
	while ( p < end && *p == ' ' )
	{
		++p;
	}
	if ( p != end )
	{
		return std::nullopt;
	}
	return Vec3d( v[0], v[1], v[2] );
}

struct SourcedClass
{
	EntityClass cls;
	std::string file;
};

// Reads the entry file and its includes depth-first, each file once.
class FileSet
{
public:
	explicit FileSet( const FgdLoader &loader ) : m_loader( loader ) {}

	std::optional<FgdCatalogError> Process(
	    const std::string &name, const std::string &includer, std::size_t includeLine )
	{
		if ( !m_visited.insert( FileKey( name ) ).second )
		{
			return std::nullopt; // already read, or being read (a cycle)
		}
		std::optional<std::string> text = m_loader ? m_loader( name ) : std::nullopt;
		if ( !text )
		{
			if ( includer.empty() )
			{
				return FgdCatalogError{ "cannot read FGD '" + name + "'", name, 0 };
			}
			return FgdCatalogError{
			    "cannot read included FGD '" + name + "'", includer, includeLine };
		}
		FgdParseResult parsed = ParseFgd( *text );
		if ( !parsed.ok )
		{
			return FgdCatalogError{ parsed.error, name, parsed.errorLine };
		}
		for ( std::size_t i = 0; i < parsed.includes.size(); ++i )
		{
			const std::size_t line = i < parsed.includeLines.size() ? parsed.includeLines[i] : 0;
			if ( std::optional<FgdCatalogError> error = Process( parsed.includes[i], name, line ) )
			{
				return error;
			}
		}
		files.push_back( name );
		for ( EntityClass &cls : parsed.classes )
		{
			Add( std::move( cls ), name );
		}
		return std::nullopt;
	}

	std::vector<SourcedClass> classes;
	std::vector<std::string> files;

private:
	void Add( EntityClass cls, const std::string &file )
	{
		for ( SourcedClass &existing : classes )
		{
			if ( EqualsNoCase( existing.cls.name, cls.name ) )
			{
				existing = SourcedClass{ std::move( cls ), file };
				return;
			}
		}
		classes.push_back( SourcedClass{ std::move( cls ), file } );
	}

	const FgdLoader &m_loader;
	std::set<std::string> m_visited;
};

bool IsNonZero( const std::string &value )
{
	long number = 0;
	const char *begin = value.data();
	const char *end = value.data() + value.size();
	const std::from_chars_result r = std::from_chars( begin, end, number );
	return r.ec == std::errc() && number != 0;
}

KeyDefinition ToKey( const FgdProperty &prop )
{
	KeyDefinition key;
	key.key = prop.name;
	key.displayName = prop.displayName;
	key.help = prop.help;
	key.defaultValue = prop.defaultValue;
	key.typeName = prop.type;
	key.type = hammer::ports::KeyTypeFromName( prop.type );
	key.readOnly = prop.readOnly;
	for ( const FgdChoice &choice : prop.choices )
	{
		KeyChoice out;
		out.value = choice.value;
		out.label = choice.label;
		out.defaultOn = key.type == KeyType::Flags && IsNonZero( choice.defaultValue );
		key.choices.push_back( std::move( out ) );
	}
	return key;
}

IoDefinition ToIo( const FgdIo &io )
{
	IoDefinition out;
	out.name = io.name;
	out.type = io.type;
	out.help = io.help;
	return out;
}

// Applies the first helper of each hint kind.
void ApplyHints( const EntityClass &resolved, EntityClassInfo &info )
{
	bool haveSize = false;
	bool haveColor = false;
	bool haveModel = false;
	bool haveSprite = false;
	for ( const FgdHelper &helper : resolved.helpers )
	{
		const std::string &name = helper.name;
		if ( name == "size" && !haveSize )
		{
			haveSize = true;
			std::optional<Vec3d> a = helper.args.size() >= 1 && helper.args.size() <= 2
			                             ? ParseVec3( helper.args[0] )
			                             : std::nullopt;
			std::optional<Vec3d> b =
			    helper.args.size() == 2 ? ParseVec3( helper.args[1] ) : std::nullopt;
			if ( a && helper.args.size() == 1 )
			{
				info.boxMins = Vec3d( -a->x / 2, -a->y / 2, -a->z / 2 );
				info.boxMaxs = Vec3d( a->x / 2, a->y / 2, a->z / 2 );
			}
			else if ( a && b )
			{
				info.boxMins =
				    Vec3d( std::min( a->x, b->x ), std::min( a->y, b->y ), std::min( a->z, b->z ) );
				info.boxMaxs =
				    Vec3d( std::max( a->x, b->x ), std::max( a->y, b->y ), std::max( a->z, b->z ) );
			}
		}
		else if ( name == "color" && !haveColor )
		{
			haveColor = true;
			if ( helper.args.size() == 1 )
			{
				info.color = ParseVec3( helper.args[0] );
			}
		}
		else if ( ( name == "studio" || name == "studioprop" ) && !haveModel )
		{
			haveModel = true;
			if ( !helper.args.empty() && !helper.args[0].empty() )
			{
				info.model = helper.args[0];
			}
			else if ( const KeyDefinition *model = info.FindKey( "model" ) )
			{
				info.model = model->defaultValue;
			}
		}
		else if ( name == "iconsprite" && !haveSprite )
		{
			haveSprite = true;
			if ( !helper.args.empty() )
			{
				info.sprite = helper.args[0];
			}
		}
	}
}

EntityClassKind ToKind( EntityKind kind )
{
	switch ( kind )
	{
	case EntityKind::Point:
		return EntityClassKind::Point;
	case EntityKind::Solid:
		return EntityClassKind::Solid;
	default:
		return EntityClassKind::Other;
	}
}

const EntityClass *FindNoCase( const std::vector<EntityClass> &classes, const std::string &name )
{
	for ( const EntityClass &cls : classes )
	{
		if ( EqualsNoCase( cls.name, name ) )
		{
			return &cls;
		}
	}
	return nullptr;
}

} // namespace

foundation::Expected<FgdEntityCatalog, FgdCatalogError> FgdEntityCatalog::Load(
    const std::string &entryName, const FgdLoader &loader )
{
	return FromFileSet( { entryName }, loader );
}

foundation::Expected<FgdEntityCatalog, FgdCatalogError> FgdEntityCatalog::FromTexts(
    const std::vector<std::pair<std::string, std::string>> &namedTexts )
{
	const FgdLoader loader = [&namedTexts]( const std::string &name ) -> std::optional<std::string>
	{
		const std::string key = FileKey( name );
		for ( const auto &file : namedTexts )
		{
			if ( FileKey( file.first ) == key )
			{
				return file.second;
			}
		}
		return std::nullopt;
	};
	std::vector<std::string> entries;
	for ( const auto &file : namedTexts )
	{
		entries.push_back( file.first );
	}
	return FromFileSet( entries, loader );
}

foundation::Expected<FgdEntityCatalog, FgdCatalogError> FgdEntityCatalog::FromFileSet(
    const std::vector<std::string> &entries, const FgdLoader &loader )
{
	FileSet set( loader );
	for ( const std::string &entry : entries )
	{
		if ( std::optional<FgdCatalogError> error = set.Process( entry, std::string(), 0 ) )
		{
			return foundation::MakeUnexpected( std::move( *error ) );
		}
	}

	std::vector<EntityClass> all;
	all.reserve( set.classes.size() );
	for ( const SourcedClass &sourced : set.classes )
	{
		all.push_back( sourced.cls );
	}
	for ( const SourcedClass &sourced : set.classes )
	{
		for ( const std::string &base : sourced.cls.bases )
		{
			if ( FindNoCase( all, base ) == nullptr )
			{
				return foundation::MakeUnexpected( FgdCatalogError{
				    "unknown base class '" + base + "' of '" + sourced.cls.name + "'", sourced.file,
				    sourced.cls.line } );
			}
		}
	}

	FgdEntityCatalog catalog;
	catalog.m_files = std::move( set.files );
	for ( const EntityClass &cls : all )
	{
		if ( cls.kind == EntityKind::Base )
		{
			continue;
		}
		const std::optional<EntityClass> resolved = ResolveClass( all, cls.name );
		if ( !resolved )
		{
			continue; // unreachable: the name came from 'all'
		}
		EntityClassInfo info;
		info.name = resolved->name;
		info.description = resolved->description;
		info.kind = ToKind( resolved->kind );
		for ( const FgdProperty &prop : resolved->properties )
		{
			info.keys.push_back( ToKey( prop ) );
		}
		for ( const FgdIo &io : resolved->inputs )
		{
			info.inputs.push_back( ToIo( io ) );
		}
		for ( const FgdIo &io : resolved->outputs )
		{
			info.outputs.push_back( ToIo( io ) );
		}
		ApplyHints( *resolved, info );
		catalog.m_classes.push_back( std::move( info ) );
	}
	std::sort( catalog.m_classes.begin(), catalog.m_classes.end(),
	    []( const EntityClassInfo &a, const EntityClassInfo &b )
	    {
		    return LessNoCase( a.name, b.name );
	    } );
	return catalog;
}

const hammer::ports::EntityClassInfo *FgdEntityCatalog::Find( std::string_view name ) const
{
	const auto it = std::lower_bound( m_classes.begin(), m_classes.end(), name,
	    []( const EntityClassInfo &info, std::string_view key )
	    {
		    return LessNoCase( info.name, key );
	    } );
	if ( it == m_classes.end() || !EqualsNoCase( it->name, name ) )
	{
		return nullptr;
	}
	return &*it;
}

std::vector<std::string> FgdEntityCatalog::ClassNames() const
{
	std::vector<std::string> names;
	names.reserve( m_classes.size() );
	for ( const EntityClassInfo &info : m_classes )
	{
		names.push_back( info.name );
	}
	return names;
}

} // namespace hammer::formats
