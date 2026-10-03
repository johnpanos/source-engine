//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/replace_textures.h.
//
//=============================================================================//

#include "hammer/presenters/replace_textures.h"

#include "content/asset_identity.h"
#include "hammer/scene/map_queries.h"

#include <algorithm>
#include <map>
#include <set>

namespace hammer::presenters
{

using app::ops::MaterialMatch;

namespace
{

const char *MatchName( MaterialMatch match )
{
	switch ( match )
	{
	case MaterialMatch::Partial:
		return "partial";
	case MaterialMatch::Substitute:
		return "substitute";
	case MaterialMatch::Exact:
		break;
	}
	return "exact";
}

std::string Count( int count, const char *what )
{
	return std::to_string( count ) + " " + what;
}

} // namespace

ReplaceTexturesDialog::ReplaceTexturesDialog( app::EditSession &session,
    app::SessionCommands &commands, const ports::IMaterialInfo *materials )
    : m_session( session ), m_commands( commands ), m_materials( materials )
{
}

void ReplaceTexturesDialog::Open( const std::string &currentMaterial, bool faceToolActive )
{
	m_draft = {};
	m_draft.find = currentMaterial;
	m_draft.scope = SelectionScopeAvailable() ? Scope::Selection : Scope::Everything;
	m_marksFaces = faceToolActive;
}

bool ReplaceTexturesDialog::SelectionScopeAvailable() const
{
	return !m_session.CurrentSelection().objects.empty();
}

std::string ReplaceTexturesDialog::Validate() const
{
	if ( content::FoldAssetName( m_draft.find ).empty() )
	{
		return "Enter the material to find.";
	}
	if ( m_draft.scope == Scope::Selection && !SelectionScopeAvailable() )
	{
		return "Nothing is selected to replace in.";
	}
	if ( m_draft.markOnly )
	{
		return {};
	}
	if ( m_draft.match != MaterialMatch::Substitute &&
	     !content::NormalizeAssetName( m_draft.replace ) )
	{
		return m_draft.replace.empty() ? "Enter the replacement material."
		                               : "'" + m_draft.replace + "' is not a material name.";
	}
	if ( m_draft.rescale && !RescaleAvailable() )
	{
		return "Material sizes are unavailable, so textures cannot be rescaled.";
	}
	return {};
}

app::ops::MaterialReplace ReplaceTexturesDialog::Query() const
{
	app::ops::MaterialReplace q;
	q.find = m_draft.find;
	q.replace = m_draft.replace;
	q.match = m_draft.match;
	if ( m_draft.scope == Scope::Selection )
	{
		q.within = m_session.CurrentSelection().objects;
	}
	q.includeHidden = m_draft.includeHidden && !m_draft.markOnly;
	q.rescale = m_draft.rescale;
	return q;
}

ReplaceTexturesDialog::PreviewCount ReplaceTexturesDialog::Preview() const
{
	const std::vector<scene::FaceRef> faces =
	    app::ops::FindMaterialFaces( m_session.Document(), Query() );
	PreviewCount count;
	count.faces = static_cast<int>( faces.size() );
	std::set<scene::ObjectId> solids;
	for ( const scene::FaceRef &f : faces )
	{
		solids.insert( f.solid );
	}
	count.solids = static_cast<int>( solids.size() );
	return count;
}

std::vector<ReplaceTexturesDialog::UsedMaterial> ReplaceTexturesDialog::UsedMaterials() const
{
	std::map<std::string, int> used;
	const scene::MapDocument &doc = m_session.Document();
	for ( scene::ObjectId id : doc.SolidIds() )
	{
		for ( const scene::Side &side : doc.FindSolid( id )->sides )
		{
			const auto name = content::NormalizeAssetName( side.texture.material );
			++used[name ? *name : content::FoldAssetName( side.texture.material )];
		}
	}
	std::vector<UsedMaterial> out;
	for ( const auto &[name, faces] : used )
	{
		out.push_back( { name, faces } );
	}
	return out;
}

std::vector<std::string> ReplaceTexturesDialog::Candidates() const
{
	return m_materials ? m_materials->Names() : std::vector<std::string>();
}

std::string ReplaceTexturesDialog::IdsArgument() const
{
	std::string ids;
	for ( scene::ObjectId id : m_session.CurrentSelection().objects )
	{
		ids += ( ids.empty() ? "" : " " ) + std::to_string( app::SessionCommands::ScriptId( id ) );
	}
	return ids;
}

ReplaceTexturesDialog::Outcome ReplaceTexturesDialog::Apply()
{
	if ( std::string why = Validate(); !why.empty() )
	{
		return { false, false, why };
	}
	app::CommandArgs args = { { "find", m_draft.find }, { "match", MatchName( m_draft.match ) } };
	if ( m_draft.scope == Scope::Selection )
	{
		args["ids"] = IdsArgument();
	}
	if ( m_draft.markOnly )
	{
		args["faces"] = m_marksFaces ? "1" : "0";
		const app::CommandResult marked = m_commands.Execute( "mark_material", args );
		if ( !marked )
		{
			return { false, false, marked.Error().detail };
		}
		return { true, true,
		    Count(
		        std::stoi( marked.Value() ), m_marksFaces ? "faces marked." : "solids marked." ) };
	}
	// Legacy reported "0 textures replaced." and changed nothing; the command
	// refuses an edit with no match, so the count is decided first.
	if ( Preview().faces == 0 )
	{
		return { false, true, Count( 0, "textures replaced." ) };
	}
	args["replace"] = m_draft.replace;
	args["hidden"] = m_draft.includeHidden ? "1" : "0";
	args["rescale"] = m_draft.rescale ? "1" : "0";
	const app::CommandResult replaced = m_commands.Execute( "replace_material", args );
	if ( !replaced )
	{
		return { false, false, replaced.Error().detail };
	}
	return { true, true, Count( std::stoi( replaced.Value() ), "textures replaced." ) };
}

} // namespace hammer::presenters
