//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/class_palette.h.
//
//=============================================================================//

#include "hammer/presenters/class_palette.h"

#include "presenter_text.h"

#include <algorithm>
#include <map>

namespace hammer::presenters
{

namespace
{

struct CategoryPrefix
{
	const char *prefix;
	const char *name;
};

constexpr CategoryPrefix kCategories[] = {
    { "info_", "Info" },
    { "func_", "Func" },
    { "light", "Lights" },
    { "prop_", "Props" },
    { "trigger_", "Triggers" },
    { "logic_", "Logic" },
    { "env_", "Environment" },
    { "npc_", "NPCs" },
    { "point_", "Point" },
    { "weapon_", "Weapons" },
    { "item_", "Items" },
    { "filter_", "Filters" },
    { "ai_", "AI" },
    { "path_", "Paths" },
    { "game_", "Game" },
};
constexpr const char *kOtherCategory = "Other";

std::string Trim( const std::string &text )
{
	const std::size_t first = text.find_first_not_of( " \t\r\n" );
	if ( first == std::string::npos )
	{
		return std::string();
	}
	const std::size_t last = text.find_last_not_of( " \t\r\n" );
	return text.substr( first, last - first + 1 );
}

} // namespace

std::string CategoryOf( std::string_view classname )
{
	for ( const CategoryPrefix &category : kCategories )
	{
		if ( detail::StartsWithNoCase( classname, category.prefix ) )
		{
			return category.name;
		}
	}
	return kOtherCategory;
}

ClassPalette::ClassPalette( const ports::IEntityCatalog &catalog, app::EditorSettings &settings,
    app::EditSession *session, std::size_t recentLimit )
    : m_catalog( catalog ), m_settings( settings ), m_session( session ),
      m_recentLimit( recentLimit )
{
	if ( m_session )
	{
		m_subscription = m_session->Subscribe(
		    [this]( const app::SessionEvent &event )
		    {
			    if ( event.kind != app::SessionEventKind::SelectionChanged &&
			         event.kind != app::SessionEventKind::Saved )
			    {
				    Refresh();
			    }
		    } );
	}
	Refresh();
}

void ClassPalette::SetSearch( const std::string &search )
{
	if ( search != m_search )
	{
		m_search = search;
		Refresh();
	}
}

void ClassPalette::SetKindFilter( ClassKindFilter kind )
{
	if ( kind != m_kind )
	{
		m_kind = kind;
		Refresh();
	}
}

void ClassPalette::SetCategory( std::optional<std::string> category )
{
	if ( category != m_category )
	{
		m_category = std::move( category );
		Refresh();
	}
}

ClassPalette::Result ClassPalette::SetActive( const std::string &classname )
{
	const ports::EntityClassInfo *info = m_catalog.Find( classname );
	if ( !info )
	{
		return foundation::MakeUnexpected( app::EditError{
		    app::EditErrorCode::Rejected, "unknown entity class '" + classname + "'" } );
	}
	m_settings.entityClass = info->name;
	std::erase_if( m_recent,
	    [&]( const std::string &name )
	    {
		    return detail::EqualsNoCase( name, info->name );
	    } );
	m_recent.insert( m_recent.begin(), info->name );
	if ( m_recent.size() > m_recentLimit )
	{
		m_recent.resize( m_recentLimit );
	}
	++m_revision;
	return {};
}

void ClassPalette::Refresh()
{
	std::map<std::string, std::size_t> counts; // lower-cased classname -> entities
	if ( m_session )
	{
		for ( const auto &entry : m_session->Document().Entities() )
		{
			++counts[detail::Lower( entry.second.classname )];
		}
	}
	const std::string search = Trim( m_search );
	std::vector<ClassEntry> passing;
	std::vector<std::string> used; // categories the catalog uses
	for ( const std::string &name : m_catalog.ClassNames() )
	{
		const ports::EntityClassInfo *info = m_catalog.Find( name );
		if ( !info )
		{
			continue;
		}
		ClassEntry entry;
		entry.name = info->name;
		entry.description = info->description;
		entry.kind = info->kind;
		entry.category = CategoryOf( info->name );
		if ( std::find( used.begin(), used.end(), entry.category ) == used.end() )
		{
			used.push_back( entry.category );
		}
		const auto count = counts.find( detail::Lower( info->name ) );
		entry.countInMap = count == counts.end() ? 0 : count->second;

		if ( ( m_kind == ClassKindFilter::Solid && info->kind != ports::EntityClassKind::Solid ) ||
		     ( m_kind == ClassKindFilter::Point && info->kind == ports::EntityClassKind::Solid ) )
		{
			continue;
		}
		if ( !search.empty() )
		{
			if ( detail::StartsWithNoCase( entry.name, search ) )
			{
				entry.rank = 0;
			}
			else if ( detail::ContainsNoCase( entry.name, search ) )
			{
				entry.rank = 1;
			}
			else if ( detail::ContainsNoCase( entry.description, search ) )
			{
				entry.rank = 2;
			}
			else
			{
				continue;
			}
		}
		passing.push_back( std::move( entry ) );
	}

	m_categories.clear();
	for ( const CategoryPrefix &category : kCategories )
	{
		if ( std::find( used.begin(), used.end(), category.name ) != used.end() )
		{
			m_categories.push_back( ClassCategory{ category.name, 0 } );
		}
	}
	if ( std::find( used.begin(), used.end(), kOtherCategory ) != used.end() )
	{
		m_categories.push_back( ClassCategory{ kOtherCategory, 0 } );
	}
	for ( const ClassEntry &entry : passing )
	{
		for ( ClassCategory &category : m_categories )
		{
			category.count += category.name == entry.category ? 1 : 0;
		}
	}

	m_entries.clear();
	for ( ClassEntry &entry : passing )
	{
		if ( !m_category || *m_category == entry.category )
		{
			m_entries.push_back( std::move( entry ) );
		}
	}
	std::stable_sort( m_entries.begin(), m_entries.end(),
	    []( const ClassEntry &a, const ClassEntry &b )
	    {
		    return a.rank < b.rank;
	    } );
	++m_revision;
}

} // namespace hammer::presenters
