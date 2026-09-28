//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The problems panel presentation model (RFC 0002,
//			hammer.presenters; legacy Check for Problems, CMapCheckDlg): the
//			rows of app::CheckMap for the session's document, counts by
//			severity, "go to" and the fixes.
//
//			Refresh: the check reruns when the document changes (edit, undo,
//			redo, replacement), keyed by the session revision so a selection
//			change or a save never rescans; Refresh() forces a rescan (after
//			the catalog or the material port reloads).
//
//			Rows keep CheckMap's order (by code, then first object, then
//			message). Each row carries the stable code name
//			(app::MapProblemCodeName), "error"/"warning", the message, the
//			objects and their labels (presenters::ObjectLabel), and whether it
//			is fixable.
//
//			Actions:
//			  GoTo(row)  selects the row's objects (EditSession::SelectObjects,
//			             Replace; the selection guards apply). A map-wide
//			             problem has none: Nothing.
//			  Fix(row)   the fix_problem command with the row's code and its
//			             first object (one undo step, "Fix <code name>"): it
//			             fixes that code's fixable problems whose first object
//			             is the row's; a problem without a fix is refused and
//			             nothing changes. A fixable map-wide row (no objects)
//			             fixes every problem of its code.
//			  FixAll()   the fix_all command (one undo step, "Fix all
//			             problems"); refused when no problem is fixable.
//			GoTo and an out-of-range row fail with an app::CommandError too
//			(status Rejected), so the panel has one result type.
//
//			The subscription is RAII; the panel may be destroyed before or
//			after its session, but calls need a live session.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_PROBLEMS_PANEL_H
#define HAMMER_PRESENTERS_PROBLEMS_PANEL_H

#include "foundation/expected.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/map_check.h"
#include "hammer/app/session_commands.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/ports/material_info.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace hammer::presenters
{

struct ProblemRow
{
	app::MapProblem problem;
	std::string codeName;     // "no-player-start", ...
	std::string severityName; // "error" or "warning"
	std::vector<std::string> objectLabels;
};

class ProblemsPanel
{
public:
	using Result = app::CommandResult;

	// 'catalog' and 'materials' are the ones 'commands' was composed with and
	// may be null (their checks are skipped).
	ProblemsPanel( app::EditSession &session, app::SessionCommands &commands,
	    const ports::IEntityCatalog *catalog, const ports::IMaterialInfo *materials );
	ProblemsPanel( const ProblemsPanel & ) = delete;
	ProblemsPanel &operator=( const ProblemsPanel & ) = delete;

	std::uint64_t Revision() const { return m_revision; }
	const std::vector<ProblemRow> &Rows() const { return m_rows; }
	std::size_t ErrorCount() const { return m_errors; }
	std::size_t WarningCount() const { return m_warnings; }
	std::size_t FixableCount() const { return m_fixable; }
	// The session revision the rows were computed at.
	std::uint64_t CheckedRevision() const { return m_checkedRevision; }
	// How many times the document was scanned (a rescan counter for hosts
	// and tests).
	std::size_t ScanCount() const { return m_scans; }

	void Refresh();

	Result GoTo( std::size_t row );
	Result Fix( std::size_t row );
	Result FixAll();

private:
	app::EditSession &m_session;
	app::SessionCommands &m_commands;
	const ports::IEntityCatalog *m_catalog = nullptr;
	const ports::IMaterialInfo *m_materials = nullptr;
	app::SessionSubscription m_subscription;
	std::uint64_t m_revision = 0;
	std::uint64_t m_checkedRevision = 0;
	std::size_t m_scans = 0;
	std::vector<ProblemRow> m_rows;
	std::size_t m_errors = 0;
	std::size_t m_warnings = 0;
	std::size_t m_fixable = 0;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_PROBLEMS_PANEL_H
