//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/preview.h.
//
//=============================================================================//

#include "hammer/tools/preview.h"

#include "hammer/scene/solid_geometry.h"
#include "hammer/viewport/view_policy.h"

namespace hammer::tools
{

void AppendSolidEdges( OverlayList &out, const scene::Solid &solid, OverlayRole role )
{
	for ( const viewport::WorldEdge &edge : viewport::UniqueEdges( scene::BuildGeometry( solid ) ) )
		out.Line( edge.a, edge.b, role );
}

void AppendObject( OverlayList &out, const scene::DocumentReader &doc, scene::ObjectId id,
    OverlayRole role, const ports::IEntityCatalog *catalog, double pointHalfSize )
{
	if ( const scene::Solid *solid = doc.FindSolid( id ) )
	{
		AppendSolidEdges( out, *solid, role );
		return;
	}
	if ( const scene::Entity *entity = doc.FindEntity( id ) )
	{
		if ( const std::optional<scene::Box> box =
		         viewport::EntityMarkerBox( *entity, catalog, pointHalfSize ) )
			out.Box( box->mins, box->maxs, role );
	}
}

PreviewOutcome AppendStagedPreview( OverlayList &out, const scene::MapDocument &doc,
    const app::EditSession::Operation &operation, OverlayRole role,
    const ports::IEntityCatalog *catalog, double pointHalfSize )
{
	PreviewOutcome outcome;
	scene::DocumentEdit edit( doc );
	const app::EditResult result = operation( edit );
	if ( !result )
	{
		outcome.error = result.Error().message;
		return outcome;
	}
	outcome.ok = true;
	for ( scene::ObjectId id : edit.TouchedIds() )
	{
		if ( !edit.KindOf( id ) )
			continue;
		outcome.touched.push_back( id );
		AppendObject( out, edit, id, role, catalog, pointHalfSize );
	}
	return outcome;
}

} // namespace hammer::tools
