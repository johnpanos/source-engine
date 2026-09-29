//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/visgroup_panel.h.
//
//=============================================================================//

#include "hammer/presenters/visgroup_panel.h"

namespace hammer::presenters
{

namespace
{

const char *ModeName( app::SelectMode mode )
{
	switch ( mode )
	{
	case app::SelectMode::Replace:
		return "replace";
	case app::SelectMode::Add:
		return "add";
	case app::SelectMode::Toggle:
		return "toggle";
	case app::SelectMode::Remove:
		return "remove";
	}
	return "replace";
}

} // namespace

VisgroupPanel::VisgroupPanel( app::EditSession &session, app::SessionCommands &commands )
    : m_session( session ), m_commands( commands )
{
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent &event )
	    {
		    if ( event.kind == app::SessionEventKind::Replaced )
		    {
			    m_expanded.clear();
		    }
		    if ( event.kind != app::SessionEventKind::Saved )
		    {
			    Rebuild();
		    }
	    } );
	Rebuild();
}

std::optional<std::size_t> VisgroupPanel::FindRow( int visgroupId ) const
{
	for ( std::size_t i = 0; i < m_rows.size(); ++i )
	{
		if ( m_rows[i].id == visgroupId )
		{
			return i;
		}
	}
	return std::nullopt;
}

bool VisgroupPanel::IsExpanded( int visgroupId ) const
{
	const auto it = m_expanded.find( visgroupId );
	return it == m_expanded.end() || it->second;
}

void VisgroupPanel::SetExpanded( int visgroupId, bool expanded )
{
	if ( IsExpanded( visgroupId ) == expanded )
	{
		return;
	}
	m_expanded[visgroupId] = expanded;
	if ( const std::optional<std::size_t> row = FindRow( visgroupId ) )
	{
		m_rows[*row].expanded = expanded;
	}
	++m_revision;
}

void VisgroupPanel::Emit(
    const scene::Visgroup &visgroup, int depth, std::optional<std::size_t> parent )
{
	const scene::MapDocument &doc = m_session.Document();
	const app::Selection &selection = m_session.CurrentSelection();
	VisgroupRow row;
	row.id = visgroup.id;
	row.name = visgroup.name;
	row.color = visgroup.color;
	row.depth = depth;
	row.parent = parent;
	row.childCount = visgroup.children.size();
	row.visibility = app::ops::VisgroupVisibility( doc, visgroup.id );
	const std::vector<scene::ObjectId> members = app::ops::VisgroupMembers( doc, visgroup.id );
	row.memberCount = members.size();
	row.directMemberCount = app::ops::VisgroupMembers( doc, visgroup.id, false ).size();
	std::size_t selected = 0;
	for ( scene::ObjectId id : members )
	{
		selected += selection.Contains( id ) ? 1 : 0;
	}
	row.selected = selected == 0                ? SelectedMembers::None
	               : selected == members.size() ? SelectedMembers::All
	                                            : SelectedMembers::Some;
	row.expanded = IsExpanded( visgroup.id );
	const std::size_t index = m_rows.size();
	m_rows.push_back( std::move( row ) );
	for ( const scene::Visgroup &child : visgroup.children )
	{
		Emit( child, depth + 1, index );
	}
}

void VisgroupPanel::Rebuild()
{
	m_rows.clear();
	for ( const scene::Visgroup &visgroup : m_session.Document().Settings().visgroups )
	{
		Emit( visgroup, 0, std::nullopt );
	}
	++m_revision;
}

app::CommandResult VisgroupPanel::Create( const std::string &name, int parentId )
{
	app::CommandArgs args{ { "name", name } };
	if ( parentId != 0 )
	{
		args["parent"] = std::to_string( parentId );
	}
	return m_commands.Execute( "visgroup_create", args );
}

