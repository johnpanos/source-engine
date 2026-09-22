//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless map export/compile preparation for the Hammer editor
//			(RFC 0002, hammer.formats). Compiling or exporting a map first
//			transforms the authored VMF into the geometry a compiler sees: expand
//			func_instance references, drop objects in hidden VisGroups, and (for a
//			partial compile) clip to the cordon box. This module COMPOSES the
//			single-purpose format cores -- instancing, visgroups, cordon -- into
//			that one ordered pipeline, so the shell's "Run Map"/"Export" path has a
//			single headless entry point. Strict, MFC-free, GPU-free. It adds no new
//			transform logic; it only orders the existing cores.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_MAP_EXPORT_H
#define HAMMER_FORMATS_MAP_EXPORT_H

#include "hammer/formats/cordon.h"
#include "hammer/formats/keyvalues.h"
#include "hammer/ports/file_store.h"

#include <cstddef>
#include <string>
#include <vector>

namespace hammer::formats
{

// What the export/compile prep should do. Instance expansion always runs.
struct CompileOptions
{
	std::vector<int> hiddenVisGroups; // objects in these (or descendant) groups are dropped
	bool useCordon = false;           // when true, clip to 'cordon'
	CordonBox cordon;
};

struct CompilePrepResult
{
	bool ok = false;
	KeyValueNode document; // the compile-ready VMF
	std::string error;     // diagnostic when !ok (e.g. a bad func_instance)
	int instancesExpanded = 0;
	std::size_t solidsRemovedByVisGroup = 0;
	std::size_t solidsRemovedByCordon = 0;
};

// Prepares 'root' for compile/export: (1) expand func_instances through 'store',
// (2) drop objects in hidden VisGroups, (3) optionally clip to the cordon box.
// Returns the flattened/filtered document, or ok == false with a diagnostic if
// instance expansion fails.
CompilePrepResult PrepareForCompile(
    const KeyValueNode &root, ports::IFileStore &store, const CompileOptions &options );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_MAP_EXPORT_H
