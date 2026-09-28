//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/outliner.h.
//
//=============================================================================//

#include "hammer/presenters/outliner.h"

#include "hammer/app/ops/structure_ops.h"
#include "hammer/presenters/object_label.h"
#include "hammer/scene/map_queries.h"
#include "presenter_text.h"

namespace hammer::presenters
{

using app::EditError;
using app::EditErrorCode;
using scene::ObjectId;

Outliner::Outliner( app::EditSession &session ) : m_session( session )
{
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent &event )
	    {
		    if ( event.kind == app::SessionEventKind::Replaced )
		    {
			    m_expanded.clear();
		    }
		    Rebuild();
	    } );
	Rebuild();
}

bool Outliner::IsExpanded( ObjectId id ) const
{
	const auto it = m_expanded.find( id );
	return it != m_expanded.end() ? it->second : !id.IsValid();
}

void Outliner::SetExpanded( ObjectId id, bool expanded )
{
	if ( IsExpanded( id ) == expanded )
	{
		return;
	}
	m_expanded[id] = expanded;
	for ( OutlinerRow &row : m_rows )
	{
		if ( row.id == id )
		{
			row.expanded = expanded;
		}
	}
	++m_revision;
}

void Outliner::SetFilter( const std::string &filter )
{
	if ( filter != m_filter )
	{
		m_filter = filter;
		Rebuild();
	}
}

std::optional<std::size_t> Outliner::FindRow( ObjectId id ) const
{
	for ( std::size_t i = 0; i < m_rows.size(); ++i )
	{
		if ( m_rows[i].id == id )
		{
			return i;
		}
	}
	return std::nullopt;
}

std::vector<std::size_t> Outliner::DisplayRows() const
{
	std::vector<std::size_t> shown;
	const bool filtering = !m_filter.empty();
	// Rows deeper than 'collapsedDepth' are under a collapsed row.
	int collapsedDepth = -1;
	for ( std::size_t i = 0; i < m_rows.size(); ++i )
	{
		const OutlinerRow &row = m_rows[i];
		if ( !filtering && collapsedDepth >= 0 )
		{
			if ( row.depth > collapsedDepth )
			{
				continue;
			}
			collapsedDepth = -1;
		}
		shown.push_back( i );
		if ( !filtering && !row.expanded && row.childCount > 0 )
		{
			collapsedDepth = row.depth;
		}
	}
	return shown;
}

bool Outliner::Emit( ObjectId id, OutlinerKind kind, int depth, std::optional<std::size_t> parent,
    bool parentSelected )
{
	const scene::MapDocument &doc = m_session.Document();
	OutlinerRow row;
	row.id = id;
	row.kind = kind;
	row.depth = depth;
	row.parent = parent;
	if ( kind == OutlinerKind::World )
	{
		row.label = "world";
		row.classname = "worldspawn";
	}
	else
	{
		row.label = ObjectLabel( doc, id );
		if ( const scene::Entity *e = doc.FindEntity( id ) )
		{
			row.classname = e->classname;
			row.targetname = std::string( e->Name() );
		}
		row.visible = scene::IsVisible( doc, id );
	}
	row.selected =
	    parentSelected || ( id.IsValid() && m_session.CurrentSelection().Contains( id ) );
	row.expanded = IsExpanded( id );
	row.matches = m_filter.empty() || detail::ContainsNoCase( row.label, m_filter ) ||
	              detail::ContainsNoCase( row.classname, m_filter ) ||
	              detail::ContainsNoCase( row.targetname, m_filter );

	const std::size_t index = m_rows.size();
	m_rows.push_back( row );
	std::size_t kept = 0;
	const auto it = m_children.find( id );
	if ( it != m_children.end() )
	{
		for ( const auto &child : it->second )
		{
			kept += Emit( child.first, child.second, depth + 1, index, row.selected ) ? 1 : 0;
		}
	}
	m_rows[index].childCount = kept;
	if ( kind != OutlinerKind::World && kept == 0 && !row.matches )
	{
		m_rows.resize( index ); // neither it nor a descendant matched
		return false;
	}
	return true;
}

void Outliner::Rebuild()
{
	const scene::MapDocument &doc = m_session.Document();
	m_children.clear();
	for ( const auto &entry : doc.Groups() )
	{
		m_children[entry.second.group].emplace_back( entry.first, OutlinerKind::Group );
	}
	for ( const auto &entry : doc.Entities() )
	{
		m_children[entry.second.group].emplace_back( entry.first, OutlinerKind::Entity );
	}
	for ( const auto &entry : doc.Solids() )
	{
		const scene::Solid &s = entry.second;
		m_children[s.owner.IsValid() ? s.owner : s.group].emplace_back(
		    entry.first, OutlinerKind::Solid );
	}
	m_rows.clear();
	Emit( ObjectId{}, OutlinerKind::World, 0, std::nullopt, false );
	m_children.clear();
	++m_revision;
}

Outliner::Result Outliner::Select( const std::vector<ObjectId> &ids, app::SelectMode mode )
{
	std::vector<ObjectId> objects;
	for ( ObjectId id : ids )
	{
		if ( id.IsValid() )
		{
			objects.push_back( id );
		}
	}
	return m_session.SelectObjects( objects, mode );
}

Outliner::Result Outliner::ToggleVisibility( ObjectId id )
{
	const scene::MapDocument &doc = m_session.Document();
	bool hidden = false;
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		hidden = s->hidden;
	}
	else if ( const scene::Entity *e = doc.FindEntity( id ) )
	{
		hidden = e->hidden;
	}
	else if ( const scene::Group *g = doc.FindGroup( id ) )
	{
		hidden = g->hidden;
	}
	else
	{
		return foundation::MakeUnexpected( EditError{ EditErrorCode::Rejected, "no such object" } );
	}
	auto committed = m_session.Execute( ( hidden ? "Show " : "Hide " ) + ObjectLabel( doc, id ),
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::SetHidden( edit, { id }, !hidden );
	    } );
	if ( !committed )
	{
		return foundation::MakeUnexpected( committed.Error() );
	}
	return {};
}

} // namespace hammer::presenters
