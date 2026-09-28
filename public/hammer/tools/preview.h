//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gesture previews for the tools (RFC 0002, hammer.tools). A tool
//			previews an edit by staging the SAME operation it will commit on
//			release in a scratch scene::DocumentEdit over the session's document
//			and never committing it: the preview and the committed result come
//			from one code path, and the document is untouched until release.
//			The staged objects are described as overlay primitives: a solid as
//			its unique wireframe edges (viewport::UniqueEdges), a point entity as
//			its marker box (viewport::EntityMarkerBox).
//
//=============================================================================//

#ifndef HAMMER_TOOLS_PREVIEW_H
#define HAMMER_TOOLS_PREVIEW_H

#include "hammer/app/edit_session.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/change_set.h"
#include "hammer/tools/input.h"

#include <string>
#include <vector>

namespace hammer::tools
{

struct PreviewOutcome
{
	bool ok = false;
	std::string error;                    // why the staged operation refused
	std::vector<scene::ObjectId> touched; // live objects the operation changed or created
};

// Stages 'operation' on a scratch edit of 'doc' and appends the touched live
// objects in 'role'. Nothing is appended when the operation refuses.
PreviewOutcome AppendStagedPreview( OverlayList &out, const scene::MapDocument &doc,
    const app::EditSession::Operation &operation, OverlayRole role,
    const ports::IEntityCatalog *catalog, double pointHalfSize );

// One object as overlay primitives: solid edges, or a point entity's marker.
void AppendObject( OverlayList &out, const scene::DocumentReader &doc, scene::ObjectId id,
    OverlayRole role, const ports::IEntityCatalog *catalog, double pointHalfSize );

// A solid value's wireframe edges.
void AppendSolidEdges( OverlayList &out, const scene::Solid &solid, OverlayRole role );

} // namespace hammer::tools

#endif // HAMMER_TOOLS_PREVIEW_H
