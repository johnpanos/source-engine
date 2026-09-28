//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The entity class palette presentation model (RFC 0002,
//			hammer.presenters; the Source 2 Entity tool's searchable class list
//			and legacy Hammer's class dropdown): the catalog's classes filtered,
//			ranked and grouped, the most-recently-used classes and the active
//			class the entity tool places.
//
//			Categories (CategoryOf; first matching prefix, ignoring case):
//			  info_ Info | func_ Func | light Lights | prop_ Props |
//			  trigger_ Triggers | logic_ Logic | env_ Environment | npc_ NPCs |
//			  point_ Point | weapon_ Weapons | item_ Items | filter_ Filters |
//			  ai_ AI | path_ Paths | game_ Game | anything else: Other
//			("light" has no underscore so light, light_spot and
//			light_environment share it.) Categories() lists them in this order,
//			those the catalog uses, with the count of entries passing the
//			search and kind filters.
//
//			Filters: kind (All; Point = point and point-like "other" classes,
//			which the entity tool places at a point; Solid = brush classes),
//			category (optional) and search.
//			Search: case-insensitive substring after trimming; entries rank
//			  0 the name starts with the text,
//			  1 the name contains it,
//			  2 only the description contains it,
//			and within a rank keep the catalog's (case-insensitive name) order.
//			An empty search keeps every entry at rank 0.
//
//			Active class: app::EditorSettings::entityClass is its one owner (the
//			class the Entity tool, menus and scripts place); Active() reads it
//			live and SetActive() writes it. SetActive requires a class the
//			catalog knows, stores its catalog spelling and pushes it onto
//			Recent. EditorSettings has no change notification, so a write by
//			another owner shows in Active() but does not move Revision().
//			Recent: bounded (default 8), most recent first, no duplicates
//			(ignoring case); palette-local view state.
//
//			With a session the palette also counts each class's entities in
//			the map (countInMap) and refreshes when the document changes; the
//			subscription is RAII.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_CLASS_PALETTE_H
#define HAMMER_PRESENTERS_CLASS_PALETTE_H

#include "foundation/expected.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/ports/entity_catalog.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::presenters
{

enum class ClassKindFilter
{
	All,
	Point,
	Solid,
};

struct ClassEntry
{
	std::string name;
	std::string description;
	ports::EntityClassKind kind = ports::EntityClassKind::Point;
	std::string category;
	int rank = 0;
	std::size_t countInMap = 0;
};

struct ClassCategory
{
	std::string name;
	std::size_t count = 0;
};

// The category of 'classname' (see the table above).
std::string CategoryOf( std::string_view classname );

class ClassPalette
{
public:
	using Result = foundation::Expected<void, app::EditError>;

	// 'session' may be null (no map counts). All must outlive the palette's use.
	ClassPalette( const ports::IEntityCatalog &catalog, app::EditorSettings &settings,
	    app::EditSession *session = nullptr, std::size_t recentLimit = 8 );
	ClassPalette( const ClassPalette & ) = delete;
	ClassPalette &operator=( const ClassPalette & ) = delete;

	std::uint64_t Revision() const { return m_revision; }
	const std::vector<ClassEntry> &Entries() const { return m_entries; }
	const std::vector<ClassCategory> &Categories() const { return m_categories; }

	const std::string &Search() const { return m_search; }
	void SetSearch( const std::string &search );
	ClassKindFilter KindFilter() const { return m_kind; }
	void SetKindFilter( ClassKindFilter kind );
	const std::optional<std::string> &Category() const { return m_category; }
	void SetCategory( std::optional<std::string> category );

	const std::string &Active() const { return m_settings.entityClass; }
	Result SetActive( const std::string &classname );
	const std::vector<std::string> &Recent() const { return m_recent; }

	// Re-reads the catalog and the map counts (the catalog may be reloaded).
	void Refresh();

private:
	const ports::IEntityCatalog &m_catalog;
	app::EditorSettings &m_settings;
	app::EditSession *m_session = nullptr;
	std::size_t m_recentLimit = 8;
	app::SessionSubscription m_subscription;
	std::uint64_t m_revision = 0;
	std::string m_search;
	ClassKindFilter m_kind = ClassKindFilter::All;
	std::optional<std::string> m_category;
	std::vector<std::string> m_recent;
	std::vector<ClassEntry> m_entries;
	std::vector<ClassCategory> m_categories;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_CLASS_PALETTE_H
