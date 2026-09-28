//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Hammer map document (RFC 0002, hammer.scene): the one
//			authoritative content model for an open map. It holds every solid,
//			entity and group by runtime id, plus the document-level settings
//			(worldspawn keys, visgroups, cordons, cameras and blocks the editor
//			does not interpret). It is a value: copying it copies the map.
//
//			Mutation is deliberately low level (Put/Erase/Settings). Editing
//			code does not call it directly; it stages changes in a DocumentEdit
//			(change_set.h) that the application's session commits, so every
//			change is recorded for undo. Validate() states the invariants that a
//			committed document keeps.
//
//			Iteration order is id order, which is creation order: loading keeps
//			the file's order, so a load/save round trip keeps object order.
//
//=============================================================================//

#ifndef HAMMER_SCENE_MAP_DOCUMENT_H
#define HAMMER_SCENE_MAP_DOCUMENT_H

#include "hammer/scene/map_objects.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace hammer::scene
{

struct Visgroup
{
	int id = 0; // persistent visgroup id
	std::string name;
	std::optional<Rgb> color;
	std::vector<Visgroup> children;

	friend bool operator==( const Visgroup &, const Visgroup & ) = default;
};

struct CordonBox
{
	mapgeometry::Vec3d mins;
	mapgeometry::Vec3d maxs;

	friend bool operator==( const CordonBox &, const CordonBox & ) = default;
};

struct Cordon
{
	std::string name;
	bool active = true;
	std::vector<CordonBox> boxes;

	friend bool operator==( const Cordon &, const Cordon & ) = default;
};

struct CameraBookmark
{
	mapgeometry::Vec3d position;
	mapgeometry::Vec3d look;

	friend bool operator==( const CameraBookmark &, const CameraBookmark & ) = default;
};

// The VMF 'versioninfo' block: which editor wrote the file.
struct VersionInfo
{
	int editorVersion = 400;
	int editorBuild = 0;
	int mapVersion = 0;
	int formatVersion = 100;
	bool prefab = false;

	friend bool operator==( const VersionInfo &, const VersionInfo & ) = default;
};

// The VMF 'viewsettings' block: the 2D/3D grid state saved with the map.
struct ViewSettings
{
	bool snapToGrid = true;
	bool showGrid = true;
	bool showLogicalGrid = false;
	int gridSpacing = 64;
	bool show3DGrid = false;

	friend bool operator==( const ViewSettings &, const ViewSettings & ) = default;
};

struct DocumentSettings
{
	VersionInfo version;
	ViewSettings view;
	std::uint32_t worldVmfId = 1;
	std::vector<kvtext::KeyValue> worldKeys; // worldspawn keys except id/classname
	std::vector<Visgroup> visgroups;
	std::vector<Cordon> cordons;
	bool cordonsActive = false;
	// Which block the VMF used: the older single 'cordon' { mins maxs active }
	// or the newer 'cordons' list. A codec re-emits the same form when it can
	// express the content (one cordon with one box) and 'cordons' otherwise.
	enum class CordonForm
	{
		None,
		Single,
		List,
	};
	CordonForm cordonForm = CordonForm::None;
	std::vector<CameraBookmark> cameras;
	int activeCamera = -1;

	// The first worldspawn value of 'key', or nullptr.
	const std::string *WorldKey( std::string_view key ) const;
	// Sets (appending when absent). Returns whether it changed.
	bool SetWorldKey( std::string_view key, std::string_view value );

	friend bool operator==( const DocumentSettings &, const DocumentSettings & ) = default;
};

// A visgroup found in the tree, with its parent id (0 at the top level).
struct VisgroupLookup
{
	const Visgroup *visgroup = nullptr;
	int parentId = 0;
};
VisgroupLookup FindVisgroup( const std::vector<Visgroup> &tree, int id );

// Read access shared by a committed document and a staged edit, so queries
// and operations are written once for both.
class DocumentReader
{
public:
	virtual ~DocumentReader() = default;

	virtual const Solid *FindSolid( ObjectId id ) const = 0;
	virtual const Entity *FindEntity( ObjectId id ) const = 0;
	virtual const Group *FindGroup( ObjectId id ) const = 0;
	virtual std::optional<ObjectKind> KindOf( ObjectId id ) const = 0;
	virtual const DocumentSettings &Settings() const = 0;
	// Live ids of a kind, in id order.
	virtual std::vector<ObjectId> SolidIds() const = 0;
	virtual std::vector<ObjectId> EntityIds() const = 0;
	virtual std::vector<ObjectId> GroupIds() const = 0;
};

class MapDocument final : public DocumentReader
{
public:
	// 'serial' identifies this document's ids (non-zero; see ObjectId). A new
	// document starts empty with a "worldspawn" world.
	explicit MapDocument( std::uint32_t serial = 1 );

	std::uint32_t Serial() const { return m_serial; }

	// --- Lookup ----------------------------------------------------------------
	const Solid *FindSolid( ObjectId id ) const override;
	const Entity *FindEntity( ObjectId id ) const override;
	const Group *FindGroup( ObjectId id ) const override;
	std::optional<ObjectKind> KindOf( ObjectId id ) const override;
	bool Contains( ObjectId id ) const { return KindOf( id ).has_value(); }
	std::vector<ObjectId> SolidIds() const override;
	std::vector<ObjectId> EntityIds() const override;
	std::vector<ObjectId> GroupIds() const override;

	const std::map<ObjectId, Solid> &Solids() const { return m_solids; }
	const std::map<ObjectId, Entity> &Entities() const { return m_entities; }
	const std::map<ObjectId, Group> &Groups() const { return m_groups; }
	std::size_t ObjectCount() const
	{
		return m_solids.size() + m_entities.size() + m_groups.size();
	}

	const DocumentSettings &Settings() const override { return m_settings; }

	// --- Identity ----------------------------------------------------------------
	// A fresh runtime id, never issued before by this document.
	ObjectId AllocateId();
	// A fresh persistent VMF id for a new solid, side, entity or group: larger
	// than every VMF id the document has seen.
	std::uint32_t AllocateVmfId();
	// Raises the VMF id floor to at least 'seen' (the codec calls it on load).
	void NoteVmfId( std::uint32_t seen );
	std::uint32_t NextVmfId() const { return m_nextVmfId; }
	std::uint32_t NextLocalId() const { return m_nextLocal; }
	// Raises both counters to at least the given values (never lowers them).
	void AdvanceCounters( std::uint32_t nextLocal, std::uint32_t nextVmfId );

	// --- Low-level mutation (for DocumentEdit and codecs) -------------------------
	// Inserts or replaces the object with its id. The id must carry this
	// document's serial; returns false and changes nothing otherwise.
	bool Put( Solid solid );
	bool Put( Entity entity );
	bool Put( Group group );
	// Removes the object with 'id' of any kind. Returns whether it existed.
	bool Erase( ObjectId id );
	DocumentSettings &MutableSettings() { return m_settings; }

	// --- Invariants ----------------------------------------------------------------
	// Empty when the document is consistent; otherwise one line per violation:
	// a solid owner that is not an entity, a group reference that is not a
	// group, a group cycle, duplicate side VMF ids, or a solid with fewer than
	// four sides.
	std::vector<std::string> Validate() const;

private:
	std::uint32_t m_serial = 1;
	std::uint32_t m_nextLocal = 1;
	std::uint32_t m_nextVmfId = 2; // 1 is the world
	std::map<ObjectId, Solid> m_solids;
	std::map<ObjectId, Entity> m_entities;
	std::map<ObjectId, Group> m_groups;
	DocumentSettings m_settings;
};

// True when both documents hold equal objects under equal ids and equal
// settings. Id counters and the serial are not content and are ignored.
bool SameContent( const MapDocument &a, const MapDocument &b );

} // namespace hammer::scene

#endif // HAMMER_SCENE_MAP_DOCUMENT_H
