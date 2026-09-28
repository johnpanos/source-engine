//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The outliner presentation model (RFC 0002, hammer.presenters;
//			legacy grouplist.cpp and Source 2's filterable Outliner): the
//			document's object tree as rows a tree widget binds to.
//
//			Tree: a "world" root; under it and under each group, the nested
//			groups, then the entities, then the loose solids, each in id
//			(creation) order; a brush entity's children are its solids. Rows
//			are in pre-order with their depth and parent row.
//
//			Row fields: label (presenters::ObjectLabel: entity targetname or
//			classname, "solid <vmfId>", "group <vmfId>"), classname and
//			targetname (entities), child count (after filtering), visible
//			(scene::IsVisible, the one visibility rule), selected (the object or
//			a container above it is in the selection) and expanded.
//
//			Filter: case-insensitive substring over label, classname and
//			targetname. A matching row keeps its ancestors (so it can be
//			reached); the children of a matching container are kept only when
//			they match too. The root always stays. An empty filter shows all.
//
//			Expansion: kept per object id across rebuilds (edits, undo,
//			selection changes) and cleared when the document is replaced.
//			Containers start collapsed, the root expanded. DisplayRows() lists
//			the rows under expanded ancestors; while a filter is active every
//			kept row is displayed, so matches are never hidden by collapse.
//
//			Actions: Select maps rows to EditSession::SelectObjects (the world
//			row selects nothing); ToggleVisibility flips an object's quick-hide
//			flag with ops::SetHidden as one undo step ("Hide <label>" /
//			"Show <label>").
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_OUTLINER_H
#define HAMMER_PRESENTERS_OUTLINER_H

#include "foundation/expected.h"
#include "hammer/app/edit_session.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace hammer::presenters
{

enum class OutlinerKind
{
	World,
	Group,
	Entity,
	Solid,
};

struct OutlinerRow
{
	scene::ObjectId id; // invalid for the world row
	OutlinerKind kind = OutlinerKind::World;
	std::string label;
	std::string classname;
	std::string targetname;
	int depth = 0;
	std::optional<std::size_t> parent; // row index; nothing for the root
	std::size_t childCount = 0;
	bool visible = true;
	bool selected = false;
	bool expanded = false;
	bool matches = true; // matches the filter itself (not only kept as an ancestor)
};

class Outliner
{
public:
	using Result = foundation::Expected<void, app::EditError>;

	explicit Outliner( app::EditSession &session );
	Outliner( const Outliner & ) = delete;
	Outliner &operator=( const Outliner & ) = delete;

	std::uint64_t Revision() const { return m_revision; }
	const std::vector<OutlinerRow> &Rows() const { return m_rows; }
	// Indices into Rows() a tree widget shows (see "Expansion").
	std::vector<std::size_t> DisplayRows() const;
	std::optional<std::size_t> FindRow( scene::ObjectId id ) const;

	const std::string &Filter() const { return m_filter; }
	void SetFilter( const std::string &filter );

	bool IsExpanded( scene::ObjectId id ) const;
	void SetExpanded( scene::ObjectId id, bool expanded );

	Result Select( const std::vector<scene::ObjectId> &ids, app::SelectMode mode );
	Result ToggleVisibility( scene::ObjectId id );

private:
	void Rebuild();
	bool Emit( scene::ObjectId id, OutlinerKind kind, int depth, std::optional<std::size_t> parent,
	    bool parentSelected );

	app::EditSession &m_session;
	app::SessionSubscription m_subscription;
	std::uint64_t m_revision = 0;
	std::string m_filter;
	std::map<scene::ObjectId, bool> m_expanded; // absent: the kind's default
	std::vector<OutlinerRow> m_rows;
	// Rebuild scratch: children per container (invalid id = world).
	std::map<scene::ObjectId, std::vector<std::pair<scene::ObjectId, OutlinerKind>>> m_children;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_OUTLINER_H