app::CommandResult VisgroupPanel::CreateFromSelection( const std::string &name, int parentId )
{
	app::CommandArgs args{ { "name", name }, { "selection", "1" } };
	if ( parentId != 0 )
	{
		args["parent"] = std::to_string( parentId );
	}
	return m_commands.Execute( "visgroup_create", args );
}

std::string VisgroupPanel::SelectionVisgroupName() const
{
	// Legacy CMapDoc::ShowNewVisGroupsDialog: "%d object%s".
	const std::size_t count = m_session.CurrentSelection().objects.size();
	return std::to_string( count ) + ( count == 1 ? " object" : " objects" );
}

std::vector<int> VisgroupPanel::MoveTargets( int visgroupId ) const
{
	const std::vector<int> subtree = app::ops::VisgroupSubtree( m_session.Document(), visgroupId );
	std::vector<int> targets;
	for ( const VisgroupRow &row : m_rows )
	{
		bool inside = false;
		for ( int id : subtree )
		{
			inside = inside || id == row.id;
		}
		if ( !inside )
		{
			targets.push_back( row.id );
		}
	}
	return targets;
}

app::CommandResult VisgroupPanel::Rename( int visgroupId, const std::string &name )
{
	return m_commands.Execute(
	    "visgroup_rename", { { "visgroup", std::to_string( visgroupId ) }, { "name", name } } );
}

app::CommandResult VisgroupPanel::Delete( int visgroupId )
{
	return m_commands.Execute(
	    "visgroup_delete", { { "visgroup", std::to_string( visgroupId ) } } );
}

app::CommandResult VisgroupPanel::Move( int visgroupId, int parentId )
{
	return m_commands.Execute( "visgroup_move", { { "visgroup", std::to_string( visgroupId ) },
	                                                { "parent", std::to_string( parentId ) } } );
}

app::CommandResult VisgroupPanel::AddSelection( int visgroupId, bool exclusive )
{
	app::CommandArgs args{ { "visgroup", std::to_string( visgroupId ) } };
	if ( exclusive )
	{
		args["exclusive"] = "1";
	}
	return m_commands.Execute( "visgroup_add", args );
}

app::CommandResult VisgroupPanel::RemoveSelection( int visgroupId )
{
	return m_commands.Execute(
	    "visgroup_remove", { { "visgroup", std::to_string( visgroupId ) } } );
}

app::CommandResult VisgroupPanel::SetVisible( int visgroupId, bool visible )
{
	return m_commands.Execute( "visgroup_show",
	    { { "visgroup", std::to_string( visgroupId ) }, { "on", visible ? "1" : "0" } } );
}

app::CommandResult VisgroupPanel::ToggleVisible( int visgroupId )
{
	switch ( app::ops::VisgroupVisibility( m_session.Document(), visgroupId ) )
	{
	case app::ops::VisgroupState::Shown:
		return SetVisible( visgroupId, false );
	case app::ops::VisgroupState::Hidden:
	case app::ops::VisgroupState::Mixed:
		return SetVisible( visgroupId, true );
	case app::ops::VisgroupState::Empty:
		break;
	}
	return app::CommandFailure(
	    app::CommandStatus::Rejected, "visgroup_show", "the visgroup has no members" );
}

app::CommandResult VisgroupPanel::SelectMembers( int visgroupId, app::SelectMode mode )
{
	const std::vector<scene::ObjectId> members =
	    app::ops::VisgroupMembers( m_session.Document(), visgroupId );
	if ( members.empty() )
	{
		return app::CommandFailure(
		    app::CommandStatus::Rejected, "select", "the visgroup has no members" );
	}
	std::string ids;
	for ( scene::ObjectId id : members )
	{
		ids += ( ids.empty() ? "" : " " ) + std::to_string( app::SessionCommands::ScriptId( id ) );
	}
	return m_commands.Execute( "select", { { "ids", ids }, { "mode", ModeName( mode ) } } );
}

} // namespace hammer::presenters
