//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/problems_panel.h.
//
//=============================================================================//

#include "hammer/presenters/problems_panel.h"

#include "hammer/presenters/object_label.h"

namespace hammer::presenters
{

ProblemsPanel::ProblemsPanel( app::EditSession &session, app::SessionCommands &commands,
    const ports::IEntityCatalog *catalog, const ports::IMaterialInfo *materials )
    : m_session( session ), m_commands( commands ), m_catalog( catalog ), m_materials( materials )
{
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent &event )
	    {
		    switch ( event.kind )
		    {
		    case app::SessionEventKind::Replaced:
			    Refresh();
			    break;
		    case app::SessionEventKind::Edited:
		    case app::SessionEventKind::Undone:
		    case app::SessionEventKind::Redone:
			    if ( m_session.Revision() != m_checkedRevision )
			    {
				    Refresh();
			    }
			    break;
		    case app::SessionEventKind::SelectionChanged:
		    case app::SessionEventKind::Saved:
			    break;
		    }
	    } );
	Refresh();
}

void ProblemsPanel::Refresh()
{
	const scene::MapDocument &doc = m_session.Document();
	m_rows.clear();
	m_errors = 0;
	m_warnings = 0;
	m_fixable = 0;
	for ( app::MapProblem &problem : app::CheckMap( doc, m_catalog, m_materials ) )
	{
		ProblemRow row;
		row.codeName = app::MapProblemCodeName( problem.code );
		row.severityName =
		    problem.severity == app::MapProblem::Severity::Error ? "error" : "warning";
		for ( scene::ObjectId id : problem.objects )
		{
			row.objectLabels.push_back( ObjectLabel( doc, id ) );
		}
		( problem.severity == app::MapProblem::Severity::Error ? m_errors : m_warnings )++;
		m_fixable += problem.fixable ? 1 : 0;
		row.problem = std::move( problem );
		m_rows.push_back( std::move( row ) );
	}
	m_checkedRevision = m_session.Revision();
	++m_scans;
	++m_revision;
}

ProblemsPanel::Result ProblemsPanel::GoTo( std::size_t row )
{
	if ( row >= m_rows.size() )
	{
		return app::CommandFailure( app::CommandStatus::Rejected, "select", "no such problem" );
	}
	if ( m_rows[row].problem.objects.empty() )
	{
		return app::CommandFailure(
		    app::CommandStatus::Rejected, "select", "the problem concerns the whole map" );
	}
	auto selected =
	    m_session.SelectObjects( m_rows[row].problem.objects, app::SelectMode::Replace );
	if ( !selected )
	{
		return app::CommandFailure(
		    app::CommandStatus::Rejected, "select", selected.Error().message );
	}
	return std::string();
}

ProblemsPanel::Result ProblemsPanel::Fix( std::size_t row )
{
	if ( row >= m_rows.size() )
	{
		return app::CommandFailure(
		    app::CommandStatus::Rejected, "fix_problem", "no such problem" );
	}
	app::CommandArgs args{ { "code", m_rows[row].codeName } };
	if ( !m_rows[row].problem.objects.empty() )
	{
		args["ids"] =
		    std::to_string( app::SessionCommands::ScriptId( m_rows[row].problem.objects.front() ) );
	}
	return m_commands.Execute( "fix_problem", args );
}

ProblemsPanel::Result ProblemsPanel::FixAll()
{
	return m_commands.Execute( "fix_all", {} );
}

} // namespace hammer::presenters
