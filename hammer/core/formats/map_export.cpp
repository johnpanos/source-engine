//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::formats map export/compile preparation. See
//			public/hammer/formats/map_export.h. Composes instancing + visgroups +
//			cordon; adds no new transform logic of its own.
//
//=============================================================================//

#include "hammer/formats/map_export.h"

#include "hammer/formats/instancing.h"
#include "hammer/formats/visgroups.h"

namespace hammer::formats
{

CompilePrepResult PrepareForCompile(
    const KeyValueNode &root, ports::IFileStore &store, const CompileOptions &options )
{
	CompilePrepResult result;

	// (1) Expand func_instances into a flat document.
	InstanceExpandResult expanded = ExpandInstances( root, store );
	if ( !expanded.ok )
	{
		result.ok = false;
		result.error = expanded.error;
		return result;
	}
	result.instancesExpanded = expanded.expanded;

	// (2) Drop objects in hidden VisGroups (editor-hidden geometry is not compiled).
	VisGroupFilterResult visible =
	    ApplyVisGroupFilter( expanded.document, options.hiddenVisGroups );
	result.solidsRemovedByVisGroup = visible.hiddenObjects;

	// (3) Optionally clip to the cordon box for a partial compile.
	if ( options.useCordon )
	{
		CordonResult cordoned = ApplyCordon( visible.document, options.cordon );
		result.solidsRemovedByCordon = cordoned.solidsRemoved;
		result.document = std::move( cordoned.document );
	}
	else
	{
		result.document = std::move( visible.document );
	}

	result.ok = true;
	return result;
}

} // namespace hammer::formats
