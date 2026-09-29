//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Instance-content port (RFC 0002, hammer.ports; "Rendering and
//			host contracts"). A func_instance draws the map file it names,
//			placed by its origin and angles. The viewport extraction asks this
//			port what one instance entity draws; the provider (hammer.app's
//			InstancePreview, or a fake) reads and places the file. The
//			document stays the authority: the content is derived, read-only
//			and never part of the document.
//
//			Contract. Content(entity) returns, for a func_instance entity
//			(case-insensitive class) with a "file" key, the objects it
//			merges, in world space: every solid and entity of the file (its
//			quick-hidden objects left out) with the instance's parameters,
//			name fixup and transform applied, as app/ops/instance_ops.h
//			defines the merge, and its nested func_instances replaced by
//			their own content, recursively. Object ids are local to the
//			content (owner references name objects of the same content). The
//			result depends only on the entity's keys and the files it reads;
//			equal inputs give equal content. It is null for an entity that is
//			not a func_instance. A file that cannot be found, read, decoded or
//			placed gives content with that status and no objects; a nested
//			instance that fails is left out and counted in 'nestedFailures'.
//
//			Revision() changes whenever content could have changed without a
//			document edit (a file changed on disk); consumers that keep
//			derived state re-ask then.
//
//			Threading: the caller's sequence (the session's). Borrowed; the
//			provider outlives every consumer that holds it.
//
//=============================================================================//

#ifndef HAMMER_PORTS_INSTANCE_CONTENT_H
#define HAMMER_PORTS_INSTANCE_CONTENT_H

#include "hammer/scene/change_set.h" // scene::MapObject

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace hammer::ports
{

enum class InstanceStatus
{
	Placed,       // the objects are the file's content, placed
	NoFile,       // the entity has no "file" key
	NotFound,     // no search location holds the file
	DecodeFailed, // the file could not be read or decoded
	Rejected,     // the merge rule refused it (malformed origin/angles, fixup_style, a
	              // transform that makes a solid degenerate)
	Cycle,        // the file includes itself, directly or through nested instances
};

struct InstanceContent
{
	InstanceStatus status = InstanceStatus::NoFile;
	std::string file;   // the resolved path (empty unless found)
	std::string detail; // a diagnostic for a failure
	std::vector<scene::MapObject> objects;
	int nestedFailures = 0;

	friend bool operator==( const InstanceContent &, const InstanceContent & ) = default;
};

class IInstanceContent
{
public:
	virtual ~IInstanceContent() = default;
	virtual std::shared_ptr<const InstanceContent> Content( const scene::Entity &instance ) = 0;
	virtual std::uint64_t Revision() const = 0;
};

} // namespace hammer::ports

#endif // HAMMER_PORTS_INSTANCE_CONTENT_H
