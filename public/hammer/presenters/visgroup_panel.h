//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The visgroup panel presentation model (RFC 0002,
//			hammer.presenters; legacy CFilterControl's VisGroups tab): the
//			visgroup tree with each group's visibility and membership, and the
//			visgroup actions.
//
//			Rows: DocumentSettings::visgroups in pre-order (authored order)
//			with id, name, color, depth, parent row, child count, visibility
//			(app::ops::VisgroupVisibility: Shown, Hidden, Mixed, Empty), member
//			count (ops::VisgroupMembers, recursive: the group and its
//			descendants), direct member count, and how many members the
//			selection holds (SelectedMembers: None, Some, All). Expansion is
//			kept per visgroup id (default expanded) and cleared when the
//			document is replaced.
//
//			Actions go through app::SessionCommands, the one command authority
//			(one undo step each, the command's label):
//			  Create        visgroup_create name= [parent=]  (outputs the id)
//			  Rename        visgroup_rename visgroup= name=
//			  Delete        visgroup_delete visgroup=
//			  Move          visgroup_move visgroup= parent=
//			  AddSelection  visgroup_add visgroup= [exclusive=1]
//			                (the selection; exclusive = legacy "move to")
//			  RemoveSelection visgroup_remove visgroup=
//			  SetVisible    visgroup_show visgroup= on=0|1
//			  ToggleVisible shows a Hidden or Mixed visgroup, hides a Shown
//			                one; an Empty one is refused
//			  SelectMembers select ids=<members> [mode=] (legacy "Mark"); a
//			                visgroup without members is refused
//			Unknown visgroup ids are refused by the commands.
//
//			The subscription is RAII; the panel may be destroyed before or
//			after its session, but calls need a live session and commands.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_VISGROUP_PANEL_H
#define HAMMER_PRESENTERS_VISGROUP_PANEL_H

#include "hammer/app/edit_session.h"
#include "hammer/app/ops/visgroup_ops.h"
#include "hammer/app/session_commands.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace hammer::presenters
{

enum class SelectedMembers
{
	None,
	Some,
	All,
};

struct VisgroupRow
{
	int id = 0;
	std::string name;
	std::optional<scene::Rgb> color;
	int depth = 0;
	std::optional<std::size_t> parent; // row index; nothing at the top level
	std::size_t childCount = 0;
	app::ops::VisgroupState visibility = app::ops::VisgroupState::Empty;
	std::size_t memberCount = 0;       // with descendants
	std::size_t directMemberCount = 0; // listing this id itself
	SelectedMembers selected = SelectedMembers::None;
	bool expanded = true;
};

class VisgroupPanel
{
public:
	VisgroupPanel( app::EditSession &session, app::SessionCommands &commands );
	VisgroupPanel( const VisgroupPanel & ) = delete;
	VisgroupPanel &operator=( const VisgroupPanel & ) = delete;

	std::uint64_t Revision() const { return m_revision; }
	const std::vector<VisgroupRow> &Rows() const { return m_rows; }
	std::optional<std::size_t> FindRow( int visgroupId ) const;

	bool IsExpanded( int visgroupId ) const;
	void SetExpanded( int visgroupId, bool expanded );

	app::CommandResult Create( const std::string &name, int parentId = 0 );
	app::CommandResult Rename( int visgroupId, const std::string &name );
	app::CommandResult Delete( int visgroupId );
	app::CommandResult Move( int visgroupId, int parentId );
	app::CommandResult AddSelection( int visgroupId, bool exclusive = false );
	app::CommandResult RemoveSelection( int visgroupId );
	app::CommandResult SetVisible( int visgroupId, bool visible );
	app::CommandResult ToggleVisible( int visgroupId );
	app::CommandResult SelectMembers(
	    int visgroupId, app::SelectMode mode = app::SelectMode::Replace );

private:
	void Rebuild();
	void Emit( const scene::Visgroup &visgroup, int depth, std::optional<std::size_t> parent );

	app::EditSession &m_session;
	app::SessionCommands &m_commands;
	app::SessionSubscription m_subscription;
	std::uint64_t m_revision = 0;
	std::map<int, bool> m_expanded;
	std::vector<VisgroupRow> m_rows;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_VISGROUP_PANEL_H
