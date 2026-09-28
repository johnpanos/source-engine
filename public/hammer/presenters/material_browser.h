//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The material browser presentation model (RFC 0002,
//			hammer.presenters; legacy texturebrowser.cpp and the Source 2 Asset
//			Browser's material view): the names the IMaterialInfo port knows,
//			filtered by keywords, optionally only those the map uses with a
//			per-material face count, the most-recently-used list and the active
//			material.
//
//			Names compare by the port's rule: case-insensitive with '/' and '\'
//			equivalent (Normalize). Face counts come from the document (every
//			side of every solid) and refresh when the document changes.
//
//			Keyword filter: the text split on whitespace; every word must occur
//			in the name (case-insensitive substring). Empty text matches all.
//			Rows: without "used in map", the port's names in its order; with it,
//			every material the map uses, sorted by normalized name, including
//			ones the port does not know (known = false: missing materials).
//
//			Active material: app::EditorSettings::faceTexture.material is its
//			one owner (the material new faces and "apply" use); Active() reads
//			it live and SetActive() writes it. SetActive requires a material the
//			port knows (spelled as the port lists it) and pushes it onto Recent
//			(bounded, default 8, most recent first, no duplicates). A write by
//			another owner shows in Active() without moving Revision().
//
//			The subscription is RAII; the browser may be destroyed before or
//			after its session, but calls need a live session.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_MATERIAL_BROWSER_H
#define HAMMER_PRESENTERS_MATERIAL_BROWSER_H

#include "foundation/expected.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/ports/material_info.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::presenters
{

struct MaterialRow
{
	std::string name;
	bool known = true;
	std::size_t faceCount = 0; // faces in the map using it
};

class MaterialBrowser
{
public:
	using Result = foundation::Expected<void, app::EditError>;

	MaterialBrowser( app::EditSession &session, const ports::IMaterialInfo &materials,
	    app::EditorSettings &settings, std::size_t recentLimit = 8 );
	MaterialBrowser( const MaterialBrowser & ) = delete;
	MaterialBrowser &operator=( const MaterialBrowser & ) = delete;

	// Lower case with '\' turned into '/'.
	static std::string Normalize( std::string_view name );

	std::uint64_t Revision() const { return m_revision; }
	const std::vector<MaterialRow> &Rows() const { return m_rows; }
	// Faces in the map using 'name'.
	std::size_t FaceCount( std::string_view name ) const;

	const std::string &Filter() const { return m_filter; }
	void SetFilter( const std::string &filter );
	bool UsedOnly() const { return m_usedOnly; }
	void SetUsedOnly( bool usedOnly );

	const std::string &Active() const { return m_settings.faceTexture.material; }
	Result SetActive( const std::string &material );
	const std::vector<std::string> &Recent() const { return m_recent; }

	// Re-reads the port's names (after an asset rescan).
	void Refresh();

private:
	void Recount();
	void Rebuild();

	app::EditSession &m_session;
	const ports::IMaterialInfo &m_materials;
	app::EditorSettings &m_settings;
	std::size_t m_recentLimit = 8;
	app::SessionSubscription m_subscription;
	std::uint64_t m_revision = 0;
	std::string m_filter;
	bool m_usedOnly = false;
	std::vector<std::string> m_recent;
	std::vector<std::string> m_names;             // the port's names
	std::map<std::string, std::size_t> m_counts;  // normalized name -> faces
	std::map<std::string, std::string> m_spelled; // normalized -> first spelling seen in the map
	std::vector<MaterialRow> m_rows;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_MATERIAL_BROWSER_H
