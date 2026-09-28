//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/material_browser.h.
//
//=============================================================================//

#include "hammer/presenters/material_browser.h"

#include "presenter_text.h"

#include <algorithm>

namespace hammer::presenters
{

MaterialBrowser::MaterialBrowser( app::EditSession &session, const ports::IMaterialInfo &materials,
    app::EditorSettings &settings, std::size_t recentLimit )
    : m_session( session ), m_materials( materials ), m_settings( settings ),
      m_recentLimit( recentLimit )
{
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent &event )
	    {
		    if ( event.kind != app::SessionEventKind::SelectionChanged &&
		         event.kind != app::SessionEventKind::Saved )
		    {
			    Recount();
			    Rebuild();
		    }
	    } );
	Refresh();
}

std::string MaterialBrowser::Normalize( std::string_view name )
{
	std::string out = detail::Lower( name );
	std::replace( out.begin(), out.end(), '\\', '/' );
	return out;
}

void MaterialBrowser::Refresh()
{
	m_names = m_materials.Names();
	Recount();
	Rebuild();
}

void MaterialBrowser::Recount()
{
	m_counts.clear();
	m_spelled.clear();
	for ( const auto &entry : m_session.Document().Solids() )
	{
		for ( const scene::Side &side : entry.second.sides )
		{
			const std::string key = Normalize( side.texture.material );
			++m_counts[key];
			m_spelled.emplace( key, side.texture.material );
		}
	}
}

std::size_t MaterialBrowser::FaceCount( std::string_view name ) const
{
	const auto it = m_counts.find( Normalize( name ) );
	return it == m_counts.end() ? 0 : it->second;
}

void MaterialBrowser::Rebuild()
{
	const std::vector<std::string> words = detail::Words( m_filter );
	auto passes = [&]( const std::string &name )
	{
		for ( const std::string &word : words )
		{
			if ( !detail::ContainsNoCase( name, word ) )
			{
				return false;
			}
		}
		return true;
	};
	m_rows.clear();
	if ( m_usedOnly )
	{
		std::map<std::string, std::string> portSpelling;
		for ( const std::string &name : m_names )
		{
			portSpelling.emplace( Normalize( name ), name );
		}
		for ( const auto &entry : m_counts )
		{
			const auto port = portSpelling.find( entry.first );
			MaterialRow row;
			row.name = port != portSpelling.end() ? port->second : m_spelled[entry.first];
			row.known = port != portSpelling.end() || m_materials.Exists( row.name );
			row.faceCount = entry.second;
			if ( passes( row.name ) )
			{
				m_rows.push_back( std::move( row ) );
			}
		}
	}
	else
	{
		for ( const std::string &name : m_names )
		{
			if ( passes( name ) )
			{
				m_rows.push_back( MaterialRow{ name, true, FaceCount( name ) } );
			}
		}
	}
	++m_revision;
}

void MaterialBrowser::SetFilter( const std::string &filter )
{
	if ( filter != m_filter )
	{
		m_filter = filter;
		Rebuild();
	}
}

void MaterialBrowser::SetUsedOnly( bool usedOnly )
{
	if ( usedOnly != m_usedOnly )
	{
		m_usedOnly = usedOnly;
		Rebuild();
	}
}

MaterialBrowser::Result MaterialBrowser::SetActive( const std::string &material )
{
	if ( !m_materials.Exists( material ) )
	{
		return foundation::MakeUnexpected(
		    app::EditError{ app::EditErrorCode::Rejected, "unknown material '" + material + "'" } );
	}
	std::string spelled = material;
	const std::string key = Normalize( material );
	for ( const std::string &name : m_names )
	{
		if ( Normalize( name ) == key )
		{
			spelled = name;
			break;
		}
	}
	m_settings.faceTexture.material = spelled;
	std::erase_if( m_recent,
	    [&]( const std::string &name )
	    {
		    return Normalize( name ) == key;
	    } );
	m_recent.insert( m_recent.begin(), spelled );
	if ( m_recent.size() > m_recentLimit )
	{
		m_recent.resize( m_recentLimit );
	}
	++m_revision;
	return {};
}

} // namespace hammer::presenters
