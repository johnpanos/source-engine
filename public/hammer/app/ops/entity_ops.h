//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity property operations (RFC 0002, hammer.app; "Selection and
//			property editing"): keys on one or many entities, class changes,
//			spawnflags, output connections, renaming with reference updates,
//			and worldspawn keys. Multi-entity edits are one operation (one undo
//			step) and refuse as a whole.
//
//			Reserved keys: "classname" changes through SetClass and "id" is
//			persistent identity; SetKey refuses both.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_ENTITY_OPS_H
#define HAMMER_APP_OPS_ENTITY_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/change_set.h"

#include <functional>
#include <string>
#include <vector>

namespace hammer::app::ops
{

// Sets 'key' to 'value' on each entity the ids stand for (groups expand; a
// solid of a brush entity stands for its entity). Nothing when every entity
// already has that value.
EditResult SetKey( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const std::string &key, const std::string &value );
EditResult RemoveKey(
    scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids, const std::string &key );
// Renames a key, keeping its value and position. Refuses when an entity
// already has 'to'.
EditResult RenameKey( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const std::string &from, const std::string &to );

// Changes the class. With a catalog the class must exist and match the
// entity's kind (a brush entity needs a solid class, a point entity a point
// class); the new class's defaults are added for keys the entity lacks.
EditResult SetClass( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const std::string &classname, const ports::IEntityCatalog *catalog );

// Sets or clears spawnflags bit 'flag' (a power of two).
EditResult SetSpawnFlag(
    scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids, long long flag, bool on );

EditResult AddConnection( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const scene::Connection &connection );
// Replaces connection 'index' of entity 'id'.
EditResult ReplaceConnection( scene::DocumentEdit &edit, scene::ObjectId id, std::size_t index,
    const scene::Connection &connection );
// Removes connection 'index' of entity 'id' (exactly that one, even when an
// identical connection exists at another index).
EditResult RemoveConnectionAt( scene::DocumentEdit &edit, scene::ObjectId id, std::size_t index );

// Removes the connections matching 'match' from each entity.
EditResult RemoveConnections( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const std::function<bool( const scene::Connection & )> &match );

// Renames entity 'id' (its targetname). With 'updateReferences', every
// connection target and every key that names entities (target_destination
// keys per the catalog; without one the conventional "target",
// "parentname", "filtername", "damagefilter" and "lightingorigin") that
// matched the old name exactly is updated too.
EditResult RenameEntity( scene::DocumentEdit &edit, scene::ObjectId id, const std::string &name,
    bool updateReferences, const ports::IEntityCatalog *catalog );

// Worldspawn keys (not "classname"/"id").
EditResult SetWorldKey(
    scene::DocumentEdit &edit, const std::string &key, const std::string &value );

// The entities 'ids' stand for (groups expand; brush solids map to owners).
std::vector<scene::ObjectId> EntitiesOf(
    const scene::DocumentReader &doc, const std::vector<scene::ObjectId> &ids );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_ENTITY_OPS_H
