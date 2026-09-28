//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/find_replace_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/find_replace_ops.h"

#include "hammer/app/ops/entity_ops.h"
#include "hammer/scene/map_queries.h"

#include <algorithm>
#include <cctype>

namespace hammer::app::ops
{

using scene::ObjectId;

namespace
{

char Fold( char c, bool caseSensitive )
{
	return caseSensitive ? c : static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
}

bool SameChar( char a, char b, bool caseSensitive )
{
	return Fold( a, caseSensitive ) == Fold( b, caseSensitive );
}

std::size_t Find(
    std::string_view text, std::string_view pattern, std::size_t from, bool caseSensitive )
{
	if ( pattern.empty() || text.size() < pattern.size() )
	{
		return std::string_view::npos;
	}
	for ( std::size_t i = from; i + pattern.size() <= text.size(); ++i )
	{
		bool all = true;
		for ( std::size_t j = 0; j < pattern.size() && all; ++j )
		{
			all = SameChar( text[i + j], pattern[j], caseSensitive );
		}
		if ( all )
		{
			return i;
		}
	}
	return std::string_view::npos;
}

// Iterative glob match with backtracking to the last '*'.
bool Glob( std::string_view text, std::string_view pattern, bool caseSensitive )
{
	std::size_t t = 0;
	std::size_t p = 0;
	std::size_t star = std::string_view::npos;
	std::size_t mark = 0;
	while ( t < text.size() )
	{
		if ( p < pattern.size() &&
		     ( pattern[p] == '?' ||
		         ( pattern[p] != '*' && SameChar( pattern[p], text[t], caseSensitive ) ) ) )
		{
			++t;
			++p;
		}
		else if ( p < pattern.size() && pattern[p] == '*' )
		{
			star = p++;
			mark = t;
		}
		else if ( star != std::string_view::npos )
		{
			p = star + 1;
			t = ++mark;
		}
		else
		{
			return false;
		}
	}
	while ( p < pattern.size() && pattern[p] == '*' )
	{
		++p;
	}
	return p == pattern.size();
}

bool KeyMatches( std::string_view key, std::string_view wanted )
{
	return wanted.empty() || ( key.size() == wanted.size() && Find( key, wanted, 0, false ) == 0 );
}

bool ValueMatches( std::string_view value, const EntityQuery &query )
{
	return query.valuePattern.empty() ||
	       TextMatches( value, query.valuePattern, query.valueMatch, query.caseSensitive );
}

bool EntityMatches( const scene::DocumentReader &doc, ObjectId id, const EntityQuery &query )
{
	const scene::Entity *e = doc.FindEntity( id );
	if ( !e )
	{
		return false;
	}
	if ( !query.classPattern.empty() &&
	     !TextMatches( e->classname, query.classPattern, query.classMatch, false ) )
	{
		return false;
	}
	if ( query.visibleOnly && !scene::IsVisible( doc, id ) )
	{
		return false;
	}
	if ( query.key.empty() && query.valuePattern.empty() )
	{
		return true;
	}
	for ( const kvtext::KeyValue &kv : e->keys )
	{
		if ( KeyMatches( kv.key, query.key ) && ValueMatches( kv.value, query ) )
		{
			return true;
		}
	}
	if ( query.key.empty() )
	{
		for ( const scene::Connection &c : e->connections )
		{
			if ( ValueMatches( c.target, query ) || ValueMatches( c.parameter, query ) )
			{
				return true;
			}
		}
	}
	return false;
}

// The replaced value, or nothing when 'value' does not match or is unchanged.
std::optional<std::string> Replaced(
    const std::string &value, const EntityQuery &query, const std::string &newValue )
{
	if ( !TextMatches( value, query.valuePattern, query.valueMatch, query.caseSensitive ) )
	{
		return std::nullopt;
	}
	std::string out;
	if ( query.valueMatch != TextMatch::Contains )
	{
		out = newValue;
	}
	else
	{
		std::size_t from = 0;
		for ( ;; )
		{
			const std::size_t at = Find( value, query.valuePattern, from, query.caseSensitive );
			if ( at == std::string::npos )
			{
				out.append( value, from, std::string::npos );
				break;
			}
			out.append( value, from, at - from );
			out += newValue;
			from = at + query.valuePattern.size();
		}
	}
	if ( out == value )
	{
		return std::nullopt;
	}
	return out;
}

} // namespace

bool TextMatches(
    std::string_view text, std::string_view pattern, TextMatch mode, bool caseSensitive )
{
	switch ( mode )
	{
	case TextMatch::Contains:
		return pattern.empty() || Find( text, pattern, 0, caseSensitive ) != std::string_view::npos;
	case TextMatch::Whole:
		return text.size() == pattern.size() &&
		       ( text.empty() || Find( text, pattern, 0, caseSensitive ) == 0 );
	case TextMatch::Wildcard:
		return Glob( text, pattern, caseSensitive );
	}
	return false;
}

std::vector<ObjectId> FindEntities( const scene::DocumentReader &doc, const EntityQuery &query )
{
	std::vector<ObjectId> candidates =
	    query.within ? EntitiesOf( doc, *query.within ) : doc.EntityIds();
	std::sort( candidates.begin(), candidates.end() );
	candidates.erase( std::unique( candidates.begin(), candidates.end() ), candidates.end() );
	std::vector<ObjectId> out;
	for ( ObjectId id : candidates )
	{
		if ( EntityMatches( doc, id, query ) )
		{
			out.push_back( id );
		}
	}
	return out;
}

EditResult ReplaceKeyValues(
    scene::DocumentEdit &edit, const EntityQuery &query, const std::string &newValue, int &count )
{
	count = 0;
	if ( query.valuePattern.empty() )
	{
		return Reject( "nothing to find: the value pattern is empty" );
	}
	std::vector<std::pair<ObjectId, scene::Entity>> changed;
	int values = 0;
	for ( ObjectId id : FindEntities( edit, query ) )
	{
		scene::Entity next = *edit.FindEntity( id );
		int here = 0;
		for ( kvtext::KeyValue &kv : next.keys )
		{
			if ( !KeyMatches( kv.key, query.key ) )
			{
				continue;
			}
			if ( std::optional<std::string> value = Replaced( kv.value, query, newValue ) )
			{
				kv.value = std::move( *value );
				++here;
			}
		}
		if ( query.key.empty() )
		{
			for ( scene::Connection &c : next.connections )
			{
				if ( std::optional<std::string> target = Replaced( c.target, query, newValue ) )
				{
					c.target = std::move( *target );
					++here;
				}
				if ( std::optional<std::string> parameter =
				         Replaced( c.parameter, query, newValue ) )
				{
					c.parameter = std::move( *parameter );
					++here;
				}
			}
		}
		if ( here )
		{
			values += here;
			changed.emplace_back( id, std::move( next ) );
		}
	}
	if ( changed.empty() )
	{
		return NothingToDo( "no value to replace" );
	}
	for ( auto &[id, entity] : changed )
	{
		*edit.MutableEntity( id ) = std::move( entity );
	}
	count = values;
	return {};
}

} // namespace hammer::app::ops
